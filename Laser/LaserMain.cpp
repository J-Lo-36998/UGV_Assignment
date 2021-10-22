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
bool hb = FALSE;
//For use with Time Stamps
double Prev, Next;
__int64 Frequency{}, Counter;
int Shutdown = 0x00;
//for convertion to ms
#define MILSEC 1000
//Time in ms in the loops and also when to check again
#define WAIT_TIME 1000


//Declaring Shared memory
SMObject LaserObj(TEXT("LaserData"), sizeof(SM_Laser));
SM_Laser* LaserData = (SM_Laser*)LaserObj.pData;

int main() {
	//Instantiating Sharedmemory (Creating and allowing access)
	//PMObj.SMCreate();
	LaserObj.SMAccess();
	LaserData = (SM_Laser*)LaserObj.pData;
	Laser myLaser;
	myLaser.setupSharedMemory();
	myLaser.connect("192.168.1.200", 23000);
	myLaser.getData();
	while (myLaser.ShutDownSignal() != 0xFF) {
		//Instantiating the prev time stamp/reset
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		Prev = (double)Counter / (double)Frequency * MILSEC;
		double TimeGap = 0;
		// Print the received string on the screen
		//Console::WriteLine(ResponseData);
		myLaser.sendDataToSharedMemory();
		//For HeartBeats
		//printf("%d\n", PMData->Heartbeat.Flags.Laser);
		while (TimeGap <= 4000 && myLaser.ShutDownSignal() != 0xFF) {
			//Instantiating next time stamp/reset once gets past 4000ms
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			Next = (double)Counter / (double)Frequency * MILSEC;
			TimeGap = Next - Prev;//getting the time gap
			if (myLaser.getHBFlag() == 0) {
				//Reset value of pmFail if PM still Alive
				//printf("%d\n", myLaser.getHBFlag());
				myLaser.setHeartbeat(hb);
				pmFail = 0;
				//printf("%d\n", myLaser.getHBFlag());
				//break;
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			
			else if (pmFail > 50 ) {
				Console::WriteLine("Process Mangement Failure, Critical\n");
				Thread::Sleep(1000);
				myLaser.ShutDown();
				//break;
			}
			else {	
				pmFail++;
			}
			printf("%d", pmFail);
		}
		//on shutdown signal exit and close window
		//printf("%d\n", PMData->Heartbeat.Flags.Laser);
		Thread::Sleep(10);
		if (myLaser.ShutDownSignal() == 0xFF) {
			break;
		}
	}
	myLaser.disconnect();
	//Console::ReadKey();
	//Console::ReadKey();

	myLaser.ShutDown();
	return 0;
}

