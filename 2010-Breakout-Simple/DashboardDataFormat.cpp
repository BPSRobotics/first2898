#include "DashboardDataFormat.h"

void sendIOPortData() {
	Dashboard &dash = DriverStation::GetInstance()->GetLowPriorityDashboardPacker();
	dash.AddCluster();
	{
		dash.AddCluster();
		{ //analog modules 
			dash.AddCluster();
			{
				for (int i = 1; i <= 8; i++) {
					//					dash.AddFloat((float) AnalogModule::GetInstance(1)->GetAverageVoltage(i));
					dash.AddFloat((float) i * 5.0 / 8.0);
				}
			}		
			dash.FinalizeCluster();
			dash.AddCluster();
			{
				for (int i = 1; i <= 8; i++) {
					dash.AddFloat((float) AnalogModule::GetInstance(1)->GetAverageVoltage(i));
					
				}
			}
			dash.FinalizeCluster();
		}
		dash.FinalizeCluster();

		dash.AddCluster();
		{ //digital modules
			dash.AddCluster();
			{
				dash.AddCluster();
				{
					int module = 4;
					dash.AddU8(DigitalModule::GetInstance(module)->GetRelayForward());
					dash.AddU8(DigitalModule::GetInstance(module)->GetRelayReverse());
					//					dash.AddU16((short)DigitalModule::GetInstance(module)->GetDIO());
					dash.AddU16((short) 0xAAAA);
					//					dash.AddU16((short)DigitalModule::GetInstance(module)->GetDIODirection());
					dash.AddU16((short) 0x7777);
					dash.AddCluster();
					{
						for (int i = 1; i <= 10; i++) {
							//							dash.AddU8((unsigned char) DigitalModule::GetInstance(module)->GetPWM(i));
							dash.AddU8((unsigned char) (i-1) * 255 / 9);
						}
					}
					dash.FinalizeCluster();
				}
				dash.FinalizeCluster();
			}
			dash.FinalizeCluster();

			dash.AddCluster();
			{
				dash.AddCluster();
				{
					int module = 6;
					dash.AddU8(DigitalModule::GetInstance(module)->GetRelayForward());
					dash.AddU8(DigitalModule::GetInstance(module)->GetRelayForward());
					dash.AddU16((short)DigitalModule::GetInstance(module)->GetDIO());
					dash.AddU16(DigitalModule::GetInstance(module)->GetDIODirection());
					dash.AddCluster();
					{
						for (int i = 1; i <= 10; i++) {
							//							dash.AddU8((unsigned char) DigitalModule::GetInstance(module)->GetPWM(i));
							dash.AddU8((unsigned char) i * 255 / 10);
						}
					}
					dash.FinalizeCluster();
				}
				dash.FinalizeCluster();
			}
			dash.FinalizeCluster();
		}
		dash.FinalizeCluster();

		// Can't read solenoids without an instance of the object
		dash.AddU8((char) 0);
	}
	dash.FinalizeCluster();
	dash.Finalize();
}
