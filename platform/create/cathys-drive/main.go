package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"

	"github.com/ardnew/oibot"
)

type TaskInfo struct {
	infoLog  *log.Logger
	errorLog *log.Logger
	ssuData  chan *SensorData
	botStat  chan *oibot.InfoStatus
	waitGrp  *sync.WaitGroup
}

type SerialInfo struct {
	init    bool
	path    string
	baud    int
	timeout time.Duration
}

type TaskSignal os.Signal
type TaskSignalQ chan os.Signal

const (
	resetSignal = syscall.SIGUSR1

	// duration to wait before forcefully terminating a task that was sent a
	// halt or reset signal
	joinTimeout = 5 * time.Second
)

func waitForSignal(parent TaskSignalQ, ignore []TaskSignal, child ...TaskSignalQ) TaskSignal {
	// wait until we've received a signal from the parent goroutine
	p := <-parent
	// ensure it is a signal we don't ignore
	forward := true
	for _, i := range ignore {
		if p == i {
			forward = false
			break
		}
	}
	// forward that signal onto each child goroutine
	if forward {
		for _, c := range child {
			c <- p
		}
	}
	return p
}

type velocityPair struct {
	left  int16
	right int16
}

type driveParams struct {
	velocityMMPerSec int16
	radiusMM         int16
}

const (
	minWheelVelocityMMPerSec int16 = 0
	maxWheelVelocityMMPerSec int16 = 100
)

var (
	wheelVelocityMMPerSec = []velocityPair{
		{left: 0, right: 0}, // DirStop
		{left: -maxWheelVelocityMMPerSec, right: maxWheelVelocityMMPerSec}, // DirLeft
		{left: minWheelVelocityMMPerSec, right: maxWheelVelocityMMPerSec},  // DirLeftFwd
		{left: maxWheelVelocityMMPerSec, right: maxWheelVelocityMMPerSec},  // DirFwd
		{left: maxWheelVelocityMMPerSec, right: minWheelVelocityMMPerSec},  // DirRightFwd
		{left: maxWheelVelocityMMPerSec, right: -maxWheelVelocityMMPerSec}, // DirRight
		{left: maxWheelVelocityMMPerSec, right: -maxWheelVelocityMMPerSec}, // DirRightAft
		{left: 0, right: 0}, // DirAft
		{left: -maxWheelVelocityMMPerSec, right: maxWheelVelocityMMPerSec}, // DirLeftAft
	}
)

var (
	driveDirection = []driveParams{
		{velocityMMPerSec: minWheelVelocityMMPerSec, radiusMM: 0x7FFF}, // DirStop
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: +1},     // DirLeft
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: 500},    // DirLeftFwd
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: 0x7FFF}, // DirFwd
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: -500},   // DirRightFwd
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: -1},     // DirRight
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: -1},     // DirRightAft
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: 0x7FFF}, // DirAft
		{velocityMMPerSec: maxWheelVelocityMMPerSec, radiusMM: +1},     // DirLeftAft
	}
)

