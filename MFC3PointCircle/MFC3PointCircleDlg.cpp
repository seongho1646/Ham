// MFC3PointCircleDlg.cpp

#include "pch.h"
#include "framework.h"
#include "MFC3PointCircle.h"
#include "MFC3PointCircleDlg.h"
#include "afxdialogex.h"

#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ====================
// 생성자
// ====================
CMFC3PointCircleDlg::CMFC3PointCircleDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_MFC3POINTCIRCLE_DIALOG, pParent)
    , m_nRadius(0)        // 사용자 입력 전까지는 0
    , m_nThickness(0)     // 사용자 입력 전까지는 0
    , m_bDragging(false)
    , m_nDragPointIndex(-1)
    , m_pRandomThread(nullptr)
    , m_bThreadRunning(false)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// ====================
// DDX (Edit 컨트롤 ↔ 멤버 변수)
// ====================
void CMFC3PointCircleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_RADIUS, m_nRadius);
    DDX_Text(pDX, IDC_EDIT_THICKNESS, m_nThickness);
    // 사용자가 에디트 박스에 숫자를 넣지 않으면 기본적으로 0이 들어갈 수 있음
    // 또는 변환 실패 시 에러 메시지 (MFC 기본)
    // 필요 시 DDV_MinMaxInt(pDX, m_nRadius, 1, 9999); 등으로 제한 가능
}

BEGIN_MESSAGE_MAP(CMFC3PointCircleDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CMFC3PointCircleDlg::OnBnClickedButtonReset)
    ON_BN_CLICKED(IDC_BUTTON_RANDOM, &CMFC3PointCircleDlg::OnBnClickedButtonRandom)
END_MESSAGE_MAP()

// ====================
// OnInitDialog
// ====================
BOOL CMFC3PointCircleDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 아이콘
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    srand((unsigned)time(NULL));

    // (선택) 에디트 박스에 "원 반지름", "테두리 두께"처럼 표시하고 싶다면:
    // - 이 문자열은 int로 변환이 안 되므로, UpdateData(TRUE) 시 에러 납니다.
    //   -> 사용자에게 실제 숫자를 입력하게 해야 함.
    //SetDlgItemText(IDC_EDIT_RADIUS,   _T("원 반지름"));
    //SetDlgItemText(IDC_EDIT_THICKNESS,_T("테두리 두께"));

    // 또는 Cue Banner(PlaceHolder) 기능을 쓸 수도 있으나, MFC에서 간단히 구현하기는 번거롭습니다.
    // 여기서는 그냥 빈 칸("")으로 둬서 "입력 안 된 상태"를 0으로 인식.
    SetDlgItemText(IDC_EDIT_RADIUS, _T(""));
    SetDlgItemText(IDC_EDIT_THICKNESS, _T(""));

    return TRUE;
}

// ====================
// OnPaint (Win32 GDI)
// ====================
void CMFC3PointCircleDlg::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    // 배경 흰색으로 지움
    CRect rcClient;
    GetClientRect(&rcClient);
    HBRUSH hBrushBG = ::CreateSolidBrush(RGB(255, 255, 255));
    ::FillRect(hdc, &rcClient, hBrushBG);
    ::DeleteObject(hBrushBG);

    // (A) 작은 원(점) - 검정색
    for (size_t i = 0; i < m_vecPoints.size(); ++i)
    {
        HPEN   hPen = ::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HBRUSH hNull = (HBRUSH)::GetStockObject(NULL_BRUSH);

        HPEN   hOldPen = (HPEN)::SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hNull);

        ::Ellipse(hdc,
            m_vecPoints[i].x - m_nRadius,
            m_vecPoints[i].y - m_nRadius,
            m_vecPoints[i].x + m_nRadius,
            m_vecPoints[i].y + m_nRadius);

        ::SelectObject(hdc, hOldPen);
        ::SelectObject(hdc, hOldBrush);
        ::DeleteObject(hPen);
    }

    // (B) 3점 모두 있으면 외접원(검정색)
    if (m_vecPoints.size() == 3)
    {
        double cx, cy, R;
        if (CalculateCircle(m_vecPoints[0], m_vecPoints[1], m_vecPoints[2], cx, cy, R))
        {
            HPEN   hPen = ::CreatePen(PS_SOLID, m_nThickness, RGB(0, 0, 0));
            HBRUSH hNull = (HBRUSH)::GetStockObject(NULL_BRUSH);

            HPEN   hOldPen = (HPEN)::SelectObject(hdc, hPen);
            HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hNull);

            ::Ellipse(hdc,
                (int)(cx - R), (int)(cy - R),
                (int)(cx + R), (int)(cy + R));

            ::SelectObject(hdc, hOldPen);
            ::SelectObject(hdc, hOldBrush);
            ::DeleteObject(hPen);
        }
    }

    ::EndPaint(m_hWnd, &ps);
}

