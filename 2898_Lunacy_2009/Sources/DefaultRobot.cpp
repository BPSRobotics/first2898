#include "WPILib.h"

/**
 * Based on original default robot template
 * Secondary system code is almost complete. No autonomous 
 * Victor declarations are removed.... but the enums still exist...
  .--~*teu.       u+=~~~+u.                      u+=~~~+u.   
 dF     988Nx   z8F      `8N.    .xn!~%x.      z8F      `8N. 
d888b   `8888> d88L       98E   x888   888.   d88L       98E 
?8888>  98888F 98888bu.. .@*   X8888   8888:  98888bu.. .@*  
 "**"  x88888~ "88888888NNu.   88888   X8888  "88888888NNu.  
      d8888*`   "*8888888888i  88888   88888>  "*8888888888i 
    z8**"`   :  .zf""*8888888L `8888  :88888X  .zf""*8888888L
  :?.....  ..F d8F      ^%888E   `"**~ 88888> d8F      ^%888E
 <""888888888~ 88>        `88~  .xx.   88888  88>        `88~
 8:  "888888*  '%N.       d*"  '8888>  8888~  '%N.       d*" 
 ""    "**"`      ^"====="`     888"  :88%       ^"====="`   
                                 ^"===""                 								 
 */

int x;  //variable only here so the code compiles
float slipcheck();
float defaultbeltspeed = 5; //default convyerbelt speed

float slipcheck(){
	return 1.0;
}

class DefaultRobot : public SimpleRobot
{
	RobotDrive *myRobot;			// robot drive system
	Joystick *rightStick;			// joystick 1 (arcade stick or right tank stick)
	Joystick *leftStick;			// joystick 2 (tank left stick)
	DriverStation *ds;				// driver station object
	Victor *mytopspinner;
	Victor *myconveyerbelt;
	Solenoid *myhatch;
	Relay *mybottomspinner;
	Relay *mycompressor;
	DigitalInput *mycheckball;
	Timer *mytimer;
	Accelerometer *myAccelerometer;
	AnalogChannel *myPot;
	DigitalInput *myPotRefresh;

	enum							// Driver Station jumpers to control program operation
	{ ARCADE_MODE = 1,				// Tank/Arcade jumper is on DS Input 1 (Jumper present is arcade)
	  ENABLE_AUTONOMOUS = 2,		// Autonomous/Teleop jumper is on DS Input 2 (Jumper present is autonomous)
	} jumpers;	                          
	enum
	{                                //all fake values
		rightdrive = 1,
		leftdrive = 2,
		conveyerbelt = 3,
		topspinner = 4,

		
	}pwms;
	enum                              //all fake values
	{
		gatherswitch=1,
		upswitch=2,
		downswitch=4,
		hatchswitch =6
		
	}controllerinputs;

public:
	/**
	 * 
	 * 
	 * Constructor for this robot subclass.
	 * Create an instance of a RobotDrive with left and right motors plugged into PWM
	 * ports 1 and 2 on the first digital module.
	 */
	DefaultRobot(void)
	{
		ds = DriverStation::GetInstance();
		myRobot = new RobotDrive(rightdrive, leftdrive);
		rightStick = new Joystick(1);			// create the joysticks
		leftStick = new Joystick(2);
	// these channel are all controlled by enum but may need slot val
		mycheckball = new DigitalInput(12);
		myleftvictor = new Victor(leftdrive);
		myconveyerbelt  = new Victor(conveyerbelt);
		mybottomspinner = new Relay(1); 
		mytimer = new Timer();
		myAccelerometer = new Accelerometer(11); //need real val
		myPot = new AnalogChannel(9);
		myPotRefresh = new DigitalInput(10);
		//Update the motors at least every 100ms.
		GetWatchdog().SetExpiration(100);
		
	}

	/**
	 * Drive left & right motors for 2 seconds, enabled by a jumper (jumper
	 * must be in for autonomous to operate).
	 * Currently programmed to drive forward and then turn right to block opponent
	 */
	void Autonomous(void)
	{
		GetWatchdog().SetEnabled(false);
		if (ds->GetDigitalIn(ENABLE_AUTONOMOUS) == 1)	// only run the autonomous program if jumper is in place
		{
			myRobot->Drive(0.5, 0.0);			// drive forwards half speed
			Wait(2000);							//    for 2 seconds
			myRobot->Drive(0.0, 0.0 );			// stop robot
		}
		GetWatchdog().SetEnabled(true);
	}

	/**
	 * Runs the motors under driver control with either tank or arcade steering selected
	 * by a jumper in DS Digin 0. Also an arm will operate based on a joystick Y-axis. 
	 */
	void OperatorControl(void)
	{
		int beltstatus = 0; 
		/*The beltstatus variable is to keep track of how the belt is set to be driven
		 * 1 = up -1 =down 0 = neutral*/
		while (IsOperatorControl())
		{
			GetWatchdog().Feed();

			// determine if tank or arcade mode; default with no jumper is for tank drive
			if (ds->GetDigitalIn(ARCADE_MODE) == 0) 
			{	
				myRobot->TankDrive(leftStick->GetY() * slipcheck(), rightStick->GetY() * slipcheck());	 // drive with tank style
			} 
			/* we are using Tankdrive
			 * else 
			{
				myRobot->ArcadeDrive(rightStick);	         // drive with arcade style (use right stick)
			}*/
			// beginning of the 2898 secondary system code
			
				mybottomspinner->Set(Relay::kForward); //go forward 
			
			if (ds->GetDigitalIn(upswitch) == true)
			{
				myconveyerbelt->Set(defaultbeltspeed);  // move belt up
				beltstatus = 1;
			}
			// the stopping of the main belt is controlled by next if check
			if (ds->GetDigitalIn(downswitch) == true && ds->GetDigitalIn(upswitch)==false)
			{ //if down is push and up is not pushed, proceed
				myconveyerbelt->Set(-1 * defaultbeltspeed); //set belt down
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
					myconveyerbelt->Set(defaultbeltspeed);     //life ball up one slot
					mytimer->Start();
					
				}
			
			if (mytimer->Get() == 2)
				{
					myconveyerbelt->Set(beltstatus * defaultbeltspeed); //set the conveyer belt back to its intended state
					mytimer->Stop();
					mytimer->Reset();
				}
			if (myPotRefresh->Get() == true)
			{ // not final way to set pot
				defaultbeltspeed = myPot->GetVoltage() /5;
			}
		}
	}
};

START_ROBOT_CLASS(DefaultRobot);



