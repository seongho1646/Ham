// MFC3PointCircleDlg.h

#pragma once

#include "afxdialogex.h"
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>

class CMFC3PointCircleDlg : public CDialogEx
{
    // 생성
public:
    CMFC3PointCircleDlg(CWnd* pParent = nullptr);    // 표준 생성자

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MFC3POINTCIRCLE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원

protected:
    HICON m_hIcon;
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()

public:
    // (1) 사용자 입력
    int m_nRadius;       // 클릭 지점 원의 반지름
    int m_nThickness;    // 정원의 테두리 두께

    // (2) 클릭 지점
    std::vector<POINT> m_vecPoints;

    // (3) 드래그 상태
    bool m_bDragging;
    int  m_nDragPointIndex;

    // (4) 스레드 관련
    CWinThread* m_pRandomThread;
    bool        m_bThreadRunning;

    // (5) 함수
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedButtonReset();
    afx_msg void OnBnClickedButtonRandom();
    static UINT RandomThreadFunc(LPVOID pParam);

    bool CalculateCircle(const POINT& p1, const POINT& p2, const POINT& p3,
        double& cx, double& cy, double& R);
};
