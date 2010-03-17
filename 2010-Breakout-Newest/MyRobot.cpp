#include "Relay.h"
#include "WPILib.h"
#include <iostream>

#define ENGAGE 1.0
#define RELEASE -1.0
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
	DriverStation *ds;
	DriverStationEnhancedIO *eds;
	Victor ClutchVictor;
	DigitalInput ClutchBack;
	DigitalInput EngageClutch;
	Victor wench;
	Victor FrontLeft;
	Victor BackLeft;
	Victor FrontRight;
	Victor BackRight;
	DriverStationLCD *dsLCD;
	Timer kickTimer;
	DigitalInput IsPulledBack;
	
	
public:
	RobotDemo(void):
		myRobot(5,4,7,8),	// these must be initialized in the same order
		//fl bl fr br
		stick(1),		// as they are declared above.
		gyro(2,1),
		ClutchVictor(2),
		ClutchBack(4,2),
		EngageClutch(4,1),
		wench(3),
		FrontLeft(5),
		BackLeft(4),
		FrontRight(7),
		BackRight(8),
		kickTimer(),
		IsPulledBack(4,3)
		
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

	void Autonomous(void)
	{ 
		//Drive Forward and back up
		GetWatchdog().SetEnabled(false);
		myRobot.Drive(-1.0,0); //Drive up to ball
		Wait(2);
		myRobot.Drive(0.0,0.0);//Stop
		Wait(2);
		myRobot.Drive(1.0,0.0);//Backup
		Wait(.75);
		myRobot.Drive(0.0,0.0);		
		
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
			int state = 0;
			bool shootermode = false;
			GetWatchdog().Feed();
			if(stick.GetRawButton(5))
			{
				// DriveLeft();
				FrontLeft.Set(-.75);
				BackLeft.Set(.75);
				FrontRight.Set(-.75);
				BackRight.Set(.75);
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Left Button\n");
			}
			else if(stick.GetRawButton(6))
			{
				// DriveRight();
				FrontLeft.Set(.75);
				BackLeft.Set(-.75);
				FrontRight.Set(.75);
				BackRight.Set(-.75);
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Right Button\n");
			}
			else
			myRobot.ArcadeDrive(stick);
			//myRobot.HolonomicDrive(stick.GetMagnitude(),stick.GetDirectionRadians(), 0.0); 	
			//myRobot.MecanumDrive_Polar(stick.GetMagnitude(),stick.GetDirectionRadians(),0);
			//printf("Ultrasonic: %f ",usonic.GetVoltage());// drive with arcade style (use right stick)
			//printf("Gyro: %f \n",gyro.GetVoltage());
			if (stick.GetTrigger())
			{
			//	shootermode =  true;
			}
			
			if(shootermode ==true)
			{
				switch(state)
				{	
				case 0:
					ClutchVictor.Set(ENGAGE);
					wench.Set(0.5);
					state++;
					break;
				case 1:
					if(!EngageClutch.Get())
					{
						ClutchVictor.Set(0.0);
						wench.Set(0.5);
						if (IsPulledBack.Get())
						{
							wench.Set(0.0);
							ClutchVictor.Set(RELEASE);
							state++;
						}
					}
					break;
				case 2:
					if (ClutchBack.Get()) //Need work
					{
						ClutchVictor.Set(0.0);
						state = 0;
						shootermode = false;
					}
					break;
				}
			}	
			Wait(0.005);				// wait for a motor update time
		}
	}
	void DriveLeft()
	{
		FrontLeft.Set(-.75);
		BackLeft.Set(.75);
		FrontRight.Set(-.75);
		BackRight.Set(.75);
	}
	void DriveRight()
	{
		FrontLeft.Set(.75);
		BackLeft.Set(-.75);
		FrontRight.Set(.75);
		BackRight.Set(-.75);
	}
};

START_ROBOT_CLASS(RobotDemo);

