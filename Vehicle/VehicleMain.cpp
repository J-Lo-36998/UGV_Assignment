#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
int counter{ 0 };
int pmFail{ 0 };
int VehicleHeartBeat(ProcessManagement* PMData);

int main() {
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));

	double PrevTime, NextTime;
	__int64 Frequency{}, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	while (1) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		PrevTime = (double)Counter / (double)Frequency * 1000;
		double TimeGap = 0;
		//printf("Hiii");
		while (TimeGap <= 4000 && PMData->Shutdown.Status != 0xFF) {
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			NextTime = (double)Counter / (double)Frequency * 1000;
			//Console::WriteLine("Time Gap is Currently : {0,12:F3}", NextTime - PrevTime);
			TimeGap = NextTime - PrevTime;
			//PMData->Heartbeat.Flags.Laser = 0;
			if (VehicleHeartBeat(PMData) == 0) {
				pmFail = 0;
				break;
			}
			else if (TimeGap > 1000 + pmFail * 1000) {
				pmFail++;
			}
			if (pmFail > 3) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
		}
		if (PMData->Shutdown.Status == 0xFF) {
			break;
		}
	}
	PMData->Shutdown.Status = 0xFF;
	return 0;
}

int VehicleHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.VehicleControl == 0) {
		pmFail = 0;
		printf("%d", PMData->Heartbeat.Flags.VehicleControl);
		PMData->Heartbeat.Flags.VehicleControl = 1;
		printf("%d", PMData->Heartbeat.Flags.VehicleControl);
		return 0;
	}
	else {
		return 1;
	}
}