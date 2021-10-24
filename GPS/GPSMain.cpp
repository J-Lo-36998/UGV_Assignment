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
//Heart beat flag to set if alive
bool hb = TRUE;

int main() {
	//Instantiating Sharedmemory (Creating and allowing access)
	GPS myGPS; //instance of the GPS class
	myGPS.setupSharedMemory();
	myGPS.connect("192.168.1.200", 24000);
	while (pmFail<1000) {//checks if number of 'fails' is more than a thousand
		while (myGPS.getShutdownFlag() != 1) {//shutdown command is not given
			Thread::Sleep(10);
			//if pm set hb flag back to zero its still alive
			if (myGPS.getHBFlag() == 0) {
				myGPS.setHeartbeat(hb);///set hb back to TRUE/1
				//If working normally reset 'fails' to zero
				pmFail = 0;
				break;
			}
			//If PM is dead or flag is still zero come in here 
			else {
				//increment pmFailand check again later
				pmFail++;
			}
			myGPS.getData();//gets gpsdata
			myGPS.sendDataToSharedMemory();//sends data to SM so accessible in other modules
		}
		Thread::Sleep(10);
		//on shutdown signal exit and close window
		if (myGPS.getShutdownFlag() == 1) {
			break;
		}
	}
	if (pmFail > 1000) {//if number of repeat fails > 1000, PM has died so print this message and shut down
		printf("Process Management Critical Failure: Shutting Down");
		Thread::Sleep(1000);
	}
	myGPS.ShutDown();
	return 0;
}

