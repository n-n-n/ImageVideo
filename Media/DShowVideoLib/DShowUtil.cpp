#include <dshow.h>
#include "qedit.h"
#include "DShowUtil.h"

#include <d3d9.h>
#include <vmr9.h>

#include "wxdebug.h"
#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = 0;}

namespace {

    HRESULT WindowlessVMR7(IBaseFilter* pVMR, HWND hWnd)
    {
        HRESULT hr;
        IVMRFilterConfig* pConfig = NULL;
        IVMRWindowlessControl* pWC = NULL;

        hr = pVMR->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
        DShowUtil::ThrowExceptionForHR(hr);

        hr = pConfig->SetRenderingMode(VMRMode_Windowless);
        DShowUtil::ThrowExceptionForHR(hr);

        hr = pVMR->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWC);
        DShowUtil::ThrowExceptionForHR(hr);

        pWC->SetVideoClippingWindow(hWnd);
        DShowUtil::ThrowExceptionForHR(hr);

#if 1   /* 描画領域を明確に指定 */
        RECT rect;
        GetClientRect(hWnd, &rect);
        pWC->SetVideoPosition(NULL, &rect);
#endif

        SAFE_RELEASE(pConfig);
        return hr;
    }


    HRESULT WindowlessVMR9(IBaseFilter* pVMR, HWND hWnd)
    {
        HRESULT hr;
        IVMRFilterConfig* pConfig = NULL;
        IVMRWindowlessControl9* pWC = NULL;

        hr = pVMR->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig);
        DShowUtil::ThrowExceptionForHR(hr);

        hr = pConfig->SetRenderingMode(VMR9Mode_Windowless);
        DShowUtil::ThrowExceptionForHR(hr);

        hr = pVMR->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC);
        DShowUtil::ThrowExceptionForHR(hr);

        pWC->SetVideoClippingWindow(hWnd);
        DShowUtil::ThrowExceptionForHR(hr);

#if 1   /* 描画領域を明確に指定 */
        RECT rect;
        GetClientRect(hWnd, &rect);
        pWC->SetVideoPosition(NULL, &rect);
#endif

        SAFE_RELEASE(pConfig);
        SAFE_RELEASE(pWC);
        return hr;
    }

    HRESULT WindowlessVMR(IBaseFilter* pVMR, HWND hWnd, DShowUtil::VMR_TYPE type)
    {
        switch(type) {
            case DShowUtil::VMR9: return WindowlessVMR9(pVMR, hWnd);
            case DShowUtil::VMR7: return WindowlessVMR7(pVMR, hWnd);
            default: return E_FAIL;
        }
    } 

    inline LONG Width(RECT& rect) { return rect.right - rect.left;}
    inline LONG Height(RECT& rect) { return rect.bottom - rect.top;}

    void FitAspectRatio(RECT* rect, LONG nVideoW, LONG nVideoH, LONG nVideoWR, LONG nVideoHR)
    {
        LONG nW = Width(*rect);
        LONG nH = Height(*rect);
        if (nW * nVideoHR > nH * nVideoWR) {
            LONG offset = nVideoHR * nW - nVideoWR * nH;
            offset /= (2 * nVideoHR);
            rect->left += offset;
            rect->right -= offset;
        } else if (nW * nVideoHR < nH * nVideoWR) {
            LONG offset = nVideoWR * nH - nVideoHR * nW;
            offset /= (2 * nVideoWR);
            rect->top += offset;
            rect->bottom -= offset;
        }
    }

    HRESULT SetRendererWindowVMR7(IBaseFilter* pVMR, HWND hWnd, bool bAspectRatio)
    {
        IVMRWindowlessControl* pWC = NULL;
        HRESULT hr = S_OK;

        hr = pVMR->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWC);
        if (FAILED(hr)) goto EXIT;

        hr = pWC->SetVideoClippingWindow(hWnd);
        if (FAILED(hr)) goto EXIT;

        LONG nVideoW, nVideoH;
        LONG nVideoWR, nVideoHR;
        hr = pWC->GetNativeVideoSize(&nVideoW, &nVideoH, &nVideoWR, &nVideoHR);
        if (FAILED(hr)) goto EXIT;

        /* 描画領域を明確に指定 */
        RECT rect;
        GetClientRect(hWnd, &rect);
        if (bAspectRatio) {
            FitAspectRatio(&rect, nVideoW, nVideoH, nVideoWR, nVideoHR);
        }

        hr = pWC->SetVideoPosition(NULL, &rect);
