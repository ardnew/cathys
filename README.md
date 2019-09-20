cathys - Create® 2 Autonomous Telepresence from Home Yet Secure
==

This repository contains all of the source code for the [iRobot® Create®
2][create]-based telepresence robot for the [Steven's Institute of 
Technology][stevens] Systems Engineering (Embedded Cyber-Physical Systems)
[Master's degree program][degree].


The purpose of our particular team's chosen design was to implement a 
[telepresence][telepresence] robot ([TCP/IP][tcpip] [peer-to-peer][p2p],
[audio-visual][av] communication) capable of [autonomous navigation][autonav]
with a designated human escort in secure environments.


The requirement to operate in secure environments prohibits the use of [optical
cameras][camera], [microphones][microphone], and [storing detailed maps of the
local premises][maps]. 


Additionally, for ease of deployment and relocation, we do not require [path 
traces or waypoints][tracing] typical of [line-following systems][linefollow].


Instead, the system implements a suite of [infrared][infrared] and 
[ultrasonic][ultrasound] sensors attached to the robot that are tuned and
calibrated to [identify and follow][direction] a [signal-emitter beacon][beacon]
carried by security-approved personnel. This effectively reduces the security
considerations to those equivalent of any facility that permits temporary guests
escorted by local delegates. The robot has audio-visual access to only those
facilities granted by the human escort.


Implementation
--

Development of this project is occurring simultaneously and in coordination
with the implementation of a Go library for the iRobot® Open Interface (OI)
specification; see [oibot][oibot].


There are two primary components currently implementing this system. They are
separated in this repository by their actual physical platform:


1. [platform][platform]/[arduino][platformarduino] — Arduino (C++) project sketches
	- [cathys-sensor][cathyssensor] Source code for reading, sampling, filtering, and processing data from the on-board sensor suite
		- Includes the IR signal strength and direction analysis, as well as the ultrasonic rangefinder, for the IR following capability
		- Also implements a touchscreen user interface for displaying sensor suite data and accepting input and general-purpose robot control from the user
2. [platform][platform]/[create][platformcreate] — Projects interacting directly with [iRobot® Create® 2][create] via on-board Mini-DIN serial connector
	- [cathys-drive][cathysdrive] — Source code for interacting with the [OI-compatible][oispec] robot via [oibot][oibot] driver library, providing an interface to peripheral devices
		- Handles translating the communication via serial UART between the [sensor suite and touch screen][cathyssensor] and the actual [robot][oibot]


TODO
==

1. Define functional system architecture and logical components
2. Define primary use cases and operating guidelines
3. Define high level requirements
4. Define hardware and software components implemented

[create]:https://en.wikipedia.org/wiki/IRobot_Create
[stevens]:https://en.wikipedia.org/wiki/Stevens_Institute_of_Technology
[degree]:https://www.stevens.edu/school-systems-enterprises/masters-degree-programs/systems-engineering/curriculum-overview
[telepresence]:https://en.wikipedia.org/wiki/Telepresence
[tcpip]:https://en.wikipedia.org/wiki/Internet_protocol_suite
[p2p]:https://en.wikipedia.org/wiki/Point-to-point_(telecommunications)
[av]:https://en.wikipedia.org/wiki/Audiovisual
[autonav]:https://en.wikipedia.org/wiki/Robot_navigation
[camera]:https://en.wikipedia.org/wiki/Optical_camera
[microphone]:https://en.wikipedia.org/wiki/Microphone
[maps]:https://en.wikipedia.org/wiki/Robotic_mapping
[tracing]:https://en.wikipedia.org/wiki/Motion_planning
[linefollow]:https://en.wikipedia.org/wiki/Mobile_robot#Line-following_Car
[infrared]:https://en.wikipedia.org/wiki/Infrared
[ultrasound]:https://en.wikipedia.org/wiki/Ultrasound
[direction]:https://en.wikipedia.org/wiki/Direction_finding
[beacon]:https://en.wikipedia.org/wiki/Non-directional_beacon
[oispec]:https://www.irobotweb.com/~/media/MainSite/PDFs/About/STEM/Create/iRobot_Roomba_600_Open_Interface_Spec.pdf
[platform]:https://github.com/ardnew/cathys/tree/master/platform
[platformarduino]:https://github.com/ardnew/cathys/tree/master/platform/arduino
[platformcreate]:https://github.com/ardnew/cathys/tree/master/platform/create
[cathyssensor]:https://github.com/ardnew/cathys/tree/master/platform/arduino/cathys-sensor
[cathysdrive]:https://github.com/ardnew/cathys/tree/master/platform/create/cathys-drive
[oibot]:https://github.com/ardnew/oibot
