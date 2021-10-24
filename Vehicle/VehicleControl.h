#include <UGV_module.h>
#include <smstructs.h>

#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

ref class VehicleControl : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	void VehicleControl::controls() override;
	bool getShutdownFlag() override;
	int VehicleControl::getHBFlag() override;
	int setHeartbeat(bool heartbeat) override;
	int VehicleControl::ShutDown() override;
	~VehicleControl();

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	SM_VehicleControl* VehiclePtr;
	ProcessManagement* PMPtr;
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
	String^ ResponseData;
	TcpClient^ Client;
	NetworkStream^ Stream;
};