func oibotTask(task *TaskInfo, serial *SerialInfo, clean TaskSignalQ, ssuSignal TaskSignalQ) bool {

	// initialize the object, prepare for entering into state machine
	bot := oibot.MakeOIBot(task.infoLog, task.errorLog, serial.init, serial.path, serial.baud, serial.timeout)
	// time.Sleep(500 * time.Millisecond)
	// bot.Safe()

	wait := &sync.WaitGroup{}
	wait.Add(2)

	botInSignal := make(TaskSignalQ)
	botOutSignal := make(TaskSignalQ)

	go func(b *oibot.OIBot, i *TaskInfo, s *SerialInfo, c TaskSignalQ, u TaskSignalQ, w *sync.WaitGroup, t <-chan time.Time) {
		defer w.Done()
		for {
			select {
			case l := <-c:
				switch l {
				case resetSignal:
					i.infoLog.Printf("caught signal %+v, restarting relay: bot-serial 🠊 ssu-channel", l)
				default:
					i.errorLog.Printf("caught signal %+v, terminating relay: bot-serial 🠊 ssu-channel", l)
				}
				return
			case <-t:
				if stat, ok := b.Info(); ok {
					i.infoLog.Printf("status: mode=%v, batt=%+v", stat.Mode, stat.Battery)
					if oibot.OIMPassive == stat.Mode {
						i.ssuData <- &SensorData{UserCommand: ucmdSafe, IRAngle: -1, IRIntensity: -1, Injected: true}
					}
					i.botStat <- stat
				} else {
					i.errorLog.Printf("could not decode oibot status")
					i.botStat <- nil
				}
			default:
			}
		}
	}(bot, task, serial, botInSignal, ssuSignal, wait, time.Tick(500*time.Millisecond))

	go func(b *oibot.OIBot, i *TaskInfo, s *SerialInfo, c TaskSignalQ, u TaskSignalQ, w *sync.WaitGroup) {
		const (
			baudPulseRateMS       = 10000 * time.Millisecond
			userCommandDurationMS = 1000 * time.Millisecond
			driveDurationMS       = 1000 * time.Millisecond
		)
		defer w.Done()
		userCommandLastTime := time.Now()
		driveLastTime := time.Now()
		driveLastDirection := oibot.DirStop
		for {
			select {
			case l := <-c:
				switch l {
				case resetSignal:
					i.infoLog.Printf("caught signal %+v, restarting relay: ssu-channel 🠊 bot-serial", l)
				default:
					i.errorLog.Printf("caught signal %+v, terminating relay: ssu-channel 🠊 bot-serial", l)
				}
				return
			case dat := <-i.ssuData:
				if time.Since(userCommandLastTime) > userCommandDurationMS {
					switch dat.UserCommand {
					case ucmdPassive:
						i.infoLog.Printf("sending passive: %+v", dat)
						b.Passive()
						userCommandLastTime = time.Now()
					case ucmdSafe:
						i.infoLog.Printf("sending safe: %+v", dat)
						b.Safe()
						userCommandLastTime = time.Now()
					case ucmdFull:
						i.infoLog.Printf("sending full: %+v", dat)
						b.Full()
						userCommandLastTime = time.Now()
					case ucmdReset:
						i.infoLog.Printf("sending reset: %+v", dat)
						b.Passive()
						<-time.After(1000 * time.Millisecond)
						b.Reset()
						b.Close()
						// send the reset signal only to the ssu task so that we can ensure
						// the order in which tasks are started. the ssu task automatically
						// sends a reset signal to the bot task once it receives this.
						u <- resetSignal
						userCommandLastTime = time.Now()
					case ucmdOff:
						i.infoLog.Printf("sending off: %+v", dat)
						b.Power()
						userCommandLastTime = time.Now()
					case ucmdNONE, ucmdCOUNT:
					default:
					}
				}
				if time.Since(driveLastTime) > driveDurationMS {
					var (
						sym rune
						dir oibot.Direction
					)

					if dat.IRIntensity > minIRIntensity {
						sym, dir = oibot.AngleRune(dat.IRAngle, 3)
					} else {
						sym, dir = oibot.AngleRuneUnknown, oibot.DirStop
					}

					if dir != driveLastDirection {

						//wheelMMPerSec := wheelVelocityMMPerSec[dir]
						//i.infoLog.Printf("drive: (%d°,%.1f%%): %d⇈%d [ %c ]", dat.IRAngle, dat.IRIntensity, wheelMMPerSec.left, wheelMMPerSec.right, sym)
						//b.DriveWheels(wheelMMPerSec.right, wheelMMPerSec.left)

						driveDir := driveDirection[dir]
						i.infoLog.Printf("drive: (%d°,%.1f%%): %d@%d [ %c ]", dat.IRAngle, dat.IRIntensity, driveDir.radiusMM, driveDir.velocityMMPerSec, sym)
						b.Drive(driveDir.velocityMMPerSec, driveDir.radiusMM)

						driveLastTime = time.Now()
						driveLastDirection = dir
					}
				}

			default:
			}
		}
	}(bot, task, serial, botOutSignal, ssuSignal, wait)

	s := waitForSignal(clean, []TaskSignal{}, botInSignal, botOutSignal)

	wait.Wait()

	return resetSignal == s
}

