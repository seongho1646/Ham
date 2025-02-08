// MFC3PointCircle.h

#pragma once

#ifndef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함하십시오."
#endif

#include "resource.h"       // 주 실행 기호

class CMFC3PointCircleApp : public CWinApp
{
public:
    CMFC3PointCircleApp() noexcept;

public:
    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()
};
