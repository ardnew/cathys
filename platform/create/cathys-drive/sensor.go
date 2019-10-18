package main

import (
	"encoding/json"
	"fmt"
	"log"
	"strings"

	"github.com/tarm/serial"
)

const (
	defaultBaudRateBPS int     = 115200
	minIRIntensity     float32 = 100.0 / 6.0 // num IR diodes == 6
)

const (
	ucmdNONE = iota - 1
	ucmdPassive
	ucmdSafe
	ucmdFull
	ucmdReset
	ucmdOff
	ucmdCOUNT
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
	return count
}

func (s *Sensor) Data() (*SensorData, bool) {
	// for help with the magic numbers used here, see the JSON output definition
	// in cathys-sensor.ino as well as the calculator tool (for the serialization
	// lib used in that project) here:
	//   https://arduinojson.org/v6/assistant/
	const (
		jsonObjSize     = 16
		jsonObjCount    = 3
		jsonStringsSize = 35                 // the sum of all key identifiers
		jsonMarkupSize  = 2 + 4*jsonObjCount // outer brackets and key delimiters
		jsonDataSize    = jsonObjSize*jsonObjCount + jsonStringsSize + jsonMarkupSize + 1

		// the buffer is twice as big as one JSON struct so that we can guarantee
		// to have a newline delimiter somewhere in the stream, since our reads do
		// not necessarily begin or end with any relation to the JSON delimiters.
		jsonBufSize = 1.5*jsonDataSize + 1
	)
	var (
		data = SensorData{UserCommand: ucmdNONE, IRAngle: -1, IRIntensity: -1}
	)
	buf := make([]byte, jsonBufSize)
	if s.Read(buf) > 0 {
		for _, str := range strings.Fields(string(buf)) {
			// do a preliminary sanity check before trying to unmarshal. this really
			// only helps reduce the number of errors logged to output.
			if strings.HasPrefix(str, "{") && strings.HasSuffix(str, "}") {
				if err := json.Unmarshal([]byte(str), &data); nil != err {
					s.errorLog.Printf("failed to unmarshal JSON data: %+v", str)
				} else {
					//if data.IRAngle > 0 && data.IRIntensity > 0
					return &data, true
				}
			}
		}
	}
	return nil, false
}

func (s *Sensor) Write(buf []byte) {
	if n, err := s.port.Write(buf); n <= 0 || nil != err {
		s.errorLog.Printf("failed to write to serial port: %+v: %s", buf, err)
	}
}
