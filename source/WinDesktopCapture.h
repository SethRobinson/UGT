//  ***************************************************************
//  WinDesktopCapture - Creation date: 04/12/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef WinDesktopCapture_h__
#define WinDesktopCapture_h__
#pragma once

#include "Renderer/SoftSurface.h"
#include "Renderer/Surface.h"

class WinDesktopCapture
{
public:
	WinDesktopCapture();
	virtual ~WinDesktopCapture();
	bool Capture(int x, int y, int width, int height);

	SoftSurface * GetSoftSurface() { return &m_captureSoftSurface; }
	Surface * GetSurface() { return &m_captureSurface; }

protected:

	SoftSurface m_captureSoftSurface;
	SoftSurface m_captureSoftSurfaceRGBA;
	Surface m_captureSurface;

private:
};

#endif // WinDesktopCapture_h__
