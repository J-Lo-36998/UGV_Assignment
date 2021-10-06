#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
int counter{ 0 };

int LaserHeartBeat(ProcessManagement* PMData, int FailCheck) {
	if (PMData->Heartbeat.Flags.Laser == 0 && FailCheck <= 3) {
		printf("%d", PMData->Heartbeat.Flags.Laser);
		PMData->Heartbeat.Flags.Laser = 1;
		printf("%d", PMData->Heartbeat.Flags.Laser);
		return 0;
	}
	else {
		Thread::Sleep(250);
		return 1;
	}
}
int main() {
	//Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	int TScounter = 0;
	array<double>^ TSvalues = gcnew array<double>(100);
	double TimeGap;
	__int64 Frequency, Counter, oldCounter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceFrequency((LARGE_INTEGER*)&oldCounter);

	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	while (1) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeGap = (double)(Counter - oldCounter) / (double)Frequency * 1000; //ms
		oldCounter = Counter;
		if (TScounter < 100) {
			TSvalues[TScounter++] = TimeGap;
		}
		Thread::Sleep(25);
		//Did PM put my flag down?
			//true-> put flag up
			//false-> is the pm time stamp older by agreed time gap
				//True->shutdown all
				//
		//Console::WriteLine("Laser time stamps: {0,12:F3} {1, 12:X2}", TimeStamp, Shutdown);
		//PMData->Heartbeat.Flags.Laser = 0;
		int FailCheck{ 0 };
		int failure{ 0 };
		while (FailCheck <= 3) {
			failure += LaserHeartBeat(PMData, FailCheck);
			if (failure > 3) {
				PMData->Shutdown.Status = 0xFF;
			}
			FailCheck++;
		}
		if (PMData->Shutdown.Flags.Laser == 1)
			break;
		if (_kbhit())
			break;
	}
	for (int i = 0; i < 100; i++) {
		Console::WriteLine("{0,12:F3}", TSvalues[i]);
	}
	return 0;
}
