#include "window.h"
#include "helpers.h"
#include "libGUI.h"
#include "libDAQmx.h"
#include "TextureModel.h"
#include "AccSynthHashMatrix.h"
//#include "ConstantThread.h"

#define _USE_MATH_DEFINES
#include <math.h>

//-------------------------------------------------------------------------------------
using namespace std;
//-------------------------------------------------------------------------------------

#define IDB_START   21001
#define IDB_STOP    21002

//---------------------------------- Global variables ---------------------------------
//bool      started = false;          // indicator of generation signal thread is started
bool      running = false;          // indicator of generation signal thread is running
//double    vibrationsData[1000];     // Samples which being sent to NI DAQ
//bool      modelsLoaded = false;     // Indicate does the models have been loaded already
//string    imageDirectory;           // Directory which contains images of Materials
//string    mainDir;                  // Application Directory
//string    outputDirectory;          // Output directory
double    outData[1000];

//------------------------- Global WINAPI interface variables -------------------------
HINSTANCE Instance;
HWND      lModel;
HWND      lVelosity;
HWND      hCombo;
HWND      status;
HWND      startButton;
int       Controls = 7000;
HWND      velocityTrack;

//---------------------------------- GDI+ Variables ------------------------------------
ULONG_PTR gdiplusToken;
Gdiplus::GdiplusStartupInput gdiplusStartupInput;

// Local variables for operations
const wchar_t*     imgExt = L".jpg";        // Extensions of loaded images
string             imageFilePath = "";      // Path to the image that should be loaded
TextureModel* model;
double force, velocity = 0.0f;

// Functions callback which receives messages from DAQ library
void SetProgress(const char* value);
void SetError(const char* value);

// Forward functions declaration
LRESULT CALLBACK Event_Callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//void InitPaths();
int StartThread(int mdl);
int EndThread();
void WaitTillAllThreadsEnd();
void WorkBody();

/// <summary>
/// Initialize GUI controls' layout
/// </summary>
int InitInterfaceValues()
{
    margin            = 10;
    commonWidth       = windowWidth - 35;
    commonHeight      = 25;

    int staticWidth = 70;
    int labelHeight = 18;

    imageX            = margin;
    imageY            = margin;
    imageWidth        = commonWidth;
    imageHeight       = 330;

    panelX            = margin;
    panelY            = imageHeight + 2 * margin;
    panelWidth        = commonWidth;
    panelHeight       = 3 * labelHeight + 4 * margin;

    int panelWidth = commonWidth - 35;

    staticVelocityX       = panelX + margin;
    staticVelocityY       = panelY + 2.5 * margin;
    staticVelocityWidth   = staticWidth;
    staticVelocityHeight  = labelHeight + 1.5 * margin;

    trackbarX      = staticVelocityX + staticVelocityWidth + margin;
    trackbarY      = panelY + 2.5 * margin;
    trackbarWidth  = commonWidth - 5.5 * margin - staticVelocityWidth - velocityLabelWidth - 45;
    trackbarHeight = labelHeight + 1.5 * margin;

    velocityLabelX = trackbarX + trackbarWidth + margin;
    velocityLabelY = staticVelocityY;
    velocityLabelWidth = 45;
    velocityLabelHeight = labelHeight + 1.5 * margin;

    //portLabelX        = staticPortX + staticPortWidth  + margin;
    //portLabelY        = staticPortY;
    //portLabelWidth    = panelWidth - (staticPortWidth + margin);
    //portLabelHeight   = labelHeight;

    

    staticModelX      = panelX + margin;
    staticModelY = trackbarY + trackbarHeight + margin;
    staticModelWidth  = staticWidth;
    staticModelHeight = labelHeight;

    modelLabelX       = staticModelX + staticModelWidth + margin;
    modelLabelY       = staticModelY;
    modelLabelWidth   = panelWidth - (staticModelWidth + margin);
    modelLabelHight   = labelHeight;

    //staticVelocityX      = panelX + margin;
    //staticVelocityY      = staticModelY + staticModelHeight + margin;
    //staticVelocityWidth  = staticWidth;
    //staticVelocityHeight = labelHeight;

    

    int buttonsLine = panelY + panelHeight + margin;

    comboModelX          = margin;
    comboModelY          = buttonsLine;
    comboModelWidth      = commonWidth - (140 + 2 * margin);
    comboModelHeight     = 250;

    startButtonX         = comboModelX + comboModelWidth + margin;
    startButtonY         = buttonsLine;
    startButtonWidth     = 70;
    startButtonHeight    = commonHeight;

    stopButtonX          = startButtonX + startButtonWidth + margin;
    stopButtonY          = buttonsLine;
    stopButtonWidth      = 70;
    stopButtonHeight     = commonHeight;

    return 1;
}

