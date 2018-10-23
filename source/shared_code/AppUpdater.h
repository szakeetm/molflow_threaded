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

/*
App updater methods and dialogs.
Adding it to a program requires the following:

----------On program startup----------
- Create a new instance of the AppUpdater class
- Call the RequestUpdateCheck() method
- If the return value is ANSWER_ASKNOW, display a dialog (for example the included UpdateCheckDialog) where the user can decide whether he wants update checks. Based on his answer call SetUserUpdatePreference(bool answer) method

----------On program exit-------------
- Call IncreaseSessionCount()

----------Periodically during the program run (for example in the main event loop)-----------
- Check for background update check process' result:
- IsUpdateAvailable() tells if an update was found
	- (Optional) Create a log window that has a public method Log(string message) to see the update process result. You can use the included UpdateLogWindow
	- If IsUpdateAvailable() is true, display a dialog (for example the included UpdateFoundDialog to ask whether to install that update)

	- If the users asks to install, call InstallLatestUpdate(), then ClearAvailableUpdates()
	- If he asks to skip the available update(s), call SkipAvailableUpdates(), then ClearAvailableUpdates();
	- If he asks to ask later, call ClearAvailableUpdates()
	- If he asks to disable update checking, call ClearAvailableUpdates() then SetUserUpdatePreference(false);

---------In program settings window------------
- Query update check setting by IsUpdateCheckAllowed()
- Set update check setting by SetUserUpdatePreference()

--------Shipped config file--------------------
<?xml version="1.0"?>
<UpdaterConfigFile>
	<ServerConfig>
		<RemoteFeed url="https://company.com/autoupdate.xml" />
		<PublicWebsite url="https://company.com/" downloadsPage="https://company.com/content/downloads" />
		<GoogleAnalytics projectId="UA-12345678-1" />
	</ServerConfig>
	<LocalConfig>
		<Permission allowUpdateCheck="false" appLaunchedBeforeAsking="0" askAfterNbLaunches="3" />
		<Branch name="appname_public" />
		<GoogleAnalytics cookie="not_set" />
		<SkippedVersions />
	</LocalConfig>
</UpdaterConfigFile>

------------Example implementation in Molflow/Synrad--------------
-----In Interface::OneTimeSceneInit_post()----------

appUpdater = new AppUpdater(appName, appVersionId, "updater_config.xml");
int answer = appUpdater->RequestUpdateCheck();
if (answer == ANSWER_ASKNOW) {
updateCheckDialog = new UpdateCheckDialog(appName, appUpdater);
updateCheckDialog->SetVisible(true);
wereEvents = true;

----In GlobalSettings-------------

if (mApp->appUpdater) { //Updater initialized
	chkCheckForUpdates->SetState(mApp->appUpdater->IsUpdateCheckAllowed());
}
//...
if (mApp->appUpdater) {
	if (mApp->appUpdater->IsUpdateCheckAllowed() != updateCheckPreference) {
		mApp->appUpdater->SetUserUpdatePreference(updateCheckPreference);
	}
}


---In interface constructor ----------
appUpdater = NULL; //We'll initialize later, when the app name and version id is known

---In Interface::OnExit()-------------
if (appUpdater) {
	appUpdater->IncreaseSessionCount();
}


-----in Interface::FrameMove()-----------
//Check if app updater has found updates
if (appUpdater && appUpdater->IsUpdateAvailable()) {
	if (!updateLogWindow) {
		updateLogWindow = new UpdateLogWindow(this);
	}
	if (!updateFoundDialog) {
		updateFoundDialog = new UpdateFoundDialog(appName, appVersionName, appUpdater, updateLogWindow);
		updateFoundDialog->SetVisible(true);
		wereEvents = true;
		}
	}
}



*/

#pragma once
#include <string>
#include <vector>
#include <thread>
#include <tuple>
#include <PugiXML/pugixml.hpp>
using namespace pugi;
#include "GLApp/GLWindow.h"
#include "Interface.h" //DoEvents

