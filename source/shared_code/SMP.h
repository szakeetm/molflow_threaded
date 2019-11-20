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
#pragma once

#include <mutex>
bool LockMutex(std::timed_mutex& m,size_t milliseconds=8000);
void ReleaseMutex(std::timed_mutex& m);



//Starting processes in fore- or background, only in Windows, used for compressor
#ifdef _WIN32

#define STARTPROC_NORMAL 0
#define STARTPROC_BACKGROUND 1
#define STARTPROC_FOREGROUND 2

size_t  StartProc(const char *pname, int mode);

#endif
