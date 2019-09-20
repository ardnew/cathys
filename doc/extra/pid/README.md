Description
--

This package contains the template C psuedo code developed by Justin Linck as
prototype for the Roomba wheels motor control. It is a [PID controller][pid].

The idea is to use this algorithm as a filter to progressively adjust wheel
speed so that the robot turns proportional to the IR sensor direction.

Motivation
--

The [Arduino sensor library][sensor] already applies a low-pass filter, using a
rolling arithmetic mean, on the received IR analog signal to smooth out the 
spikes and limit outliers in sensor data; this produces a **linear** factor
applied to the stream output at each poll event.

However, this PID algorithm accounts for prior **rates of change** and adjusts
the stream using a differential (**quadratic**) factor. This produces a more
natural mechanical movement towards the source signal, proportionally speeding
up or slowing down the turn rate depending on the **difference** between current
angle and destination angle.

Adaptation
--
This PID algorithm is designed for application to the smooth, filtered IR data
produced by the [Arduino sensor library][sensor]; i.e. no filtering or regression
analysis is applied to the data input to the PID algorithm.

However, the location to apply this algorithm is still TBD. The options are:

1. Implement in the [Arduino sensor library][sensor] at the time immediately following the low-pass filter. The resulting value produced by the differential PID factor is then output as the primary output of the sensor library. The original data produced by the low-pass filter would not be output or available externally to the library. The original raw sensor data remains private regardless.
2. Implement in the [Create drive controller][drive] at the time immediately following receipt of the low-pass filter output via serial UART. The serial communication is a little less reliable, so the input to the PID algorithm may not be as smooth, but the drive controller will retain access to both the low-pass filter data as well as the PID-processed data. Not sure if this is desirable, or if the benefit outweighs the potential loss in signal accuracy. Again, the original raw sensor data is still not unavailable.

[pid]:https://en.wikipedia.org/wiki/PID_controller
[sensor]:https://github.com/ardnew/cathys/tree/master/platform/arduino/cathys-sensor
[drive]:https://github.com/ardnew/cathys/tree/master/platform/create/cathys-drive

