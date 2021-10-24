#include  "VehicleControl.h"

int VehicleControl::connect(String^ hostName, int portNumber)
{
	// YOUR CODE HERE
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