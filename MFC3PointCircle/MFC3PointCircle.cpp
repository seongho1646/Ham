// MFC3PointCircle.cpp

#include "pch.h"
#include "framework.h"
#include "MFC3PointCircle.h"
#include "MFC3PointCircleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMFC3PointCircleApp, CWinApp)
END_MESSAGE_MAP()

CMFC3PointCircleApp::CMFC3PointCircleApp() noexcept
{
    // TODO: 여기에 생성 코드를 추가합니다.
    // InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

CMFC3PointCircleApp theApp;

BOOL CMFC3PointCircleApp::InitInstance()
{
    CWinApp::InitInstance();

    // 공용 컨트롤 초기화 등(위저드 자동생성 코드)
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    AfxEnableControlContainer();

    // MFC3PointCircleDlg 대화상자 실행
    CMFC3PointCircleDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();

    return FALSE; // 대화상자 닫힌 후 종료
}
