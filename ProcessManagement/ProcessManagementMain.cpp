#using <System.dll>

#include <conio.h>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {
	//Tele-operation
	array<String^>^ ModuleList = gcnew array<String^> {"Laser", "Display", "Vehicle", "GPS", "Camera"};
	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 1, 1, 0, 0 };
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

	for (int i = 0; i < ModuleList->Length; i++) {
		if (Process::GetProcessesByName(ModuleList[i])->Length == 0) {
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->FileName = ModuleList[i];
			ProcessList[i]->Start();
			Console::WriteLine("The process " + ModuleList[i] + ".exe started");
		}
	}
	return 0;
