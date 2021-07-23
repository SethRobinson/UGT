//  ***************************************************************
//  UpdateChecker - Creation date: 07/25/2021
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2021 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef UpdateChecker_h__
#define UpdateChecker_h__

#include "Network/NetHTTP.h"

class UpdateChecker
{
public:
	UpdateChecker();
	virtual ~UpdateChecker();

	void CheckForUpdate();
	void Update();

protected:
	NetHTTP m_netHTTP;

private:
};

#endif // UpdateChecker_h__#pragma once
