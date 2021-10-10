
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

SMObject LaserObj(TEXT("Laser"), sizeof(ProcessManagement));
ProcessManagement* LaserData = (ProcessManagement*)LaserObj.pData;

SMObject DisplayObj(TEXT("Display"), sizeof(ProcessManagement));
ProcessManagement* DisplayData = (ProcessManagement*)DisplayObj.pData;

SMObject VehicleObj(TEXT("Vehicle"), sizeof(ProcessManagement));
ProcessManagement* VehicleData = (ProcessManagement*)VehicleObj.pData;

SMObject GPSObj(TEXT("GPS"), sizeof(ProcessManagement));
ProcessManagement* GPSData = (ProcessManagement*)GPSObj.pData;

SMObject CameraObj(TEXT("Camera"), sizeof(ProcessManagement));
ProcessManagement* CameraData = (ProcessManagement*)CameraObj.pData;



bool IsProcessRunning(const char* processName);
void StartProcesses();
void RestartProcesses();
//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("Camera.exe"),
	TEXT("LASER.exe"),
	TEXT("Display.exe"),
	TEXT("Vehicle.exe"),
	TEXT("GPS.exe")
	
};

int LaserPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.Laser == 1) {
		PMData->Heartbeat.Flags.Laser = 0;
		return 0;
	}
	else {
		Thread::Sleep(100);
		return 1;
	}
}

int DisplayPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.OpenGL == 1) {
		PMData->Heartbeat.Flags.OpenGL = 0;
		
		return 0;
	}
	else {
		Thread::Sleep(100);
		return 1;
	}
}

int VehiclePmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.VehicleControl == 1) {
		PMData->Heartbeat.Flags.VehicleControl = 0;
		return 0;
	}
	else {
		Thread::Sleep(100);
		return 1;
	}
}

int GpsPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.GPS == 1) {
		PMData->Heartbeat.Flags.GPS = 0;
		return 0;
	}
	else {
		Thread::Sleep(100);
		return 1;
	}
}


int CameraPmHeartBeat(ProcessManagement* PMData) {
	if (PMData->Heartbeat.Flags.Camera == 1) {
		PMData->Heartbeat.Flags.Camera = 0;
		return 0;
	}
	else {
		Thread::Sleep(100);
		return 1;
	}
}

bool LaserFail(ProcessManagement* PMData, int ProcessFailed[NUM_UNITS]) {
	if (LaserPmHeartBeat(PMData) == 0) {
		ProcessFailed[0] = 0;
		return FALSE;
	}
	else {
		ProcessFailed[0]++;
	}
	if (ProcessFailed[0] > 10) {

		return TRUE;
	}
	return FALSE;
}

bool DisplayFail(ProcessManagement* PMData, int ProcessFailed[NUM_UNITS]) {
	if (DisplayPmHeartBeat(PMData) == 0) {
		ProcessFailed[1] = 0;
		return FALSE;
	}
	else {
		ProcessFailed[1]++;
	}
	if (ProcessFailed[1] > 10) {
		return TRUE;
	}
	return FALSE;
}

bool VehicleFail(ProcessManagement* PMData, int ProcessFailed[NUM_UNITS]) {
	if (VehiclePmHeartBeat(PMData) == 0) {
		
		ProcessFailed[2] = 0;
		return FALSE;
	}
	else {
		ProcessFailed[2]++;
	}
	if (ProcessFailed[2] > 10) {
		return TRUE;
	}
	return FALSE;
}

bool GpsFail(ProcessManagement* PMData, int ProcessFailed[NUM_UNITS]) {
	if (GpsPmHeartBeat(PMData) == 0) {
		ProcessFailed[3] = 0;
		return FALSE;
	}
	else {
		ProcessFailed[3]++;
	}
	if (ProcessFailed[3] > 10) {
		return TRUE;
	}
	return FALSE;
}

bool CameraFail(ProcessManagement* PMData, int ProcessFailed[NUM_UNITS]) {
	if (CameraPmHeartBeat(PMData) == 0) {
		ProcessFailed[4] = 0;
		return FALSE;
	}
	else {
		ProcessFailed[4]++;
	}
	if (ProcessFailed[4] > 10) {
		return TRUE;
	}
	return FALSE;
}

int main(){
	//start all 5 modules
	StartProcesses();
	
	PMObj.SMCreate();
	PMObj.SMAccess();

	LaserObj.SMCreate();
	LaserObj.SMAccess();
	
	DisplayObj.SMCreate();
	DisplayObj.SMAccess();

	VehicleObj.SMCreate();
	VehicleObj.SMAccess();

	GPSObj.SMCreate();
	GPSObj.SMAccess();

	PMData = (ProcessManagement*)PMObj.pData;
	LaserData = (ProcessManagement*)LaserObj.pData;
	DisplayData = (ProcessManagement*)DisplayObj.pData;
	VehicleData = (ProcessManagement*)VehicleObj.pData;
	GPSData = (ProcessManagement*)GPSObj.pData;
	CameraData = (ProcessManagement*)CameraObj.pData;
	//Console::ReadKey();
	int ProcessFailed[NUM_UNITS] = {0};
	
	while (!_kbhit()) {
		//int FailCheck = { 0 };
		//while (FailCheck <= 3) {
		//	ProcessFailed[0] += LaserPmHeartBeat(PMData, FailCheck);
		//	ProcessFailed[1] += DisplayPmHeartBeat(PMData, FailCheck);
		//	ProcessFailed[2] += VehiclePmHeartBeat(PMData, FailCheck);
		//	ProcessFailed[3] += GpsPmHeartBeat(PMData, FailCheck);
		//	ProcessFailed[4] += CameraPmHeartBeat(PMData, FailCheck);
		//	////Critical Procesess
		//	if (ProcessFailed[0] > 3) {
		//		PMData->Shutdown.Status = 0xFF;
		//		Console::WriteLine("Critical failure of Laser, shutting down");
		//	}
		//	if (ProcessFailed[0] > 3) {
		//		Console::WriteLine("Critical failure of Display, shutting down");
		//		PMData->Shutdown.Status = 0xFF;
		//	}
		//	if (ProcessFailed[2] > 3) {
		//		Console::WriteLine("Critical failure of Vehicle, shutting down");
		//		PMData->Shutdown.Status = 0xFF;
		//	}
		//	//////non-critical processes
		//	if (ProcessFailed[3] > 3) {
		//		Console::WriteLine("Non-critical failure of GPS, Restarting");
		//		PMData->Shutdown.Flags.GPS = 1;
		//		RestartProcesses();
		//	}
		//	if (ProcessFailed[4] > 3) {
		//		Console::WriteLine("Non-critical failure of Camera, Restarting");
		//		PMData->Shutdown.Flags.Camera = 1;
		//		RestartProcesses();
		//	}
		//	FailCheck++;
		//}
		
		/*if (LaserPmHeartBeat(PMData) == 0) {
			printf("%d fail", ProcessFailed[0]);
			ProcessFailed[0] = 0;
		}
		else {
			ProcessFailed[0]++;
		}
		if (ProcessFailed[0] > 100) {
			
			PMData->Shutdown.Status = 0xFF;
		}*/

		if (LaserFail(PMData, ProcessFailed) == TRUE) {
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
		}
		/*if (PMData->Shutdown.Status == 0xFF)
			break;*/
		if (_kbhit())
			break;
	}
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
