#include "GPS.h"
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
int GPS::connect(String^ hostName, int portNumber) 
{
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
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	return 1;
}
int GPS::setupSharedMemory() 
{
	// YOUR CODE HERE
	//Declaring and accessing PM Shared memory
	PMData = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMData->SMAccess();
	PMPtr = (ProcessManagement*)PMData->pData;
	//Declaring and accessing GPS Shared memory
	SensorData = new SMObject(TEXT("SM_GPS"), sizeof(SM_GPS));
	SensorData->SMAccess();
	GpsPtr = (SM_GPS*)SensorData->pData;
	//Initialising shutdown flag to zero
	PMPtr->Shutdown.Flags.GPS = 0;
	return 1;
}
int GPS::getData() 
{
	Thread::Sleep(100);
	//if data is coming in for checking
	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
		//To find the header in the data string
		unsigned int dataHeader = 0;
		unsigned char Data;
		int i = 0;
		do //finds the header 0xaa44121c
		{
			Data = ReadData[i++]; //gets next char in data array
			dataHeader = ((dataHeader << 8) | Data);//each time shifts by 2 hex characters which is 8 bits to check for header again
		} 
		while (dataHeader != 0xaa44121c);//checks if data header is the same as the header of raw data
		Start = i - 4; //go back to begining of header/first element (starting point of data array)
	}
	return 1;
}
int GPS::checkData() 
{
	// YOUR CODE HERE
	return 1;
}
int GPS::sendDataToSharedMemory() 
{
	GPSdata = new GPSstruct;
	unsigned char* BPtr = nullptr;
	BPtr = (unsigned char*)GPSdata;//Pointer to beginning of data location
	//Iterate throught data and add to pointer to GPS data struct
	for (int i = Start; i < Start + sizeof(GPSstruct); i++) {
		*(BPtr++) = ReadData[i];
	}
	//checks if the checksum is equal to the CRC32 value, if it is then data is good
	if (GPSdata->Checksum == CalculateBlockCRC32(108,(unsigned char*)GPSdata)) {
		GpsPtr->northing = GPSdata->Northing;//Assigns the Northing data to SM
		GpsPtr->easting = GPSdata->Easting; //Assign easting data to SM
		GpsPtr->height = GPSdata->Height;//Assign height to SM

		//Print norhting to console
		Console::WriteLine("Northing: {0, 12:F3}", GpsPtr->northing);
		//Print easting to console
		Console::WriteLine("Easting: {0, 12:F3}", GpsPtr->easting);
		//Print height to console
		Console::WriteLine("Height: {0, 12:F3}", GpsPtr->height);
		//write checksum value
		Console::WriteLine("Check Sum Value is:	{0, 12:F3}", GPSdata->Checksum);
		//Writes cec32 valute for comparison
		Console::WriteLine("CRC32 value is:		{0, 12:F3}", CalculateBlockCRC32(108, (unsigned char*)GPSdata));
	}
	return 1;
}
bool GPS::getShutdownFlag() 
{
	//returns shutdown flag status
	return PMPtr->Shutdown.Flags.GPS;
}
int GPS::getHBFlag() {
	//returns hearbeat flag
	return PMPtr->Heartbeat.Flags.GPS;
}
int GPS::setHeartbeat(bool heartbeat) 
{
	//sets heartbeat
	PMPtr->Heartbeat.Flags.GPS = heartbeat;
	return 1;
}
int GPS::ShutDown(){
	//if this is called either PM is dead or shutdown command given so exit all and shutdown all
	PMPtr->Shutdown.Status = 0xFF;
	exit(0);
	return 1;
}
GPS::~GPS()
{
	// Free memmory
	delete PMData;
	delete SensorData;
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