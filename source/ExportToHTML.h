//  ***************************************************************
//  ExportToHTML - Creation date: 01/31/2021
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2021 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ExportToHTML_h__
#define ExportToHTML_h__

class ExportToHTML
{
public:
	ExportToHTML();
	virtual ~ExportToHTML();

	string ExportToString(string mode, bool bAddEndingCRs);

	bool Export();

protected:
	void AddOverlays(string* pHTML, const string itemTemplate);

private:
};

#endif // ExportToHTML_h__#pragma once
