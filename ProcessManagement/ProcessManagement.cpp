
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

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

SMObject LaserObj(TEXT("SM_Laser"), sizeof(SM_Laser));
SM_Laser* LaserData = (SM_Laser*)LaserObj.pData;

SMObject GpsObj(TEXT("SM_GPS"), sizeof(SM_GPS));
SM_GPS* GpsData = (SM_GPS*)GpsObj.pData;

SMObject VehicleObj(TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
SM_VehicleControl* VehicleData = (SM_VehicleControl*)VehicleObj.pData;

//Checks if Laser still Alive
int LaserPmHeartBeat(ProcessManagement* PMData, int &LaserFail) {
	if (PMData->Heartbeat.Flags.Laser == 1) {
		PMData->Heartbeat.Flags.Laser = 0;
		return 0;
	}
	else {
		//increment the failure
		LaserFail++;
		return 1;
	}
}
//Checks if Display still alive
int DisplayPmHeartBeat(ProcessManagement* PMData, int& DispFail) {
	if (PMData->Heartbeat.Flags.OpenGL == 1) {
		PMData->Heartbeat.Flags.OpenGL = 0;
		return 0;
	}
	else {
		DispFail++;//increment fail counter
		return 1;
	}
}
//Checks if Vehicle is still Alive
int VehiclePmHeartBeat(ProcessManagement* PMData, int &VFail) {
	if (PMData->Heartbeat.Flags.VehicleControl == 1) {
		PMData->Heartbeat.Flags.VehicleControl = 0;
		return 0;
	}
	else {
		//increment fail counter
		VFail++;
		return 1;
	}
}
//Checks if GPS is still alive
int GpsPmHeartBeat(ProcessManagement* PMData, int &GpsFail) {
	if (PMData->Heartbeat.Flags.GPS == 1) {
		PMData->Heartbeat.Flags.GPS = 0;
		return 0;
	}
	else {
		//increment the fail counter
		GpsFail++;
		return 1;
	}
}
//Checks if Camera is still Alive
int CameraPmHeartBeat(ProcessManagement* PMData, int &CamFail) {
	if (PMData->Heartbeat.Flags.Camera == 1) {
		PMData->Heartbeat.Flags.Camera = 0;
		return 0;
	}
	else {
		CamFail++;//increment the fail counter
		return 1;
	}
}
//Checks for laser failure
void LaserFailure(ProcessManagement* PMData, int& LaserFail) {
	if (LaserPmHeartBeat(PMData, LaserFail) == 0) {
		LaserFail = 0;//laser is alive so reset its fail counter
	}
	else if (LaserFail > 100) {
		Console::Write("Critical Failure of Laser module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
	}
	Thread::Sleep(5);
}
//checks for failure of Display
void DispFailure(ProcessManagement* PMData, int& DispFail) {
	if (DisplayPmHeartBeat(PMData, DispFail) == 0) {
		DispFail = 0;//display is alive so reset its fail counter
	}
	else if (DispFail > 100) {
		Console::Write("Critical Failure of Display module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
	}
	Thread::Sleep(5);
}
//Checks for failure of vehicle
void VehicleFailure(ProcessManagement* PMData, int& VFail) {
	if (VehiclePmHeartBeat(PMData, VFail) == 0) {
		VFail = 0;//vehicle is alive so reset its fail counter
	}
	else if (VFail > 100) {
		Console::Write("Critical Failure of Vehicle module: Shutting Down\n");
		PMData->Shutdown.Status = 0xFF;
	}
	Thread::Sleep(5);
}
//Checks for failure of GPS
void GPSFailure(ProcessManagement* PMData, int& GpsFail, array<Process^>^ ProcessList) {
	if (GpsPmHeartBeat(PMData, GpsFail) == 0) {
		GpsFail = 0;//gps i alive so reset its fail counter
	}
	else if (GpsFail > 100) {
		Console::Write("Non-Critical Failure of GPS module: Restarting\n");
		PMData->Shutdown.Flags.GPS = 1;
		ProcessList[3]->Start();
		Console::WriteLine("GPS Module Restarted");
		Thread::Sleep(500);//sleep so only re-opens once
	}
	Thread::Sleep(5);

}

void CameraFailure(ProcessManagement* PMData, int& CamFail, array<Process^>^ ProcessList) {
	if (CameraPmHeartBeat(PMData, CamFail) == 0) {
		CamFail = 0; //camera is alive so reset its fail counter
	}
	else if (CamFail > 100) {
		Console::Write("Non-Critical Failure of Camera module: Restarting\n");
		PMData->Shutdown.Flags.Camera = 1;
		ProcessList[4]->Start();
		Console::Write("Camera Module Restarted\n");
		Thread::Sleep(500);//sleep so only re-opens once
	}
	Thread::Sleep(5);
}
int main(){
	//Shared memory instantiation and access
	//PM shared mem
	PMObj.SMCreate();
	PMObj.SMAccess();
	//Laser shared mem
	LaserObj.SMCreate();
	LaserObj.SMAccess();
	//GPS Shared mem
	GpsObj.SMCreate();
	GpsObj.SMAccess();
	//Vehicle Shared mem
	VehicleObj.SMCreate();
	VehicleObj.SMAccess();

	//PM shared mem
	PMData = (ProcessManagement*)PMObj.pData;

	//Laser shared mem
	LaserData = (SM_Laser*)LaserObj.pData;

	//GPS Shared mem
	GpsData = (SM_GPS*)GpsObj.pData;

	//Vehicle Shared mem
	VehicleData = (SM_VehicleControl*)VehicleObj.pData;

	//start all 5 modules
	array<String^>^ ModuleList = gcnew array<String^>{"Laser1", "Display2", "Vehicle", "GPS", "Camera"}; //names of modules to be started
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);
	for (int i = 0; i < ModuleList->Length; i++) {//iterate through Process list and start each process
		if (Process::GetProcessesByName(ModuleList[i])->Length == 0) {
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->FileName = ModuleList[i];
			ProcessList[i]->Start();
			Console::WriteLine("The Process " + ModuleList[i] + ".exe  started");
		}
	}
	//Failure counter for each module
	int LaserFail{ 0 };//Laser
	int DispFail{ 0 };//Display
	int VFail{ 0 };//Vehicle
	int GpsFail{ 0 };//GPS
	int CamFail{ 0 };//Camera

	while (!_kbhit()) {//no kb hit so not shutdown
		while ( PMData->Shutdown.Status != 0xFF) {
			Thread::Sleep(10);
			//Laser Section
			LaserFailure(PMData, LaserFail);
			////Display Section
			DispFailure(PMData, DispFail);
			//////Vehicle Section 
			VehicleFailure(PMData, VFail);
			////GPS Section (Non Critical)
			GPSFailure(PMData, GpsFail, ProcessList);
			////Camera Section (Non Critical)
			CameraFailure(PMData, CamFail, ProcessList);
			if (_kbhit()) {
				PMData->Shutdown.Status = 0xFF;
				break;
			}
		}
		if (PMData->Shutdown.Status == 0xFF) {
			break;
		}
	}
	Console::WriteLine("Please Press a Key to Exit");
	Console::ReadKey();
	PMData->Shutdown.Status = 0xFF;//make sure everything is off
	return 0;
}
