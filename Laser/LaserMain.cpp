#include "Laser.h" 
#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>

#include <math.h>
#using <System.dll>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
//Counter to keep track of how many times PM Fails
int pmFail{ 0 };
bool hb = TRUE;

int main() {
	Laser myLaser;
	myLaser.setupSharedMemory();
	myLaser.connect("192.168.1.200", 23000);
	while (pmFail < 1000) {
		while (myLaser.getShutdownFlag() != 1) {
			Thread::Sleep(10);
			if (myLaser.getHBFlag() == 0) {
				//Reset value of pmFail if PM still Alive
				myLaser.setHeartbeat(hb);
				pmFail = 0;
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			else {	
				pmFail++;
			}	
			myLaser.getData();
			myLaser.sendDataToSharedMemory();
		}
		Thread::Sleep(10);
		if (myLaser.getShutdownFlag() == 1) {
			break;
		}
	}
	if (pmFail > 1000) {
		printf("Process Management Critical Failure: Shutting Down");
		Thread::Sleep(1000);
	}
	myLaser.disconnect();
	myLaser.ShutDown();
	return 0;
}

