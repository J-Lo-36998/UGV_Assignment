#include  "VehicleControl.h"

int VehicleControl::connect(String^ hostName, int portNumber)
{
	// YOUR CODE HERE
	// LMS151 port number must be 2111
// Pointer to TcpClent type object on managed heap

// arrays of unsigned chars to send and receive data
// String command to ask for Channel 1 analogue voltage from the PLC
// These command are available on Galil RIO47122 command reference manual
// available online

	Client = gcnew TcpClient(hostName, portNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 1500;//ms
	Client->SendTimeout = 1500;//ms
	Client->ReceiveBufferSize = 2048;
	Client->SendBufferSize = 512;
	//declaring stream
	Stream = Client->GetStream();
	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(64);
	ReadData = gcnew array<unsigned char>(2048);
	String^ StudID = gcnew String("5267217\n");
	// Creat TcpClient object and connect to it
	// Get the network streab object associated with clien so we 
	// can use it to read and write
	
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
	return 1;
}
int VehicleControl::setupSharedMemory()
{
	// YOUR CODE HERE
	PMData = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMData->SMAccess();
	PMPtr = (ProcessManagement*)PMData->pData;

	SensorData = new SMObject(TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
	SensorData->SMAccess();
	VehiclePtr = (SM_VehicleControl*)SensorData->pData;

	PMPtr->Shutdown.Flags.VehicleControl = 0;
	return 1;
}
int VehicleControl::getData()
{
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::checkData()
{
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::sendDataToSharedMemory()
{
	// YOUR CODE HERE
	
	
	return 1;
}
void VehicleControl::controls() {
	VehiclePtr->flag = !VehiclePtr->flag;
	String^ vehicleControl = gcnew String("#	" + VehiclePtr->Steering + "	" + VehiclePtr->Speed + "		" + VehiclePtr->flag + "	#");
	Console::WriteLine(vehicleControl);
	SendData = System::Text::Encoding::ASCII->GetBytes(vehicleControl);
	Stream->Write(SendData, 0, SendData->Length);
	System::Threading::Thread::Sleep(10);
}
bool VehicleControl::getShutdownFlag()
{
	// YOUR CODE HERE
	return PMPtr->Shutdown.Flags.VehicleControl;
}
int VehicleControl::getHBFlag() {
	return PMPtr->Heartbeat.Flags.VehicleControl;
}
int VehicleControl::setHeartbeat(bool heartbeat)
{
	// YOUR CODE HERE
	heartbeat = 1;
	PMPtr->Heartbeat.Flags.VehicleControl = heartbeat;
	return 1;
}
int VehicleControl::ShutDown() {
	PMPtr->Shutdown.Status = 0xFF;
	exit(0);
	return 1;
}
VehicleControl::~VehicleControl()
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