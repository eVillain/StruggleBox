//
//  Timer.cpp
//  DungeonSmith
//
//  Created by The Drudgerist on 20/04/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//

#include "Timer.h"

#ifdef __APPLE__
#include <sys/time.h>       // Needed for universal timer
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <time.h>
#endif

#include <sys/stat.h>       // Creating folders
#include <sys/types.h>      // Creating folders
#include <sstream>          // string stream

#if defined(_WIN32)
//========================================================================
// Seconds()/Milliseconds()
// universal timers, rather hackish but working
//========================================================================
long Timer::Microseconds()
{
    __int64 count, freq;
    QueryPerformanceCounter((LARGE_INTEGER*)&count);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    return (long)(1000000 * count / freq);
}
double Timer::Milliseconds()
{
    __int64 count, freq;
    QueryPerformanceCounter((LARGE_INTEGER*)&count);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    return 1000.0*(double)count/(double)freq;
}
double Timer::Seconds()
{
    __int64 count, freq;
    QueryPerformanceCounter((LARGE_INTEGER*)&count);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    return (double)count/(double)freq;
}
std::string Timer::TimeStamp()
{
	time_t rawTime;
	time(&rawTime);
	char buffer[80];

	struct tm timeInfo;
	localtime_s(&timeInfo,&rawTime);

	strftime(buffer, 80, "%A, %B, %d, %Y, %I:%M:%S %p", &timeInfo);

    std::string output(buffer);
    return output;
}
#elif defined __APPLE__
//========================================================================
// Seconds()/Milliseconds()
// universal timers, rather hackish but working
//========================================================================
long Timer::Microseconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec*1000000 + time.tv_usec);
}
double Timer::Milliseconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec*1000.0 + time.tv_usec/1000.0);
}
double Timer::Seconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec + time.tv_usec/1000000.0);
}
std::string Timer::TimeStamp()
{
    struct timeval time;
    time_t nowtime;
    struct tm *nowtm;
    gettimeofday(&time, NULL);
    nowtime = time.tv_sec;
    nowtm = localtime(&nowtime);
    std::ostringstream output;
    output << nowtm->tm_hour << ":" << nowtm->tm_min << ":" << nowtm->tm_sec;
    return output.str();
}
#endif