EXIT:
        SAFE_RELEASE(pWC);
        return hr;
    }


    HRESULT SetRendererWindowVMR9(IBaseFilter* pVMR, HWND hWnd, bool bAspectRatio)
    {
        IVMRWindowlessControl9* pWC = NULL;
        HRESULT hr = S_OK;

        hr = pVMR->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC);
        if (FAILED(hr)) goto EXIT;

        hr = pWC->SetVideoClippingWindow(hWnd);
        if (FAILED(hr)) goto EXIT;

        LONG nVideoW, nVideoH;
        LONG nVideoWR, nVideoHR;
        hr = pWC->GetNativeVideoSize(&nVideoW, &nVideoH, &nVideoWR, &nVideoHR);
        if (FAILED(hr)) goto EXIT;

        /* 描画領域を明確に指定 */
        RECT rect;
        GetClientRect(hWnd, &rect);
        if (bAspectRatio) {
            FitAspectRatio(&rect, nVideoW, nVideoH, nVideoWR, nVideoHR);
        }

        hr = pWC->SetVideoPosition(NULL, &rect);
EXIT:
        SAFE_RELEASE(pWC);
        return hr;
    }
}

HRESULT DShowUtil::CreateVMR(IBaseFilter** ppBF, VMR_TYPE type)
{
    switch(type) 
    {
    case VMR9:
        return CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)ppBF);
    case VMR7:
        return CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)ppBF);
    default:
        return E_FAIL;
    }

}

void DShowUtil::ThrowExceptionForHR(HRESULT hr) 
{
    if (S_OK != hr) {
        // Make a buffer to hold the string
        wchar_t buf[MAX_ERROR_TEXT_LEN];

        // If a string is returned, build a com error from it
        if (AMGetErrorText(hr, buf, MAX_ERROR_TEXT_LEN) > 0)
        {
            throw buf;
        }
    }
}

