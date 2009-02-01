#include "WPILib.h"

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class get food for hot plate and bring back to me
 * is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */

int x;
class DefaultRobot : public SimpleRobot
{
	RobotDrive *myRobot;			// robot drive system
	DigitalInput *armUpperLimit;	// arm upper limit switch
	DigitalInput *armLowerLimit;	// arm lower limit switch
	Joystick *rightStick;			// joystick 1 (arcade stick or right tank stick)
	Joystick *leftStick;			// joystick 2 (tank left stick)
	Joystick *armStick;				// joystick 3 to control arm
	DriverStation *ds;				// driver station object
	Victor *myrightvictor;
	Victor *myleftvictor;
	Victor *mytopspinner;
	Victor *myconveyerbelt;
	Solenoid *myhatch;
	Relay *mybottomspinner;
	Relay *mycompressor;
	DigitalInput *mycheckball;
	

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
		bottomspinner =4,
		compressor =6,
		openhatch =7
		
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
		mycheckball = new DigitalInput(x);
		myrightvictor = new Victor(rightdrive);
		mytopspinner  = new Victor(topspinner);
		myleftvictor = new Victor(leftdrive);
		myconveyerbelt  = new Victor(conveyerbelt);
		mycompressor = new Relay(compressor);
		mybottomspinner = new Relay(bottomspinner);
		myhatch = new Solenoid(openhatch);

		//Update the motors at least every 100ms.
		GetWatchdog().SetExpiration(100);
		
	}

	/**
	 * Drive left & right motors for 2 seconds, enabled by a jumper (jumper
	 * must be in for autonomous to operate).
	 */
	void Autonomous(void)
	{
		GetWatchdog().SetEnabled(false);
		if (ds->GetDigitalIn(ENABLE_AUTONOMOUS) == 1)	// only run the autonomous program if jumper is in place
		{
			myRobot->Drive(0.5, 0.0);			// drive forwards half speed
			Wait(2000);							//    for 2 seconds
			myRobot->Drive(0.0, 0.0);			// stop robot
		}
		GetWatchdog().SetEnabled(true);
	}

	/**
	 * Runs the motors under driver control with either tank or arcade steering selected
	 * by a jumper in DS Digin 0. Also an arm will operate based on a joystick Y-axis. 
	 */
	void OperatorControl(void)
	{
		bool gathervariable = 0;
		int beltstatus = 0; 
		/*The beltstatus variable is to keep track of how the belt is set to be driven
		 * 1 = up -1 =down 0 = neutral*/
		while (IsOperatorControl())
		{
			GetWatchdog().Feed();

			// determine if tank or arcade mode; default with no jumper is for tank drive
			if (ds->GetDigitalIn(ARCADE_MODE) == 0) 
			{	
				myRobot->TankDrive(leftStick, rightStick);	 // drive with tank style
			} 
			else
			{
				myRobot->ArcadeDrive(rightStick);	         // drive with arcade style (use right stick)
			}
			// beginning of the 2898 secondary system code
			if (ds->GetDigitalIn(gatherswitch) == true)       //check to turn on gather
			{
				gathervariable = 1;
				//todo relay output here
			}
			else
			{
				gathervariable =0;
				//todo relay stop
			}
			if (ds->GetDigitalIn(upswitch) == true)
			{
				myconveyerbelt->Set(.5);  // move belt up
				beltstatus = 1;
			}
			else {
				myconveyerbelt->Set(0);  //stop belt
				beltstatus = 0;
			}
			if (ds->GetDigitalIn(downswitch) == true && ds->GetDigitalIn(upswitch)==false)
			{ //if down is push and up is not pushed, proceed
				myconveyerbelt->Set(-.5); //set belt down
				beltstatus = -1;
			}
			else
			{
				myconveyerbelt->Set(0); //stop belt
			beltstatus = 0;
			}
			if (gathervariable == true)
			{
				if (mycheckball->Get() == true) //unsure if "get" works
				{
					myconveyerbelt->Set(.5);
					Wait(1);
					myconveyerbelt->Set(beltstatus * .5);
				}
			}
			if (ds->GetDigitalIn(hatchswitch) == true)
				{
				//Todo: learn how to stop relay
				}
			else
			 	{
				//Todo: learn how to stop relay
				
			 	}
		}
	}
};

