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
	
	//todo: this
	//*encoderRight = new Encoder();
	//*encoderLeft = new Encoder();
	
	servoPan = new Servo(9);
	servoTilt = new Servo(10);
	camera = new Vision(servoPan, servoTilt) ;
	
	//set up camera stuff
	
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
	
	camera->LookFor(&td1, &td2); //set the color
	
	//Update the motors at least every 100ms.
	GetWatchdog().SetExpiration(100);
	
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
			
	while( IsAutonomous() )	
	{
		//this is where auto mode works
		GetWatchdog().SetEnabled(false);
		
		//todo figure out what auto is suppose to do other than tracking
		
		myRobot->Drive(0.5, 0.0);			// drive forwards half speed
		Wait(500);	//btw these are in miliseconds -SG
		
		myRobot->Drive( 1.0, 0.5);
		Wait(500);
		
        myRobot->Drive (.5, -0.5);
        Wait(500);
        
		myRobot->Drive(0.0, 0.0);			// stop robot	
		
		//cam stuff here?
		myRobot->Drive(0.25, 1.0); //look for targets by turning
		//cam tracking!
		if (camera->DoTracking()) //note camera does try to regulate the execution so currently there is no start stop methods to solve this problem
		{ //or putting logic to check weather or not to care about if it sees a target.
			
			myRobot->Drive(0.25, 0.0); //I see target move towards it
		}
		else
		{
			myRobot->Drive(0.25, 1.0); //I don't start to turn again
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
