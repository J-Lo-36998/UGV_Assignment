
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>
#include <string> 
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <tchar.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#endif


#include "Camera.hpp"
#include "Ground.hpp"
#include "KeyManager.hpp"

#include "Shape.hpp"
#include "Vehicle.hpp"
#include "MyVehicle.hpp"

#include "Messages.hpp"
#include "HUD.hpp"
#include "UGV_module.h"

#include <conio.h>
#include <Windows.h>
#using <System.dll>
#include <SMObject.h>
#include <smstructs.h>

void display();
void reshape(int width, int height);
void idle();

void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);
void special_keydown(int keycode, int x, int y);
void special_keyup(int keycode, int x, int y);

void mouse(int button, int state, int x, int y);
void dragged(int x, int y);
void motion(int x, int y);

using namespace std;
using namespace scos;

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
// Used to store the previous mouse location so we
//   can calculate relative mouse movement.
int prev_mouse_x = -1;
int prev_mouse_y = -1;

// vehicle control related variables
Vehicle * vehicle = NULL;
//HUD* myHUD = NULL;
double speed = 0;
double steering = 0;
int setup = 0;
//PM fail counter
int pmFail{ 0 };
//For using time stamps
double Prev, Next;
__int64 Frequency{}, Counter;
int Shutdown = 0x00;
//for convertion to ms
#define MILSEC 1000
//Time in ms in the loops and also when to check again
#define WAIT_TIME 1000

//Declaring Shared memory
//Process Management SM
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
ProcessManagement* PMData;
//Laser SM
SMObject LaserObj(TEXT("SM_Laser"), sizeof(SM_Laser));
SM_Laser* LaserData = (SM_Laser*)LaserObj.pData;
//GPS SM
SMObject GpsObj(TEXT("SM_GPS"), sizeof(SM_GPS));
SM_GPS* GpsData = (SM_GPS*)GpsObj.pData;
//Vehicle SM
SMObject VehicleObj(TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
SM_VehicleControl* VehicleData = (SM_VehicleControl*)VehicleObj.pData;

//Function to change and check the Displays hb 
int DisplayHeartBeat(ProcessManagement* PMData, int &pmFail) {
	//PM is not dead if value of hb Flag reset to zero
	if (PMData->Heartbeat.Flags.OpenGL == 0) {
		PMData->Heartbeat.Flags.OpenGL = 1;//set displays hb flag to 1
		return 0;//return zero if PM still alive
	}
	else {
		return 1;//if Pm dead, return 1
	}
}
//int _tmain(int argc, _TCHAR* argv[]) {
int main(int argc, char ** argv) {
	//Accessing the PM shared memmory
	PMObj.SMAccess();
	
	//Accessing Laser Shared memmory
	LaserObj.SMAccess();

	//Accessing Gps shared memmory
	GpsObj.SMAccess();

	//Accessing Vehicle Shared memory
	VehicleObj.SMAccess();

	PMData = (ProcessManagement*)PMObj.pData;//PM sm
	LaserData = (SM_Laser*)LaserObj.pData;//Laser sm
	GpsData = (SM_GPS*)GpsObj.pData;//GPS sm
	VehicleData = (SM_VehicleControl*)VehicleObj.pData;//vehicle sm

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL");

	Camera::get()->setWindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(special_keydown);
	glutSpecialUpFunc(special_keyup);

	glutMouseFunc(mouse);
	glutMotionFunc(dragged);
	glutPassiveMotionFunc(motion);

	// -------------------------------------------------------------------------
	// Please uncomment the following line of code and replace 'MyVehicle'
	//   with the name of the class you want to show as the current 
	//   custom vehicle.
	// -------------------------------------------------------------------------
	vehicle = new MyVehicle();	

	glutMainLoop();

	if (vehicle != NULL) {
		delete vehicle;
	}
	return 0;
}


void display() {
	// -------------------------------------------------------------------------
	//  This method is the main draw routine. 
	// -------------------------------------------------------------------------

	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(Camera::get()->isPursuitMode() && vehicle != NULL) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		double dx = cos(vehicle->getRotation() * 3.141592765 / 180.0);
		double dy = sin(vehicle->getRotation() * 3.141592765 / 180.0);
		Camera::get()->setDestPos(x + (-3 * dx), y + 7, z + (-3 * dy));
		Camera::get()->setDestDir(dx, -1, dy);
	}
	Camera::get()->updateLocation();
	Camera::get()->setLookAt();
	//Drawing the laser data 
	glPushMatrix();
		vehicle->positionInGL();//set coordinate system to the car
		glTranslatef(0.5, 0, 0);//0.5 is to front of car so data is displayed from the front of car
		glColor3f(1, 1, 1);//Set colour to white
		glBegin(GL_LINES); //Draws lines
		//glVertex3f(0, 0, 0);
		for (int i = 0; i < 361; i++) {
			glVertex3f(LaserData->x[i]/1000, 0, -(LaserData->y[i]/1000)); //draws 1st point line at laser x,y
			glVertex3f(LaserData->x[i]/1000,1, -(LaserData->y[i] / 1000));//draws 2nd point 1 unit up to connect line
		}
		glEnd();//end drawing
	glPopMatrix();

	Ground::draw();
	
	// draw my vehicle
	if (vehicle != NULL) {
		vehicle->draw();

	}
	// draw HUD
	HUD::Draw(GpsData->northing, GpsData->easting, GpsData->height);//Passing in arguments to render gps data on the hud (data obtained with shared mem)
	glutSwapBuffers();
};