IPin *DShowUtil::GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, GUID media_type)
{
    BOOL       bFound = FALSE;
    IEnumPins  *pEnum;
    IPin       *pPin;

    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (PinDir == PinDirThis) // 引数で指定した方向のピンならbreak
        {
            if (media_type != GUID_NULL) {
                AM_MEDIA_TYPE mt;
                pPin->ConnectionMediaType(&mt);
                GUID this_type = mt.majortype;
                CoTaskMemFree(mt.pbFormat);
                if (this_type == media_type) {
                    bFound = TRUE;
                    break;
                }
            } else {
                bFound = TRUE;
                break;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    return (bFound ? pPin : 0);
}


void DShowUtil::ConnectFilters(IFilterGraph2* pFG, IBaseFilter* pOut, IBaseFilter* pIn, GUID type)
{
    HRESULT hr;
    IPin* outPin = GetPin(pOut, PINDIR_OUTPUT, type);
    IPin* inPin  = GetPin(pIn,  PINDIR_INPUT, type);

    hr = pFG->Connect(outPin, inPin);
    ThrowExceptionForHR(hr);

    outPin->Release();
    inPin->Release();
}


HRESULT DShowUtil::AutoRenderVideoStream(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst)
{
    HRESULT hr;
    BSTR bstr = ::SysAllocString(file);
    ICaptureGraphBuilder2* pCapGB2 = NULL;
    IMediaControl* pMC = NULL;

    /* 
    ICaputreGraphBuilder2::RenderStreamでソースフィルターが追加されるので、
    既存のソースを取り除く
    */
    pFG->RemoveFilter(pSrc);

    //グラフ作成補助用
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pCapGB2);
    if (FAILED(hr)) goto EXIT;

    // MediaCtrl
    hr = pFG->QueryInterface(IID_IMediaControl, (LPVOID*)&pMC);
    if (FAILED(hr)) goto EXIT;

    // これをコールしないと失敗する模様
    hr = pMC->RenderFile(bstr);
    if (FAILED(hr)) goto EXIT;

    // フィルターグラフを自動接続する 
    hr = pCapGB2->RenderStream(NULL, &MEDIATYPE_Video, pSrc, pMiddle, pDst);
    if (FAILED(hr)) goto EXIT;

EXIT:
    SAFE_RELEASE(pCapGB2);
    SAFE_RELEASE(pMC);
    ::SysFreeString(bstr);
    return hr;
}


void DShowUtil::ConfigSampleGrabber(ISampleGrabber* pSG)
{
    HRESULT hr;

    AM_MEDIA_TYPE mediaType;
    memset(&mediaType, 0x00, sizeof(AM_MEDIA_TYPE));
    mediaType.majortype = MEDIATYPE_Video;
    mediaType.subtype = MEDIASUBTYPE_RGB24; // 32の対応状況はよくない
    mediaType.formattype = GUID_NULL;

    hr = pSG->SetMediaType(&mediaType);
    ThrowExceptionForHR(hr);

    hr = pSG->SetBufferSamples(true);
    ThrowExceptionForHR(hr);
}


HRESULT DShowUtil::AutoRenderVideoStreamReplace(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst)
{
    HRESULT hr;
    IBaseFilter* pEnd = NULL;
    IPin* pIn = NULL;
    IPin* pOut = NULL;

    hr = AutoRenderVideoStream(file, pFG, pSrc, pMiddle, pDst);
    if (FAILED(hr)) goto EXIT;

    DbgLog((LOG_TRACE, 0, "AutoRenderStream"));
    DumpGraph(pFG, 0);

    // フィルター取得
    hr = pFG->FindFilterByName(L"Video Renderer", &pEnd);
    if (FAILED(hr)) goto EXIT;

    // 接続済みのInput Pinを取得
    pIn = GetPin(pEnd, PINDIR_INPUT, GUID_NULL);
    // 接続先のOutput Pinを取得
    hr = pIn->ConnectedTo(&pOut);
    if (FAILED(hr)) goto EXIT;

    // Input Pinの接続を解除
    hr = pFG->Disconnect(pIn);
    if (FAILED(hr)) goto EXIT;
    SAFE_RELEASE(pIn);

    // デフォルトフィルターを解除
    hr = pFG->RemoveFilter(pEnd);
    if (FAILED(hr)) goto EXIT;
    SAFE_RELEASE(pEnd);

    // Output Pinの接続を解除
    hr = pFG->Disconnect(pOut);
    if (FAILED(hr)) goto EXIT;

    // フィルターに追加
    // 先に追加すると勝手にグラフができるのでここで追加
    hr = pFG->AddFilter(pDst, L"Replaced Filter");
    if (FAILED(hr)) goto EXIT;

    // Input Pinを取得
    pIn = GetPin(pDst, PINDIR_INPUT, GUID_NULL);
    // Input Pin/Output Pinを接続
    hr = pFG->Connect(pOut, pIn);
    if (FAILED(hr)) goto EXIT;

EXIT:
    SAFE_RELEASE(pOut);
    SAFE_RELEASE(pIn);
    SAFE_RELEASE(pEnd);

    DbgLog((LOG_TRACE, 0, "AutoRenderStreamReplace"));
    DumpGraph(pFG, 0);

    return hr;
}

/* 
Color Space Converterを挿入する
*/
HRESULT DShowUtil::AutoRenderVideoStream2(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst)
{
    HRESULT hr;
    IEnumPins* pEnum = NULL;
    IPin* pPin = NULL;
    IPin *pIn = NULL, *pOut = NULL;
    IBaseFilter* pBF = NULL;
    bool bFound = false;

    hr = pFG->AddFilter(pDst, L"New Filter");
    if (FAILED(hr)) goto EXIT;

    /* Add Color Space Converter */
    hr = CoCreateInstance(CLSID_Colour, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pBF);
    if (FAILED(hr)) goto EXIT;
    hr = pFG->AddFilter(pBF, L"Color Space Converter");
    if (FAILED(hr)) goto EXIT;
    SAFE_RELEASE(pBF);

    // Cut off temporaly
    hr = pFG->RemoveFilter(pMiddle);
    if (FAILED(hr)) goto EXIT;

    // Enum Filters
    hr = pSrc->EnumPins(&pEnum);
    if (FAILED(hr)) goto EXIT;

    // Loop through all the pins
    while (S_OK == pEnum->Next(1, &pPin, NULL))
    {
        // Try to render this pin. 
        // It's OK if we fail some pins, if at least one pin renders.
        HRESULT hr2 = pFG->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);

        SAFE_RELEASE(pPin);

        if (SUCCEEDED(hr2))
        {
            bFound = true;
        }
    }

    DumpGraph(pFG, 0);

    // Color Space Converter を取得
    hr = pFG->FindFilterByName(L"Color Space Converter", &pBF);
    if (FAILED(hr)) goto EXIT;

    // Color 接続済みのInput Pinを取得
    pIn = GetPin(pBF, PINDIR_INPUT, GUID_NULL);
    SAFE_RELEASE(pBF);

    // 接続先のOutput Pinを取得
    hr = pIn->ConnectedTo(&pOut);
    if (FAILED(hr)) goto EXIT;

    // Input Pinの接続を解除
    hr = pFG->Disconnect(pIn);
    if (FAILED(hr)) goto EXIT;

    // Output Pinの接続を解除
    hr = pFG->Disconnect(pOut);
    if (FAILED(hr)) goto EXIT;

    // Cut off temporaly
    hr = pFG->AddFilter(pMiddle, L"Middle");
    if (FAILED(hr)) goto EXIT;

    // Input Pinを取得
    pPin = GetPin(pMiddle, PINDIR_INPUT, GUID_NULL);

    hr = pFG->Connect(pOut, pPin);
    if (FAILED(hr)) goto EXIT;

    SAFE_RELEASE(pOut);
    SAFE_RELEASE(pPin);

    // Input Pinを取得
    pPin = GetPin(pMiddle, PINDIR_OUTPUT, GUID_NULL);

    hr = pFG->Connect(pPin, pIn);
    if (FAILED(hr)) goto EXIT;

EXIT:
    SAFE_RELEASE(pIn);
    SAFE_RELEASE(pOut);
    SAFE_RELEASE(pPin);
    SAFE_RELEASE(pEnum);
    return hr;
}



