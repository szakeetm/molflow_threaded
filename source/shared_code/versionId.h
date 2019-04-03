#pragma once
#include <string>

#ifdef MOLFLOW
//Hard-coded identifiers, update these on new release and rebuild solution
//---------------------------------------------------
static const std::string appName = "Molflow";
static const int appVersionId = 2708; //Compared with available updates
static const std::string appVersionName = "2.7.8";
//---------------------------------------------------
#ifdef _DEBUG
static const std::string appTitle = "MolFlow+ debug version (Compiled " __DATE__ " " __TIME__ ")";
#else
static const std::string appTitle = "Molflow+ " + appVersionName + " (" __DATE__ ")";
#endif
#endif