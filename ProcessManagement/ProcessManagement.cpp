//
//#using <System.dll>
//
//#include <Windows.h>
//#include <tchar.h>
//#include <TlHelp32.h>
//#include <stdio.h>
//#include <iostream>
//#include <conio.h>
//#include "SMStructs.h"
//#include <SMObject.h>
//#include <smstructs.h>
//#include <conio.h>
//
//
////using namespace System;
////using namespace System::Diagnostics;
////using namespace System::Threading;
////using namespace System;
////using namespace System::Net::Sockets;
////using namespace System::Net;
////using namespace System::Text;
//
//using namespace System;
//using namespace System::Net::Sockets;
//using namespace System::Net;
//using namespace System::Text;
//
//#define NUM_UNITS 4
//
//bool IsProcessRunning(const char* processName);
//void StartProcesses();
//
////defining start up sequence
//TCHAR Units[10][20] = //
//{
//	TEXT("GPS.exe"),
//	TEXT("Camera.exe"),
//	TEXT("Display.exe"),
//	TEXT("Laser.exe"),
//	TEXT("VehicleControl.exe")
//};
//
//int main()
//{
//	//start all 5 modules
//	StartProcesses();
//	//Tele-operation
//	//Declarations + Initialization
//	//SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
//	//array<String^>^ ModuleList = gcnew array<String^> {"Laser", "Display", "Vehicle", "GPS", "Camera"};
//	//array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 1, 1, 0, 0 };
//	//array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);
//
//	////SM Creation and seeking access
//	//PMObj.SMCreate();
//	//PMObj.SMAccess();
//
//	//ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
//	//for (int i = 0; i < ModuleList->Length; i++) {
//	//	if (Process::GetProcessesByName(ModuleList[i])->Length == 0) {
//	//		ProcessList[i] = gcnew Process;
//	//		ProcessList[i]->StartInfo->FileName = ModuleList[i];
//	//		ProcessList[i]->Start();
//	//		Console::WriteLine("The process " + ModuleList[i] + ".exe started");
//	//	}
//	//}
//	////Main Loop
//	//while (!_kbhit()) {
//	//	Thread::Sleep(1000);
//	//}
//	//Console::ReadKey();
//	//PMData->Shutdown.Status = 0xFF;
//	////Clearing and Shutdown
//	return 0;
//}
//
//
////Is process running function
//bool IsProcessRunning(const char* processName)
//{
//	bool exists = false;
//	PROCESSENTRY32 entry;
//	entry.dwSize = sizeof(PROCESSENTRY32);
//
//	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
//
//	if (Process32First(snapshot, &entry))
//		while (Process32Next(snapshot, &entry))
//			if (!_stricmp((const char *)entry.szExeFile, processName))
//				exists = true;
//
//	CloseHandle(snapshot);
//	return exists;
//}
//
//
//void StartProcesses()
//{
//	STARTUPINFO s[10];
//	PROCESS_INFORMATION p[10];
//
//	for (int i = 0; i < NUM_UNITS; i++)
//	{
//		if (!IsProcessRunning((const char *)Units[i]))
//		{
//			ZeroMemory(&s[i], sizeof(s[i]));
//			s[i].cb = sizeof(s[i]);
//			ZeroMemory(&p[i], sizeof(p[i]));
//
//			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
//			{
//				printf("%s failed (%d).\n", Units[i], GetLastError());
//				_getch();
//			}
//			std::cout << "Started: " << Units[i] << std::endl;
//			Sleep(100);
//		}
//	}
//}
//


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

int LaserPmHeartBeat(ProcessManagement* PMData, int counter) {
	if (PMData->Heartbeat.Flags.Laser == 1) {
		PMData->Heartbeat.Flags.Laser = 0;
		return 0;
	}
	else {
		Thread::Sleep(500);
		return 1;
	}
}

int DisplayPmHeartBeat(ProcessManagement* PMData, int counter) {
	if (PMData->Heartbeat.Flags.OpenGL == 1) {
		PMData->Heartbeat.Flags.OpenGL = 0;
		
		return 0;
	}
	else {
		Thread::Sleep(500);
		return 1;
	}
}

int VehiclePmHeartBeat(ProcessManagement* PMData, int counter) {
	if (PMData->Heartbeat.Flags.VehicleControl == 1) {
		PMData->Heartbeat.Flags.VehicleControl = 0;
		return 0;
	}
	else {
		Thread::Sleep(500);
		return 1;
	}
}

int GpsPmHeartBeat(ProcessManagement* PMData, int counter) {
	if (PMData->Heartbeat.Flags.GPS == 1) {
		PMData->Heartbeat.Flags.GPS = 0;
		return 0;
	}
	else {
		Thread::Sleep(500);
		return 1;
	}
}

int CameraPmHeartBeat(ProcessManagement* PMData, int counter) {
	if (PMData->Heartbeat.Flags.Camera == 1) {
		PMData->Heartbeat.Flags.Camera = 0;
		return 0;
	}
	else {
		Thread::Sleep(500);
		return 1;
	}
}

int main(){
	//start all 5 modules
	StartProcesses();
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	//Console::ReadKey();
	while (!_kbhit()) {
		int FailCheck = { 0 };
		std::array<int, NUM_UNITS> ProcessFailed = {0};
		while (FailCheck <= 3) {
			ProcessFailed[0] += LaserPmHeartBeat(PMData, FailCheck);
			ProcessFailed[1] += DisplayPmHeartBeat(PMData, FailCheck);
			ProcessFailed[2] += VehiclePmHeartBeat(PMData, FailCheck);
			ProcessFailed[3] += GpsPmHeartBeat(PMData, FailCheck);
			ProcessFailed[4] += CameraPmHeartBeat(PMData, FailCheck);
			////Critical Procesess
			if (ProcessFailed[0] > 3) {
				PMData->Shutdown.Status = 0xFF;
				Console::WriteLine("Critical failure of Laser, shutting down");
			}
			if (ProcessFailed[0] > 3) {
				Console::WriteLine("Critical failure of Display, shutting down");
				PMData->Shutdown.Status = 0xFF;
			}
			if (ProcessFailed[2] > 3) {
				Console::WriteLine("Critical failure of Vehicle, shutting down");
				PMData->Shutdown.Status = 0xFF;
			}
			////non-critical processes
			if (ProcessFailed[3] > 3) {
				Console::WriteLine("Non-critical failure of GPS, Restarting");
				PMData->Shutdown.Flags.GPS = 1;
				RestartProcesses();

			}
			if (ProcessFailed[4] > 3) {
				Console::WriteLine("Non-critical failure of Camera, Restarting");
				PMData->Shutdown.Flags.Camera = 1;
				RestartProcesses();
			}
			FailCheck++;
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
