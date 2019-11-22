//This file is used for all Windows builds and contains the WinMain entry point and Windows app code

#pragma once

const char * GetAppName();
extern bool	g_bAppFinished;
void InitVideoSize();
extern int g_winVideoScreenX;
extern int g_winVideoScreenY;
extern string g_videoModeNameForce; //if set, video mode will be forced to this instead of what is set in main.cpp
extern bool g_bIsFullScreen;
void CheckWindowsMessages();

class VideoModeEntry
{
public:
	VideoModeEntry(string _name, int _x, int _y, ePlatformID _platformID, eOrientationMode _forceOrientation = ORIENTATION_DONT_CARE) : name(_name), x(_x), y(_y), platformID (_platformID), forceOrientation (_forceOrientation){};
	string name;
	int x, y;
	ePlatformID platformID;
	eOrientationMode forceOrientation; //so LockLandscape() won't actually do it when using the 'iPhone Landscape' mode.. we 
	//really want it to stay in portrait mode.
};


