/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include "File.h"
#include <cstring>
#define NOMINMAX
#ifdef _WIN32
#include <Windows.h> //Showwindow
#endif

#include <errno.h>
#include <filesystem>

std::string exec(std::string command);
std::string exec(const char* cmd);

int main(int argc,char* argv[]) {
	std::cout << "MolFlow / SynRad wrapper for 7-zip executable\n";
	std::cout << "Renames a file, compresses it and on success it deletes the original.\n\n";
	char key;
	std::string result;
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << " ";
	}
	std::cout << "\n\n";
	if (argc<3) {
		std::cout<<"Incorrect arguments\nUsage: compress FILE_TO_COMPRESS NEW_NAME_NAME_IN ARCHIVE [include_file1 include_file2 ...]\nType any letter and press ENTER to quit\n";
#ifdef _WIN32
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
#endif
		std::cin>>key;
		return 0;
	}
	std::string command;
	std::string fileName;
	std::string fileNameWith7z;
	std::string fileNameGeometry;
	fileName = argv[1];
	std::cout<<"\nargv0: "<<argv[0];
	std::cout<<"\nargv1: "<<argv[1];
	std::cout<<"\nargv2: "<<argv[2]<<"\n";
	fileNameWith7z = fileName + "7z";
	std::string sevenZipName = "7za";
#ifdef _WIN32
	sevenZipName += ".exe";
#else
	sevenZipName = "./" + sevenZipName;
#endif
	if (!FileUtils::Exist(sevenZipName)) {
		printf("%s",("\n" + sevenZipName + " not found. Cannot compress.\n").c_str());
			std::cin>>key;
			return 0;
	}
	
	fileNameGeometry = FileUtils::GetPath(fileName) + argv[2];
	/*
	command = "move \"" + fileName + "\" \"" + fileNameGeometry + "\"";
	result=exec(command);
	*/
	std::filesystem::rename(fileName, fileNameGeometry);
	std::filesystem::remove(fileNameWith7z);
	command = "";

#ifdef _WIN32
	//Trick so Windows command line supports UNC (network) paths
	std::string cwd = FileUtils::get_working_path();
	command += "cmd /C \"pushd \"" + cwd + "\"&&";
#endif

	command+=sevenZipName + " u -t7z \"" + fileNameWith7z + "\" \"" + fileNameGeometry + "\"";
	for (int i=3;i<argc;i++) { //include files
		bool duplicate=false;
		for (int j=3;!duplicate && j<i;j++) { //check for duplicate include files
			if (strcmp(argv[i],argv[j])==0)
				duplicate=true;
		}
		if (!duplicate) {
			command += " \"";
			command += argv[i];
			command += "\""; //add as new input file
		}
	}
#ifdef _WIN32
	command+="&&popd\"";
#endif

	std::cout << "\nCommand:\n" << command << "\n\nStarting compression...";
#ifdef _WIN32
	std::cout << "\nYou can continue using Molflow/Synrad while compressing.\n"; //On Windows, compress.exe is launched as a background process
#endif
	result=exec(command);
	size_t found;
	found=result.find("Everything is Ok");
	if (found!=std::string::npos) {
		printf("\nCompression seems legit. Deleting original GEO file.\n");
		std::filesystem::remove(fileNameGeometry);
		return 0;
	}

	//Handle errors:
#ifdef _WIN32
	ShowWindow( GetConsoleWindow(), SW_RESTORE ); //Make window visible on error
#endif
	std::filesystem::rename(fileNameGeometry, fileName);
	printf("\nSomething went wrong during the compression, read above. GEO file kept."
		"\nType any letter and press Enter to exit\n");
	std::cin>>key;
	return 0;
}

std::string exec(std::string command) {
	return exec(command.c_str());
}

std::string exec(const char* cmd) { //Execute a command and return what it prints to the command line / terinal
    FILE* pipe = 
#ifdef _WIN32
		_popen
#else
		popen
#endif
		(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
		        printf("%s",buffer);
    }
	result=result+'0';
#ifdef _WIN32
	_pclose
#else
	pclose
#endif
	(pipe);
    return result;
}