#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
int counter{ 0 };

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
		if (PMData->Shutdown.Status)
			break;
		if (_kbhit())
			break;

		//Did PM put my flag down?
			//true-> put flag up
			//false-> is the pm time stamp older by agreed time gap
				//True->shutdown all
				//
		//Console::WriteLine("Laser time stamps: {0,12:F3} {1, 12:X2}", TimeStamp, Shutdown);
		//PMData->Heartbeat.Flags.Laser = 0;
	}
	for (int i = 0; i < 100; i++) {
		Console::WriteLine("{0,12:F3}", TSvalues[i]);
	}
	return 0;
}
