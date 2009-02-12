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

/****************************************************************\
 * Includes
\****************************************************************/
//	standard stuff
#include <iostream.h>
#include "math.h"

//	robotic stuff
#include "AxisCamera.h"
#include "BaeUtilities.h" 
#include "FrcError.h"
#include "TrackAPI.h"
#include "Target.h"
#include "WPILib.h"

/****************************************************************\
 * Global Variables
\****************************************************************/
// To locally enable debug printing: set the debugFlag to a 1, to disable set to 0
static int TwoColorDemo_debugFlag = 0; 

/****************************************************************\
 * Constants
\****************************************************************/
#define PI 3.14159265358979

// for 160x120, 50 pixels = 0.38%
#define MIN_PARTICLE_TO_IMAGE_PERCENT 	0.25 // target is too small
#define MAX_PARTICLE_TO_IMAGE_PERCENT 	20.0 // target is too close
#define DEFAULTBELTSPEED 				0.5  //I Changed this to a constant because using global variables is ussally frowned apon


/****************************************************************\
 * Macros
\****************************************************************/
#define DPRINTF if(TwoColorDemo_debugFlag)dprintf

/****************************************************************\
 * Classes
\****************************************************************/
class DefaultRobot : public SimpleRobot
{
	//there stuff
	RobotDrive *myRobot;			// robot drive system
	Joystick *rightStick;			// joystick 1 (arcade stick or right tank stick)
	Joystick *leftStick;			// joystick 2 (tank left stick)
	DriverStation *ds;				// driver station object
	
	//our stuff
	Victor *myrightvictor;
	Victor *myleftvictor;
	Victor *mytopspinner;
	Victor *myconveyerbelt;
	
	Relay *mybottomspinner;
	DigitalInput *mycheckball;
	Timer *mytimer;
	Accelerometer *myAccelerometer;
	Encoder *RightEncoder; //added by Steven
	Encoder *LeftEncoder;  //
	
	//tracking stuff
	Servo *horizontalServo;  		// first servo object
	Servo *verticalServo;			// second servo object
	float horizontalDestination;	// servo destination (0.0-1.0)
	float verticalDestination;		// servo destination (0.0-1.0)
	float horizontalPosition, verticalPosition;	// current servo positions
	float servoDeadband;			// percentage servo delta to trigger move
	int framesPerSecond;			// number of camera frames to get per second
	float panControl;				// to slow down pan
	double sinStart;				// control where to start the sine wave input for pan
	TrackingThreshold td1, td2;		// color thresholds
	
	ParticleAnalysisReport par1, par2;		// particle analysis reports
	
	static const int leftsign = 1;
	static const int rightsign = 1;
	
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
		gatherswitch = 1,
		upswitch = 2,
		downswitch = 4,
		hatchswitch = 6
		
	}controllerinputs;

public:
	//Constructors
	DefaultRobot(void);
	
	//Camera Stuff
	void setServoPositions(float normalizedHorizontal, float normalizedVertical);
	void adjustServoPositions(float normDeltaHorizontal, float normDeltaVertical);
	
	//Game Modes!
	void Autonomous(void);
	void OperatorControl(void);
};

/****************************************************************\
 * Prototypes
\****************************************************************/
//	Camera stuff
int tooClose(ParticleAnalysisReport* par);
int bigEnough(ParticleAnalysisReport* par);

//	our stuff
float slipcheck();
