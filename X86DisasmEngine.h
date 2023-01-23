

#pragma once

#ifndef __AFXWIN_H__
	#error "Incluya "stdafx.h" antes de incluir este archivo para generar el archivo PCH"
#endif

#include "resource.h"


// CX86DisasmEngineApp:
class CX86DisasmEngineApp : public CWinApp
{
public:
	CX86DisasmEngineApp();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CX86DisasmEngineApp theApp;
