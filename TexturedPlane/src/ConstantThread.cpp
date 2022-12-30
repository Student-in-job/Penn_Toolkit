#pragma once
#include "ConstantThread.h"

namespace constthread {

    void BodyMethod()
    {
        DWORD    dwLateStep;                    /* late step count */
        LARGE_INTEGER liPerfFreq;               /* 64 bit frequency */
        LARGE_INTEGER liPerfTemp;               /* used for query */
        UI64 uWait;                             /* tick rate / freq */
        UI64 uRem = 0;                          /* tick rate % freq */
        UI64 uPrev;                             /* previous tick based on original tick */
        UI64 uDelta;                            /* current tick - previous */
        UI64 u2ms;                              /* 2ms of ticks */
        UI64 i;

        /* ... */ /* wait for some event to start thread */
        QueryPerformanceFrequency(&liPerfFreq);
        u2ms = ((UI64)(liPerfFreq.QuadPart) + 499) / ((UI64)500);

        timeBeginPeriod(1);                 /* set period to 1ms */
        Sleep(128);                         /* wait for it to stabilize */

        QueryPerformanceCounter((PLARGE_INTEGER)&liPerfTemp);
        uPrev = liPerfTemp.QuadPart;
        
        while(running){
//        for (i = 0; i < (uFreq * 30); i++) {
            /* update uWait and uRem based on uRem */
            uWait = ((UI64)(liPerfFreq.QuadPart) + uRem) / uFreq;
            uRem = ((UI64)(liPerfFreq.QuadPart) + uRem) % uFreq;
            /* wait for uWait ticks */
            while (1) {
                QueryPerformanceCounter((PLARGE_INTEGER)&liPerfTemp);
                uDelta = (UI64)(liPerfTemp.QuadPart - uPrev);
                if (uDelta >= uWait)
                    break;
                if ((uWait - uDelta) > u2ms)
                    Sleep(1);
            }
            if (uDelta >= (uWait * 2))
                dwLateStep += 1;
            uPrev += uWait;
            /* fixed frequency code goes here */
            /*  along with some type of break when done */
            DoWork();
        }

        timeEndPeriod(1); /* restore period */
    }

    void RunAsync(int frequency)
    {
        uFreq = (UI64)frequency;
        std::thread newThread(BodyMethod);
        newThread.detach();
        running = true;
    }

    //bool isRunning()
    //{
    //    return running;
    //}

    void Stop()
    {
        running = false;
    }
}