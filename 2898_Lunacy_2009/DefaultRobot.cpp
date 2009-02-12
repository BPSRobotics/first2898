/****************************************************************
 * 
 * This File is the main file. all of the function of the robot
 * is in this file.
 * 
 *  .--~*teu.       u+=~~~+u.                      u+=~~~+u.   	
 *  dF     988Nx   z8F      `8N.    .xn!~%x.      z8F      `8N. 	
 * d888b   `8888> d88L       98E   x888   888.   d88L       98E 	
 * ?8888>  98888F 98888bu.. .@*   X8888   8888:  98888bu.. .@*  	
 *  "**"  x88888~ "88888888NNu.   88888   X8888  "88888888NNu.  	
 *       d8888*`   "*8888888888i  88888   88888>  "*8888888888i 	
 *     z8**"`   :  .zf""*8888888L `8888  :88888X  .zf""*8888888L	
 *   :?.....  ..F d8F      ^%888E   `"**~ 88888> d8F      ^%888E	
 *  <""888888888~ 88>        `88~  .xx.   88888  88>        `88~	
 *  8:  "888888*  '%N.       d*"  '8888>  8888~  '%N.       d*" 	
 *  ""    "**"`      ^"====="`     888"  :88%       ^"====="`   
 *                                 ^"===""   
 *
 * Programming Notes:
 *
 * Steven: Might be a good idea to get use to proper indenting, 
 * like keeping alignment with the scope of the program makes checking for 
 * forgotten brackets and stuff
 * also I personally find that using empty lines to seperate chunks of code
 * make readability easier
 * 2-10-09
 * 
 * Steven: BTW if this revision fails go back to 9 (camera first intergration) or even 8 (before camera)!
 * 2-11-09
 * 
 ****************************************************************/

/****************************************************************\
 * Includes
\****************************************************************/
#include "DefaultRobot.h" //everything is in there

/****************************************************************\
 * Functions
\****************************************************************/
/** Simple test to see if the color is taking up too much of the image */

//todo: Move this camera stuff into its own class!
int tooClose(ParticleAnalysisReport* par) 
{
	if (par->particleToImagePercent > MAX_PARTICLE_TO_IMAGE_PERCENT)
	{
		return 1;
	}
	return 0;
}

/** Simple test to see if the color is large enough */
int bigEnough(ParticleAnalysisReport* par) 
{
	if (par->particleToImagePercent < MIN_PARTICLE_TO_IMAGE_PERCENT)
	{
		return 0;
	}
	return 1;
}

//placeholding code
float slipcheck(){
	return 1.0;
}


