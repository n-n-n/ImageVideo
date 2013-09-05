#pragma once

#include "CaptureCore.h"

#include <vector>
#include <string>

class CameraCaptureCore;

class CameraCaptureCoreDevice
{
public:
    CameraCaptureCoreDevice();
    ~CameraCaptureCoreDevice();
    std::vector<std::wstring> DeviceList;
    CameraCaptureCore* Create(int index, HWND hWnd, HWND hEvent);
};

class CameraCaptureCore : public CaptureCore
{
public:
    CameraCaptureCore(int index, HWND hWnd, VMR_TYPE type);
    ~CameraCaptureCore(void);
    void Start();
    void Stop();
    void ResizeVideoWindow() { return resizeVideoWindow();}
   // frametime = floor(10000000.0/fps+0.5)
    void ResetCamera(int width, int height, __int64 frametime);
    static __int64 FrameTimeFromFps(double fps);
    struct Mode 
    {
        ULONG VideoStandard;
        SIZE InputSize;
        SIZE MinCroppingSize;
        SIZE MaxCroppingSize;
        int CropGranularityX;
        int CropGranularityY;
        int CropAlignX;
        int CropAlignY;
        SIZE MinOutputSize;
        SIZE MaxOutputSize;
        int OutputGranularityX;
        int OutputGranularityY;
        int StretchTapsX;
        int StretchTapsY;
        int ShrinkTapsX;
        int ShrinkTapsY;
        LONGLONG MinFrameInterval;
        LONGLONG MaxFrameInterval;
        LONG MinBitsPerSecond;
        LONG MaxBitsPerSecond;
    };
    std::vector<Mode> ModeList;
    IMediaEventEx *pME_;
private:

    IVideoWindow  *pVW_;
    IMediaControl *pMC_;
    IAMStreamConfig* pConfig_;

    enum PLAYSTATE {Stopped, Paused, Running, Init};

    PLAYSTATE psCurrent_;

    HRESULT queryInterfaces();
    HRESULT setupVideoWindow();
    void resizeVideoWindow(); 
    static HRESULT findCaptureDevice(int index, IBaseFilter** ppBaseFilter);
    static HRESULT showPropertyPage(IBaseFilter* pBaseFilter);
};


