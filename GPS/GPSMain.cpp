#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
#include "GPS.h"
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

int main() {
	//Instantiating Sharedmemory (Creating and allowing access)
	GPS myGPS;
	myGPS.setupSharedMemory();

	myGPS.connect("192.168.1.200", 24000);

	while (pmFail<1000) {
		//printf("%d\n", PMData->Heartbeat.Flags.GPS);
		while (myGPS.getShutdownFlag() != 1) {
			//Instantiating next time stamp/reset once gets past 4000ms
			Thread::Sleep(10);
			if (myGPS.getHBFlag() == 0) {
				//printf("%d\n", myGPS.getHBFlag());
				myGPS.setHeartbeat(hb);
				//printf("%d\n", myGPS.getHBFlag());
				pmFail = 0;
				break;
			}
			//If PM is dead come in here and increment pmFail and check at another time
			else {
				pmFail++;
			}
			
			myGPS.getData();
			myGPS.sendDataToSharedMemory();
		}
		//printf("%d\n", PMData->Heartbeat.Flags.GPS);
		Thread::Sleep(10);
		//on shutdown signal exit and close window
		if (myGPS.getShutdownFlag() == 1) {
			break;
		}
	}
	if (pmFail > 1000) {
		printf("Process Management Critical Failure: Shutting Down");
		Thread::Sleep(1000);
	}
	myGPS.ShutDown();
	return 0;
}