//functions for class "DefaultRobot"
/**
 * 
 * 
 * Constructor for this robot subclass.
 * Create an instance of a RobotDrive with left and right motors plugged into PWM
 * ports 1 and 2 on the first digital module.
*/
DefaultRobot::DefaultRobot(void)
{ 
	
	//set up there stuff
	ds = DriverStation::GetInstance();
	myRobot = new RobotDrive(rightdrive, leftdrive);
	rightStick = new Joystick(1);			// create the joysticks
	leftStick = new Joystick(2);
	
	//set up our stuff
	// these channel are all controlled by enum but may need slot val
	mycheckball = new DigitalInput(7); //fake
	mytopspinner  = new Victor(topspinner);
	myconveyerbelt  = new Victor(conveyerbelt);
	mybottomspinner = new Relay(1); 
	mytimer = new Timer();
	myAccelerometer = new Accelerometer(1); //need real val
	
	//set up camera stuff
	horizontalServo = new Servo(9); 		// create horizontal servo on PWM 9
	verticalServo = new Servo(10);			// create vertical servo on PWM 10
	servoDeadband = 0.01;					// move if > this amount 
	framesPerSecond = 15;					// number of camera frames to get per second
	sinStart = 0.0;							// control where to start the sine wave for pan
	memset(&par1,0,sizeof(ParticleAnalysisReport));			// initialize particle analysis report
	memset(&par2,0,sizeof(ParticleAnalysisReport));			// initialize particle analysis report

	/* image data for tracking - override default parameters if needed */
	/* recommend making PINK the first color because GREEN is more 
	 * subsceptible to hue variations due to lighting type so may
	 * result in false positives */
	// PINK
	sprintf (td1.name, "PINK");
	td1.hue.minValue = 220;   
	td1.hue.maxValue = 255;  
	td1.saturation.minValue = 75;   
	td1.saturation.maxValue = 255;      
	td1.luminance.minValue = 85;  
	td1.luminance.maxValue = 255;
	// GREEN
	sprintf (td2.name, "GREEN");
	td2.hue.minValue = 55;   
	td2.hue.maxValue = 125;  
	td2.saturation.minValue = 58;   
	td2.saturation.maxValue = 255;    
	td2.luminance.minValue = 92;  
	td2.luminance.maxValue = 255;
	
	/* set up debug output: 
	 * DEBUG_OFF, DEBUG_MOSTLY_OFF, DEBUG_SCREEN_ONLY, DEBUG_FILE_ONLY, DEBUG_SCREEN_AND_FILE 
	 */
	SetDebugFlag(DEBUG_SCREEN_ONLY);
	
	/* start the CameraTask	 */
	if (StartCameraTask(framesPerSecond, 0, k320x240, ROT_0) == -1) {
		DPRINTF( LOG_ERROR,"Failed to spawn camera task; exiting. Error code %s", 
				GetVisionErrorText(GetLastVisionError()) );
	}
	/* allow writing to vxWorks target */
	Priv_SetWriteFileAllowed(1); 
	
	//Update the motors at least every 100ms.
	GetWatchdog().SetExpiration(100);
	
}

/**
 * Set servo positions (0.0 to 1.0) translated from normalized values (-1.0 to 1.0). 
 * 
 * @param normalizedHorizontal Pan Position from -1.0 to 1.0.
 * @param normalizedVertical Tilt Position from -1.0 to 1.0.
 */
void DefaultRobot::setServoPositions(float normalizedHorizontal, float normalizedVertical)	{

	float servoH = NormalizeToRange(normalizedHorizontal);
	float servoV = NormalizeToRange(normalizedVertical);
	
	float currentH = horizontalServo->Get();		
	float currentV = verticalServo->Get();
	
	/* make sure the movement isn't too small */
	if ( fabs(servoH - currentH) > servoDeadband ) {
		horizontalServo->Set( servoH );
		/* save new normalized horizontal position */
		horizontalPosition = RangeToNormalized(servoH, 1);
	}
	if ( fabs(servoV - currentV) > servoDeadband ) {
		verticalServo->Set( servoV );
		verticalPosition = RangeToNormalized(servoV, 1);
	}
}	

/**
 * Adjust servo positions (0.0 to 1.0) translated from normalized values (-1.0 to 1.0). 
 * 
 * @param normalizedHorizontal Pan adjustment from -1.0 to 1.0.
 * @param normalizedVertical Tilt adjustment from -1.0 to 1.0.
 */
void DefaultRobot::adjustServoPositions(float normDeltaHorizontal, float normDeltaVertical)	{
					
	/* adjust for the fact that servo overshoots based on image input */
	normDeltaHorizontal /= 8.0;
	normDeltaVertical /= 4.0;
	
	/* compute horizontal goal */
	float currentH = horizontalServo->Get();
	float normCurrentH = RangeToNormalized(currentH, 1);
	float normDestH = normCurrentH + normDeltaHorizontal;	
	/* narrow range keep servo from going too far */
	if (normDestH > 1.0) normDestH = 1.0;
	if (normDestH < -1.0) normDestH = -1.0;			
	/* convert input to servo range */
	float servoH = NormalizeToRange(normDestH);

	/* compute vertical goal */
	float currentV = verticalServo->Get();
	float normCurrentV = RangeToNormalized(currentV, 1);
	float normDestV = normCurrentV + normDeltaVertical;	
	if (normDestV > 1.0) normDestV = 1.0;
	if (normDestV < -1.0) normDestV = -1.0;
	/* convert input to servo range */
	float servoV = NormalizeToRange(normDestV, 0.2, 0.8);

	/* make sure the movement isn't too small */
	if ( fabs(currentH-servoH) > servoDeadband ) {
		horizontalServo->Set( servoH );		
		/* save new normalized horizontal position */
		horizontalPosition = RangeToNormalized(servoH, 1);
	}			
	if ( fabs(currentV-servoV) > servoDeadband ) {
		verticalServo->Set( servoV );
		verticalPosition = RangeToNormalized(servoV, 1);
	}
}

