cathys - Create 2® Autonomous Telepresence from Home Yet Secure
--

This repository contains all of the source code for the iRobot® Create 2®-based
telepresence robot for the Steven's Institute of Technology Systems Engineering
(Cyber Physical Systems) Master's degree program.

The purpose of our particular team's chosen design was to implement a 
telepresence robot (TCP/IP peer-to-peer, audio-visual communication) capable of 
autonomous navigation with a designated human escort in secure environments.

The requirement to operate in secure environments prohibits the use of optical
cameras, microphones, and storing detailed maps of the local premises. 

Additionally, for ease of deployment and relocation, we do not require path
traces or waypoints typical of line-following systems.

Instead, the system implements a suite of infrared and ultrasonic sensors
attached to the robot that are tuned and calibrated to identify and follow a
signal-emitter beacon carried by security-approved personnel. This effectively
reduces the security considerations to those equivalent of any facility that
permits temporary guests escorted by local delegates. The robot has audio-
visual access to only those facilities granted by the human escort.

TODO:

  1. Define functional system architecture and logical components
  2. Define primary use cases and operating guidelines
  3. Define high level requirements
  4. Define hardware and software components implemented

