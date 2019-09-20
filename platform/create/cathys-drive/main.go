package main

import (
	"log"
	"os"
	"sync"
	"time"

	"github.com/ardnew/oibot"
)

func main() {

	defer func() {
		if r := recover(); r != nil {
			// empty
		}
	}()

	if len(os.Args) <= 2 {
		log.Fatalf("usage:\n\t%s <roomba-dev> <sensor-dev>", os.Args[0])
	}

	infoLog := log.New(os.Stdout, "[ ] ", log.Ltime)
	errorLog := log.New(os.Stderr, "[!] ", log.Ltime|log.Lshortfile)

	rooPath := os.Args[1]
	rooBaud := oibot.DefaultBaudRateBPS

	snsPath := os.Args[2]
	snsBaud := defaultBaudRateBPS // defined in sensor.go

	userCmd := make(chan string)
	sensorData := make(chan *SensorData)

	pool := new(sync.WaitGroup)
	pool.Add(2) // 1 for Roomba routine + 1 for sensor routine

	go func(group *sync.WaitGroup, infoLog *log.Logger, errorLog *log.Logger, path string, baud int) {
		defer group.Done()

		roo := oibot.MakeRoomba(infoLog, errorLog, path, baud)

		roo.Start()
		roo.Safe()

		//roo.Drive(-100, 2)
		//time.Sleep(1000 * time.Millisecond)
		//data = roo.Battery()
		//time.Sleep(1000 * time.Millisecond)
		//roo.DriveStop()
		//infoLog.Printf("%v (%T)", data, data)

		for {
			select {
			case <-userCmd:

			case data := <-sensorData:
				infoLog.Printf("sensor data: %v (%T)", data, data)

			default:

				//roo.Sensor(packet)0
			}
		}

	}(pool, infoLog, errorLog, rooPath, rooBaud)

	go func(group *sync.WaitGroup, infoLog *log.Logger, errorLog *log.Logger, path string, baud int) {
		defer group.Done()

		sns := MakeSensor(infoLog, errorLog, path, baud)
		poll := time.Tick(1 * time.Millisecond)

		for {
			select {
			case <-poll:
				sensorData <- sns.Data()

			default:
			}
		}

	}(pool, infoLog, errorLog, snsPath, snsBaud)

	pool.Wait()
}
