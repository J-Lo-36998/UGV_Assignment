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
#include "SMStructs.h"
#include "SMObject.h"

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#define NUM_UNITS 5

bool IsProcessRunning(const char* processName);
void StartProcesses();

//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("GPS.exe"),
	TEXT("LASER.exe"),
	TEXT("Display.exe"),
	TEXT("Camera.exe"),
	TEXT("Vehicle.exe")
};

int main()
{
	//start all 5 modules
	StartProcesses();
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
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

