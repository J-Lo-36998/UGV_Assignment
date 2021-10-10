#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
#include <array>
#include "UGV_module.h"
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace Net;
using namespace Sockets;
using namespace Text;
int failure{ 0 };

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData;

SMObject LaserObj(TEXT("Laser"), sizeof(ProcessManagement));
ProcessManagement* LaserData;
//
//int LaserHeartBeat(ProcessManagement* PMData) {
//	if (PMData->Heartbeat.Flags.Laser == 0) {
//		printf("%d", PMData->Heartbeat.Flags.Laser);
//		PMData->Heartbeat.Flags.Laser = 1;
//		printf("%d", PMData->Heartbeat.Flags.Laser);
//		return 0;
//	}
//	else {
//		return 1;
//	}
//}

int main() {

	//Tcp client delcaration and initialisation
	//TcpClient^ Client;
	//int PortNumber = 23000;
	//Client = gcnew TcpClient("192.168.1.200", PortNumber);
	//Client->NoDelay = TRUE;
	//Client->ReceiveTimeout = 500;
	//Client->SendTimeout = 500;
	//Client->ReceiveBufferSize = 1024;
	//Client->SendBufferSize = 1024;
	//array<unsigned char>^ SendData;
	//SendData = gcnew array<unsigned char>(1024);

	//SM_VehicleControl Data;

	//String^ Message;
	////Sending Data
	//Message = gcnew String("#");
	//Message = Message + Data.Steering.ToString("F3") + " " + Data.Speed.ToString("F3") + "1 #";
	//
	//SendData = Encoding::ASCII->GetBytes(Message);


	////Send to Server
	//SM_Laser LaserData;
	//unsigned char* BytePtr;
	//BytePtr = (unsigned char*)(&LaserData);
	//for (int i{ 0 }; i < sizeof(SM_Laser); i++) {
	//	SendData[i] = *(BytePtr + i);
	//}
	//NetworkStream^ Stream = Client->GetStream();
	//Stream->Write(SendData, 0, SendData->Length);

	//Shared memory Declaration and initialisation
	

	PMObj.SMCreate();
	PMObj.SMAccess();
	LaserObj.SMCreate();
	LaserObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;
	LaserData = (ProcessManagement*)LaserObj.pData;
	while (1) {
		Thread::Sleep(25);
		//Did PM put my flag down?
			//true-> put flag up
			//false-> is the pm time stamp older by agreed time gap
				//True->shutdown all
				//
		//Console::WriteLine("Laser time stamps: {0,12:F3} {1, 12:X2}", TimeStamp, Shutdown);
		//PMData->Heartbeat.Flags.Laser = 0;
		if (LaserData->Heartbeat.Flags.Laser == 0 ) {
			printf("%d", LaserData->Heartbeat.Flags.Laser);
			LaserData->Heartbeat.Flags.Laser = 1;
			printf("%d", LaserData->Heartbeat.Flags.Laser);
			
		}
		if (LaserData->Heartbeat.Flags.ProcessManagement == 1) {
			LaserData->Heartbeat.Flags.ProcessManagement = 0;
			failure = 0;
		}
		else {
			failure++;
			if (failure > 10) {
				PMData->Shutdown.Status = 0xFF;
			}
		}

		

		if (PMData->Shutdown.Flags.Laser == 1)
			break;
		if (_kbhit())
			break;
	}
	return 0;
}