/// <summary>
/// Main start of program
/// </summary>
INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow)
{
    HWND hWnd;
    MSG msg;

    // Initialize GDI+.
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc      = { 0 };
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = Event_Callback;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = L"Penn Haptic Toolkit app";

    Instance = hInstance;
    RegisterClass(&wc);

    hWnd = CreateWindow(
        wc.lpszClassName, L"Penn Haptic Toolkit app", WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX |WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 1;
}

/// <summary>
/// Initialize the main controls of the main window
/// </summary>
/// <param name="hwnd">The handle to the main window</param>
void InitControls(HWND hwnd)
{
    HWND    hwndSinePanel = CreateWindowW(L"BUTTON", L"Main controls", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        panelX, panelY, panelWidth, panelHeight, hwnd, (HMENU)Controls++, Instance, NULL);

    SWindow sVelocitySize = { staticVelocityX, staticVelocityY, staticVelocityWidth, staticVelocityHeight };
    HWND    sVelosity = CreateLabel(hwnd, sVelocitySize, SS_RIGHT);
    SetLabel(sVelosity, L"Velocity:");

    SWindow VelositySize = { velocityLabelX, velocityLabelY, velocityLabelWidth, velocityLabelHeight };
    lVelosity = CreateLabel(hwnd, VelositySize);

    //SWindow lPortSize = { portLabelX, portLabelY, portLabelWidth, portLabelHeight };
    //lPort = CreateLabel(hwnd, lPortSize);

    velocityTrack = CreateWindowW(TRACKBAR_CLASSW, L"Speed Control", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        trackbarX, trackbarY, trackbarWidth, trackbarHeight, hwnd, (HMENU)Controls++, Instance, NULL);
    ConfigureTrackbar(velocityTrack, 0, 150, 5, 80);

    SWindow sModelSize = { staticModelX, staticModelY, staticModelWidth, staticModelHeight };
    HWND    sModel = CreateLabel(hwnd, sModelSize, SS_RIGHT);
    SetLabel(sModel, L"Model:");

    SWindow modelSize = { modelLabelX, modelLabelY, modelLabelWidth, modelLabelHight };
    lModel = CreateLabel(hwnd, modelSize);
    
    //SWindow sVelositySize = { staticVelocityX, staticVelocityY, staticVelocityWidth, staticVelocityHeight };
    //HWND    sVelosity = CreateLabel(hwnd, sVelositySize, SS_RIGHT);
    //SetLabel(sVelosity, L"Velocity:");

    

    SWindow startButtonSize = { startButtonX, startButtonY, startButtonWidth, startButtonHeight };
    startButton = CreateButton(hwnd, startButtonSize, L"Start", IDB_START);

    SWindow stopButtonSize = { stopButtonX, stopButtonY, stopButtonWidth, stopButtonHeight };
    HWND    hwndButtonEnd = CreateButton(hwnd, stopButtonSize, L"Stop", IDB_STOP);

    hCombo = CreateWindowW(L"Combobox", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_AUTOHSCROLL,
        comboModelX, comboModelY, comboModelWidth, comboModelHeight, hwnd, (HMENU)Controls++, Instance, NULL);

    status = CreateStatusbar(hwnd);
}

/// <summary>
/// reads images from image directory, and populates the names of textures to the combobox
/// </summary>
void InitCombobox()
{
    for (int i = 0; i < 100; i++)
    {
        //SetComboValue(hCombo, ToWChar(texArray[i]));
        SetComboValue(hCombo, ToWChar(model->GetModelName(i)));
    }
}

/// <summary>
/// The main callback which handles all the events on main form
/// </summary>
/// <param name="hWnd">Handle to the main window</param>
/// <param name="uMsg">Message to the window</param>
/// <param name="wParam">wParam</param>
/// <param name="lParam">lParajm</param>
LRESULT CALLBACK Event_Callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE: // Initialization of interface and application
        {
            TCHAR result[100];
            GetModuleFileName(NULL, result, 100);
            model = new TextureModel(ToString(result));
            //constthread::DoWork = &WorkBody;
            force = 2.3f;
            model->Interpolate(force, velocity);

            //matrix = generateHashMatrix();
            //InitPaths();
            InitInterfaceValues();
            InitControls(hWnd);
            InitCombobox();
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDB_START:
                {
                    int model = SendMessage(hCombo, CB_GETCURSEL, NULL, NULL);
                    if (model == CB_ERR)
                        DisplayError(L"Model has not being chosen!\nPlease choose model.");
                    else
                        StartThread(model);
                    EnableWindow(startButton, false);
                    //EnableWindow(hCombo, false);
                    break;
                }
                case IDB_STOP:
                {
                    EnableWindow(startButton, true);
                    //EnableWindow(hCombo, true);
                    EndThread();
                    break;
                }
            }
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                wchar_t strText[100];
                int index = SendMessage(hCombo, CB_GETCURSEL, NULL, NULL);
                model->setTextureModel(index);
                SendMessage(hCombo, CB_GETLBTEXT, index, (LPARAM)strText);
                string selectedString = ToString(strText);
                SetLabel(lModel, ToWChar(selectedString));
                //imageFilePath = imageDirectory + selectedString + ToString(imgExt);
                //imageFilePath = model->GetModelName(model->getTextNum());
                imageFilePath = model->GetCurrentModelImage();
                RECT rect = { imageX, imageY, imageX + imageWidth, imageY + imageHeight };
                InvalidateRect(hWnd, &rect, TRUE);
            }
            break;
        }
        case WM_PAINT:
        {
            if (!imageFilePath.empty()) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                SWindow imageSize = { imageX, imageY, imageWidth, imageHeight };
                LoadPicture(hdc, imageSize, imageFilePath);
                EndPaint(hWnd, &ps);
            }
            break;
        }
        case WM_HSCROLL:
        {
            LRESULT pos = SendMessageW(velocityTrack, TBM_GETPOS, 0, 0);
            wchar_t buf[5];
            wsprintfW(buf, L"%ld", pos);
            int contVal = wcstoul(buf, NULL, 0);
            force = std::stod(buf);
            //if (model != NULL)
            //    model->Interpolate(force, velocity);
            SetLabel(lVelosity, force);
            break;
        }
        case WM_DESTROY:
        {
            if (running)
                EndThread();
            WaitTillAllThreadsEnd();
            //WaitForLibClose();
            PostQuitMessage(0);
            Gdiplus::GdiplusShutdown(gdiplusToken);
            break;
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/// <summary>
/// Creates main directories of models and images
/// </summary>
//void InitPaths()
//{
//    wchar_t directory[MAX_PATH];
//    GetCurrentDirectoryW(MAX_PATH, directory);
//    mainDir = ToString(directory);
//
//    outputDirectory = mainDir + "\\output\\";
//    imageDirectory = mainDir + "\\resources\\images\\";
//}

/// <summary>
/// Succesfully stops thread
/// </summary>
/// <returns>Returns one if success and zero if some problem is occured</returns>
int EndThread()
{
    //StopLib();
    StopLoop();
    //constthread::Stop();
    return 1;
}

/// <summary>
/// Create the thread which generates chosen model of texture from the list
/// </summary>
/// <param name="mdl">The chosen model</param>
/// <returns>Returns one if success and zero if some problem is occured</returns>
int StartThread(int mdl)
{
    try
    {
        if (running)
            return 0;
        else
        {
            StartWriteLoop();
            //constthread::RunAsync(1000);
            //thread libData(startLib, (char*)mainDir.c_str(), mdl);
            //libData.detach();
            //SetStatusValue(status, L"Started haptics Lab", 0);
        }        
    }
    catch (const std::exception&)
    {
        return 0;
    }
    return 1;
}

/// <summary>
/// Wits till all the threads will be stopped before main window close
/// </summary>
void WaitTillAllThreadsEnd()
{
//    while (running && constthread::running) {}
}

void WorkBody()
{
    double vibration = model->GetVibrations();
    WriteData(vibration);
}

#ifdef ASYNC_READ
void DoAction(float* data) {}
#endif

void SetProgress(const char* value)
{
    SetStatusValue(status, value, 0);
}

void SetError(const char* value)
{
    DisplayError(value);
}
