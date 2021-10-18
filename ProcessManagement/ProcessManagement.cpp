
#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>
#include <array>
#include "SMStructs.h"
#include "SMObject.h"

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;
#define NUM_UNITS 5
//for convertion to ms
#define MILSEC 1000
//Time in ms in the loops and also when to check again
#define WAIT_TIME 1000
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

bool IsProcessRunning(const char* processName);
void StartProcesses();
void RestartProcesses();
//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("LASER.exe"),
	TEXT("Display.exe"),
	TEXT("Vehicle.exe"),
	TEXT("GPS.exe"),
	TEXT("Camera.exe")
};
//Checks if Laser still Alive
int LaserPmHeartBeat(ProcessManagement* PMData, int &LaserFail) {
	if (PMData->Heartbeat.Flags.Laser == 1) {
		//printf("%d", PMData->Heartbeat.Flags.Laser);
		LaserFail = 0;
		PMData->Heartbeat.Flags.Laser = 0;
		//printf("%d", PMData->Heartbeat.Flags.Laser);
		return 0;
	}
	else {
		//Thread::Sleep(100);
		LaserFail++;
		return 1;
	}
}
//Checks if Display still alive
int DisplayPmHeartBeat(ProcessManagement* PMData, int& DispFail) {
	if (PMData->Heartbeat.Flags.OpenGL == 1) {
		//printf("%d\n", PMData->Heartbeat.Flags.OpenGL);
		DispFail = 0;
		PMData->Heartbeat.Flags.OpenGL = 0;
		//printf("%d\n", PMData->Heartbeat.Flags.OpenGL);
		return 0;
	}
	else {
		//Thread::Sleep(100);
		DispFail++;
		return 1;
	}
}
//Checks if Vehicle is still Alive
int VehiclePmHeartBeat(ProcessManagement* PMData, int &VFail) {
	if (PMData->Heartbeat.Flags.VehicleControl == 1) {
		VFail = 0;
		PMData->Heartbeat.Flags.VehicleControl = 0;
		return 0;
	}
	else {
		//Thread::Sleep(100);
		VFail++;
		return 1;
	}
}
//Checks if GPS is still alive
int GpsPmHeartBeat(ProcessManagement* PMData, int &GpsFail) {
	if (PMData->Heartbeat.Flags.GPS == 1) {
		PMData->Heartbeat.Flags.GPS = 0;
		GpsFail = 0;
		return 0;
	}
	else {
		//Thread::Sleep(100);
		GpsFail++;
		return 1;
	}
}
//Checks if Camera is still Alive
int CameraPmHeartBeat(ProcessManagement* PMData, int &CamFail) {
	if (PMData->Heartbeat.Flags.Camera == 1) {
		CamFail = 0;
		PMData->Heartbeat.Flags.Camera = 0;
		return 0;
	}
	else {
		//Thread::Sleep(100);
		CamFail++;
		return 1;
	}
}

