#include "GPS.h"
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

int GPSHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.GPS == 0) {
		printf("%d", PMData->Heartbeat.Flags.GPS);
		PMData->Heartbeat.Flags.GPS = 1;
		printf("%d", PMData->Heartbeat.Flags.GPS);
		return 0;
	}
	else {
		return 1;
	}
}

int main() {

	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;
	while (1) {

		Thread::Sleep(25);
		//Did PM put my flag down?
			//true-> put flag up
			//false-> is the pm time stamp older by agreed time gap
				//True->shutdown all
				//
		//PMData->Heartbeat.Flags.GPS = 0;
		if (GPSHeartBeat(PMData) == 0) {
			failure = 0;
		}
		else {
			failure++;
		}
		printf("\n%d cock", failure);
		if (failure > 100) {
			//Console::ReadKey();
			PMData->Shutdown.Status = 0xFF;
		}
		if (PMData->Shutdown.Flags.GPS == 1)
			break;
		if (_kbhit())
			break;
	}
	return 0;
}
