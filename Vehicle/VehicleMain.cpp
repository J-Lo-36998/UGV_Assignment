#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int VehicleHeartBeat(ProcessManagement* PMData, int FailCheck) {
	if (PMData->Heartbeat.Flags.VehicleControl == 0 && FailCheck <= 3) {
		printf("%d", PMData->Heartbeat.Flags.VehicleControl);
		PMData->Heartbeat.Flags.VehicleControl = 1;
		printf("%d", PMData->Heartbeat.Flags.VehicleControl);
		return 0;
	}
	else {
		Thread::Sleep(500);
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
		//Thread::Sleep(25);
		int FailCheck{ 0 };
		int failure{ 0 };
		while (FailCheck <= 3) {
			failure += VehicleHeartBeat(PMData, FailCheck);
			if (failure > 3) {
				PMData->Shutdown.Status = 0xFF;
			}
			FailCheck++;
		}
		if (PMData->Shutdown.Status)
			break;
		if (_kbhit())
			break;
	}
	return 0;
}