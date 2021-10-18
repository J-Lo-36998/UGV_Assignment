#include <zmq.hpp>
#include <Windows.h>
#include "SMStructs.h"
#include "SMFcn.h"
#include "SMObject.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <turbojpeg.h>
#include <conio.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

void display();
void idle();
//PM failure counter
int pmFail{ 0 };
//For Timestamp use
double PrevTime, NextTime;
__int64 Frequency{}, Counter;
int Shutdown = 0x00;
//Declaring Shared Memory
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData;
//for convertion to ms
#define MILSEC 1000
//Time in ms in the loops and also when to check again
#define WAIT_TIME 1000
int CameraHeartBeat(ProcessManagement* PMData, int& pmFail) {
	//PM is not dead if value of hb Flag reset to zero
	if (PMData->Heartbeat.Flags.Camera == 0) {
		//if pm not dead pmFail variable is reset
		pmFail = 0;
		//printf("%d\n", PMData->Heartbeat.Flags.Camera);//Printing prev value of hb (for checking what PM changed it to)
		PMData->Heartbeat.Flags.Camera = 1;
		//printf("%d\n", PMData->Heartbeat.Flags.Camera);//Printing new Value of hb flag(For CHecking)
		return 0;//return zero if PM still alive
	}
	else {
		pmFail++;
		return 1;//if Pm dead, return 1
	}
}

GLuint tex;

//ZMQ settings
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

int main(int argc, char** argv){
	//Define window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;
	//Instantiating Shared Memory
	PMObj.SMCreate();
	PMObj.SMAccess();
	PMData = (ProcessManagement*)PMObj.pData;

	//GL Window setup
	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - Camera");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glGenTextures(1, &tex);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	//Instantiating Shared Memory
	
	glutMainLoop();
	return 1;
}


void display(){
	//Set camera as gl texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Map Camera to window
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();
	glutSwapBuffers();
}
void idle(){

	//receive from zmq
	zmq::message_t update;
	if (subscriber.recv(&update, ZMQ_NOBLOCK))
	{
		//Receive camera data
		long unsigned int _jpegSize = update.size();
		std::cout << "received " << _jpegSize << " bytes of data\n";
		unsigned char* _compressedImage = static_cast<unsigned char*>(update.data());
		int jpegSubsamp = 0, width = 0, height = 0;

		//JPEG Decompression
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char* buffer = new unsigned char[width * height * 3]; //!< will contain the decompressed image
		printf("Dimensions:  %d   %d\n", height, width);
		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);

		//load texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
		delete[] buffer;
	}

	display();
	//Instantiating old time stamp and the declaring time gap (so it restes after every loop)
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	PrevTime = (double)Counter / (double)Frequency * MILSEC;
	double TimeGap = 0;
	//printf("Hiii");
	while (TimeGap <= 5* WAIT_TIME && PMData->Shutdown.Status != 0xFF) {
		//Instantiating next time stamp/reset once gets past 4000ms
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		NextTime = (double)Counter / (double)Frequency * MILSEC;
		//Console::WriteLine("Time Gap is Currently : {0,12:F3}", NextTime - PrevTime);
		TimeGap = NextTime - PrevTime;
		//PMData->Heartbeat.Flags.Laser = 0;
		if (CameraHeartBeat(PMData, pmFail) == 0) {
			pmFail = 0;
			break;
		}
		//If PM is dead come in here and increment pmFail and check at another time stamp
		else if (pmFail > 1000) {
			printf("Process Management Critical Failure: Shutting Down");
			Thread::Sleep(1000);
			PMData->Shutdown.Status = 0xFF;
		}
		else {
			//Do nothing
		}
	}
	if (PMData->Shutdown.Status == 0xFF) {
		exit(0);
	}
}