void reshape(int width, int height) {

	Camera::get()->setWindowDimensions(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};

double getTime()
{
#if defined(WIN32)
	LARGE_INTEGER freqli;
	LARGE_INTEGER li;
	if(QueryPerformanceCounter(&li) && QueryPerformanceFrequency(&freqli)) {
		return double(li.QuadPart) / double(freqli.QuadPart);
	}
	else {
		static ULONGLONG start = GetTickCount64();
		return (GetTickCount64() - start) / 1000.0;
	}
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + (t.tv_usec / 1000000.0);
#endif
}

void idle() {
	
	if (KeyManager::get()->isAsciiKeyPressed('a')) {
		Camera::get()->strafeLeft();
	}

	if (KeyManager::get()->isAsciiKeyPressed('c')) {
		Camera::get()->strafeDown();
	}

	if (KeyManager::get()->isAsciiKeyPressed('d')) {
		Camera::get()->strafeRight();
	}

	if (KeyManager::get()->isAsciiKeyPressed('s')) {
		Camera::get()->moveBackward();
	}

	if (KeyManager::get()->isAsciiKeyPressed('w')) {
		Camera::get()->moveForward();
	}

	if (KeyManager::get()->isAsciiKeyPressed(' ')) {
		Camera::get()->strafeUp();
	}

	speed = 0;
	steering = 0;

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		steering = Vehicle::MAX_LEFT_STEERING_DEGS * -1;
		VehicleData->Steering = vehicle->getSteering();
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		steering = Vehicle::MAX_RIGHT_STEERING_DEGS * -1;
		VehicleData->Steering = vehicle->getSteering();
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_UP)) {
		speed = Vehicle::MAX_FORWARD_SPEED_MPS;

	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_DOWN)) {
		speed = Vehicle::MAX_BACKWARD_SPEED_MPS;
		
	}
	VehicleData->Steering = vehicle->getSteering();//send steering data to vehicle sm so it can control
	VehicleData->Speed = vehicle->getSpeed();//send speed data to vehicle sm for control
	const float sleep_time_between_frames_in_seconds = 0.025;

	static double previousTime = getTime();
	const double currTime = getTime();
	const double elapsedTime = currTime - previousTime;
	previousTime = currTime;
	
	display();
	//while shutdown command is not given or PM is not dead
	while (PMData->Shutdown.Status != 0xFF) {
		Thread::Sleep(10);
		if (DisplayHeartBeat(PMData, pmFail) == 0) {//if hb flag is zero, pm is alive and working
			pmFail = 0;
			break;
		}
		//If hb = 1 for a while, shut down as PM is dead
		else if (pmFail > 1000) {//PM is Dead, shutdown as critical
			Console::WriteLine("Process Mangement Failure, Critical\n");
			Thread::Sleep(1000);
			PMData->Shutdown.Status = 0xFF;//complete shutdown as PM is critical
		}
		//If display hb flag is still 1, increment pmFail and check again later
		else {
			pmFail++;
		}
	}
	Thread::Sleep(10);
	// do a simulation step
	if (vehicle != NULL) {
		vehicle->update(speed, steering, elapsedTime);
	}
	
#ifdef _WIN32 
	Sleep(sleep_time_between_frames_in_seconds * 1000);
	
#else
	usleep(sleep_time_between_frames_in_seconds * 1e6);
#endif
	//exit display.exe with shutdown signal
	if (PMData->Shutdown.Status == 0xFF) {
		exit(0);
	}
};

void keydown(unsigned char key, int x, int y) {

	// keys that will be held down for extended periods of time will be handled
	//   in the idle function
	KeyManager::get()->asciiKeyPressed(key);

	// keys that react ocne when pressed rather than need to be held down
	//   can be handles normally, like this...
	switch (key) {
	case 27: // ESC key
		exit(0);
		break;      
	case '0':
		Camera::get()->jumpToOrigin();
		break;
	case 'p':
		Camera::get()->togglePursuitMode();
		break;
	}

};

void keyup(unsigned char key, int x, int y) {
	KeyManager::get()->asciiKeyReleased(key);
};

void special_keydown(int keycode, int x, int y) {

	KeyManager::get()->specialKeyPressed(keycode);

};

void special_keyup(int keycode, int x, int y) {  
	KeyManager::get()->specialKeyReleased(keycode);  
};

void mouse(int button, int state, int x, int y) {

};

void dragged(int x, int y) {

	if (prev_mouse_x >= 0) {

		int dx = x - prev_mouse_x;
		int dy = y - prev_mouse_y;

		Camera::get()->mouseRotateCamera(dx, dy);
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
};

void motion(int x, int y) {

	prev_mouse_x = x;
	prev_mouse_y = y;
};