// ====================
// 마우스 이벤트
// ====================
void CMFC3PointCircleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // ** 여기서 사용자 입력 값(반지름, 두께)을 항상 가져와서 검사 **
    UpdateData(TRUE);

    // 1) 반지름이 0 이하이면 팝업
    if (m_nRadius <= 0)
    {
        AfxMessageBox(_T("원 반지름을 먼저 입력하세요 (정수)."));
        return; // 클릭 로직 진행 중단
    }
    // 2) 두께가 0 이하이면 팝업
    if (m_nThickness <= 0)
    {
        AfxMessageBox(_T("테두리 두께를 먼저 입력하세요 (정수)."));
        return;
    }

    // 3점이 있다면 드래그 시작 체크
    if (m_vecPoints.size() == 3)
    {
        for (int i = 0; i < 3; i++)
        {
            int dx = point.x - m_vecPoints[i].x;
            int dy = point.y - m_vecPoints[i].y;
            if ((dx * dx + dy * dy) <= (m_nRadius * m_nRadius))
            {
                m_bDragging = true;
                m_nDragPointIndex = i;
                break;
            }
        }
    }

    // 아직 3점 미만이면 점 추가
    if (m_vecPoints.size() < 3)
    {
        POINT pt = { point.x, point.y };
        m_vecPoints.push_back(pt);

        // Static 표시
        if (m_vecPoints.size() == 1)
        {
            CString str;
            str.Format(_T("(%d, %d)"), point.x, point.y);
            SetDlgItemText(IDC_STATIC_PT1, str);
        }
        else if (m_vecPoints.size() == 2)
        {
            CString str;
            str.Format(_T("(%d, %d)"), point.x, point.y);
            SetDlgItemText(IDC_STATIC_PT2, str);
        }
        else if (m_vecPoints.size() == 3)
        {
            CString str;
            str.Format(_T("(%d, %d)"), point.x, point.y);
            SetDlgItemText(IDC_STATIC_PT3, str);
        }

        Invalidate(FALSE);
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFC3PointCircleDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bDragging && m_nDragPointIndex >= 0 && m_nDragPointIndex < 3)
    {
        m_vecPoints[m_nDragPointIndex].x = point.x;
        m_vecPoints[m_nDragPointIndex].y = point.y;

        // Static 좌표 업데이트
        if (m_nDragPointIndex == 0)
        {
            CString str;
            str.Format(_T("(%d, %d)"), point.x, point.y);
            SetDlgItemText(IDC_STATIC_PT1, str);
        }
        else if (m_nDragPointIndex == 1)
        {
            CString str;
            str.Format(_T("(%d, %d)"), point.x, point.y);
            SetDlgItemText(IDC_STATIC_PT2, str);
        }
        else if (m_nDragPointIndex == 2)
        {
            CString str;
            str.Format(_T("(%d, %d)"), point.x, point.y);
            SetDlgItemText(IDC_STATIC_PT3, str);
        }

        Invalidate(FALSE);
    }

    CDialogEx::OnMouseMove(nFlags, point);
}

void CMFC3PointCircleDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bDragging = false;
    m_nDragPointIndex = -1;
    CDialogEx::OnLButtonUp(nFlags, point);
}

