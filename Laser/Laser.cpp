
#include "Laser.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <array>
int Laser::connect(String^ hostName, int portNumber)
{
// LMS151 port number must be 2111
// Pointer to TcpClent type object on managed heap

// arrays of unsigned chars to send and receive data
// String command to ask for Channel 1 analogue voltage from the PLC
// These command are available on Galil RIO47122 command reference manual
// available online

String^ StudID = gcnew String("z5267217\n");
// String to store received data for display
String^ ResponseData;
// Creat TcpClient object and connect to it
Client = gcnew TcpClient(hostName, portNumber);
// Configure connection
Client->NoDelay = true;
Client->ReceiveTimeout = 1500;//ms
Client->SendTimeout = 1500;//ms
Client->ReceiveBufferSize = 2048;
Client->SendBufferSize = 1024;
//declaring stream
Stream = Client->GetStream();
// unsigned char arrays of 16 bytes each are created on managed heap
SendData = gcnew array<unsigned char>(64);
ReadData = gcnew array<unsigned char>(7500);
// Get the network streab object associated with clien so we 
// can use it to read and write
NetworkStream^ Stream = Client->GetStream();
//Authenticate user
// Convert string command to an array of unsigned char
SendData = System::Text::Encoding::ASCII->GetBytes(StudID);
Stream->Write(SendData, 0, SendData->Length);
// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
System::Threading::Thread::Sleep(200);
// Read the incoming data
Stream->Read(ReadData, 0, ReadData->Length);
ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
// Print the received string on the screen
Console::WriteLine(ResponseData);
//Console::ReadKey();
return 1;
}
int Laser::setupSharedMemory()
{
	// YOUR CODE HERE
	PMData = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	
	PMData->SMAccess();
	PMPtr = (ProcessManagement*)PMData->pData;

	SensorData = new SMObject(TEXT("SM_Laser"), sizeof(SM_Laser));
	SensorData->SMAccess();
	LaserPtr = (SM_Laser*)SensorData->pData;

	PMPtr->Shutdown.Flags.Laser = 0;
	return 1;
}
int Laser::getData()
{
	String^ AskScan = gcnew String("sRN LMDscandata");
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);
	return 1;
}
int Laser::checkData()
{
	// YOUR CODE HERE
	//find length of array and make sure equal to correct value
	return 1;
}
int Laser::sendDataToSharedMemory()
{
	// YOUR CODE HERE
	// Write command asking for data
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(200);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	//Writing Laser Data onto screen
	array<wchar_t>^ Space = { ' ' };
	array<String^>^ StringArray = ResponseData->Split(Space);//check if this is 386
	double StartAngle = System::Convert::ToInt32(StringArray[23], 16);
	double Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
	int NumRanges = System::Convert::ToInt32(StringArray[25], 16);

	array<double>^ Range = gcnew array<double>(NumRanges);
	array<double>^ RangeX = gcnew array<double>(NumRanges);
	array<double>^ RangeY = gcnew array<double>(NumRanges);

	//Console::WriteLine(StringArray->Length);
	if (NumRanges == 361 && (StringArray[0]->EndsWith("sRA")) == TRUE && StringArray->Length == 393) {
		for (int i = 0; i < NumRanges; i++) {
			Range[i] = System::Convert::ToInt32(StringArray[26 + i], 16);
			RangeX[i] = Range[i] * sin(i * Resolution * (M_PI / 180));
			RangeY[i] = -Range[i] * cos(i * Resolution) * (M_PI / 180);
			LaserPtr->x[i] = RangeX[i];
			LaserPtr->y[i] = RangeY[i];
			printf("\nX direction: %f\n", LaserPtr->x[i]);
			printf("Y direction: %f\n", LaserPtr->y[i]);
		}
	}
	else {
	//do nothing /skip
		//Console::WriteLine(NumRanges);
	}
	return 1;
}
bool Laser::getShutdownFlag()
{
	// YOUR CODE HERE
	return PMPtr->Shutdown.Status;
}
int Laser::ShutDownSignal() {
	return PMPtr->Shutdown.Status;
}
int Laser::getHBFlag(){
	return PMPtr->Heartbeat.Flags.Laser;
}
int Laser::setHeartbeat(bool heartbeat)
{
	heartbeat = 1;
	PMPtr->Heartbeat.Flags.Laser = heartbeat;
	return 1;
	// YOUR CODE HERE
	
}
int Laser::ShutDown() {
	PMPtr->Shutdown.Status = 0xFF;
	exit(0);
	return 1;
}
int Laser::disconnect() {
	Stream->Close();
	Client->Close();
	return 1;
}
Laser::~Laser()
{
	// YOUR CODE HERE
}

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}