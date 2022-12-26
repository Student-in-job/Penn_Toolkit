#pragma once
#include <NIDAQmx.h>

#define MAX_VALUES 6
//#define ASYNC_READ 1;

int32 CVICALLBACK DataAquisitionLoop();
#ifdef ASYNC_READ
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData);
extern void DoAction(float* data);
#endif
int32 CVICALLBACK DataGenerationLoop();
int32 CVICALLBACK DataWriteLoop();
void GetData(float(&data)[MAX_VALUES]);
void WriteData(double value);
int StartGenerationLoop();
int StartWriteLoop();
int StartReadLoop();
int StopLoop();