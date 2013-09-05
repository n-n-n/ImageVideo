#pragma once
#include "ManageRawImage.h"

#pragma comment(lib, "DshowVideoLib.lib")

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

class CameraCaptureCoreDevice;
class CameraCaptureCore;

namespace CommonManage {

public ref class CameraCapture
{
public:
    CameraCapture(CameraCaptureCore *p);
    ~CameraCapture(void);
    !CameraCapture(void);

    void Start();

    delegate void SampleEventHandler(double dTime, BYTE* pBuf, int nBuf);
    SampleEventHandler^ SampleEvent;
    property int Width { int get(); }
    property int Height { int get(); }
    property int Stride { int get(); }
    void ResetSampleEvent();
    RawImage^ SnapShot();
    // frametime = floor(10000000.0/fps+0.5)
    void ResetCamera(int width, int height, __int64 frametime);
protected:
    CameraCaptureCore *pCore_;

    Threading::Thread^ eventThread_;
    Threading::ManualResetEvent^ manualResetEvent_;
    void EventHandler();

    enum class MediaState
    {
        Stopped,
        Playing,
        Exiting,
    };
    MediaState mediaState_;

};


public ref class CameraCaptureDevice
{
public:
    CameraCaptureDevice();
    ~CameraCaptureDevice(void);
    !CameraCaptureDevice(void);
    List<String^>^ GetDevice();
    CameraCapture^ Create(int device_index, Control^, IntPtr hEvent);
protected:
    CameraCaptureCoreDevice *pCore_;
};

}
