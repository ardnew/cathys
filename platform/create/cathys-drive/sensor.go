package main

import (
	"encoding/json"
	"fmt"
	"log"
	"strings"
	"time"

	"github.com/ardnew/oibot"
	"github.com/tarm/serial"
)

const (
	defaultBaudRateBPS int = 115200
)

type Sensor struct {
	port     *serial.Port
	infoLog  *log.Logger
	errorLog *log.Logger
	path     string
	baud     int
}

type SensorData struct {
	UserCommand int16   `json:"user-command"`
	IRAngle     int16   `json:"ir-angle"`
	IRIntensity float32 `json:"ir-intensity"`
}

func MakeSensor(infoLog *log.Logger, errorLog *log.Logger, path string, baud int) *Sensor {
	var s *Sensor
	if port, err := serial.OpenPort(&serial.Config{Name: path, Baud: baud}); nil != err {
		errorLog.Panic(fmt.Errorf("failed to open serial port: %s (%d): %s", path, baud, err))
	} else {
		s = &Sensor{port: port, infoLog: infoLog, errorLog: errorLog, path: path, baud: baud}
	}
	return s
}

func (s *Sensor) Read(buf []byte) int {
	count := 0
	if n, err := s.port.Read(buf); nil != err {
		s.errorLog.Panic(fmt.Errorf("failed to read from serial port: %s", err))
	} else {
		count = n
	}
	time.Sleep(oibot.SerialTransferDelayMS)
	return count
}

func (s *Sensor) Data() *SensorData {
	// for help with the magic numbers used here, see the JSON output definition
	// in cathys-sensor.ino as well as the calculator tool (for the serialization
	// lib used in that project) here:
	//   https://arduinojson.org/v6/assistant/
	const (
		jsonDataDelimiter = "\n"
		jsonObjSize       = 16
		jsonObjCount      = 3
		jsonStringsSize   = 35                 // the sum of all key identifiers
		jsonMarkupSize    = 2 + 4*jsonObjCount // outer brackets and key delimiters
		jsonDataSize      = jsonObjSize*jsonObjCount + jsonStringsSize + jsonMarkupSize + 1

		// the buffer is twice as big as one JSON struct so that we can guarantee
		// to have a newline delimiter somewhere in the stream, since our reads do
		// not necessarily begin or end with any relation to the JSON delimiters.
		jsonBufSize = 2 * jsonDataSize
	)
	var (
		data = SensorData{}
	)
	buf := make([]byte, jsonBufSize)
	if s.Read(buf) > 0 {

		for _, str := range strings.Split(string(buf), jsonDataDelimiter) {
			if len(str) > 0 && str[0] == byte('{') && str[len(str)-1] == byte('}') {
				if err := json.Unmarshal([]byte(str), &data); nil != err {
					s.errorLog.Panic(fmt.Errorf("failed to unmarshal JSON data: %v", str))
				} else {
					s.infoLog.Printf("%v (%T)", data, data)
				}
			}
		}
	}
	return &data
}
