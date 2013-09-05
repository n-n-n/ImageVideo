#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include "qedit.h"

#include "CaptureCore.h"
#include "DShowUtil.h"

using namespace DShowUtil;

#include "wxdebug.h"
#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = 0;}


class CaptureCore::CSampleCB : public ISampleGrabberCB
{
public:
    explicit CSampleCB(CBFUNC func, int nBuf)
        : func_(func), pBuf_(NULL), nBuf_(nBuf), time_(-1)
    {
        if (nBuf_ > 0) {
            pBuf_ = new BYTE[nBuf_];
            memset(pBuf_, 0x00, nBuf);
        }

        InitializeCriticalSection(&cs_);
    }
    ~CSampleCB()
    {
        if (pBuf_) delete [] pBuf_;
        DeleteCriticalSection(&cs_);
    }
    HRESULT STDMETHODCALLTYPE SampleCB(double dTime, IMediaSample* pSample)
    {
        return E_FAIL;
    }
    HRESULT STDMETHODCALLTYPE BufferCB(double time, BYTE* pBuf, long nBuf)
    {
        if (nBuf <= 0) return S_OK;
        if (pBuf) setBuffer(pBuf, nBuf, time);
        func_(time, pBuf, nBuf);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject
    )
    {
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef(void) { return 0;}
    ULONG STDMETHODCALLTYPE Release(void)  { return 0;}

    inline void LockBuffer(BYTE** ppBuf, int *pnBuf, double* pTime) 
    {
        EnterCriticalSection(&cs_); 
        *ppBuf = pBuf_;
        *pnBuf = nBuf_;
        *pTime = time_; /* sec */
    }
    inline void UnlockBuffer()  { LeaveCriticalSection(&cs_);}
    inline int Size() { return nBuf_;}
private:
    CBFUNC func_;
    BYTE* pBuf_;
    double time_;
    long nBuf_;
    inline void setBuffer(BYTE* pBuf, long nBuf, double time)
    {
        EnterCriticalSection(&cs_);
        time_ = time;
        nBuf = (nBuf > nBuf_ ? nBuf_: nBuf);
        memcpy(pBuf_, pBuf, nBuf);
        LeaveCriticalSection(&cs_);
    }
    CRITICAL_SECTION cs_;
};


CaptureCore::CaptureCore(HWND hWnd, VMR_TYPE type) 
: 
// values
nWidth_(0), nHeight_(0), nStride_(0), nImageSize_(0), nFrameTime_(0),
// interfaces
pFG_(NULL),
pSG_(NULL),
pRenderer_(NULL),
// others
pBuf_(0),
pSampleCB_(0),
vmr_type_(type),
hWnd_(hWnd)
{
}

CaptureCore::~CaptureCore(void)
{
    SAFE_RELEASE(pSG_);
    SAFE_RELEASE(pRenderer_);
    SAFE_RELEASE(pFG_);
    if (pBuf_) delete [] pBuf_;
    if (pSampleCB_) delete pSampleCB_;
}

void CaptureCore::beginInit(IBaseFilter** ppSG_BF)
{
   HRESULT hr;

    // GraphBuilder
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID*)&pFG_);
    ThrowExceptionForHR(hr);

    //SampleGrabber
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_ISampleGrabber, (void**)&pSG_);
    ThrowExceptionForHR(hr);

    //Grabberを設定
    {
        AM_MEDIA_TYPE mediaType;
        ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));

        //メディアの種類をRGB24に設定する
        mediaType.majortype = MEDIATYPE_Video;
        mediaType.subtype = MEDIASUBTYPE_RGB24; // 32の対応状況はよくない
        //mediaType.formattype = FORMAT_VideoInfo;

        hr = pSG_->SetMediaType(&mediaType);
        ThrowExceptionForHR(hr);

        hr = pSG_->SetBufferSamples(TRUE);
        ThrowExceptionForHR(hr);
    }

    IBaseFilter* pSG_BF = 0;
    hr = pSG_->QueryInterface(IID_IBaseFilter, (void**)&pSG_BF);
    ThrowExceptionForHR(hr);

    hr = pFG_->AddFilter(pSG_BF, L"Grabber");
    ThrowExceptionForHR(hr);

    *ppSG_BF = pSG_BF;
}

