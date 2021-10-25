
#include "Laser.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <array>
int Laser::connect(String^ hostName, int portNumber)
{
// LMS151 port number must be 2111
//String for student Authentication
String^ StudID = gcnew String("z5267217\n");

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
// Get the network streab object associated with client so we can use it to read and write
NetworkStream^ Stream = Client->GetStream();
//Authenticating user
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
String^ AskScan = gcnew String("sRN LMDscandata");
SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);
return 1;
}
int Laser::setupSharedMemory()
{
	// Setting up and accessing the PM Shared memmory
	PMData = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMData->SMAccess();
	PMPtr = (ProcessManagement*)PMData->pData;
	//Setting up and accessing the Laser Shared memory to store data
	SensorData = new SMObject(TEXT("SM_Laser"), sizeof(SM_Laser));
	SensorData->SMAccess();
	LaserPtr = (SM_Laser*)SensorData->pData;
	//initialise the shut down falg of laser to zero (i.e its on)
	PMPtr->Shutdown.Flags.Laser = 0;
	return 1;
}
int Laser::getData()
{
	//Getting the Laser Data
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(200);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	return 1;
}
int Laser::checkData()
{
	// YOUR CODE HERE
	//find length of array and make sure equal to correct value
	array<wchar_t>^ Space = { ' ' };
	array<String^>^ StringArray = ResponseData->Split(Space);//check if this is 386
	double StartAngle = System::Convert::ToInt32(StringArray[23], 16);
	double Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
	int NumRanges = System::Convert::ToInt32(StringArray[25], 16);
	//Laser data is valid if it is 361 elemnets long and first 2 strings are sRA LMDscandata
	if (NumRanges == 361 && (StringArray[0]->EndsWith("sRA")) == TRUE && StringArray[1]->EndsWith("LMDscandata")) {
		return 1;//if laseer data is good return 1
	}
	else {
		return 0;//laser data bad
	}
}
int Laser::sendDataToSharedMemory()
{
	// YOUR CODE HERE
	// Write command asking for data
	//Writing Laser Data onto screen
	array<wchar_t>^ Space = { ' ' };
	array<String^>^ StringArray = ResponseData->Split(Space);//check if this is 386
	double StartAngle = System::Convert::ToInt32(StringArray[23], 16);
	double Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
	int NumRanges = System::Convert::ToInt32(StringArray[25], 16);

	array<double>^ Range = gcnew array<double>(NumRanges);//To store all laser data
	array<double>^ RangeX = gcnew array<double>(NumRanges);//Store x - coords
	array<double>^ RangeY = gcnew array<double>(NumRanges);//Store y - coords

	if (Laser::checkData()==1) {
		for (int i = 0; i < NumRanges; i++) {
			Range[i] = System::Convert::ToInt32(StringArray[26 + i], 16);
			RangeX[i] = Range[i] * sin(i * Resolution * (M_PI / 180));
			RangeY[i] = -Range[i] * cos(i * Resolution * (M_PI / 180));
			LaserPtr->x[i] = RangeX[i];//Assigning x co-ordinate to laser SM
			LaserPtr->y[i] = RangeY[i];//Assigning y co-ordinate to laser SM
			printf("\nX direction: %f\n", LaserPtr->x[i]);//Print x data to laser console
			printf("Y direction: %f\n", LaserPtr->y[i]);//Print y data to laser console
		}
	}
	else {
		//do nothing as invalid data is coming in
	}
	return 1;
}
bool Laser::getShutdownFlag()
{
	// returns current shutdown flag status for laser
	return PMPtr->Shutdown.Flags.Laser;
}
int Laser::getHBFlag(){
	//gets the current laser hb value
	return PMPtr->Heartbeat.Flags.Laser;
}
int Laser::setHeartbeat(bool heartbeat)
{
	// set hb to 1 as it is alive
	PMPtr->Heartbeat.Flags.Laser = heartbeat;
	return 1;	
}
int Laser::ShutDown() {
	//if this is called PM is probably dead or shutdown signal is given
	PMPtr->Shutdown.Status = 0xFF;
	exit(0);
	return 1;
}
int Laser::disconnect() {
	//disconnects from the data stream
	Stream->Close();
	Client->Close();
	return 1;
}
Laser::~Laser()
{
	// Free memmory
	delete PMData;
	delete SendData;
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