#include "PlatformPrecomp.h"
#include "WinDesktopCapture.h"

WinDesktopCapture::WinDesktopCapture()
{
}

WinDesktopCapture::~WinDesktopCapture()
{
}


bool WinDesktopCapture::Capture(int x, int y, int width, int height)
{
	LogMsg("Doing capture...");
	HDC hdc = GetDC(NULL); // get the desktop device context
	HDC hDest = CreateCompatibleDC(hdc); // create a device context to use yourself

	HBITMAP hbDesktop = CreateCompatibleBitmap(hdc, width, height);
	
	// use the previously created device context with the bitmap
	SelectObject(hDest, hbDesktop);
	// copy from the desktop device context to the bitmap device context
	// call this once per 'frame'
	BitBlt(hDest, 0, 0, width, height, hdc, x, y, SRCCOPY);


	//copy it to our own soft surface.  We're using RGBA otherwise GetDIBits will use a weird stride that is.. bad.  Very bad.
	m_captureSoftSurfaceRGBA.Init(width, height, SoftSurface::SURFACE_RGBA);
	
	BMPImageHeader header = m_captureSoftSurfaceRGBA.BuildBitmapHeader();
	GetDIBits(hdc, hbDesktop, 0, height, m_captureSoftSurfaceRGBA.GetPixelData(), (LPBITMAPINFO) &header, DIB_RGB_COLORS);
	
	//m_captureSoftSurfaceRGBA.FillColor(glColorBytes(255, 0, 0, 255));

	m_captureSoftSurface.Init(width, height, SoftSurface::SURFACE_RGB);

	m_captureSoftSurface.Blit(0, 0, &m_captureSoftSurfaceRGBA);
	m_captureSoftSurface.FlipRedAndBlue();
	m_captureSurface.InitFromSoftSurface(&m_captureSoftSurface);
	m_captureSurface.FillColor(glColorBytes(0, 0, 0, 255));
//	m_captureSurface.HardKill();
	//m_captureSoftSurface.FillColor(glColorBytes(255, 0, 0, 255));
	// ..delete the bitmap you were using to capture frames..
	DeleteObject(hbDesktop);

	// after the recording is done, release the desktop context you got..
	ReleaseDC(NULL, hdc);

	// ..and delete the context you created
	DeleteDC(hDest);

	return true;
}