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

//Value to set the vehicle heartbeat to
bool hb = TRUE;

int main() {
	//instance of vehicle control class
	VehicleControl myUGV;
	//setting up shared shared memmory for hb and control
	myUGV.setupSharedMemory();
	//connects to server on UGV
	myUGV.connect("192.168.1.200", 25000);
	while (pmFail<1000) {//checks if pmFail has 1000 continuous 'fails'
		while (myUGV.getShutdownFlag() != 1) {// while shutdown signal not given
			Thread::Sleep(10);
			//if current hb value is zero, pm is alive and working
			if (myUGV.getHBFlag() == 0) {
				//Reset value of pmFail if PM still Alive
				myUGV.setHeartbeat(hb);
				pmFail = 0;
				break;
			}
			//If PM is dead come in here and increment pmFail and check again later for repeat fail
			else {
				pmFail++;
			}
			myUGV.controls();
		}
		Thread::Sleep(10);
		//on shutdown signal exit and close window
		if (myUGV.getShutdownFlag() == 1) {
			break;
		}
	}
	if (pmFail > 1000) {//after this amount of repeat fails, pm is dead so print then shutdown
		printf("Process Management Critical Failure: Shutting Down");
		Thread::Sleep(1000);
	}
	myUGV.ShutDown();//shutsdown vehicle
	return 0;
}

