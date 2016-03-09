//
//  SysCore.cpp
//  NeverEngine
//
//  Created by Ville-Veikko Urrila on 8/28/12.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifdef __APPLE__
#include <mach-o/dyld.h>    // Needed for application path
#include <sys/time.h>       // Needed for universal timer
#include <unistd.h>
#include <dirent.h>         // Needed for directory structures
#endif

#ifdef _WIN32
#include <windows.h>
#include <iostream>
//#include <stringapiset.h>
#include <xstring>
#endif

#include <sys/stat.h>       // Creating folders
#include <sys/types.h>      // Creating folders
#include <sstream>          // string stream

#include "SysCore.h"

namespace SysCore {
    
    std::string relativePath = "NO_PATH";
    std::string GetPath() { return relativePath; };
    uint32_t rand_x[5];                                  // Mother of all randoms history buffer

#if defined(_WIN32)    
	// Convert a wide Unicode string to an UTF8 string
    std::string utf8_encode(const std::wstring &wstr)
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo( size_needed, 0 );
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }
    // Convert an UTF8 string to a wide Unicode String
    std::wstring utf8_decode(const std::string &str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo( size_needed, 0 );
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
	// Convert widechars to string
	std::string wchar_t2string(const wchar_t *wchar)
{
    std::string str = "";
    int index = 0;
    while(wchar[index] != 0)
    {
        str += (char)wchar[index];
        ++index;
    }
    return str;
}
void string2wchar_t(const std::string &str,  wchar_t* wchar)
//wchar_t *string2wchar_t(const std::string &str,  wchar_t* wchar)
{
//    wchar_t wchar[260];
    unsigned int index = 0;
    while(index < str.size())
    {
        wchar[index] = (wchar_t)str[index];
        ++index;
    }
    wchar[index] = 0;
//    return wchar;
}
	//========================================================================
    // SetRelativePath()
    // Puts the path for the .app (OSX) or .exe (Windows) into a string
    //========================================================================
    void SetRelativePath()
    {
        TCHAR path[MAX_PATH];
        HINSTANCE hInstance = GetModuleHandle(NULL);
        int pathLength = GetModuleFileName(hInstance, path, MAX_PATH);
		if (pathLength != 0) {
			std::wstring widePath( path, pathLength );
			std::string utf8Path = utf8_encode( widePath );
            // Cull executable name from path
            int found=utf8Path.find_last_of("/\\") + 1; //leave last forwardslash
			relativePath = utf8Path.substr(0,found);
			printf( "runtime path: %s \n", relativePath.c_str() );
		} else {
			relativePath = "FAILTRAIN, ALL ABOARD!";
            printf("SysCore couldn't get path, length returned 0");
		}
    }
    
    
    //========================================================================
    // GetSeconds()/GetMilliseconds()
    // universal timers, rather hackish but working
    //========================================================================
	long GetMicroseconds() {
		__int64 count, freq;
		QueryPerformanceCounter((LARGE_INTEGER*)&count);
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		return (long)(1000000 * count / freq);
	}
    double GetMilliseconds() {
        __int64 count, freq;
        QueryPerformanceCounter((LARGE_INTEGER*)&count);
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        return 1000.0*(double)count/(double)freq;
    }
    double GetSeconds() {
        __int64 count, freq;
        QueryPerformanceCounter((LARGE_INTEGER*)&count);
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        return (double)count/(double)freq;
    }
    std::string GetTimeStamp() {
        time_t tTime = time(NULL);
//        std::string output(asctime(localtime(&tTime)));
        std::string output(strftime(localtime(&tTime)));
        return output;
    }
    //========================================================================
    // GetFiles()
    // retrieves list of files in dir
    //========================================================================
    bool GetFiles(const std::string dir, std::vector<std::string> &fileNames)
    {
        WIN32_FIND_DATA FindFileData;
        wchar_t FileName[260];
		string2wchar_t(dir, FileName);
        HANDLE hFind = FindFirstFile(FileName, &FindFileData);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        fileNames.push_back(wchar_t2string(FindFileData.cFileName));
        
        while (FindNextFile(hFind, &FindFileData))
            fileNames.push_back(wchar_t2string(FindFileData.cFileName));
        
        return true;
    }
    //========================================================================
    // GetAllFiles()
    // retrieves list of all files in dir
    //========================================================================
    bool GetAllFiles(const std::string dir, std::vector<std::string> &fileNames)
    {
        std::string directory = dir;
        directory.append("*");
        return GetFiles(directory, fileNames);
    }
    //========================================================================
    // GetFilesOfType()
    // retrieves list of files with name ending in type in dir
    //========================================================================
    bool GetFilesOfType(const std::string dir, const std::string type, std::vector<std::string> &fileNames)
    {
        std::string directory = dir;
		directory.append("*");
        directory.append(type);
        return GetFiles(directory, fileNames);;
    }
	//========================================================================
	// DoesFolderExist()
	// checks if folder at path exists
	//========================================================================
	bool DoesFolderExist(const std::string dir) {
		DWORD ftyp = GetFileAttributesA(dir.c_str());
		return (ftyp != INVALID_FILE_ATTRIBUTES &&
			(ftyp & FILE_ATTRIBUTE_DIRECTORY));
	}
	//========================================================================
	// CreateFolder()
	// Creates folder at path unless exists
	//========================================================================
	bool CreateFolder(const std::string dir) {
		if (DoesFolderExist(dir)) return false;

		wchar_t FileName[260];
		string2wchar_t(dir, FileName);

		CreateDirectory(FileName, NULL);
		return true;
	}
	//========================================================================
    // DoesFileExist()
    // checks if file at path exists
    //========================================================================
    bool DoesFileExist( const std::string dir, const std::string fileName )
    {
		std::string dirAndName = dir;
		dirAndName.append( fileName );

        WIN32_FIND_DATA FindFileData;
        //wchar_t * FilePath = string2wchar_t(dirAndName);
        wchar_t FilePath[260];
		string2wchar_t(dirAndName, FilePath);
        HANDLE hFind = FindFirstFile(FilePath, &FindFileData);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        return true;
    }

#elif defined __APPLE__
    //========================================================================
    // SetRelativePath()
    // Puts the path for the .app (OSX) or .exe (Windows) into a string
    //========================================================================
    void SetRelativePath()
    {
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            std::string str(path);
            // Cull last few folders
            size_t found=str.find_last_of("/\\");
            found = str.substr(0,found).find_last_of("/\\");
            found = str.substr(0,found).find_last_of("/\\");
            found = str.substr(0,found).find_last_of("/\\") + 1; //leave last forwardslash
            //printf("relative path is %s, %i\n", str.substr(0,found).c_str() , found);
            relativePath = str.substr(0,found);
        } else {
            relativePath = "FAILTRAIN, ALL ABOARD!";
            printf("SysCore couldn't get path, buffer too small; need size %u\n", size);
        }
    }
    //========================================================================
    // GetAllFiles()
    // retrieves list of all files in dir
    //========================================================================
    bool GetAllFiles(const std::string dir, std::vector<std::string> &fileNames)
    {
        DIR *dp;
        struct dirent *dirp;
        if((dp  = opendir(dir.c_str())) == NULL) {
            printf("Error opening directory: %s\n", dir.c_str() );
            return false;
        }
        
        while ((dirp = readdir(dp)) != NULL) {
            fileNames.push_back(std::string(dirp->d_name));
        }
        closedir(dp);
        return true;
    }
    //========================================================================
    // GetFilesOfType()
    // retrieves list of files with name ending in type in dir
    //========================================================================
    bool GetFilesOfType(const std::string dir, const std::string type, std::vector<std::string> &fileNames)
    {
        DIR *dp;
        struct dirent *dirp;
        if((dp  = opendir(dir.c_str())) == NULL) {
            printf("Error opening directory: %s\n", dir.c_str() );
            return false;
        }
        size_t typeLen = type.length();
        if ( typeLen > 0 ) {
            while ((dirp = readdir(dp)) != NULL) {
                size_t fileNameLen = strlen(dirp->d_name);
                if ( fileNameLen > typeLen ) {
                    const char *endChars = &dirp->d_name[fileNameLen-typeLen];
                    if ( strcmp( type.c_str(), endChars ) == 0 ) {
                        fileNames.push_back(std::string(dirp->d_name));
                    }
                }
            }
        }
        closedir(dp);
        return true;
    }
    //========================================================================
    // DoesFolderExist()
    // checks if folder at path exists
    //========================================================================
    bool DoesFolderExist( const std::string dir ) {
        DIR *dp;
        if((dp  = opendir( dir.c_str())) == NULL) {
            printf("Can't open directory: %s\n", dir.c_str() );
            return false;
        } else {
            closedir(dp);
        }
        return true;
    }
    //========================================================================
    // DoesFileExist()
    // checks if file at path exists
    //========================================================================
    bool DoesFileExist( const std::string dir, const std::string fileName ) {
        DIR *dp;
        struct dirent *dirp;
        if((dp  = opendir( dir.c_str())) == NULL) {
            printf("Can't open directory: %s\n", dir.c_str() );
            return false;
        } else {
            size_t nameLen = strlen( fileName.c_str() );
            if ( nameLen > 0 ) {
                while ((dirp = readdir(dp)) != NULL) {
                    size_t fileNameLen = strlen(dirp->d_name);
                    if ( fileNameLen == nameLen ) {
                        if ( strcmp( fileName.c_str(), dirp->d_name ) == 0 ) {
                            closedir(dp);
                            return true;
                        }
                    }
                }
            }
            closedir(dp);
        }
        return false;
    }
    //========================================================================
    // CreateFolder()
    // Creates folder at path unless exists
    //========================================================================
    bool CreateFolder( const std::string dir ) {
        DIR *dp;
        if( (dp = opendir( dir.c_str())) == NULL ) {
            mode_t process_mask = umask(0);
            int result_code = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            umask(process_mask);
            if ( result_code != 0 ) printf("[SysCore] Can't create directory: %s\n", dir.c_str() );
            else return true;
        } else {
            closedir(dp);
            printf("[SysCore] Directory already exists: %s\n", dir.c_str() );
            return true;
        }
        return false;
    }
    //========================================================================
    // GetSeconds()/GetMilliseconds()
    // universal timers, rather hackish but working
    //========================================================================
    long GetMicroseconds(){
        struct timeval time;
        gettimeofday(&time, NULL);
        return (time.tv_sec*1000000 + time.tv_usec);
    }
    double GetMilliseconds(){
        struct timeval time;
        gettimeofday(&time, NULL);
        return (time.tv_sec*1000.0 + time.tv_usec/1000.0);
    }
    double GetSeconds() {
        struct timeval time;
        gettimeofday(&time, NULL);
        return (time.tv_sec + time.tv_usec/1000000.0);
    }
    std::string GetTimeStamp() {
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
    //========================================================================
    // Random()/GetMilliseconds()
    // Mother of all randoms!
    //========================================================================
    // Output random bits
    uint32_t RandomBits() {
        uint64_t sum;
        sum = (uint64_t)2111111111UL * (uint64_t)rand_x[3] +
        (uint64_t)1492 * (uint64_t)(rand_x[2]) +
        (uint64_t)1776 * (uint64_t)(rand_x[1]) +
        (uint64_t)5115 * (uint64_t)(rand_x[0]) +
        (uint64_t)rand_x[4];
        rand_x[3] = rand_x[2];  rand_x[2] = rand_x[1];  rand_x[1] = rand_x[0];
        rand_x[4] = (uint32_t)(sum >> 32);                  // Carry
        rand_x[0] = (uint32_t)sum;                          // Low 32 bits of sum
        return rand_x[0];
    } 
    // returns a random number between 0 and 1:
    double RandomDouble() {
        return (double)RandomBits() * (1./(65536.*65536.));
    }
    // returns integer random number in desired interval:
    int RandomInt(int min, int max) {
        // Output random integer in the interval min <= x <= max
        // Relative error on frequencies < 2^-32
        if (max <= min) {
            if (max == min) return min; else return 0x80000000;
        }
        // Assume 64 bit integers supported. Use multiply and shift method
        uint32_t interval;                  // Length of interval
        uint64_t longRand;                   // Random bits * interval
        uint32_t intRand;                      // Longran / 2^32
        
        interval = (uint32_t)(max - min + 1);
        longRand  = (uint64_t)RandomBits() * interval;
        intRand = (uint32_t)(longRand >> 32);
        // Convert back to signed and return result
        return (int32_t)intRand + min;
    }    
    // this function initializes the random number generator:
    void RandomSeed (int seed) {
        int i;
        uint32_t s = seed;
        // make random numbers and put them into the buffer
        for (i = 0; i < 5; i++) {
            s = s * 29943829 - 1;
            rand_x[i] = s;
        }
        // randomize some more
        for (i=0; i<19; i++) RandomBits();
    }
}
