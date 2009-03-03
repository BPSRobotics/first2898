/****************************************************************
 * 
 * This file takes care of the tracking portion of the NI vision
 * stuff there is functions in there to get the current pan and
 * tilt of camera and X and Y of the target to get feedback to
 * the code operating the bot
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
#ifndef __VISION__H__
#define __VISION__H__
#include <iostream.h>
#include "math.h"

//	robotic stuff
#include "AxisCamera.h"
#include "BaeUtilities.h" 
#include "FrcError.h"
#include "TrackAPI.h"
#include "Target.h"

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

/****************************************************************\
 * Macros
\****************************************************************/
#define DPRINTF if(TwoColorDemo_debugFlag)dprintf

/****************************************************************\
 * Classes
\****************************************************************/
class Vision 
{
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
	
	public:
		Vision(Servo *HServo, Servo *VServo, float Deadband = 0.01, int FPS = 15);
		int tooClose(ParticleAnalysisReport* par);
		int bigEnough(ParticleAnalysisReport* par);
		void setServoPositions(float normalizedHorizontal, float normalizedVertical);
		void adjustServoPositions(float normDeltaHorizontal, float normDeltaVertical);
		//todo: add single target tracking after season
		void LookFor(TrackingThreshold *tt1, TrackingThreshold *tt2);
		bool DoTracking(void);
};

#endif