void LaserFailure(ProcessManagement* PMData, int& LaserFail) {
	if (LaserPmHeartBeat(PMData, LaserFail) == 0) {
		LaserFail = 0;
		//break;
	}
	else if (LaserFail > 500) {
		Console::Write("Critical Failure of Laser module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
		//break;
	}
	Thread::Sleep(5);
}
void DispFailure(ProcessManagement* PMData, int& DispFail) {
	if (DisplayPmHeartBeat(PMData, DispFail) == 0) {
		DispFail = 0;
		//break;
	}
	else if (DispFail > 500) {
		Console::Write("Critical Failure of Display module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
		//break;
	}
	Thread::Sleep(5);
}
void VehicleFailure(ProcessManagement* PMData, int& VFail) {
	if (VehiclePmHeartBeat(PMData, VFail) == 0) {
		VFail = 0;
		//break;
	}
	else if (VFail > 500) {
		Console::Write("Critical Failure of Vehicle module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
		//break;
	}
	Thread::Sleep(5);
}

void GPSFailure(ProcessManagement* PMData, int& GpsFail) {
	if (GpsPmHeartBeat(PMData, GpsFail) == 0) {
		GpsFail = 0;
		//break;
	}
	else if (GpsFail > 500) {
		Console::Write("Critical Failure of Display module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
		//break;
	}
	Thread::Sleep(5);
}

void CameraFailure(ProcessManagement* PMData, int& CamFail) {
	if (CameraPmHeartBeat(PMData, CamFail) == 0) {
		CamFail = 0;
		//break;
	}
	else if (CamFail > 500) {
		Console::Write("Critical Failure of Display module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
		//break;
	}
	Thread::Sleep(5);
}
int main(){
	//start all 5 modules
	StartProcesses();
	//Local variable declarations
	double PrevTime, NextTime;
	__int64 Frequency{}, Counter;

	//Failure counter for each module

	int LaserFail{ 0 };//Laser
	int DispFail{ 0 };//Display
	int VFail{ 0 };//Vehicle
	int GpsFail{ 0 };//GPS
	int CamFail{ 0 };//Camera
	//Shared memory instantiation
	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;
	
	//Console::ReadKey();
	int ProcessFailed[NUM_UNITS] = {0};
	
	//Thread::Sleep(25);
	while (!_kbhit()) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		PrevTime = (double)Counter / (double)Frequency * MILSEC;
		double TimeGap = 0;
		//printf("%d\n", PMData->Heartbeat.Flags.Laser);
		while (TimeGap < 5 * WAIT_TIME && PMData->Shutdown.Status != 0xFF) {
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			NextTime = (double)Counter / (double)Frequency * MILSEC;
			TimeGap = NextTime - PrevTime;
			//Laser Section
			LaserFailure(PMData, LaserFail);
			////Display Section
			DispFailure(PMData, DispFail);
			//////Vehicle Section 
			VehicleFailure(PMData, VFail);
			////GPS Section (Non Critical)
			GPSFailure(PMData, GpsFail);
			////Camera Section (Non Critical)
			CameraFailure(PMData, CamFail);
		}
		if (PMData->Shutdown.Status == 0xFF) {
			break;
		}
	}
	Console::ReadKey();
	PMData->Shutdown.Status = 0xFF;
	return 0;
}

//Is process running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp(entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}


void StartProcesses()
{
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	for (int i = 0; i < NUM_UNITS; i++)
	{
		if (!IsProcessRunning(Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));

			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
			}
			std::cout << "Started: " << Units[i] << std::endl;
			Sleep(100);
		}
	}
}

void RestartProcesses()
{
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	for (int i = 0; i < NUM_UNITS; i++)
	{
		if (!IsProcessRunning(Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));

			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
			}
			std::cout << "Restarting: " << Units[i] << std::endl;
			Sleep(100);
		}
	}
}


/*else {
				if (TimeGap > 3000) {
					Console::ReadKey();
					PMData->Shutdown.Status = 0xFF;
				}
			}*/
			/*else if (LaserPmHeartBeat(PMData) == 1) {
				if (TimeGap < 1000 && LaserFail == 0) {
					LaserFail++;
				}
				else if (TimeGap < 2000 && LaserFail == 1) {
					LaserFail++;
				}
				else if (TimeGap < 3000 && LaserFail == 2) {
					LaserFail++;
				}
				if (LaserFail == 3) {
					Console::ReadKey();
					PMData->Shutdown.Status = 0xFF;
				}
			}*/
			//printf("%d", LaserFail);
		//}
		//Thread::Sleep(25);	
	//}


			//printf("%d", timeGap);

		/*if (LaserFail(PMData, ProcessFailed) == TRUE) {
			PMData->Shutdown.Status = 0xFF;
			printf("Critical Failure of Laser Module");
		}
		if (DisplayFail(PMData, ProcessFailed) == TRUE) {
			PMData->Shutdown.Status = 0xFF;
			printf("Critical Failure of Display Module");
		}
		if (VehicleFail(PMData, ProcessFailed) == TRUE) {

			PMData->Shutdown.Status = 0xFF;
			printf("Critical Failure of Vehicle Module");
		}
		if (GpsFail(PMData, ProcessFailed) == TRUE) {
			PMData->Shutdown.Flags.GPS = 1;
			printf("Failure of Non-critical Process: GPS, Restarting");
			RestartProcesses();

		}
		if (CameraFail(PMData, ProcessFailed) == TRUE) {
			PMData->Shutdown.Flags.Camera = 1;
			printf("Failure of Non-critical Process: Camera, Restarting");
			RestartProcesses();
		}*/
		/*if (PMData->Shutdown.Status == 0xFF)
			break;*/