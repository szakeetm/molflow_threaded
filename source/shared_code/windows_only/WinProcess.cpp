/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#define NOMINMAX
#include <windows.h>
//#include <winperf.h>
#include<Psapi.h>
#include <stdio.h>
#include "SMP.h"

static bool privilegeEnabled = false;

// Enable required process privilege for system tasks

static bool EnablePrivilege() {

  HANDLE  hToken;
  LUID    DebugValue;
  TOKEN_PRIVILEGES tkp;
  size_t err_code;

  if( !privilegeEnabled ) {

  	/* Enable privileges */

    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken)) {
	     return false;
    }

    if (!LookupPrivilegeValue((LPSTR) NULL,
            SE_DEBUG_NAME,
            &DebugValue)) {
      CloseHandle(hToken);
  	  return false;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,
        false,
        &tkp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL);

	err_code=GetLastError();

    /*if (err_code != ERROR_SUCCESS) {
      CloseHandle(hToken);
  	  return false;
    }*/ //Caused privilege error codes when didn't run as administrator

    privilegeEnabled = true;

  }

  return privilegeEnabled;

}

// Kill a process

bool KillProc(DWORD pID) {

  HANDLE p;

  if( !EnablePrivilege() ) return false;
	p = OpenProcess(PROCESS_ALL_ACCESS,false,pID);
  if( p == NULL ) return false;
  if( !TerminateProcess( p, 1 ) ) {
    CloseHandle(p);
    return false;
  }
  CloseHandle(p);
  return true;

}

// Get process info

#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
#define PROCESS_COUNTER     "process"
#define PROCESSID_COUNTER   "id process"
#define PROCESSTIME_COUNTER "% Processor Time"
#define PROCESSMEM_COUNTER  "Working Set"
#define PROCESSMEMP_COUNTER "Working Set Peak"

static size_t  dwProcessIdTitle;
static size_t  dwProcessMempTitle;
static size_t  dwProcessMemTitle;
static size_t  dwProcessTimeTitle;
static CHAR   keyPerfName[1024];
static bool   counterInited = false;

// Launch the process pname and return its PID.

size_t StartProc(const char *pname,int mode) { //minimized in Debug mode, hidden in Release mode

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	/* Launch */

	memset( &si, 0, sizeof(si) );      
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	DWORD launchMode;

#ifndef _DEBUG
	
	if (mode == STARTPROC_NORMAL) {
		si.wShowWindow = SW_SHOW;
		launchMode = DETACHED_PROCESS;
	}
	else if (mode == STARTPROC_BACKGROUND) {
		si.wShowWindow = SW_MINIMIZE;
		launchMode = CREATE_NEW_CONSOLE;
	}
	else {
		si.wShowWindow = SW_SHOW;
		launchMode = CREATE_NEW_CONSOLE;
	}
#else
	launchMode = CREATE_NEW_CONSOLE;
	if (mode == STARTPROC_NORMAL) {
		si.wShowWindow = SW_MINIMIZE;
	}
	else if (mode == STARTPROC_BACKGROUND) {
		si.wShowWindow = SW_MINIMIZE;
	}
	else {
		si.wShowWindow = SW_SHOW;
	}

		  
#endif
	char* commandLine = strdup(pname);
	if (!CreateProcess(
		NULL,             // pointer to name of executable module
		commandLine,            // pointer to command line string
		NULL,             // process security attributes
		NULL,             // thread security attributes
		false,            // handle inheritance flag
		launchMode | IDLE_PRIORITY_CLASS, // creation flags
		NULL,             // pointer to new environment block
		NULL,             // pointer to current directory name
		&si,              // pointer to STARTUPINFO
		&pi               // pointer to PROCESS_INFORMATION
	)) {

	  return 0;

	}

	return pi.dwProcessId;

}

/*
bool IsProcessRunning(size_t pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, false, pid);
	size_t ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}
*/