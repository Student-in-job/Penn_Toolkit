#include "libDAQmx.h"
#include <string>
#include <thread>

#define DAQNAME "Dev3/"
#define CONCAT_DEVICE(p)    (char*)((DAQNAME + std::string(p)).c_str())
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

// Global variables for NI DAQ
static TaskHandle taskHandle;

float64	      voltage[1000];
extern double outData[1000];
extern bool   running;
bool          started;
const char*   outChannel = CONCAT_DEVICE("ai0");

// Extern Functions declaration
extern void SetProgress(const char* value);
extern void SetError(const char* value);

int32 CVICALLBACK DataAquisitionLoop()
{
    int32   error = 0;
    int32   read = 0;
    char    errBuff[2048] = { '\0' };

    SetProgress("Reading data from DAQ channel");
    DAQmxErrChk(DAQmxCreateTask("Read", &taskHandle));
    for (int index = 0; index < MAX_VALUES; index++)
    {
        std::string channel = "ai" + std::to_string(index);
        DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, CONCAT_DEVICE(channel), "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));
    }

#ifdef ASYNC_READ
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 1000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
    DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, NULL));
    DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));
#endif

    SetProgress("Reading voltage continuously. Press Enter to interrupt");
    running = true;
    while (started){}
    
    Error:
    if (DAQmxFailed(error))
    {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        SetError(errBuff);
    }
    if (taskHandle != 0)
    {
        /*********************************************/
        // DAQmx Stop Code
        /*********************************************/
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
    }
    running = false;
    return 0;
}

#ifdef ASYNC_READ
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData)
{
    int32       error = 0;
    char        errBuff[2048] = { '\0' };
    static int  totalRead = 0;
    int32       read = 0;

    /*********************************************/
    // DAQmx Read Code
    /*********************************************/
    DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, voltage, MAX_VALUES, &read, NULL));
    if (read > 0)
    {
        float volFloat[MAX_VALUES];
        for (int index = 0; index < MAX_VALUES; index++) {
            volFloat[index] = voltage[index];
        }
        DoAction(volFloat);
    }

    Error:
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        /*********************************************/
        // DAQmx Stop Code
        /*********************************************/
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
        SetError(errBuff);
        running = false;
        return 0;
    }
    return 1;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData)
{
    int32   error = 0;
    char    errBuff[2048] = { '\0' };

    // Check to see if an error stopped the task.
    DAQmxErrChk(status);

Error:
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        DAQmxClearTask(taskHandle);
        SetError(errBuff);
    }
    return 0;
}
#endif

int32 CVICALLBACK DataGenerationLoop()
{
    int			error = 0;
    char		errBuff[2048] = { '\0' };
    int         i = 1;
    TaskHandle	hTaskHandle = 0;

    /*********************************************/
    // DAQmx Configure Code
    /*********************************************/
    DAQmxErrChk(DAQmxCreateTask("Loop", &hTaskHandle));
    DAQmxErrChk(DAQmxCreateAOVoltageChan(hTaskHandle, outChannel, "", -10.0, 10.0, DAQmx_Val_Volts, ""));
    /*********************************************/
    // DAQmx Start Code
    /*********************************************/
    DAQmxErrChk(DAQmxStartTask(hTaskHandle));

    //SetProgress(L"Writing data to DAQ channel");
    running = true;
    while (started)
    {
        if (i == 1000) { i = 0; }
        /*********************************************/
        // DAQmx Write Code
        /*********************************************/
        DAQmxErrChk(DAQmxWriteAnalogScalarF64(hTaskHandle, 1, -1, outData[i++], NULL));
    }

    Error:
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        SetError(errBuff);
        running = false;
    }
    if (hTaskHandle != 0) {
        /*********************************************/
        // DAQmx Stop Code
        /*********************************************/
        DAQmxStopTask(hTaskHandle);
        DAQmxClearTask(hTaskHandle);
    }    

    SetProgress("Finished writing data");
    running = false;
    return 1;
}

int32 CVICALLBACK DataWriteLoop()
{
    char    errBuff[2048] = { '\0' };
    int32   error = 0;

    /*********************************************/
    // DAQmx Configure Code
    /*********************************************/
    DAQmxErrChk(DAQmxCreateTask("out", &taskHandle));
    DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandle, outChannel, "", -10.0, 10.0, DAQmx_Val_Volts, ""));
    /*********************************************/
    // DAQmx Start Code
    /*********************************************/
    DAQmxErrChk(DAQmxStartTask(taskHandle));

    //SetProgress(L"Writing data to DAQ channel");
    running = true;
    while (started) {}
    
    Error:
    if (DAQmxFailed(error))
    {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        SetError(errBuff);
    }
    if (taskHandle != 0) {
        /*********************************************/
        // DAQmx Stop Code
        /*********************************************/
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
    }
    running = false;
    SetProgress("Finished writing data");
    return 1;
}

void GetData(float(&data)[MAX_VALUES])
{
    int32       error = 0;
    char        errBuff[2048] = { '\0' };
    static int  totalRead = 0;
    int32       read = 0;

    /*********************************************/
    // DAQmx Read Code
    /*********************************************/
    DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, voltage, MAX_VALUES, &read, NULL));

    if (read > 0)
    {
        float volFloat[MAX_VALUES];
        for (int index = 0; index < MAX_VALUES; index++) {
            data[index] = voltage[index];
        }
    }

Error:
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        DAQmxClearTask(taskHandle);
        SetError(errBuff);
        running = false;
    }
}

void WriteData(double value)
{
    int			error = 0;
    char		errBuff[2048] = { '\0' };
    
    /*********************************************/
    // DAQmx Write Code
    /*********************************************/
    DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandle, 1, -1, value, NULL));
    Error:
    if (DAQmxFailed(error))
    {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        SetError(errBuff);
    }
}

int StartGenerationLoop()
{
    if (!running)
    {
        started = true;
        std::thread Thread(DataGenerationLoop);
        Thread.detach();
        return 1;
    }
    else
        return 0;
}

int StartWriteLoop()
{
    if (!running)
    {
        started = true;
        std::thread Thread(DataWriteLoop);
        Thread.detach();
        return 1;
    }
    else
        return 0;
}

int StartReadLoop()
{
    if (!running)
    {
        started = true;
        std::thread Thread(DataAquisitionLoop);
        Thread.detach();
        return 1;
    }
    else
        return 0;
}

int StopLoop()
{
    started = false;
    while(running){ }
    return 1;
}