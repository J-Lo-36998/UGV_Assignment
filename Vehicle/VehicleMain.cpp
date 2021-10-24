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
	VehicleControl myUGV;
	myUGV.setupSharedMemory();
	myUGV.connect("192.168.1.200", 25000);
	while (pmFail<1000) {
		while (myUGV.getShutdownFlag() != 1) {
			Thread::Sleep(10);
			if (myUGV.getHBFlag() == 0) {
				//Reset value of pmFail if PM still Alive
				myUGV.setHeartbeat(hb);
				pmFail = 0;
				break;
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			//PM Shutdown (Since PM is critical, shutdown all)
			else {
				pmFail++;
			}
			myUGV.controls();
		}
		//printf("%d\n", PMData->Heartbeat.Flags.VehicleControl);
		Thread::Sleep(10);
		//on shutdown signal exit and close window
		if (myUGV.getShutdownFlag() == 1) {
			break;
		}
	}
	if (pmFail > 1000) {
		printf("Process Management Critical Failure: Shutting Down");
		Thread::Sleep(1000);
	}
	myUGV.ShutDown();
	return 0;
}

