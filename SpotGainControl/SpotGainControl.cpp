// SpotGainControl.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c


//possible name: SudoVolumeControl

//includes
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <iostream>

// Global variables
#define TOGGLE_CONTROL_BUTTON 0x8801

// The main window class name.
static TCHAR szWindowClass[] = _T("SpotGainControl");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Spotify Gain Control");

HINSTANCE hInst;

// WINDOWS Prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// Other Prototypes
void controlVolume();
void start();
void stop();

bool state;
DWORD WINAPI MonitorThread(LPVOID lpParameter);


HANDLE mainThreadHandle;

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindow explained:
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Toggle Capture");
    HWND EnableControlButton = NULL;
    switch (message)
    {
    case WM_CREATE:
       EnableControlButton = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed 
            L"ENABLED",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            175,         // x position 
            220,         // y position 
            100,        // Button width
            30,        // Button height
            hWnd,     // Parent window
            (HMENU)TOGGLE_CONTROL_BUTTON,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.
        start();
    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case BN_CLICKED:
                switch (LOWORD(wParam)) {
                case TOGGLE_CONTROL_BUTTON:
                    if (state) {
                        stop();
                        SendMessage(EnableControlButton, WM_SETTEXT, 0, (LPARAM)_T("DISABLED"));
                    }
                    else {
                        start();
                        if(EnableControlButton != NULL)
                            SendMessage(EnableControlButton, WM_SETTEXT, 0, (LPARAM)_T("ENABLED"));
                    }
                    
                }
        }
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // Here your application is laid out.
        // in the top left corner.
        TextOut(hdc,
            175, 200,
            greeting, _tcslen(greeting));

        // End application-specific layout section.

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

DWORD WINAPI MonitorThread(LPVOID lpParameter)
{
    unsigned int& myCounter = *((unsigned int*)lpParameter);
    std::cout << "*-- MONITOR THREAD STARTING --*";
    while (true) {
        //listener for keypresses
        if (GetKeyState(VK_PAUSE) & 0x8000) //if pause break is pressed
        {
            if (GetKeyState(VK_VOLUME_DOWN) & 0x8000) {
                std::cout << "VK_VOLUME_DOWN";
            }
            if (GetKeyState(VK_VOLUME_UP) & 0x8000) {
                std::cout << "VK_VOLUME_UP";
            }
            if (GetKeyState(VK_VOLUME_MUTE) & 0x8000) {
                std::cout << "VK_VOLUME_MUTE";
            }
        }
    }
    return 0;
}


void start() {
    unsigned threadId;
    mainThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &MonitorThread, 0, threadId);
    std::cout << "main thread started";
    state = true;
}

void stop() {
    DWORD exitCode = NULL;
    TerminateThread(mainThreadHandle, exitCode);
    CloseHandle(mainThreadHandle);
    std::cout << "main thread stopped: " << exitCode;
}

//actual control of the volume. Needs to be in a separate thread.
void controlVolume()
{

}