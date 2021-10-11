
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
//int LaserFail, DispFail, GPSFail;
int LaserPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.Laser == 1) {
		//printf("%d", PMData->Heartbeat.Flags.Laser);
		PMData->Heartbeat.Flags.Laser = 0;
		//printf("%d", PMData->Heartbeat.Flags.Laser);
		return 0;
	}
	else {
		//Thread::Sleep(100);
		return 1;
	}
}

int DisplayPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.OpenGL == 1) {
		//printf("%d\n", PMData->Heartbeat.Flags.OpenGL);
		PMData->Heartbeat.Flags.OpenGL = 0;
		//printf("%d\n", PMData->Heartbeat.Flags.OpenGL);
		return 0;
	}
	else {
		//Thread::Sleep(100);
		return 1;
	}
}

int VehiclePmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.VehicleControl == 1) {
		PMData->Heartbeat.Flags.VehicleControl = 0;
		return 0;
	}
	else {
		//Thread::Sleep(100);
		return 1;
	}
}

int GpsPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.GPS == 1) {
		PMData->Heartbeat.Flags.GPS = 0;
		return 0;
	}
	else {
		//Thread::Sleep(100);
		return 1;
	}
}

int CameraPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.Camera == 1) {
		PMData->Heartbeat.Flags.Camera = 0;
		return 0;
	}
	else {
		//Thread::Sleep(100);
		return 1;
	}
}
int main(){
	//start all 5 modules
	StartProcesses();
	
	double PrevTime, NextTime;
	__int64 Frequency{}, Counter;
	int Shutdown = 0x00;
	int LaserFail{ 0 };
	int DispFail{ 0 };
	int VFail{ 0 };
	int GpsFail{ 0 };
	int CamFail{ 0 };
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;
	
	//Console::ReadKey();
	int ProcessFailed[NUM_UNITS] = {0};
	
	//Thread::Sleep(25);
	while (!_kbhit()) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		PrevTime = (double)Counter / (double)Frequency * 1000;
		double TimeGap = 0;
		while (TimeGap < 5000 && PMData->Shutdown.Status != 0xFF) {
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			NextTime = (double)Counter / (double)Frequency * 1000;
			//Console::WriteLine("Time Gap is Currently : {0,12:F3}", NextTime - PrevTime);
			TimeGap = NextTime - PrevTime;
			//printf("%d\n", PMData->Heartbeat.Flags.Laser);
			///start
			//Laser
			if (LaserPmHeartBeat(PMData) == 0) {
				LaserFail = 0;
				break;
			}
			else if (TimeGap > 1000 + LaserFail * 1000) {
				printf("hi");
				LaserFail++;
			}
			if (LaserFail > 3) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
//			printf("%d", LaserFail);
			//Display
			if (DisplayPmHeartBeat(PMData) == 0) {
				DispFail = 0;
				break;
			}
			else if (TimeGap > 1000 + DispFail * 1000) {
				DispFail++;
			}
			if (DispFail > 3) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
			//Vehicle
			if (VehiclePmHeartBeat(PMData) == 0) {
				VFail = 0;
				break;
			}
			else if (TimeGap > 1000 + VFail * 1000) {
				VFail++;
			}
			if (VFail > 3) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
			//GPS
			if (GpsPmHeartBeat(PMData) == 0) {
				GpsFail = 0;
				break;
			}
			else if (TimeGap > 1000 + VFail * 1000) {
				GpsFail++;
			}
			if (GpsFail > 3) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
			//Camera
			if (CameraPmHeartBeat(PMData) == 0) {
				CamFail = 0;
				break;
			}
			else if (TimeGap > 1000 + VFail * 1000) {
				CamFail++;
			}
			if (CamFail > 3) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
			//printf("%d", VFail);
			/*LaserCheck(PMData, TimeGap, LaserFail);
			if (LaserFail > 3) {
				printf("HI");
				PMData->Shutdown.Status = 0xFF;
			}*/
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