#include "StdAfx.h"
#include <vector>
#include <string>
#include <windows.h>
#include <dshow.h>
#include "CameraCaptureCore.h"
#include "CameraCapture.h"
#include <msclr/lock.h>
//#include "RawImageUtil.h"


using namespace System::Runtime::InteropServices;
namespace CommonManage {

CameraCaptureDevice::CameraCaptureDevice(void)
{
    pCore_ = new CameraCaptureCoreDevice();
}

CameraCaptureDevice::~CameraCaptureDevice(void)
{
    this->!CameraCaptureDevice();
}

CameraCaptureDevice::!CameraCaptureDevice(void)
{
    if (0 != pCore_) {
        delete pCore_;
        pCore_ = 0;
    }
}

List<String^>^ CameraCaptureDevice::GetDevice()
{
    List<String^>^ ret = gcnew List<String^>();
    for (unsigned int i = 0; i < pCore_->DeviceList.size(); i++){
        ret->Add(gcnew String(pCore_->DeviceList[i].c_str()));
    }

    return ret;
}

CameraCapture^ CameraCaptureDevice::Create(int index, Control^ ctrl, IntPtr hEvent)
{
	return gcnew CameraCapture(pCore_->Create(index, (ctrl != nullptr ? (HWND)ctrl->Handle.ToPointer() : 0), (HWND)hEvent.ToPointer()));
}

CameraCapture::CameraCapture(CameraCaptureCore* p)
: 	pCore_(p)
{
      int hr;
      OAEVENT hEvent;
      hr = pCore_->pME_->GetEventHandle(&hEvent);

      manualResetEvent_ = gcnew Threading::ManualResetEvent(false);
      //引数はfalseか？
      manualResetEvent_->SafeWaitHandle = gcnew Microsoft::Win32::SafeHandles::SafeWaitHandle(static_cast<IntPtr>(hEvent), false);

      mediaState_ = MediaState::Stopped;

      eventThread_ = gcnew Threading::Thread(gcnew Threading::ThreadStart(this, &CameraCapture::EventHandler));
      eventThread_->Name = gcnew String("Camera Media Event Thread");
      eventThread_->Start();
}

CameraCapture::!CameraCapture()
{
    if(mediaState_ != MediaState::Exiting) {
        mediaState_ = MediaState::Exiting;
    }
    if( nullptr != manualResetEvent_) {
        manualResetEvent_->Set();
    }

    if( nullptr != eventThread_)
    {
        eventThread_->Join();
        delete eventThread_;
        eventThread_ = nullptr;
    }

    if( nullptr != manualResetEvent_)
    {
        if(nullptr != manualResetEvent_->SafeWaitHandle)
        {
            delete manualResetEvent_->SafeWaitHandle;
            manualResetEvent_->SafeWaitHandle = nullptr;
        }

        delete manualResetEvent_;
        manualResetEvent_ = nullptr;
    }

    { 
        msclr::lock l(this);
        if (pCore_) {
            delete pCore_;
            pCore_ = 0;
        }
    }
}

CameraCapture::~CameraCapture()
{
    this->!CameraCapture();
}

int CameraCapture::Width::get()
{
    return pCore_->Width();
}

int CameraCapture::Height::get()
{
    return pCore_->Height();
}

int CameraCapture::Stride::get()
{
    return pCore_->Stride();
}

void CameraCapture::Start()
{
    pCore_->Start();
}
void CameraCapture::ResetSampleEvent()
{
    if (nullptr != SampleEvent)
    {
        pCore_->SetCallback((CameraCaptureCore::CBFUNC)
            Marshal::GetFunctionPointerForDelegate(SampleEvent).ToPointer());
    }
}

RawImage^ CameraCapture::SnapShot()
{
    raw_image_t* p = 0;
    RawImage^ img = nullptr;

    try{
#ifndef GRABBER
        //バッファの読み込み
        long nSize = 0;
        BYTE* src = NULL;
        pCore_->GetCurrentCache(&src, &nSize);
        if (src == NULL || 0 == nSize) return nullptr;

        //IntPtrをraw_image型に変換
        p = RawImage_CreateDirty(Width, Height, 3);
        int src_stride = nSize/Height;
        int dst_stride = Width * 3;
        for (unsigned int i = 0; i < Height; i++) {
            memcpy(p->raw + (Height - i - 1) * dst_stride, src + i * src_stride, dst_stride);
        }
        //raw_image型をRawImageクラスに変換
        img = RawImage::LoadFromRawImageNoCopy(p);
#endif

    }
    catch(std::bad_alloc &e)
    {
        Diagnostics::Debug::WriteLine(gcnew String(e.what()));
        GC::Collect();
        GC::WaitForPendingFinalizers();
        return nullptr;
    }
    catch(Exception ^e)
    {
        System::Diagnostics::Debug::WriteLine(e->Message);
        return nullptr;
    }

    return img;
}

void CameraCapture::ResetCamera(int width, int height, __int64 frametime)
{
    pCore_->ResetCamera(width, height, frametime);
}

void CameraCapture::EventHandler()
{
    int hr;
    LONG ec;
    LONG_PTR p1,p2;

    //イベント処理
    while(1) 
    {
        try{
            //次のイベントが来るまで待機
            manualResetEvent_->WaitOne(-1, true);
            if(mediaState_ == MediaState::Exiting)
            {
                break;
            }
            else
            {
                if (S_OK == pCore_->pME_->GetEvent(&ec, &p1, &p2, 0))
                {
                    // If the clip is finished playing
                    switch(ec)
                    {
                    case EC_COMPLETE:
                        pCore_->Stop();
                        break;
                    case EC_PAUSED:
                        break;
                    case EC_STREAM_CONTROL_STOPPED:
                        break;
                    case EC_STEP_COMPLETE:
                        break;
                    }
                    // Release any resources the message allocated
                    hr = pCore_->pME_->FreeEventParams(ec, p1, p2);
                }

                // If the error that exited the loop wasn't due to running out of events
                if (hr != E_ABORT)
                {
                }
            }
        }
        catch(Exception^ e)
        {
            System::Windows::Forms::MessageBox::Show(e->ToString());
        }
    }
}

}