/**
 * Drive left & right motors for 2 seconds, enabled by a jumper (jumper
 * must be in for autonomous to operate).
 * Currently programmed to drive forward and then turn right to block opponent
 */
void DefaultRobot::Autonomous(void)
{
	//todo: make sure this works?
	
	DPRINTF(LOG_DEBUG, "Autonomous");				
	
	DPRINTF(LOG_DEBUG, "SERVO - looking for COLOR %s ABOVE %s", td2.name, td1.name);
	
	// initialize position and destination variables
	// position settings range from -1 to 1
	// setServoPositions is a wrapper that handles the conversion to range for servo 
	horizontalDestination = 0.0;		// final destination range -1.0 to +1.0
	verticalDestination = 0.0;
	
	// initialize pan variables
	// incremental tasking toward dest (-1.0 to 1.0)
	float incrementH, incrementV;
	// pan needs a 1-up number for each call
	int panIncrement = 0;							
	
	// current position range -1.0 to +1.0
	horizontalPosition = RangeToNormalized(horizontalServo->Get(),1);	
	verticalPosition = RangeToNormalized(verticalServo->Get(),1);			
	
	// set servos to start at center position
	setServoPositions(horizontalDestination, verticalDestination);

	// for controlling loop execution time 
	float loopTime = 0.1;		
	//float loopTime = 0.05;											
	double currentTime = GetTime();
	double lastTime = currentTime;
									
	// search variables 
	bool foundColor = 0; 
	double savedImageTimestamp = 0.0;
	bool staleImage = false; 
			
	while( IsAutonomous() )	
	{
		//this is where auto mode works
		GetWatchdog().SetEnabled(false);
		
		//todo figure out what auto is suppose to do other than tracking
		
		/*myRobot->Drive(0.5, 0.0);			// drive forwards half speed
		Wait(500);	//btw these are in miliseconds -SG
		
		myRobot->Drive( 1.0, 0.5);
		Wait(500);
		
        myRobot->Drive (.5, 0.-5);
        Wait(500);
        
		myRobot->Drive(0.0, 0.0 );			// stop robot
		//break if camera in == true
		printf("AUTO MODE"); */	

		// calculate gimbal position based on colors found 
		if ( FindTwoColors(td1, td2, ABOVE, &par1, &par2) ){
			//PrintReport(&par2);
			foundColor = true;
			// reset pan		
			panIncrement = 0;  		
			if (par1.imageTimestamp == savedImageTimestamp) {
				// This image has been processed already, 
				// so don't do anything for this loop 
				staleImage = true;
				DPRINTF(LOG_DEBUG, "STALE IMAGE");
				
			} else {
				// The target was recognized
				// save the timestamp
				staleImage = false;
				savedImageTimestamp = par1.imageTimestamp;	
				DPRINTF(LOG_DEBUG,"image timetamp: %lf", savedImageTimestamp);

				// Here is where your game-specific code goes
				// when you recognize the target
				
				// get center of target 
				// Average the color two particles to get center x & y of combined target
				horizontalDestination = (par1.center_mass_x_normalized + par2.center_mass_x_normalized) / 2;	
				verticalDestination = (par1.center_mass_y_normalized + par2.center_mass_y_normalized) / 2;							
			}
		} else {  // need to pan 
			foundColor = false;
		} 
							
		if(foundColor && !staleImage) {	
			/* Move the servo a bit each loop toward the destination.
			 * Alternative ways to task servos are to move immediately vs.
			 * incrementally toward the final destination. Incremental method
			 * reduces the need for calibration of the servo movement while
			 * moving toward the target.
			 */
			incrementH = horizontalDestination - horizontalPosition;
			// you may need to reverse this based on your vertical servo installation
			//incrementV = verticalPosition - verticalDestination;
			incrementV = verticalDestination - verticalPosition;
			adjustServoPositions( incrementH, incrementV );  
			
			ShowActivity ("** %s & %s found: Servo: x: %f  y: %f ** ", 
					td1.name, td2.name, horizontalDestination, verticalDestination);	
			
		} else { //if (!staleImage) {  // new image, but didn't find two colors
			
			// adjust sine wave for panning based on last movement direction
			if(horizontalDestination > 0.0)	{ sinStart = PI/2.0; }
			else { sinStart = -PI/2.0; }

			/* pan to find color after a short wait to settle servos
			 * panning must start directly after panInit or timing will be off */				
			if (panIncrement == 3) {
				panInit(8.0);		// number of seconds for a pan
			}
			else if (panIncrement > 3) {					
				panForTarget(horizontalServo, sinStart);	
				
				/* Vertical action: In case the vertical servo is pointed off center,
				 * center the vertical after several loops searching */
				if (panIncrement == 20) { verticalServo->Set( 0.5 );	}
			}
			panIncrement++;		

			ShowActivity ("** %s and %s not found                                    ", td1.name, td2.name);
		}  // end if found color

		// sleep to keep loop at constant rate
		// this helps keep pan consistant
		// elapsed time can vary significantly due to debug printout
		currentTime = GetTime();			
		lastTime = currentTime;					
		if ( loopTime > ElapsedTime(lastTime) ) {
			Wait( loopTime - ElapsedTime(lastTime) );	// seconds
		}			
		GetWatchdog().SetEnabled(true); //reenable the watch dog
	}  // end while

	DPRINTF(LOG_DEBUG, "end Autonomous");
	ShowActivity ("Autonomous end                                            ");
}

