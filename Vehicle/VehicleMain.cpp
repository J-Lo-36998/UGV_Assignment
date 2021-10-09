#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int failure{ 0 };

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData;

int VehicleHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.VehicleControl == 0) {
		printf("%d", PMData->Heartbeat.Flags.VehicleControl);
		PMData->Heartbeat.Flags.VehicleControl = 1;
		printf("%d", PMData->Heartbeat.Flags.VehicleControl);
		return 0;
	}
	else {
		return 1;
	}
}

int main() {

	//Declaration
	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;
	while (1) {
		//QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		//TimeStamp = (double)Counter / (double)Frequency * 1000; //ms
		//Console::WriteLine("GPS time stamps: {0,12:F3} {1, 12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);
		if (VehicleHeartBeat(PMData) == 0) {
			failure = 0;
		}
		else {
			failure++;
		}
		if (failure > 100) {
			//Console::ReadKey();
			printf("\n%d cock", failure);
			PMData->Shutdown.Status = 0xFF;
		}
		if (PMData->Shutdown.Flags.VehicleControl == 1)
			break;
		if (_kbhit())
			break;
	}
	return 0;
}