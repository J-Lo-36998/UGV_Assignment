
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
#define NUM_UNITS 1

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
SMObject LaserObj(TEXT("Laser"), sizeof(ProcessManagement));



ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
ProcessManagement* LaserData = (ProcessManagement*)LaserObj.pData;


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
int LaserFailure{ 0 };
bool LaserFail(ProcessManagement* LaserData) {
	if (LaserData->Heartbeat.Flags.Laser == 1) {
		LaserData->Heartbeat.Flags.Laser = 0;
		LaserFailure = 0;
		
	}
	else {
		LaserFailure++;
		if (LaserFailure > 1000) {
			LaserData->Shutdown.Flags.Laser = 1;
			return TRUE;
		}
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

	PMData = (ProcessManagement*)PMObj.pData;
	LaserData = (ProcessManagement*)PMObj.pData;
	//Console::ReadKey();
	int ProcessFailed[NUM_UNITS] = {0};
	
	while (!_kbhit()) {
		if (LaserFail(LaserData) == TRUE) {
			printf("Critical Failure of Laser Module");
			break;
		}
	
		/*if (PMData->Shutdown.Status == 0xFF)
			break;*/
		if (_kbhit())
			break;
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
