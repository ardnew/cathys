package main

import (
	"fmt"
	"log"
	"math"
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
	userCmd  chan int16
	exitSig  CleanSignal
}

type SerialInfo struct {
	path    string
	baud    int
	timeout time.Duration
}

type AtomicValue struct {
	*sync.Mutex
	bool // currently only need bool support. append as required
}

func MakeAtomicValue(val ...interface{}) *AtomicValue {
	return &AtomicValue{
		Mutex: &sync.Mutex{}, bool: val[0].(bool),
	}
}

func (a *AtomicValue) GetBool() bool {
	a.Mutex.Lock()
	val := a.bool
	a.Mutex.Unlock()
	return val
}
func (a *AtomicValue) SetBool(val bool) {
	a.Mutex.Lock()
	a.bool = val
	a.Mutex.Unlock()
}

type CleanSignal chan os.Signal

func cleanup(parent CleanSignal, child ...CleanSignal) {
	// wait until we've received a signal from the parent goroutine
	p := <-parent
	// forward that signal onto each child goroutine
	for _, c := range child {
		c <- p
	}
	//time.Sleep(time.Millisecond * 100)
}

func oibotTask(task *TaskInfo, serial *SerialInfo, clean CleanSignal) bool {

	// initialize the object, prepare for entering into state machine
	bot := oibot.MakeOIBot(task.infoLog, task.errorLog, serial.path, serial.baud, serial.timeout)

	// always attempt to re-establish connection with the roomba by default
	res := MakeAtomicValue(true)

	infExit := make(CleanSignal)
	datExit := make(CleanSignal)

	//bot.Drive(-100, 2)
	//time.Sleep(1000 * time.Millisecond)
	//data = bot.Battery()
	//time.Sleep(1000 * time.Millisecond)
	//bot.DriveStop()
	//infoLog.Printf("%v (%T)", data, data)

	go func(b *oibot.OIBot, i *TaskInfo, s *SerialInfo, c CleanSignal, d CleanSignal, a *AtomicValue, t <-chan time.Time) {
		for {
			select {
			case l := <-c:
				switch l {
				case syscall.SIGUSR1:
					i.infoLog.Printf("caught signal %+v, restarting bot sensor suite relay", l)
					i.exitSig <- nil
				default:
					a.SetBool(false)
					i.errorLog.Printf("caught signal %+v, terminating bot health serial monitor", l)
					i.exitSig <- l
				}
				return
			case <-t:
				if stat, ok := b.Info(); ok {
					i.infoLog.Printf("status: mode=%v, batt=%+v", stat.Mode, stat.Battery)
					i.botStat <- stat
				} else {
					i.errorLog.Printf("could not decode oibot status")
					i.botStat <- nil
				}
			default:
			}
		}
	}(bot, task, serial, infExit, datExit, res, time.Tick(1000*time.Millisecond))

	go func(b *oibot.OIBot, i *TaskInfo, s *SerialInfo, c CleanSignal, d CleanSignal, a *AtomicValue) {
		const (
			userCommandDurationMS = 1000 * time.Millisecond
			driveDurationMS       = 500 * time.Millisecond
		)
		userCommandLastTime := time.Now()
		driveLastTime := time.Now()
		for {
			select {
			case l := <-c:
				switch l {
				case syscall.SIGUSR1:
					i.infoLog.Printf("caught signal %+v, restarting bot sensor suite relay", l)
					i.exitSig <- nil
				default:
					a.SetBool(false)
					i.errorLog.Printf("caught signal %+v, terminating bot sensor suite relay", l)
					i.exitSig <- l
					b.Passive()
				}
				return
			case dat := <-i.ssuData:
				if time.Since(userCommandLastTime) > userCommandDurationMS {
					switch dat.UserCommand {
					case ucmdPassive:
						i.infoLog.Printf("sending passive: %+v", dat)
						//b.Passive()
						userCommandLastTime = time.Now()
					case ucmdSafe:
						i.infoLog.Printf("sending safe: %+v", dat)
						//b.Safe()
						userCommandLastTime = time.Now()
					case ucmdFull:
						i.infoLog.Printf("sending full: %+v", dat)
						//b.Full()
						userCommandLastTime = time.Now()
					case ucmdReset:
						i.infoLog.Printf("sending reset: %+v", dat)
						//b.Reset()
						userCommandLastTime = time.Now()
					case ucmdOff:
						i.infoLog.Printf("sending off: %+v", dat)
						//b.Power()
						userCommandLastTime = time.Now()
					case ucmdNONE, ucmdCOUNT:
					default:
					}
				}
				if dat.IRIntensity > minIRIntensity {
					if time.Since(driveLastTime) > driveDurationMS {
						radius := float32(-1*dat.IRAngle) / 90.0 * float32(oibot.MaxDriveRadiusMM)
						if radius < 0 {
							radius += float32(oibot.MaxDriveRadiusMM)
						} else {
							radius -= float32(oibot.MaxDriveRadiusMM)
						}
						i.infoLog.Printf("drive: %f @ %f", radius, dat.IRIntensity)
						b.Drive(int16(dat.IRIntensity), int16(radius))
						driveLastTime = time.Now()
					}
				}

			default:
			}
		}
	}(bot, task, serial, datExit, infExit, res)

	cleanup(clean, infExit, datExit)

	a := res.GetBool()

	return a
}

