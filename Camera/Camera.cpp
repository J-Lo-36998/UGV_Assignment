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

int CameraHeartBeat(ProcessManagement* PMData, int counter) {
	if (PMData->Heartbeat.Flags.Camera == 0 && counter <= 3) {
		printf("%d", PMData->Heartbeat.Flags.Camera);
		PMData->Heartbeat.Flags.Camera = 1;
		printf("%d", PMData->Heartbeat.Flags.Camera);
		return 0;
	}
	else {
		Thread::Sleep(250);
		return 1;
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

	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	PMObj.SMCreate();
	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	while (1) {
		//QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		//TimeStamp = (double)Counter / (double)Frequency * 1000; //ms
		//Console::WriteLine("Camera time stamps: {0,12:F3} {1, 12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);
		int counter = 0;
		int fail = 0;
		while (counter <= 3) {
			fail += CameraHeartBeat(PMData, counter);
			if (fail > 3) {
				PMData->Shutdown.Status = 0xFF;
			}
			counter++;
		}
		
		if (PMData->Shutdown.Status == 0xFF)
			break;
		if (_kbhit())
			break;
	}

	//glutMainLoop();

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
}

