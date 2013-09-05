#include "CameraCaptureCore.h"
#include "DShowUtil.h"
#include "wxdebug.h"

using namespace DShowUtil;
// Application-defined message to notify app of filtergraph events
#define WM_GRAPHNOTIFY  WM_APP+1

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

namespace 
{

void DeleteMediaType(AM_MEDIA_TYPE* pmt)
{
    if (0 != pmt->cbFormat) 
    {
        CoTaskMemFree((LPVOID)pmt->pbFormat);
        pmt->pbFormat = NULL;
        pmt->cbFormat = 0;
    }
    if (NULL != pmt->pUnk) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }
    CoTaskMemFree(pmt);
}

void SetMode(CameraCaptureCore::Mode&mode, const VIDEO_STREAM_CONFIG_CAPS& scc)
{
    mode.VideoStandard = scc.VideoStandard;
    mode.InputSize = scc.InputSize;
    mode.MinCroppingSize = scc.MinCroppingSize;
    mode.MaxCroppingSize = scc.MaxCroppingSize;
    mode.CropGranularityX = scc.CropGranularityX;
    mode.CropGranularityY = scc.CropGranularityY;
    mode.CropAlignX = scc.CropAlignX;
    mode.CropAlignY = scc.CropAlignY;
    mode.MinOutputSize = scc.MinOutputSize;
    mode.MaxOutputSize = scc.MaxOutputSize;
    mode.OutputGranularityX = scc.OutputGranularityX;
    mode.OutputGranularityY = scc.OutputGranularityY;
    mode.StretchTapsX = scc.StretchTapsX;
    mode.StretchTapsY = scc.StretchTapsY;
    mode.ShrinkTapsX = scc.ShrinkTapsX;
    mode.ShrinkTapsY = scc.ShrinkTapsY;
    mode.MinFrameInterval = scc.MinFrameInterval;
    mode.MaxFrameInterval = scc.MaxFrameInterval;
    mode.MinBitsPerSecond = scc.MinBitsPerSecond;
    mode.MaxBitsPerSecond = scc.MaxBitsPerSecond;
}

HRESULT GetFriendlyName(std::wstring& name, IMoniker *pMoniker)
{
    HRESULT hr;
    name.clear();

    IPropertyBag *pPropBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
    if (SUCCEEDED(hr))
    {
        // フィルタのフレンドリ名を取得するには、次の処理を行う。
        VARIANT varName;
        VariantInit(&varName);
        hr = pPropBag->Read(L"FriendlyName", &varName, 0);
        if (SUCCEEDED(hr))
        {
            name = std::wstring(varName.bstrVal);
        }
        VariantClear(&varName);
    }
    SAFE_RELEASE(pPropBag);
    return hr;
}

}


CameraCaptureCoreDevice::CameraCaptureCoreDevice()
{
    HRESULT hr = S_OK;
    IMoniker* pMoniker =NULL;
    ICreateDevEnum *pDevEnum =NULL;
    IEnumMoniker *pClassEnum = NULL;

    // Create the system device enumerator
    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    ThrowExceptionForHR(hr);

    // Create an enumerator for the video capture devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    ThrowExceptionForHR(hr);

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        ThrowExceptionForHR(E_FAIL);
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    while(S_OK == pClassEnum->Next(1, &pMoniker, NULL))
    {
        std::wstring name;
        hr = GetFriendlyName(name, pMoniker);
        if (SUCCEEDED(hr)) {
            DeviceList.push_back(name);
        }
        SAFE_RELEASE(pMoniker);
    }

    SAFE_RELEASE(pDevEnum);
    SAFE_RELEASE(pClassEnum);
}

CameraCaptureCoreDevice::~CameraCaptureCoreDevice()
{
}

CameraCaptureCore* CameraCaptureCoreDevice::Create(int index, HWND hWnd, HWND hEvent)
{
    return new CameraCaptureCore(index, hWnd, CaptureCore::VMR9);
}


