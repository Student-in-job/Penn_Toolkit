#pragma once
#include <Windows.h>
#include <thread>
typedef unsigned long long UI64; /* unsigned 64 bit int */

namespace constthread
{
	UI64 uFreq;                  /* process frequency */
	bool running = false;
	void EmptyWork() {};
	void (*DoWork)() = &EmptyWork;

	void BodyMethod();
	void RunAsync(int frequency);
	//bool isRunning();
	void Stop();
	//extern void DoWork();
}