// ====================
// 초기화 버튼
// ====================
void CMFC3PointCircleDlg::OnBnClickedButtonReset()
{
    // (1) 화면에 그려진 점/원 정보 초기화
    m_vecPoints.clear();
    m_bDragging = false;
    m_nDragPointIndex = -1;

    // (2) Static 좌표 표시도 초기화
    SetDlgItemText(IDC_STATIC_PT1, _T(""));
    SetDlgItemText(IDC_STATIC_PT2, _T(""));
    SetDlgItemText(IDC_STATIC_PT3, _T(""));

    // (3) ★ 사용자가 입력한 m_nRadius, m_nThickness도 초기화
    m_nRadius = 0;
    m_nThickness = 0;
    // 그리고 Edit 박스 내용을 비워 줌
    SetDlgItemText(IDC_EDIT_RADIUS, _T(""));
    SetDlgItemText(IDC_EDIT_THICKNESS, _T(""));

    // (4) 화면도 지우고 다시 갱신
    Invalidate(TRUE);
}

// ====================
// 랜덤 이동 버튼
// ====================
void CMFC3PointCircleDlg::OnBnClickedButtonRandom()
{
    // ** 여기서도 반지름/두께 검사 가능 **
    UpdateData(TRUE);
    if (m_nRadius <= 0)
    {
        AfxMessageBox(_T("반지름을 먼저 입력하세요 (정수)."));
        return;
    }
    if (m_nThickness <= 0)
    {
        AfxMessageBox(_T("테두리 두께를 먼저 입력하세요 (정수)."));
        return;
    }

    if (m_vecPoints.size() < 3) return;
    if (m_bThreadRunning) return;

    m_bThreadRunning = true;
    m_pRandomThread = AfxBeginThread(RandomThreadFunc, this);
}

UINT CMFC3PointCircleDlg::RandomThreadFunc(LPVOID pParam)
{
    CMFC3PointCircleDlg* pDlg = reinterpret_cast<CMFC3PointCircleDlg*>(pParam);
    if (!pDlg) return 0;

    CRect rc;
    pDlg->GetClientRect(&rc);
    int width = rc.Width();
    int height = rc.Height();

    for (int i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < pDlg->m_vecPoints.size(); ++j)
        {
            pDlg->m_vecPoints[j].x = rand() % (width - 100) + 50;
            pDlg->m_vecPoints[j].y = rand() % (height - 100) + 50;
        }

        // 좌표 Static
        if (pDlg->m_vecPoints.size() >= 3)
        {
            CString str1, str2, str3;
            str1.Format(_T("(%d, %d)"), pDlg->m_vecPoints[0].x, pDlg->m_vecPoints[0].y);
            str2.Format(_T("(%d, %d)"), pDlg->m_vecPoints[1].x, pDlg->m_vecPoints[1].y);
            str3.Format(_T("(%d, %d)"), pDlg->m_vecPoints[2].x, pDlg->m_vecPoints[2].y);

            pDlg->SetDlgItemText(IDC_STATIC_PT1, str1);
            pDlg->SetDlgItemText(IDC_STATIC_PT2, str2);
            pDlg->SetDlgItemText(IDC_STATIC_PT3, str3);
        }

        pDlg->Invalidate(FALSE);
        Sleep(500);
    }

    pDlg->m_bThreadRunning = false;
    return 0;
}

// ====================
// 3점 외접원 계산
// ====================
bool CMFC3PointCircleDlg::CalculateCircle(const POINT& p1,
    const POINT& p2,
    const POINT& p3,
    double& cx, double& cy, double& R)
{
    double denominator = (double)((p2.x - p1.x) * (p3.y - p1.y)
        - (p2.y - p1.y) * (p3.x - p1.x));
    if (fabs(denominator) < 1e-12)
        return false;

    double x1 = (double)p1.x, y1 = (double)p1.y;
    double x2 = (double)p2.x, y2 = (double)p2.y;
    double x3 = (double)p3.x, y3 = (double)p3.y;

    double A = x2 - x1;
    double B = y2 - y1;
    double C = x3 - x1;
    double D = y3 - y1;

    double E = A * (x1 + x2) + B * (y1 + y2);
    double F = C * (x1 + x3) + D * (y1 + y3);
    double G = 2.0 * (A * (y3 - y2) - B * (x3 - x2));

    if (fabs(G) < 1e-12) return false;

    cx = (D * E - B * F) / G;
    cy = (A * F - C * E) / G;
    R = sqrt((x1 - cx) * (x1 - cx) + (y1 - cy) * (y1 - cy));
    return true;
}
