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
 * Brian :  I have included the slipcheck function (linear) for use in robot drive in auto mode
 * I do not know if I will be able to test it on wendsday but this code should work and it is the
 * best yet....  
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
#include "Encoder.h"
#include "math.h"

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
	
	currentrightmotoroutput = 0; //one time declaration for slipcheck function
	currentleftmotoroutput = 0;
	currentautomotoroutput =0; // one time delcaration for slipcheck in auto mode
	beltstatus = 0;  //mypickupballcheck function
	
	//set up there stuff
	ds = DriverStation::GetInstance();
	myRobot = new RobotDrive(rightdrive, leftdrive);
	rightStick = new Joystick(1);			// create the joysticks
	leftStick = new Joystick(2);
	
	//set up our stuff
	// these channel are all controlled by enum but may need slot val
	myopticalballchecker = new DigitalInput(1); //optical sensor to detect balls
	mytopspinner  = new Victor(topspinner); //victor controlling the top ball shooter
	myconveyorbelt  = new Victor(conveyorbelt); //victor controlling the conveyorbelt
	mybottomspinner = new Relay(1); //relay controlling the bottom ball collector
	mytimer = new Timer(); //timer controlling the pickup of balls
	myautotimer =new Timer();
	//myAccelerometer = new Accelerometer(1); //we dont have one
	//myAccelerometer->SetSensitivity(.3); //sensitivity is 300mV/g
	//myAccelerometer->SetZero(1.5) ;//0g is 1.5 volts
	//this sets up the encoder with the a channel on 2, and the b channel on 3
	//encoder = new Encoder(2,3);
	//encoder->Start();
	
	//todo: this
	//*encoderRight = new Encoder();
	//*encoderLeft = new Encoder();
	
	servoPan = new Servo(9);
	servoTilt = new Servo(10);
	camera = new Vision(servoPan, servoTilt) ;
	
	//set up camera stuff
	
	/* image data for tracking - override default parameters if needed */
	/* recommend making PINK the first color because GREEN is more 
	 * subsceptible to  hue variations due to lighting type so may
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
	//todo: test code
	//also, the motor values (or at least the robot drive class) seemed reversed. Look into that

	
	DPRINTF(LOG_DEBUG, "Autonomous");
	myautotimer->Reset();
	myautotimer->Start();		
	while( IsAutonomous() )	
	{
		//this is where auto mode works
		GetWatchdog().SetEnabled(false);
		if (myautotimer->Get() < 4)
		{
			//myRobot->Drive(slipcheck(currentautomotoroutput,-1.0); // reversed
			pickupball();
			pickupballtimercheck(1.2);
		}
		else
		{
			mybottomspinner->Set(Relay::kReverse );  
			myautotimer->Stop();
			myRobot->Drive(0.5,0.6); // turn on dime
			pickupball();
			pickupballtimercheck(1.2);
		}
		

//		//cam stuff here?
//		myRobot->Drive(0.25, 1.0); //look for targets by turning
//		//cam tracking!
//		if (camera->DoTracking()) //note camera does try to regulate the execution so currently there is no start stop methods to solve this problem
//		{ //or putting logic to check weather or not to care about if it sees a target.
//			myRobot->Drive(0.25, 0.0); //I see target move towards it
//		}
//		else
//		{
//			myRobot->Drive(0.25, 1.0); //I don't start to turn again
//		}
//		
//		GetWatchdog().SetEnabled(true); //reenable the watch dog
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
	int counter = 0;
	while (IsOperatorControl())
	{
		counter += 1;
		//printf("TELE MODE");
		GetWatchdog().Feed();

		// determine if tank or arcade mode; default with no jumper is for tank drive
			
		//myRobot->TankDrive(slipcheck(currentleftmotoroutput,leftStick->GetY()) * leftsign, slipcheck(currentrightmotoroutput,rightStick->GetY()) * rightsign);	 // drive with tank style
		if (leftStick->GetTrigger() == false && rightStick->GetTrigger() == false)
		{
			myRobot->TankDrive(pow(slipcheck(currentleftmotoroutput,leftStick->GetY()),3) * leftsign, pow(slipcheck(currentrightmotoroutput,rightStick->GetY()),3) * rightsign); 
		}
		else
			myRobot->TankDrive(leftStick,rightStick);
		
		/* we are using Tankdrive
		 * else 
		{
			myRobot->ArcadeDrive(rightStick);	         // drive with arcade style (use right stick)
		}*/
		// beginning of the 2898 secondary system code
		// turn on the relay controlling the bottom spinner
		mybottomspinner->Set(Relay::kReverse );  
		//print out the value of our optical sensor to check for balls
		// if button 3 pressed pressed, move belt down?
		if (ds->GetDigitalIn(1) == false)
		{
			myconveyorbelt->Set(DEFAULTBELTSPEED);
			beltstatus =1;
		}
		// else if button 2 pressed move belt up?
		else if(ds->GetDigitalIn(2)== false)
		{
			myconveyorbelt->Set(-1 * DEFAULTBELTSPEED);
			beltstatus = -1;
		}
		//if trigger pushed move belt up and start top spinner
		if (ds->GetDigitalIn(3) == false)
		{
			mytopspinner->Set(-1.0);
			myconveyorbelt->Set(DEFAULTBELTSPEED);
			beltstatus =1;
		}
		// if trigger false, stop top spinner
		else if (ds->GetDigitalIn(3) == true)
		{
			mytopspinner->Set(0.0);
		}	
		// if light sensor sees ball then move belt up and start time
		pickupball();
		
		/* check if timer has reached x seconds and if so stop and reset
		 * timer and restore belt to it's default status */
		pickupballtimercheck(1.0); // i changed this to 1.0 from 15.0 -bg seemed way too long
		}
	} //end robot object declaration

/*The slipcheck function is designed to increment the output to our motors
 * it needs to be calibrated, eventually the robot will isolate between
 * the motoroutput requested, and a motoroutput one arbitrary increment below*/

float DefaultRobot::slipcheck(float &currentmotoroutput, float joystickyvalue )
{	
	//printf "button test %d",ds->GetDigitalIn(1);
	if (currentmotoroutput < joystickyvalue)
	{
		currentmotoroutput += SLIPCONSTANT;
	}
	else if (currentmotoroutput > joystickyvalue) // I have changed this line
	{
		currentmotoroutput -= SLIPCONSTANT;
	}
	
	return currentmotoroutput;
	
}

void DefaultRobot::pickupball()
{
	if (myopticalballchecker->Get() == false)
	//unsure if "get" works for digital sensor but docs are vague
	{
		myconveyorbelt->Set(DEFAULTBELTSPEED);     //life ball up one slot
		mytimer->Start();
	}
	else 
	{
		if (ds->GetDigitalIn(1) == true && ds->GetDigitalIn(2)== true && ds->GetDigitalIn(3)== true)
		{
			myconveyorbelt->Set(0);
		}
	}
}

void DefaultRobot::pickupballtimercheck(float time)
{
	if (mytimer->Get() >= time) //i have decreased this value by .1 -bg
	{
		/*The beltstatus variable is to keep track of how the belt is set to be driven
		* 1 = up -1 =down 0 = neutral*/
		myconveyorbelt->Set(beltstatus * DEFAULTBELTSPEED); //set the conveyer belt back to its intended state
		mytimer->Stop();
		mytimer->Reset();
	}
}

START_ROBOT_CLASS(DefaultRobot);
