#include "GPS.h"
#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;


int GPSHeartBeat(ProcessManagement* PMData, int FailCheck) {
	if (PMData->Heartbeat.Flags.GPS == 0 && FailCheck <= 3) {
		printf("%d", PMData->Heartbeat.Flags.GPS);
		PMData->Heartbeat.Flags.GPS = 1;
		printf("%d", PMData->Heartbeat.Flags.GPS);
		return 0;
	}
	else {
		Thread::Sleep(250);
		return 1;
	}
}

int main() {

	//Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	while (1) {
		//QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		//TimeStamp = (double)Counter / (double)Frequency * 1000; //ms
		//Console::WriteLine("GPS time stamps: {0,12:F3} {1, 12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);
		//Did PM put my flag down?
			//true-> put flag up
			//false-> is the pm time stamp older by agreed time gap
				//True->shutdown all
				//
		//PMData->Heartbeat.Flags.GPS = 0;
		int FailCheck{ 0 };
		int failure{ 0 };
		while (FailCheck <= 3) {
			failure += GPSHeartBeat(PMData, FailCheck);
			if (failure > 3) {
				printf("PM dead critical failure, shut down");
				PMData->Shutdown.Status = 0xFF;
			}
			FailCheck++;
		}
		if (PMData->Shutdown.Flags.GPS == 1)
			break;
		if (_kbhit())
			break;
	}
	return 0;
}