func senseTask(task *TaskInfo, serial *SerialInfo, clean CleanSignal) {

	// initialize the object, prepare for entering into state machine
	ssu := MakeSensor(task.infoLog, task.errorLog, serial.path, serial.baud)

	inpExit := make(CleanSignal)
	outExit := make(CleanSignal)

	go func(s *Sensor, i *TaskInfo, c CleanSignal, t <-chan time.Time) {
		for {
			select {
			case l := <-c:
				i.errorLog.Printf("caught signal %+v, terminating sensor suite serial monitor", l)
				i.exitSig <- l
				return
			case <-t:
				if data, ok := s.Data(); ok {
					i.ssuData <- data
				}
			default:
			}
		}
	}(ssu, task, inpExit, time.Tick(10*time.Millisecond))

	go func(s *Sensor, i *TaskInfo, c CleanSignal, t <-chan time.Time) {
		const (
			oibotNotConnected = 0
			oibotConnected    = 1
		)
		for {
			select {
			case l := <-c:
				i.errorLog.Printf("caught signal %+v, terminating sensor suite input relay", l)
				i.exitSig <- l
				return
			case <-t:
				//i.infoLog.Printf("setting mode = %s", mode)
			case stat := <-i.botStat:
				if nil == stat {
					statOut := fmt.Sprintf("%d %s %d\n", oibotNotConnected, "N/C", 0)
					s.Write([]byte(statOut))
				} else {
					if mode, ok := oibot.OIModeStr(stat.Mode); ok {
						batt := float64(stat.Battery.BatteryChargemAh) / float64(stat.Battery.BatteryCapacitymAh) * 100.0
						statOut := fmt.Sprintf("%d %s %d\n", oibotConnected, mode, int32(math.Round(batt)))
						s.Write([]byte(statOut))
					}
				}
			default:
			}
		}
	}(ssu, task, outExit, time.Tick(250*time.Millisecond))

	cleanup(clean, inpExit, outExit)
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
	botExit := make(CleanSignal)

	ssuPath := os.Args[2]
	ssuBaud := defaultBaudRateBPS // defined in sensor.go
	ssuExit := make(CleanSignal)

	clean := make(CleanSignal)
	signal.Notify(clean, os.Interrupt, syscall.SIGTERM, syscall.SIGINT)

	task := TaskInfo{
		infoLog:  log.New(os.Stdout, "[ ] ", log.Ltime),
		errorLog: log.New(os.Stderr, "[!] ", log.Ltime|log.Lshortfile),
		ssuData:  make(chan *SensorData),
		botStat:  make(chan *oibot.InfoStatus),
		userCmd:  make(chan int16),
		exitSig:  make(CleanSignal),
	}

	go func(i *TaskInfo, s *SerialInfo, c CleanSignal) {
		for {
			if oibotTask(i, s, c) {
				i.infoLog.Print("restarting bot task")
			} else {
				break
			}
		}
	}(&task, &SerialInfo{botPath, botBaud, oibot.DefaultReadTimeoutMS}, botExit)

	go func(i *TaskInfo, s *SerialInfo, c CleanSignal) {
		senseTask(i, s, c)
	}(&task, &SerialInfo{ssuPath, ssuBaud, oibot.NeverReadTimeoutMS}, ssuExit)

	cleanup(clean, botExit, ssuExit)

	task.infoLog.Print("exited gracefully")
}
