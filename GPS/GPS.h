
#include <UGV_module.h>
#include <smstructs.h>

#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);
#pragma pack(1)
struct GPSstruct
{
	unsigned int Header;
	unsigned char Discards1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];
	unsigned int Checksum;
};
ref class GPS : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int getHBFlag()override;
	int setHeartbeat(bool heartbeat) override;
	int GPS::ShutDown() override;
	~GPS();

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	//unsigned char* BPtr;
	ProcessManagement* PMPtr;
	String^ ResponseData;
	SM_GPS* GpsPtr;
	GPSstruct* GPSdata;
	int Start;
};