/* 
Color Space Converterを挿入する
*/
HRESULT DShowUtil::AutoRenderVideoStreamWindowless(const wchar_t* /*file*/, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst, HWND hWnd, VMR_TYPE type)
{
    HRESULT hr;
    IEnumPins* pEnum = NULL;
    IPin* pPin = NULL;
    IPin *pIn = NULL, *pOut = NULL;
    IBaseFilter* pBF = NULL;
    bool bFound = false;

    hr = pFG->AddFilter(pDst, L"New Filter");
    if (FAILED(hr)) goto EXIT;

    ::WindowlessVMR(pDst, hWnd, type);

    hr = CoCreateInstance(CLSID_Colour, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pBF);
    if (FAILED(hr)) goto EXIT;

    hr = pFG->AddFilter(pBF, L"Color Space Converter");
    if (FAILED(hr)) goto EXIT;

    SAFE_RELEASE(pBF);

    // Cut off temporaly
    hr = pFG->RemoveFilter(pMiddle);
    if (FAILED(hr)) goto EXIT;

    // Enum Filters
    hr = pSrc->EnumPins(&pEnum);
    if (FAILED(hr)) goto EXIT;

    // Loop through all the pins
    while (S_OK == pEnum->Next(1, &pPin, NULL))
    {
        // Try to render this pin. 
        // It's OK if we fail some pins, if at least one pin renders.
        HRESULT hr2 = pFG->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);

        SAFE_RELEASE(pPin);

        if (SUCCEEDED(hr2))
        {
            bFound = true;
        }
    }

    DumpGraph(pFG, 0);

    // Color Space Converter を取得
    hr = pFG->FindFilterByName(L"Color Space Converter", &pBF);
    if (FAILED(hr)) goto EXIT;

    // Color 接続済みのInput Pinを取得
    pIn = GetPin(pBF, PINDIR_INPUT, GUID_NULL);
    SAFE_RELEASE(pBF);

    // 接続先のOutput Pinを取得
    hr = pIn->ConnectedTo(&pOut);
    if (FAILED(hr)) goto EXIT;

    // Input Pinの接続を解除
    hr = pFG->Disconnect(pIn);
    if (FAILED(hr)) goto EXIT;

    // Output Pinの接続を解除
    hr = pFG->Disconnect(pOut);
    if (FAILED(hr)) goto EXIT;

    // Cut off temporaly
    hr = pFG->AddFilter(pMiddle, L"Middle");
    if (FAILED(hr)) goto EXIT;

    // Input Pinを取得
    pPin = GetPin(pMiddle, PINDIR_INPUT, GUID_NULL);

    hr = pFG->Connect(pOut, pPin);
    if (FAILED(hr)) goto EXIT;

    SAFE_RELEASE(pOut);
    SAFE_RELEASE(pPin);

    // Input Pinを取得
    pPin = GetPin(pMiddle, PINDIR_OUTPUT, GUID_NULL);

    hr = pFG->Connect(pPin, pIn);
    if (FAILED(hr)) goto EXIT;

