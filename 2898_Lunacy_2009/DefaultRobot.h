/****************************************************************
 * 
 * This file tucks away the nasty declaring that used to be ok
 * when it was "virgin" code but, the camera code just and whent
 * and did all this dirty style of coding.
 * yes, I know I'm a perfectionist. then again most enginners are.
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
 *****************************************************************/


//constants
#define DEFAULTBELTSPEED 				-1.0  //I Changed this to a constant because using global variables is usally frowned apon
#define SLIPCONSTANT .0004                    //looks like you set this back to original state steven? -brian

/****************************************************************\
 * Includes
\****************************************************************/
#include "Vision.h"
#include "WPILib.h"
#include <math.h>

class DefaultRobot : public SimpleRobot
{
	float currentrightmotoroutput; //one time declaration for slipcheck function
	float currentleftmotoroutput; // one time declaration for slipcheck function
	int beltstatus;
	//there stuff
	RobotDrive *myRobot;			// robot drive system
	Joystick *rightStick;			// joystick 1 (arcade stick or right tank stick)
	Joystick *leftStick;			// joystick 2 (tank left stick)
	DriverStation *ds;				// driver station object
	
	//our stuff
	Victor *myrightvictor;
	Victor *myleftvictor;
	Victor *mytopspinner;
	Victor *myconveyorbelt;
	
	Relay *mybottomspinner;
	DigitalInput *myopticalballchecker;
	Timer *mytimer;
	Timer *myautotimer;
	Accelerometer *myAccelerometer;
	
	Encoder *encoderRight; //added by Steven
	Encoder *encoderLeft;  // we will need it later
	
	Servo *servoPan;
	Servo *servoTilt;
	
	TrackingThreshold td1, td2;		// color thresholds for camera
	Vision *camera; // a new class OMG!

	static const int leftsign = 1;
	static const int rightsign = 1;
	
	enum							// Driver Station jumpers to control program operation
	{ ARCADE_MODE = 1,				// Tank/Arcade jumper is on DS Input 1 (Jumper present is arcade)
	  ENABLE_AUTONOMOUS = 2,		// Autonomous/Teleop jumper is on DS Input 2 (Jumper present is autonomous)
	} jumpers;	                          
	enum
	{                                //all fake values
		rightdrive = 4,
		leftdrive = 3,
		conveyorbelt = 2,
		topspinner = 1,

		
	}pwms;
	enum                              //all fake values
	{
		gatherswitch = 1,
		upswitch = 2,
		downswitch = 4,
		hatchswitch = 6
		
	}controllerinputs;

	public:
	//Constructors
	DefaultRobot(void);
	//member functions
	float slipcheck(float &currentmotoroutput, float joystickyvalue);
	void pickupball();
	void pickupballtimercheck(float time);
	//Game Modes!
	void Autonomous(void);
	void OperatorControl(void);
};

/****************************************************************\
 * Prototypes
\****************************************************************/