CameraCaptureCore::CameraCaptureCore(int index, HWND hWnd, VMR_TYPE type) 
: CaptureCore(hWnd, type),
  pVW_(NULL), pMC_(NULL), pME_(NULL), pConfig_(NULL),
  psCurrent_(Stopped)
{
    IBaseFilter* pSG_BF;
    beginInit(&pSG_BF);

    HRESULT hr = queryInterfaces();

    ICaptureGraphBuilder2 *pCapture = NULL;
    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void **) &pCapture);
    ThrowExceptionForHR(hr);

    // Attach the filter graph to the capture graph
    hr = pCapture->SetFiltergraph(pFG_);
    ThrowExceptionForHR(hr);

    // Use the system device enumerator and class enumerator to find
    // a video capture/preview device, such as a desktop USB video camera.
    IBaseFilter* pSrcFilter = NULL;
    hr = findCaptureDevice(index, &pSrcFilter);
    ThrowExceptionForHR(hr);

    {
        hr = pCapture->FindInterface(
            &PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video,
            pSrcFilter,
            IID_IAMStreamConfig,
            (void**)&pConfig_);

        int nCount;
        int nSize;
        hr = pConfig_->GetNumberOfCapabilities(&nCount, &nSize);
        AM_MEDIA_TYPE *pmt;
        VIDEO_STREAM_CONFIG_CAPS scc;

        ModeList.clear();

        for (int i = 0; i < nCount; i++) {
            hr = pConfig_->GetStreamCaps(i, &pmt, (BYTE*)&scc);
            if (FAILED(hr)) {
                break;
            }
            Mode mode;
            SetMode(mode, scc);
            ModeList.push_back(mode);
            DeleteMediaType(pmt);
        }
    }

    hr = showPropertyPage(pSrcFilter);
    ThrowExceptionForHR(hr);
    // Add Capture filter to our graph.
    hr = pFG_->AddFilter(pSrcFilter, L"Video Capture");
    if (FAILED(hr))
    {
        pSrcFilter->Release();
        ThrowExceptionForHR(hr);
    }

    rendererInit();

    // Render the preview pin on the video capture filter
    // Use this instead of g_pGraph->RenderFile
    hr = pCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                   pSrcFilter, pSG_BF, pRenderer_);
    if (FAILED(hr))
    {
        pSrcFilter->Release();
    }

   // AutoRenderVideoStreamWindowless(NULL, pGraph_, pSrcFilter, pSG_BF, pRenderer_, hWnd, DShowUtil::VMR7);

    // Now that the filter has been added to the graph and we have
    // rendered its stream, we can release this reference to the filter.

    // Set video window style and position
    //hr = setupVideoWindow();
    //ThrowExceptionForHR(hr);

    endInit(pSrcFilter, pSG_BF);

    SAFE_RELEASE(pCapture);
}

CameraCaptureCore::~CameraCaptureCore(void)
{
    // Stop previewing data
    Stop();

    // parent window.
    if(pVW_)
    {
        pVW_->put_Visible(OAFALSE);
        pVW_->put_Owner(NULL);
    }

    // Release DirectShow interfaces
    SAFE_RELEASE(pConfig_);
    SAFE_RELEASE(pMC_);
    SAFE_RELEASE(pME_);
    SAFE_RELEASE(pVW_);

    /* XXX
    親クラスでReleaseするとアクセス違反になるので、
     強引にゼロにしてReleaseが行われないようにする 
     どこかにバグあり。
    */
    if (pFG_) {
        pFG_ = 0;
    }
}

void CameraCaptureCore::ResetCamera(int width, int height, __int64 frametime)
{
    AM_MEDIA_TYPE *pmt;
    HRESULT hr = pConfig_->GetFormat(&pmt);
    VIDEOINFOHEADER *vh = (VIDEOINFOHEADER*)pmt->pbFormat;
    vh->bmiHeader.biWidth = (width != nWidth_ ? width : nWidth_);
    vh->bmiHeader.biHeight = (height != nHeight_ ? height : nHeight_);
    vh->AvgTimePerFrame = (frametime != nFrameTime_ ? frametime : nFrameTime_);
    hr = pConfig_->SetFormat(pmt);
    DeleteMediaType(pmt);
    if (SUCCEEDED(hr)) {
        setSize();
    }
}

HRESULT CameraCaptureCore::queryInterfaces()
{
   HRESULT hr;

    // Obtain interfaces for media control and Video Window
    hr = pFG_->QueryInterface(IID_IMediaControl,(LPVOID *) &pMC_);
    if (FAILED(hr)) return hr;

    if (NULL != hWnd_) {
        hr = pFG_->QueryInterface(IID_IVideoWindow, (LPVOID *) &pVW_);
        if (FAILED(hr)) return hr;
    }

    hr = pFG_->QueryInterface(IID_IMediaEventEx, (LPVOID *) &pME_);
    if (FAILED(hr)) return hr;

    return hr;
}

HRESULT CameraCaptureCore::setupVideoWindow()
{
    HRESULT hr;
    if (NULL == pVW_) return S_OK;
 
    // Set the video window to be a child of the main window
    hr = pVW_->put_Owner((OAHWND)hWnd_);
    if (FAILED(hr)) return hr;
    
    // Set video window style
    hr = pVW_->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN| WS_CLIPSIBLINGS);
    if (FAILED(hr)) return hr;

    // Make the video window visible, now that it is properly positioned
    hr = pVW_->put_Visible(OATRUE);
    if (FAILED(hr)) return hr;

    // Use helper function to position video window in client rect 
    // of main application window
    resizeVideoWindow();

    return hr;
}

