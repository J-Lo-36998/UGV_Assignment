#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
#define _USE_MATH_DEFINES
#include <math.h>
#using <System.dll>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
//Counter to keep track of how many times PM Fails
int pmFail{ 0 };

//For use with Time Stamps
double Prev, Next;
__int64 Frequency{}, Counter;
int Shutdown = 0x00;
//for convertion to ms
#define MILSEC 1000
//Time in ms in the loops and also when to check again
#define WAIT_TIME 1000


//Declaring Shared memory
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

int LaserHeartBeat(ProcessManagement* PMData, int &pmFail) {
	//PM is not dead if value of hb Flag reset to zero
	if (PMData->Heartbeat.Flags.Laser == 0) {
		//if pm not dead pmFail variable is reset
		pmFail = 0;
		//printf("%d\n", PMData->Heartbeat.Flags.Laser); //Printing prev value of hb (for checking what PM changed it to) 
		PMData->Heartbeat.Flags.Laser = 1;
		//printf("%d\n", PMData->Heartbeat.Flags.Laser);//Printing new Value of hb flag (For Checking)
		return 0;//return zero if PM still alive
	}
	else {
		//if Pm dead, return 1
		pmFail++;
		return 1;
	}
}

int main() {
	//Instantiating Sharedmemory (Creating and allowing access)
	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;

	// LMS151 port number must be 2111
	int PortNumber = 23000;
	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
	// String command to ask for Channel 1 analogue voltage from the PLC
	// These command are available on Galil RIO47122 command reference manual
	// available online
	String^ AskScan = gcnew String("sRN LMDscandata");
	String^ StudID = gcnew String("z5267217\n");
	// String to store received data for display
	String^ ResponseData;

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	


	// Get the network streab object associated with clien so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	//Authenticate user
	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(StudID);
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	Console::WriteLine(ResponseData);
	//Console::ReadKey();

	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

	while (PMData->Shutdown.Status != 0xFF) {
		//Instantiating the prev time stamp/reset
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		Prev = (double)Counter / (double)Frequency * MILSEC;
		double TimeGap = 0;
		
		// Write command asking for data
		Stream->WriteByte(0x02);
		Stream->Write(SendData, 0, SendData->Length);
		Stream->WriteByte(0x03);
		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);
		// Read the incoming data
		Stream->Read(ReadData, 0, ReadData->Length);
		// Convert incoming data from an array of unsigned char bytes to an ASCII string
		ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
		// Print the received string on the screen
		//Console::WriteLine(ResponseData);
		array<wchar_t>^ Space = { ' ' };
		array<String^>^ StringArray = ResponseData->Split(Space);
		double StartAngle = System::Convert::ToInt32(StringArray[23], 16);
		double Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
		int NumRanges = System::Convert::ToInt32(StringArray[25], 16);

		array<double>^ Range = gcnew array<double>(NumRanges);
		array<double>^ RangeX = gcnew array<double>(NumRanges);
		array<double>^ RangeY = gcnew array<double>(NumRanges);

		for (int i = 0; i < NumRanges; i++) {
			Range[i] =System::Convert::ToInt32(StringArray[26 + i], 16);
			RangeX[i] = Range[i] * sin(i * Resolution * (M_PI/180));
			RangeY[i] = -Range[i] * cos(i * Resolution) * (M_PI / 180);
			printf("\nX direction: %f\n", RangeX[i]);
			printf("Y direction: %f\n", RangeY[i]);
		}
		//For HeartBeats
		printf("%d\n", PMData->Heartbeat.Flags.Laser);
		while (TimeGap <= 4000 && PMData->Shutdown.Status!= 0xFF) {
			//Instantiating next time stamp/reset once gets past 4000ms
			
			QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
			Next = (double)Counter / (double)Frequency * MILSEC;
			TimeGap = Next - Prev;//getting the time gap
			if (LaserHeartBeat(PMData, pmFail) == 0) {
				//Reset value of pmFail if PM still Alive
				pmFail = 0;
				break;
			}
			//If PM is dead come in here and increment pmFail and check at another time stamp
			else if (pmFail > 1000) {
				
				Console::WriteLine("Process Mangement Failure, Critical\n");
				Thread::Sleep(1000);
				PMData->Shutdown.Status = 0xFF;
				break;
			}
		}
		//on shutdown signal exit and close window
		//printf("%d\n", PMData->Heartbeat.Flags.Laser);
		Thread::Sleep(10);
		if (PMData->Shutdown.Status == 0xFF) {
			break;
		}
	}
	Stream->Close();
	Client->Close();

	//Console::ReadKey();
	//Console::ReadKey();

	PMData->Shutdown.Status = 0xFF;
	return 0;
}