#define ANSWER_DONTASKYET 1
#define ANSWER_ALREADYDECIDED 2
#define ANSWER_ASKNOW 3

class UpdateManifest {
public:
	std::string name; //Version name, like "Molflow 2.6 beta"
	std::string date; //Release date
	std::string changeLog; //Changes since the last published version
	int versionId; //Will be compared with appVersionId

	std::string zipUrl; //URL of ZIP file to download
	std::string zipName; //Local download target fileName
	std::string folderName; //Folder name expected in ZIP file, also the unzipped program is moved to this sister folder
	std::vector<std::string> filesToCopy; //Config files to copy to new dir
};

class GLButton;
class GLLabel;
class GLList;

class UpdateLogWindow : public GLWindow {
public:
	UpdateLogWindow(Interface* mApp);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
	void ClearLog();
	void Log(const std::string& line);
	void OnResize();
private:
	void RebuildList();
	GLList *logList;
	GLButton *okButton,*copyButton;
	std::vector<std::string> lines;
	Interface* mApp;
	bool isLocked;
};

class AppUpdater;

class UpdateFoundDialog : public GLWindow {
public:
	UpdateFoundDialog(const std::string& appName, const std::string& appVersionName, AppUpdater* appUpdater, UpdateLogWindow* logWindow);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
private:
	GLLabel *questionLabel;
	GLButton *updateButton, *laterButton, *skipButton, *disableButton;
	AppUpdater* updater;
	UpdateLogWindow* logWnd;
};

class AppUpdater {
public:
	AppUpdater(const std::string& appName, const int& versionId, const std::string& configFile);

	bool IsUpdateAvailable();
	bool IsUpdateCheckAllowed();
	void ClearAvailableUpdates();
	std::string GetLatestUpdateName();
	std::string GetCumulativeChangeLog();

	int RequestUpdateCheck(); //Host app requesting update check, and is prepared to treat a possible "ask user if I can check for updates" dialog. Usually called on app startup. If we already have user permission, launches async updatecheck process

	void SetUserUpdatePreference(bool answer);
	void SkipAvailableUpdates();
	void InstallLatestUpdate(UpdateLogWindow* logWindow);
	void IncreaseSessionCount();
	
private:

	//Initialized by constructor:
	int currentVersionId;
	std::string applicationName;
	std::string configFileName;

	//Initialized by shipped config file:
	std::string branchName;
	std::string feedUrl,publicWebsite,publicDownloadsPage;
	std::string googleAnalyticsTrackingId;
	
	//Values that are generated during run
	std::string	userId; //User unique identifier. Default value: "not_set"
	std::vector<int> skippedVersionIds;
	std::thread updateThread;
	bool allowUpdateCheck;
	int appLaunchedWithoutAsking, askAfterNbLaunches; //Number of app launches before asking if user wants to check for updates. 0: default (shipping) value, -1: user already answered

	std::vector<UpdateManifest> availableUpdates; //empty in the beginning, populated upon update check

	//Methods
	void SaveConfig();
	void LoadConfig();
	void PerformUpdateCheck(); //Actually check for updates (once we have user permission)
	
	std::vector<UpdateManifest> DetermineAvailableUpdates(const pugi::xml_node& updateFeed, const int& currentVersionId, const std::string& branchName);
	void DownloadInstallUpdate(const UpdateManifest& update, UpdateLogWindow *logWindow=NULL); //Download, unzip, move new version and copy config files. Return operation result as a user-readable message
	
	UpdateManifest GetLatest(const std::vector<UpdateManifest>& updates);
	std::string GetCumulativeChangeLog(const std::vector<UpdateManifest>& updates);
	void SkipVersions(const std::vector<UpdateManifest>& updates);

	void GenerateUserId();
	
};

class UpdateCheckDialog : public GLWindow {
public:
	UpdateCheckDialog(const std::string& appName, AppUpdater* appUpdater);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
private:
	GLLabel *questionLabel;
	GLButton *allowButton, *declineButton, *laterButton, *privacyButton;
	AppUpdater* updater;
};