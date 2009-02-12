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
 * notes:
 * needs single target tracking.              								 
 *****************************************************************/

/****************************************************************\
 * Includes
\****************************************************************/

#include "Vision.h"

/****************************************************************\
 * Functions
\****************************************************************/
Vision::Vision(Servo *HServo, Servo *VServo, float Deadband, int FPS)
{
	horizontalServo = HServo; 		// create horizontal servo on PWM 9
	verticalServo = VServo;			// create vertical servo on PWM 10
	servoDeadband = Deadband;					// move if > this amount 
	framesPerSecond = FPS;					// number of camera frames to get per second
	sinStart = 0.0;							// control where to start the sine wave for pan
	memset(&par1,0,sizeof(ParticleAnalysisReport));			// initialize particle analysis report
	memset(&par2,0,sizeof(ParticleAnalysisReport));			// initialize particle analysis report
	
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
	
	// initialize position and destination variables
	// position settings range from -1 to 1
	// setServoPositions is a wrapper that handles the conversion to range for servo 
	horizontalDestination = 0.0;		// final destination range -1.0 to +1.0
	verticalDestination = 0.0;
	
	// current position range -1.0 to +1.0
	horizontalPosition = RangeToNormalized(horizontalServo->Get(),1);	
	verticalPosition = RangeToNormalized(verticalServo->Get(),1);			
	
	// set servos to start at center position
	setServoPositions(horizontalDestination, verticalDestination);
	
}

/** Simple test to see if the color is taking up too much of the image */
int Vision::tooClose(ParticleAnalysisReport* par)
{
	if (par->particleToImagePercent > MAX_PARTICLE_TO_IMAGE_PERCENT)
	{
		return 1;
	}
	return 0;
}

/** Simple test to see if the color is large enough */
int Vision::bigEnough(ParticleAnalysisReport* par)
{
	if (par->particleToImagePercent < MIN_PARTICLE_TO_IMAGE_PERCENT)
	{
		return 0;
	}
	return 1;
}

/**
 * Set servo positions (0.0 to 1.0) translated from normalized values (-1.0 to 1.0). 
 * 
 * @param normalizedHorizontal Pan Position from -1.0 to 1.0.
 * @param normalizedVertical Tilt Position from -1.0 to 1.0.
 */
void Vision::setServoPositions(float normalizedHorizontal, float normalizedVertical)
{

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
void Vision::adjustServoPositions(float normDeltaHorizontal, float normDeltaVertical)
{
					
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
//todo: add single target tracking after season
void Vision::LookFor(TrackingThreshold *tt1, TrackingThreshold *tt2)
{
	DPRINTF(LOG_DEBUG, "SERVO - looking for COLOR %s ABOVE %s", td2.name, td1.name);
}

//todo: fill a struct with the info on the target position and add start stop tracking
bool Vision::DoTracking(void)
{
	
	// initialize pan variables
	// incremental tasking toward dest (-1.0 to 1.0)
	static float incrementH, incrementV;
	// pan needs a 1-up number for each call
	static int panIncrement = 0;							
	
	// for controlling loop execution time 
	static float loopTime = 0.1;		
	//float loopTime = 0.05;											
	static double currentTime = GetTime();
	static double lastTime = currentTime;
									
	// search variables 
	static bool foundColor = 0; 
	static double savedImageTimestamp = 0.0;
	static bool staleImage = false; 
	
	if ( FindTwoColors(td1, td2, ABOVE, &par1, &par2) )
	{
		//PrintReport(&par2);
		foundColor = true;
		
		// reset pan		
		panIncrement = 0;  		
		if (par1.imageTimestamp == savedImageTimestamp) 
		{
			// This image has been processed already, 
			// so don't do anything for this loop 
			staleImage = true;
			DPRINTF(LOG_DEBUG, "STALE IMAGE");
		} 
		else
		{
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
	}
	else
	{  // need to pan 
		foundColor = false;
	} 					
		if(foundColor && !staleImage) 
		{	
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
			
		} 
		else
		{ //if (!staleImage) {  // new image, but didn't find two colors
			// adjust sine wave for panning based on last movement direction
			if(horizontalDestination > 0.0)	
			{
				sinStart = PI/2.0;
			}
			else
			{ 
				sinStart = -PI/2.0;
			}

			/* pan to find color after a short wait to settle servos
			 * panning must start directly after panInit or timing will be off */				
			if (panIncrement == 3) 
			{
				panInit(8.0);		// number of seconds for a pan
			}
			else if (panIncrement > 3) 
			{					
				panForTarget(horizontalServo, sinStart);	
				
				/* Vertical action: In case the vertical servo is pointed off center,
				 * center the vertical after several loops searching */
				if (panIncrement == 20) 
				{ 
					verticalServo->Set( 0.5 );
				}
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
			//todo: potential bug!
		}	
		return foundColor;
}
