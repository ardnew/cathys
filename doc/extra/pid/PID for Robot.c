#define Kp 0.0 // experiment to determine this, start by something small that just makes your bot follow the line at a slow speed
#define Kd 0.0 // experiment to determine this, slowly increase the speeds and adjust this value. ( Note: Kp < Kd) 
#define Ki 0.0 // experiment to determine this, start this value at 0. 
#define centerPosition 2.5 //this the the value of a centered IR signal
#define rightPosition 5 // this is the values of the most right IR signal position
#define maxSpeed 255 // max speed of the robot 
#define minSpeed -255 // min speed of the robot
#define positionSignalError = -9999 //error value from getPosition that means that signal is too weak to follow
#define slowingDistance = 36.0 //distance in inches from  an obstacle that the robot should start to move slower
#define stoppingDistance = 18.0 //distance in inches from  an obstacle that the robot should start stop







int main () {

  float currentPosition = 0.0;
  float lastPosition = rightPosition //set the last position to rightPositiion so that is robot will search right if it has no signal on initialization
  float obstacleDistance = 0.0; //This is the distance to the closest obstacle
  float error = 0.0;
  float integral = 0.0;
  float deltaSpeed = 0.0;
  float lastError = 0.0;
  float obstacleDistance = 0.0; //This is the distance to the closest obstacle
  float baseSpeed = 0.0; // this is the speed that the robot should move at if the IR signal is directly in front
  

  while(1) {
    currentPosition = getPosition(); //This function should return a float that corresponds to the current IR signal direction, with a centered position being equal to centerPosition defined above and values greater than centerPosition being a signal to the right
    
	obstacleDistance = getRange(); // This function should return the distance in inches to the nearest obstacle/the guide
	if (obstacleDistance > slowingDistance) obstacleDistance =  slowingDistance; //limit obstacleDistance to slowing distance so that the baseSpeed equation works
	if (obstacleDistance < stoppingDistance) obstacleDistance =  slowingDistance; //limit obstacleDistance to stoppingDistance so that the baseSpeed equation works
	baseSpeed = (int)maxSpeed * ((obstacleDistance - stoppingDistance)/(slowingDistance - stoppingDistance)) //this should slow the robot to a stop as is approaches an obstacle or the guide
	if (currentPosition = positionSignalError) { //if position signal has no solution, then the baseSpeed should go to zero and the currentPosition should be set to lastPosition. This means that the robot will stop and turn in the direction of last signal
		baseSpeed = 0.0;
		currentPosition = lastPosition;
	}//if.. 
	
	error = currentPosition - centerPosition;
	integral = integral + error; //this is a sum of the total errors
	deltaSpeed = Kp * error + Kd * (error - lastError) + Ki * integral; //PID equation
    lastError = error;
	lastPosition = currentPosition;
	
    int rightMotorSpeed = (int)baseSpeed + (int)deltaSpeed;
    int leftMotorSpeed = (int)baseSpeed - (int)deltaSpeed;
	
	if (rightMotorSpeed > maxSpeed ) rightMotorSpeed = rightMaxSpeed; // prevent the motor from going beyond max speed
    if (leftMotorSpeed > maxSpeed ) leftMotorSpeed = leftMaxSpeed; // prevent the motor from going beyond max speed
    if (rightMotorSpeed < minSpeed) rightMotorSpeed = minSpeed; // prevent the motor from going beyond min speed
    if (leftMotorSpeed < minSpeed) leftMotorSpeed = minSpeed; // prevent the motor from going beyond minax speed
	//This motor speeds should be given to the robot

   }//while(1)
 } //main