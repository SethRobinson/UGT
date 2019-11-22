//  ***************************************************************
//  SmartCURL - Creation date: 04/06/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//Note:  This was used to debug stuff, it's all inside NetHTTP_libCURL.cpp now, don't use this
#ifndef SmartCURL_h__
#define SmartCURL_h__

#include <curl/curl.h>


class SmartCURL
{
public:
	SmartCURL();
	virtual ~SmartCURL();

	void Update();
	int Start();

	char *m_pReceiveBuff = NULL;
	size_t m_receivedSize = 0;

protected:

	CURLM *m_CURL_multi_handle = NULL;
	int m_CURL_handles_still_running = 0;
	CURL *m_CURL_handle = NULL;
	string postData;
};

#endif // SmartCURL_h__#pragma once
