
#include <UGV_module.h>
#include <smstructs.h>

#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

ref class Laser : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int ShutDownSignal() override;
	int getHBFlag() override;
	int setHeartbeat(bool heartbeat) override;
	int ShutDown() override;
	int disconnect() override;
	~Laser();

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	ProcessManagement* PMPtr;
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
	String^ ResponseData;
	TcpClient^ Client;
	NetworkStream^ Stream;
};


