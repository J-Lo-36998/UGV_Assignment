#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
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
//Declaring Shared memory
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
int GpsHeartBeat(ProcessManagement* PMData) {
	//PM is not dead if value of hb Flag reset to zero
	if (PMData->Heartbeat.Flags.GPS == 0) {
		//if pm not dead pmFail variable is reset
		pmFail = 0;
		printf("%d\n", PMData->Heartbeat.Flags.GPS); //Printing prev value of hb (what PM changed it to)
		PMData->Heartbeat.Flags.GPS = 1;
		printf("%d\n", PMData->Heartbeat.Flags.GPS);//Printing new Value of hb flag
		return 0;//return zero if PM still alive
	}
	else {
		//if Pm dead, return 1
		return 1;
	}
}

int main() {
	//Instantiating Sharedmemory (Creating and allowing access)
	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;
	while (PMData->Shutdown.Status != 0xFF) {
		//Instantiating the prev time stamp/reset
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		Prev = (double)Counter / (double)Frequency * MILSEC;
		double TimeGap = 0;
		while (TimeGap <= 4* WAIT_TIME && PMData->Shutdown.Status != 0xFF) {
			//Instantiating next time stamp/reset once gets past 4000ms
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			Next = (double)Counter / (double)Frequency * MILSEC;
			TimeGap = Next - Prev;//To obtain time gap between times
			if (GpsHeartBeat(PMData) == 0) {
				//Reset value of pmFail if PM still Alive
				pmFail = 0;
				break;
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			else if (TimeGap > WAIT_TIME + pmFail * WAIT_TIME) {
				pmFail++;
			}
			//PM Shutdown (Since PM is critical, shutdown all)
			if (pmFail > 3) {
				Console::WriteLine("Process Mangement Failure, Critical\n");
				PMData->Shutdown.Status = 0xFF;
				break;
			}
		}
		//on shutdown signal exit and close window
		if (PMData->Shutdown.Status == 0xFF) {
			break;
		}
	}
	PMData->Shutdown.Status = 0xFF;
	return 0;
}