void CameraCaptureCore::resizeVideoWindow()
{
    // Resize the video preview window to match owner window size
    if (pVW_)
    {
        RECT rc;
        // Make the preview video fill our window
        GetClientRect(hWnd_, &rc);
        pVW_->SetWindowPosition(0, 0, rc.right, rc.bottom);
    }
}


HRESULT CameraCaptureCore::findCaptureDevice(int index, IBaseFilter **ppSrcFilter)
{
    HRESULT hr = S_OK;
    IBaseFilter * pSrc = NULL;
    IMoniker* pMoniker =NULL;
    ICreateDevEnum *pDevEnum =NULL;
    IEnumMoniker *pClassEnum = NULL;

    if (!ppSrcFilter)
    {
        return E_POINTER;
    }
   
    // Create the system device enumerator
    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr)) return hr;

    // Create an enumerator for the video capture devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr)) return hr;

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL) return E_FAIL;

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    int ii = 0;
    while(S_OK == pClassEnum->Next(1, &pMoniker, NULL)) 
    {
#if 0
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
        if (SUCCEEDED(hr))
        {
            // フィルタのフレンドリ名を取得するには、次の処理を行う。
            VARIANT varName;
            VariantInit(&varName);
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            VariantClear(&varName);
        }
        SAFE_RELEASE(pPropBag);
#endif
        if (hr == S_FALSE)  hr = E_FAIL;
        if (ii == index) {
            hr = S_OK;
            break;
        } else {
            hr = E_FAIL;
            SAFE_RELEASE(pMoniker);
        }
        ii++;
    }

    if (SUCCEEDED(hr))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr)) return hr;
    }

    // Copy the found filter pointer to the output parameter.
    *ppSrcFilter = pSrc;
    (*ppSrcFilter)->AddRef();

    SAFE_RELEASE(pSrc);
    SAFE_RELEASE(pMoniker);
    SAFE_RELEASE(pDevEnum);
    SAFE_RELEASE(pClassEnum);
    return hr;
}

HRESULT CameraCaptureCore::showPropertyPage(IBaseFilter* pBase)
{
    HRESULT hr;
    ISpecifyPropertyPages *pProp = 0;
    CAUUID caGUID = { 0, NULL};
    FILTER_INFO filterInfo ={ NULL, NULL};
    IUnknown *pFilter = 0;

    hr = pBase->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
    if (FAILED(hr)) goto EXIT;

    hr = pBase->QueryFilterInfo(&filterInfo); 
    if (FAILED(hr)) goto EXIT;

    hr = pBase->QueryInterface(IID_IUnknown,(void **)&pFilter);
    if (FAILED(hr)) goto EXIT;

    hr = pProp->GetPages(&caGUID);
    if (FAILED(hr)) goto EXIT;

    OleCreatePropertyFrame(
        HWND_DESKTOP, /* Parent window */
        0, /* Reserved */
        0, /* Reserved */
        filterInfo.achName, /* Caption for the dialog box */
        1, /* Number of objects (just the filter) */
        &pFilter, /* Array of object pointers. */
        caGUID.cElems,/* Number of property pages */
        caGUID.pElems, /* Array of property page CLSIDs */
        0, /* Locale identifier */
        0, /* reserved */
        NULL /* reserved */
        );

    if (0 != caGUID.cElems) CoTaskMemFree(caGUID.pElems);

EXIT:
    SAFE_RELEASE(pProp);
    SAFE_RELEASE(pFilter);
    if (NULL != filterInfo.pGraph) filterInfo.pGraph->Release();
    return hr;
}

void CameraCaptureCore::Start()
{
    if (!pMC_) return;
    if (psCurrent_ != Running)
    {
        // Start previewing video data
        pMC_->Run();
        psCurrent_ = Running;
    }
}

void CameraCaptureCore::Stop()
{
    if (!pMC_) return;
    if (psCurrent_ == Running)
    {
        // Stop previewing video data
        pMC_->StopWhenReady();
        psCurrent_ = Stopped;
    }
}

#include <cmath>
__int64 CameraCaptureCore::FrameTimeFromFps(double fps)
{
    return (__int64)floor((10000000.0/fps+0.5));
}

/*
void CameraCaptureCore::HandleGraphEvent(void)
{
    LONG evCode;
    LONG_PTR evParam1, evParam2;
    HRESULT hr=S_OK;

    if (!pME_) return;

    while(SUCCEEDED(pME_->GetEvent(&evCode, &evParam1, &evParam2, 0)))
    {
        //
        // Free event parameters to prevent memory leaks associated with
        // event parameter data.  While this application is not interested
        // in the received events, applications should always process them.
        //
        hr = pME_->FreeEventParams(evCode, evParam1, evParam2);
        
        // Insert event processing code here, if desired
    }
}
*/
