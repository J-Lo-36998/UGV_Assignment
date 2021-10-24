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
//value of hb to set the hb flag to
bool hb = TRUE;

int main() {
	Laser myLaser; //declaring instance of Laser class
	myLaser.setupSharedMemory();//setting up shared memory
	myLaser.connect("192.168.1.200", 23000);//connecting to laser range finder
	while (pmFail < 1000) {//checking for number of repeat fails
		while (myLaser.getShutdownFlag() != 1) {
			Thread::Sleep(10);
			if (myLaser.getHBFlag() == 0) {//Checking if PM set laser flag to zero (means pm is still alive)			
				myLaser.setHeartbeat(hb);//Setting hb to 1 or TRUE if PM is alive
				pmFail = 0;//Reset value of pmFail if PM still Alive
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			else {	
				pmFail++;
			}	
			myLaser.getData();//getting laser data
			myLaser.sendDataToSharedMemory();//sending data to sm for access in other modules
		}
		Thread::Sleep(10);
		if (myLaser.getShutdownFlag() == 1) {
			break;
		}
	}
	if (pmFail > 1000) {//after 1000 repeat fails, PM is dead print this and then shutdown
		printf("Process Management Critical Failure: Shutting Down");
		Thread::Sleep(1000);
	}
	myLaser.disconnect();//Disconnects from laser 
	myLaser.ShutDown();//shut down laser 
	return 0;
}