void CaptureCore::rendererInit()
{
    HRESULT hr = CreateVMR(&pRenderer_, (DShowUtil::VMR_TYPE)vmr_type_);
    ThrowExceptionForHR(hr);
    ConfigSampleGrabber(pSG_);
}

void CaptureCore::endInit(IBaseFilter* pSource, IBaseFilter* pSG_BF)
{
    //動画サイズを取得
    setSizeInfo(pSG_);

    DbgLog((LOG_TRACE, 0, "Finished"));
    DumpGraph(pFG_, 0);

    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pSG_BF);

}

void CaptureCore::setSizeInfo(ISampleGrabber* p)
{
    HRESULT hr;
    AM_MEDIA_TYPE media;

    hr = pSG_->GetConnectedMediaType(&media);
    ThrowExceptionForHR(hr);

    if( (media.formattype != FORMAT_VideoInfo) || (media.pbFormat == NULL) ) {
        throw "Unknown Grabber Media Format";
    }

    //ヘッダ構造の取得
    VIDEOINFOHEADER *videoInfoHeader = (VIDEOINFOHEADER*)media.pbFormat;

    //サイズの情報を取得
    nWidth_ = videoInfoHeader->bmiHeader.biWidth;
    nHeight_= videoInfoHeader->bmiHeader.biHeight;
    nStride_ = videoInfoHeader->bmiHeader.biSizeImage / nHeight_;
    nImageSize_  = videoInfoHeader->bmiHeader.biSizeImage;
    nFrameTime_	= videoInfoHeader->AvgTimePerFrame;

    DbgLog((LOG_TRACE, 0, "(%d, %d) stride:%d", nWidth_, nHeight_, nStride_));
    DbgLog((LOG_TRACE, 0, "size:%d av_frame_time:%ld", nImageSize_, nFrameTime_));


    DbgLog((LOG_TRACE, 0, "bit_count %d  compression: 0x%08X" 
        ,videoInfoHeader->bmiHeader.biBitCount,	videoInfoHeader->bmiHeader.biCompression));

    pBuf_ = new BYTE[nImageSize_];

    CoTaskMemFree(media.pbFormat);
}

void CaptureCore::SetCallback(CaptureCore::CBFUNC func)
{
    if (NULL != func && NULL != pSG_) 
    {
        if (NULL != pSampleCB_)	{
            pSG_->SetCallback(NULL, 1);
            delete pSampleCB_;
        }
        pSampleCB_ = new CSampleCB(func, nImageSize_);
        pSG_->SetCallback(pSampleCB_, 1);
    }
}

void CaptureCore::GetCallbackCache(BYTE** ppBuf, int* pnSize, double* pTime)
{
    if (pSampleCB_) {
        BYTE* pBuf;
        pSampleCB_->LockBuffer(&pBuf, pnSize, pTime);
        memcpy(pBuf_, pBuf, *pnSize);
        pSampleCB_->UnlockBuffer();
        *ppBuf = pBuf_ ;
    }
}

void CaptureCore::GetCurrentCache(BYTE** ppBuf, long* pnSize)
{
    HRESULT hr;
    long nBuf = nImageSize_;
    hr = pSG_->GetCurrentBuffer(&nBuf, (long*)pBuf_);
    if (hr < 0) {
        *pnSize = 0;
        *ppBuf = NULL;
    } else 
    {
        *pnSize = nBuf;
        *ppBuf = pBuf_;
    }
}

void CaptureCore::SetCtrl(HWND hWnd)
{
    HRESULT hr;
    hr = SetRendererWindow(pRenderer_, (DShowUtil::VMR_TYPE)vmr_type_, hWnd, true);
}