/**
 * Runs the motors under driver control with either tank or arcade steering selected
 * by a jumper in DS Digin 0. Also an arm will operate based on a joystick Y-axis. 
 */
void DefaultRobot::OperatorControl(void)
{
	int beltstatus = 0; 
	/*The beltstatus variable is to keep track of how the belt is set to be driven
	 * 1 = up -1 =down 0 = neutral*/
	while (IsOperatorControl())
	{
		printf("TELE MODE");
		GetWatchdog().Feed();

		// determine if tank or arcade mode; default with no jumper is for tank drive
			
		myRobot->TankDrive(leftStick->GetY() * slipcheck() * leftsign, rightStick->GetY() * slipcheck() * rightsign);	 // drive with tank style
		 
		/* we are using Tankdrive
		 * else 
		{
			myRobot->ArcadeDrive(rightStick);	         // drive with arcade style (use right stick)
		}*/
		// beginning of the 2898 secondary system code
		
		mybottomspinner->Set(Relay::kForward); //go forward 
		if (ds->GetDigitalIn(upswitch) == true)
		{
			myconveyerbelt->Set(DEFAULTBELTSPEED);  // move belt up
			beltstatus = 1;
		}
		// the stopping of the main belt is controlled by next if check
		if (ds->GetDigitalIn(downswitch) == true && ds->GetDigitalIn(upswitch)==false)
		{ //if down is push and up is not pushed, proceed
			myconveyerbelt->Set(-1 * DEFAULTBELTSPEED); //set belt down
			beltstatus = -1;
		}
		else
		{
			myconveyerbelt->Set(0); //stop belt
			beltstatus = 0;
		}
		
		if (mycheckball->Get() == true) 
			//unsure if "get" works
			{
				myconveyerbelt->Set(DEFAULTBELTSPEED);     //life ball up one slot
				mytimer->Start();
				
			}
		
		if (mytimer->Get() == 2)
			{
				myconveyerbelt->Set(beltstatus * DEFAULTBELTSPEED); //set the conveyer belt back to its intended state
				mytimer->Stop();
				mytimer->Reset();
			}
	}
}

START_ROBOT_CLASS(DefaultRobot);
