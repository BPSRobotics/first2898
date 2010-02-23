#include "Relay.h"
#include "WPILib.h"
#include <iostream>

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
	Victor FrontLeft;
	Victor BackLeft;
	Victor FrontRight;
	Victor BackRight;
	DriverStationLCD *dsLCD;
	Timer kickTimer;
	
	
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
		wench(3),
		FrontLeft(5),
		BackLeft(4),
		FrontRight(7),
		BackRight(8),
		kickTimer()
		
	{
		dsLCD = DriverStationLCD::GetInstance();
		ds = DriverStation::GetInstance();
		eds = &(ds->GetEnhancedIO());
		GetWatchdog().SetExpiration(0.1);
		
		myRobot.SetInvertedMotor(RobotDrive::kFrontLeftMotor,true);
		myRobot.SetInvertedMotor(RobotDrive::kFrontRightMotor,true);
		myRobot.SetInvertedMotor(RobotDrive::kRearRightMotor,true);
		myRobot.SetInvertedMotor(RobotDrive::kRearLeftMotor,true);
		
	}

	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{
		GetWatchdog().SetEnabled(false);
		
		myRobot.Drive(0.5, 0.0); 	// drive forwards half speed
		Wait(1.0); 				//    for 1 seconds
		myRobot.Drive(0.0, 0.0); 	// stop robot
		switch (ds->GetLocation())  //returns int value 1,2 or 3 for starting postion
		{
			case 1:
			myRobot.Drive(0.5, 0.0);
			Wait(1.0);
			myRobot.Drive(0.0, 0.0); 
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Possition 1\n");
			
			break;
			case 2:
			Wait(4.0);
			myRobot.Drive(0.5, 0.0);
			Wait(1.0);
			myRobot.Drive(0.0, 0.0); 
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Possition 2\n");
			break;
			case 3:
			myRobot.Drive(0.5, 0.0);
			Wait(8.0);
			myRobot.Drive(0.0, 0.0); 
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Possition 3\n");
			break;			
		}
	}
	
	/**
	 * Runs the motors with holonomic steering
	 */
	void OperatorControl(void)
	{
		AxisCamera &camera = AxisCamera::GetInstance();
		camera.WriteResolution(AxisCamera::kResolution_320x240);
		camera.WriteCompression(20);
		camera.WriteBrightness(0);
		GetWatchdog().SetEnabled(true);
		//dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Hello World");
		//dsLCD->Printf(DriverStationLCD::kUser_Line1, 11, "Time: %4.1f", GetClock());
		//dsLCD->UpdateLCD();
				 
		while (IsOperatorControl())
		{
			GetWatchdog().Feed();
			if(stick.GetRawButton(4))
			{
				FrontLeft.Set(-.75);
				BackLeft.Set(.75);
				FrontRight.Set(-.75);
				BackRight.Set(.75);
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Left Button\n");
			}
			else if(stick.GetRawButton(5))
			{
				FrontLeft.Set(.75);
				BackLeft.Set(-.75);
				FrontRight.Set(.75);
				BackRight.Set(-.75);
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Right Button\n");
			}
			else if(stick.GetRawButton(3))
			{
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Calm down Devante\n");
			}
			else
				myRobot.ArcadeDrive(stick);
			
			
			//myRobot.HolonomicDrive(-stick.GetX(),0.0 ,stick.GetY()); 	
			//myRobot.MecanumDrive_Polar(stick.GetMagnitude(),stick.GetDirectionRadians(),0);
			printf("Ultrasonic: %f ",usonic.GetVoltage());// drive with arcade style (use right stick)
			printf("Gyro: %f \n",gyro.GetVoltage());
			if(stick.GetTrigger() && (kickTimer.Get() == 0)) //If timer is at zero
			{
				kickTimer.Start();
				spike.Set(Relay::kForward); //why deoesn't work 
				wench.Set(1.0);
			}
			if(kickTimer.Get() >0.25)
			{
				spike.Set(Relay::kReverse); //why deoesn't work 
				wench.Set(0.0);
			}
			if(KickerBackLimit.Get() >0.5)
			{
				spike.Set(Relay::kOff); //why deoesn't work 
				kickTimer.Stop();
				kickTimer.Reset();
			}
			
			/*if(KickerLimit.Get())
			{
				spike.Set(Relay::kReverse); //why deoesn't work 
				wench.Set(0.0);
			}
			if(KickerBackLimit.Get())
			{
				spike.Set(Relay::kOff); //why deoesn't work 	
			}
			*/
			dsLCD->UpdateLCD();
			Wait(0.005);				// wait for a motor update time
			//spike.Set(Relay::kReverse); //why deoesn't work 		
			
		}
	}
};

START_ROBOT_CLASS(RobotDemo);

