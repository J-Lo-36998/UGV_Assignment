#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
#include "VehicleControl.h"
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

//Counter to keep track of how many times PM Fails
int pmFail{ 0 };
//For use with Time Stamps
double Prev, Next;
__int64 Frequency{}, Counter;
int Shutdown = 0x00;
//for convertion to ms
#define MILSEC 1000
//Time in ms in the loops and also when to check again
#define WAIT_TIME 1000
bool hb = FALSE;
//Declaring Shared memory
//SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
//ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

int main() {
	//Instantiating Sharedmemory (Creating and allowing access)
	/*PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;*/
	VehicleControl myVehicle;
	myVehicle.setupSharedMemory();
	while (myVehicle.getShutdownFlag()!=1) {
		//Instantiating the prev time stamp/reset
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		Prev = (double)Counter / (double)Frequency * MILSEC;
		double TimeGap = 0;
		//printf("%d\n", PMData->Heartbeat.Flags.VehicleControl);
		while (TimeGap <= 4000 && myVehicle.getShutdownFlag() != 1) {
			//Instantiating next time stamp/reset once gets past 4000ms
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			Next = (double)Counter / (double)Frequency * MILSEC;
			TimeGap = Next - Prev;//To obtain time gap between times
			if (myVehicle.getHBFlag() == 0) {
				//Reset value of pmFail if PM still Alive
				myVehicle.setHeartbeat(hb);
				pmFail = 0;
				break;
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			//PM Shutdown (Since PM is critical, shutdown all)
			else if (pmFail > 1000) {
				printf("Process Management Critical Failure: Shutting Down");
				Thread::Sleep(1000);
				myVehicle.ShutDown();
			}

		}
		//printf("%d\n", PMData->Heartbeat.Flags.VehicleControl);
		Thread::Sleep(10);
		//on shutdown signal exit and close window
		if (myVehicle.getShutdownFlag() == 1) {
			break;
		}
	}
	myVehicle.ShutDown();
	return 0;
}

