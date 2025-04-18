// TriangleCircumcircleGUI.cpp : Defines the entry point for the application.
// GUI: Allows user to input point radius (affects drawn point size and circle computation)

#include <windows.h>
#include <commctrl.h>
#include <math.h>

#pragma comment(lib, "Comctl32.lib")

#define MAX_POINTS 3
#define IDC_RADIO1 101
#define IDC_RADIO2 102
#define IDC_RADIO3 103
#define IDC_INPUT_RADIUS 104

POINT points[MAX_POINTS];
int pointCount = 0;
int dragIndex = -1;
BOOL dragging = FALSE;
int pointRadius = 6; // default

HWND hInput;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Compute distance between two points
double distance(POINT a, POINT b) {
    return hypot(a.x - b.x, a.y - b.y);
}

// Draw triangle and circumcircle
void DrawScene(HDC hdc) {
    if (pointCount >= 3) {
        // Draw triangle
        MoveToEx(hdc, points[0].x, points[0].y, NULL);
        for (int i = 1; i < MAX_POINTS; ++i)
            LineTo(hdc, points[i].x, points[i].y);
        LineTo(hdc, points[0].x, points[0].y);

        // Compute circumcircle center
        POINT A = points[0], B = points[1], C = points[2];
        double D = 2 * (A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
        if (fabs(D) < 1e-6) return;

        double A2 = A.x * A.x + A.y * A.y;
        double B2 = B.x * B.x + B.y * B.y;
        double C2 = C.x * C.x + C.y * C.y;

        POINT center;
        center.x = (LONG)((A2 * (B.y - C.y) + B2 * (C.y - A.y) + C2 * (A.y - B.y)) / D);
        center.y = (LONG)((A2 * (C.x - B.x) + B2 * (A.x - C.x) + C2 * (B.x - A.x)) / D);

        double r = distance(center, A);

        // Draw circumcircle
        Ellipse(hdc, (int)(center.x - r), (int)(center.y - r), (int)(center.x + r), (int)(center.y + r));
    }

    // Draw points
    for (int i = 0; i < pointCount; i++) {
        Ellipse(hdc, points[i].x - pointRadius, points[i].y - pointRadius,
            points[i].x + pointRadius, points[i].y + pointRadius);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    InitCommonControls();

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("CircumcircleGUIApp");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, TEXT("Circumcircle GUI"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void UpdateRadius(HWND hwnd) {
    TCHAR buf[10];
    GetWindowText(hInput, buf, 10);
    int r = _wtoi(buf);
    if (r >= 1 && r <= 50) {
        pointRadius = r;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CreateWindow(TEXT("STATIC"), TEXT("Point Radius:"), WS_CHILD | WS_VISIBLE,
            620, 20, 80, 20, hwnd, NULL, NULL, NULL);

        hInput = CreateWindow(TEXT("EDIT"), TEXT("6"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            700, 20, 50, 20, hwnd, (HMENU)IDC_INPUT_RADIUS, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Set"), WS_CHILD | WS_VISIBLE,
            760, 20, 40, 20, hwnd, (HMENU)105, NULL, NULL);
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 105) {
            UpdateRadius(hwnd);
        }
        break;
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        for (int i = 0; i < pointCount; ++i) {
            if (abs(points[i].x - x) < pointRadius * 2 && abs(points[i].y - y) < pointRadius * 2) {
                dragging = TRUE;
                dragIndex = i;
                SetCapture(hwnd);
                return 0;
            }
        }
        if (pointCount < MAX_POINTS) {
            points[pointCount++] = { x, y };
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    }
    case WM_MOUSEMOVE:
        if (dragging && dragIndex != -1) {
            points[dragIndex].x = LOWORD(lParam);
            points[dragIndex].y = HIWORD(lParam);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    case WM_LBUTTONUP:
        dragging = FALSE;
        dragIndex = -1;
        ReleaseCapture();
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawScene(hdc);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1)); // 시스템 윈도우 배경색
        return 1; // handled
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}