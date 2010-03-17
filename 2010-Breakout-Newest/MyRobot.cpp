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
		usonic(2,2),
		spike(4,3,Relay::kBothDirections),
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

	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{ 
		GetWatchdog().SetEnabled(false);
		switch (1)  //returns int value 1,2 or 3 for starting postion HARD CODED
		{ 
		case 1:
			myRobot.Drive(-1.0,0); //Drive up to ball
			Wait(2);
			myRobot.Drive(0.0,0.0);//Stop
			Wait(4);
			myRobot.Drive(1.0,0.0);//Backup
			Wait(.75);
			break;
		case 2:
			DriveLeft();
			/*FrontLeft.Set(-.75);
			BackLeft.Set(.75);
			FrontRight.Set(-.75);
			BackRight.Set(.75);
			*/
			Wait(.25);
			break;
		case 3:
			DriveLeft();
			/*FrontLeft.Set(-.75);
			BackLeft.Set(.75);
			FrontRight.Set(-.75);
			BackRight.Set(.75);
			*/
			Wait(.25);
			break;
		}
			
		myRobot.Drive(0.0,0.0);		
		
	}
	
	/**
	 * Runs the motors with holonomic steering
	 */
	void OperatorControl(void)
	{
		/*AxisCamera &camera = AxisCamera::GetInstance();
		camera.WriteResolution(AxisCamera::kResolution_320x240);
		camera.WriteCompression(20);
		camera.WriteBrightness(0);*/
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
					spike.Set(Relay::kForward);
					wench.Set(0.5);
					state++;
					break;
				case 1:
					if(!EngageClutch.Get())
					{
						spike.Set(Relay::kOff);
						wench.Set(0.5);
						if (IsPulledBack.Get())
						{
							wench.Set(0.0);
							spike.Set(Relay::kReverse);
							state++;
						}
					}
					break;
				case 2:
					if (ClutchBack.Get()) //Need work
					{
						spike.Set(Relay::kOff);
						state = 0;
						shootermode = false;
					}
					break;
				}
			}	
				
			
			
			
			
			
		/*if(stick.GetTrigger() || ds->GetDigitalIn(2)) //If timer is at zero
			{
				printf("we kick now\n");
				spike.Set(Relay::kReverse);  
				wench.Set(0.5);
				kickTimer.Start();
			}	
			if (WenchLimit.Get() || (kickTimer.Get() > 1))
			{
				printf("Wench Limit\n");
				spike.Set(Relay::kForward);  
				wench.Set(0.0);
			}
			if(!KickerLimit.Get())
			{
				printf("Kicker Limit\n");
				spike.Set(Relay::kOff);  
			}
			if(!KickerBackLimit.Get() || (kickTimer.Get() > 1))
			{
				printf("Kicker Limit 2\n");
				spike.Set(Relay::kOff); 
				kickTimer.Stop();
				kickTimer.Reset();
			}
			*/
			//spike.Set(Relay::kForward);
			//dsLCD->UpdateLCD();
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
	/*void Mecanum(float x, float y, float z)
	{
		
	}*/
	
};

START_ROBOT_CLASS(RobotDemo);

