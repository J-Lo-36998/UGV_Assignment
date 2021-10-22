#include "GPS.h"
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
int GPS::connect(String^ hostName, int portNumber) 
{
	// YOUR CODE HERE
	// LMS151 port number must be 2111
// Pointer to TcpClent type object on managed heap

// arrays of unsigned chars to send and receive data
// String command to ask for Channel 1 analogue voltage from the PLC
// These command are available on Galil RIO47122 command reference manual
// available online

	
	// String to store received data for display
	
	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient(hostName, portNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 1500;//ms
	Client->SendTimeout = 1500;//ms
	Client->ReceiveBufferSize = 2048;
	Client->SendBufferSize = 1024;
	ReadData = gcnew array<unsigned char>(7500);
	//declaring stream
	Stream = Client->GetStream();
	// unsigned char arrays of 16 bytes each are created on managed heap
	
	// Get the network streab object associated with clien so we 
	// can use it to read and writeNetworkStream^ Stream = Client->GetStream();
	//Authenticate user
	// Convert string command to an array of unsigned char
	
	//Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	//Stream->Read(ReadData, 0, ReadData->Length);
	//ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	//Console::WriteLine(ResponseData);

	return 1;
}
int GPS::setupSharedMemory() 
{
	// YOUR CODE HERE
	PMData = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMData->SMAccess();
	PMPtr = (ProcessManagement*)PMData->pData;

	SensorData = new SMObject(TEXT("SM_GPS"), sizeof(SM_GPS));
	SensorData->SMAccess();
	GpsPtr = (SM_GPS*)SensorData->pData;

	PMPtr->Shutdown.Flags.GPS = 0;
	return 1;
}
int GPS::getData() 
{
	// YOUR CODE HERE
	Thread::Sleep(100);
	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
	}
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	return 1;
}
int GPS::checkData() 
{
	// YOUR CODE HERE
	return 1;
}
int GPS::sendDataToSharedMemory() 
{
	// YOUR CODE HERE
	BPtr = (unsigned char*)GpsPtr;
	for (int i = 0; i < 112; i++) {
		Console::WriteLine("{0, 4:X2}", ReadData[i]);
		*(BPtr++) = ReadData[i];
	}
	Console::WriteLine("Northing: {0, 12:F3}", GpsPtr->northing);
	Console::WriteLine("Easting: {0, 12:F3}", GpsPtr->easting);
	Console::WriteLine("Height: {0, 12:F3}", GpsPtr->height);
	return 1;
}
bool GPS::getShutdownFlag() 
{
	// YOUR CODE HERE
	return PMPtr->Shutdown.Flags.GPS;
}
int GPS::getHBFlag() {
	return PMPtr->Heartbeat.Flags.GPS;
}
int GPS::setHeartbeat(bool heartbeat) 
{
	// YOUR CODE HERE
	heartbeat = 1;
	PMPtr->Heartbeat.Flags.GPS = heartbeat;
	return 1;
}
int GPS::ShutDown(){
	PMPtr->Shutdown.Status = 0xFF;
	exit(0);
	return 1;
}
GPS::~GPS()
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