EXIT:
    SAFE_RELEASE(pIn);
    SAFE_RELEASE(pOut);
    SAFE_RELEASE(pPin);
    SAFE_RELEASE(pEnum);

    return hr;
}


HRESULT DShowUtil::SetRendererWindow(IBaseFilter* pVMR, VMR_TYPE type, HWND hWnd, bool bAspectRatio)
{
    switch(type) {
    case DShowUtil::VMR9: return SetRendererWindowVMR9(pVMR, hWnd, bAspectRatio);
    case DShowUtil::VMR7: return SetRendererWindowVMR7(pVMR, hWnd, bAspectRatio);
    default: return E_FAIL;
    } 
}

namespace DShowUtil {

HRESULT GetVideoFileInfo(const wchar_t* file, double* pd_frame_rate, double* pd_stream_length)
{
    HRESULT hr;
    IMediaDet* pMediaDet = NULL;
    BSTR bstr = NULL;
    hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC, IID_IMediaDet, (LPVOID*)&pMediaDet);
    if (FAILED(hr)) goto EXIT;

    bstr = ::SysAllocString(file);
    hr = pMediaDet->put_Filename(bstr);
    if (FAILED(hr)) goto EXIT;

    long num;
    hr = pMediaDet->get_OutputStreams(&num);
    if (FAILED(hr)) goto EXIT;
    for (int i = 0; i < num; i++) {
        hr = pMediaDet->put_CurrentStream(i);
        if (FAILED(hr)) continue;
        AM_MEDIA_TYPE mt;
        hr = pMediaDet->get_StreamMediaType(&mt);
        if(mt.formattype != FORMAT_VideoInfo 
            && mt.formattype != FORMAT_VideoInfo2) 
        {
            continue;
        }        
        CoTaskMemFree(mt.pbFormat);

        hr = pMediaDet->get_FrameRate(pd_frame_rate);
        hr = pMediaDet->get_StreamLength(pd_stream_length);
    }


EXIT:
    SAFE_RELEASE(pMediaDet);
    if (bstr) {
        ::SysFreeString(bstr);
    }
    return hr;
}

}