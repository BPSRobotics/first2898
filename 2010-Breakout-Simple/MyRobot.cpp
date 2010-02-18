#include "Relay.h"
#include "WPILib.h"
#include <iostream>
#include "DashboardDataFormat.h"

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */ 
class RobotDemo : public SimpleRobot
{
	RobotDrive myRobot; // robot drive system
	Joystick stick; // only joystick
	AnalogChannel gyro;
	AnalogChannel usonic;
	DriverStation *ds;
	DriverStationEnhancedIO *eds;
	Relay spike;
	DigitalInput KickerLimit;
	DigitalInput KickerBackLimit;
	Victor wench;
	
public:
	RobotDemo(void):
		myRobot(5,4,7,8),	// these must be initialized in the same order
		//fl bl fr br
		stick(1),		// as they are declared above.
		gyro(2,1),
		usonic(2,2),
		spike(4,1),
		KickerLimit(4,8),
		KickerBackLimit(4,7),
		wench(3)
	{
		ds = DriverStation::GetInstance();
		eds = &(ds->GetEnhancedIO());
		GetWatchdog().SetExpiration(0.1);
	}

	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{
		GetWatchdog().SetEnabled(false);
		myRobot.Drive(0.5, 0.0); 	// drive forwards half speed
		Wait(2.0); 				//    for 2 seconds
		myRobot.Drive(0.0, 0.0); 	// stop robot
	}

	/**
	 * Runs the motors with holonomic steering
	 */
	void OperatorControl(void)
	{
		//AxisCamera &camera = AxisCamera::GetInstance();
		GetWatchdog().SetEnabled(true);
		
		/*eds->SetDigitalOutput(9,true);
		 * eds->SetLED(down, false);
	 
		int b = 3;
		for(int i= 0; i<20; i++){
		for (int i=8; i>3; i--)
		{
			eds->SetLED(i, true);
			eds->SetLED(b, true);
			b++;
			Wait(.2);
		}
		b=4;
		for (int i=5; i<9; i++)
		{
			eds->SetLED(i, false);
			eds->SetLED(b, false);
			b--;
			Wait(.2);
		}
		}
		*/
		while (IsOperatorControl())
		{
		
			GetWatchdog().Feed();
			sendIOPortData();
			float rotation;
			if(stick.GetRawButton(5))
				rotation = .1;
			else if(stick.GetRawButton(4))
				rotation = -.1;
			else
				rotation =0;
			myRobot.HolonomicDrive(stick.GetMagnitude(),stick.GetDirectionDegrees() ,rotation);
			printf("Ultrasonic: %f ",usonic.GetVoltage());// drive with arcade style (use right stick)
			printf("Gyro: %f \n",gyro.GetVoltage());
			//eds->SetDigitalOutput(10,true);
			if(stick.GetTrigger())
			{
				//eds->SetDigitalOutput(10,false);
				spike.Set(Relay::kForward); //why deoesn't work 
				wench.Set(1.0);
			}
			if(KickerLimit.Get())
			{
				spike.Set(Relay::kReverse); //why deoesn't work 
				wench.Set(0.0);
			}
			if(KickerBackLimit.Get())
			{
				spike.Set(Relay::kOff); //why deoesn't work 	
			}
			Wait(0.005);				// wait for a motor update time
			//spike.Set(Relay::kReverse); //why deoesn't work 
			
		}
	}
};

START_ROBOT_CLASS(RobotDemo);

