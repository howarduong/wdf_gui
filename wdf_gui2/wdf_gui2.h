
// wdf_gui2.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Cwdf_gui2App:
// �йش����ʵ�֣������ wdf_gui2.cpp
//

class Cwdf_gui2App : public CWinApp
{
public:
	Cwdf_gui2App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Cwdf_gui2App theApp;