func senseTask(task *TaskInfo, serial *SerialInfo, clean TaskSignalQ, botSignal TaskSignalQ) bool {

	// initialize the object, prepare for entering into state machine
	ssu := MakeSensor(task.infoLog, task.errorLog, serial.path, serial.baud)

	wait := &sync.WaitGroup{}
	wait.Add(2)

	ssuInSignal := make(TaskSignalQ)
	ssuOutSignal := make(TaskSignalQ)

	go func(s *Sensor, i *TaskInfo, c TaskSignalQ, w *sync.WaitGroup, t <-chan time.Time) {
		defer w.Done()
		for {
			select {
			case l := <-c:
				switch l {
				case resetSignal:
					i.errorLog.Printf("caught signal %+v, restarting relay: ssu-serial 🠊 bot-channel", l)
				default:
					i.errorLog.Printf("caught signal %+v, terminating relay: ssu-serial 🠊 bot-channel", l)
				}
				return
			case <-t:
				if data, ok := s.Data(); ok {
					i.ssuData <- data
				}
			default:
			}
		}
	}(ssu, task, ssuInSignal, wait, time.Tick(10*time.Millisecond))

	go func(s *Sensor, i *TaskInfo, c TaskSignalQ, w *sync.WaitGroup) {
		defer w.Done()
		for {
			select {
			case l := <-c:
				switch l {
				case resetSignal:
					i.errorLog.Printf("caught signal %+v, restarting relay: bot-channel 🠊 ssu-serial", l)
				default:
					i.errorLog.Printf("caught signal %+v, terminating relay: bot-channel 🠊 ssu-serial", l)
				}
				return
			case stat := <-i.botStat:
				if fmt, ok := s.FormatUplinkStatus(stat); ok {
					s.Write([]byte(fmt))
				}
			default:
			}
		}
	}(ssu, task, ssuOutSignal, wait)

	s := waitForSignal(clean, []TaskSignal{}, ssuInSignal, ssuOutSignal)

	wait.Wait()

	return resetSignal == s
}

func main() {

	defer func() {
		if r := recover(); r != nil {
			// empty
		}
	}()

	if len(os.Args) <= 2 {
		log.Fatalf("usage:\n\t%s <roomba-dev> <sensor-dev>", os.Args[0])
	}

	botPath := os.Args[1]
	botBaud := oibot.DefaultBaudRateBPS
	botSign := make(TaskSignalQ)

	ssuPath := os.Args[2]
	ssuBaud := defaultBaudRateBPS // defined in sensor.go
	ssuSign := make(TaskSignalQ)

	clean := make(TaskSignalQ)
	signal.Notify(clean, os.Interrupt, syscall.SIGTERM, syscall.SIGINT, resetSignal)

	task := TaskInfo{
		infoLog:  log.New(os.Stdout, "[ ] ", log.Ltime),
		errorLog: log.New(os.Stderr, "[!] ", log.Ltime|log.Lshortfile),
		ssuData:  make(chan *SensorData),
		botStat:  make(chan *oibot.InfoStatus),
		waitGrp:  &sync.WaitGroup{},
	}

	task.waitGrp.Add(2)

	go func(i *TaskInfo, s *SerialInfo, c TaskSignalQ, n TaskSignalQ) {
		defer i.waitGrp.Done()
		for {
			if oibotTask(i, s, c, n) {
				rem := oibot.BootupTimeMS
				ind := 1 * time.Second
				pre := fmt.Sprintf("%s%v", i.infoLog.Prefix(), time.Now().Format("15:04:05"))
				fmt.Fprintf(i.infoLog.Writer(), "%s restarting bot task in: ", pre)
				for range time.Tick(ind) {
					fmt.Fprintf(i.infoLog.Writer(), "%v ", rem)
					rem -= ind
					if rem <= 0 {
						fmt.Fprintln(i.infoLog.Writer())
						break
					}
				}
				s.init = false
			} else {
				break
			}
		}
	}(&task, &SerialInfo{true, botPath, botBaud, oibot.DefaultReadTimeoutMS}, botSign, ssuSign)

	go func(i *TaskInfo, s *SerialInfo, c TaskSignalQ, n TaskSignalQ) {
		defer i.waitGrp.Done()
		for {
			if senseTask(i, s, c, n) {
				i.infoLog.Print("restarting ssu task")
				s.init = false
				// if we are restarting, notify the bot task to restart
				n <- resetSignal
			} else {
				break
			}
		}
	}(&task, &SerialInfo{true, ssuPath, ssuBaud, oibot.NeverReadTimeoutMS}, ssuSign, botSign)

	for {
		s := waitForSignal(clean, []TaskSignal{resetSignal}, botSign, ssuSign)
		if resetSignal != s {
			break
		} else {
			ssuSign <- s
		}
	}

	finished := make(chan interface{})

	go func(i *TaskInfo, f chan interface{}) {
		i.waitGrp.Wait()
		task.infoLog.Print("exiting gracefully")
		f <- true // value unused
	}(&task, finished)

	go func(i *TaskInfo, f chan interface{}) {
		<-time.After(joinTimeout)
		task.infoLog.Print("exiting forcefully")
		f <- false // value unused
	}(&task, finished)

	<-finished
}
