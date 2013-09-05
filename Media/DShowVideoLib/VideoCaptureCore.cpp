#include <dshow.h>
#include "qedit.h"
#include "VideoCaptureCore.h"
#include "DShowUtil.h"

#include "wxdebug.h"
#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = 0;}

#if defined(_DEBUG) || defined(DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

using namespace DShowUtil;


class VideoCaptureCore::CSampleCB : public ISampleGrabberCB
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
	~CSampleCB() {
		if (pBuf_) delete [] pBuf_;
		DeleteCriticalSection(&cs_);
	}

	HRESULT STDMETHODCALLTYPE SampleCB(double dTime, IMediaSample* pSample)
	{
        if (pSample) {
            int nBuf;
            nBuf = pSample->GetSize();
            BYTE* pBuf;
            pSample->GetPointer(&pBuf);
            func_(dTime, pBuf, nBuf);
        }
		return S_OK;
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
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE AddRef( void) { return 0;}
	ULONG STDMETHODCALLTYPE Release( void) { return 0;}

	void LockBuffer(BYTE** ppBuf, int *pnBuf, double* pTime)
	{ 
		EnterCriticalSection(&cs_); 
		*ppBuf = pBuf_;
		*pnBuf = nBuf_;
		*pTime = time_; /* sec */
	}
	void UnlockBuffer() { LeaveCriticalSection(&cs_);}
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
        if (nBuf != nBuf_) {
            nBuf_ = nBuf;
            delete [] pBuf_;
            pBuf_ = new BYTE[nBuf];
        }
		memcpy(pBuf_, pBuf, nBuf);
		LeaveCriticalSection(&cs_);
	}
	CRITICAL_SECTION cs_;
};

VideoCaptureCore::VideoCaptureCore(const wchar_t* filename, HWND hWnd, VMR_TYPE type)
:   // values
	nWidth_(0),
	nHeight_(0),
	nStride_(0),
	nImageSize_(0),
	nFrameTime_(0),
	// interfaces
	pFG_(NULL),
	pSG_(NULL),
	pRenderer_(NULL),
	pVideoWindow_(NULL),
	pMediaEvent_(NULL),
	pMediaSeeking_(NULL),
	pVideoFrameStep_(NULL),
	pMediaFilter_(NULL),
	pMediaCtrl_(NULL), 
	// others
	pBuf_(0),
	pSampleCB_(0),
	vmr_type_(type)
{
	HRESULT hr;

	// GraphBuilder
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID*)&pFG_);
	ThrowExceptionForHR(hr);

	queryInterfaces();

	hr = pMediaSeeking_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
	ThrowExceptionForHR(hr);

	//SampleGrabber
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_ISampleGrabber, (void**)&pSG_);
	ThrowExceptionForHR(hr);

	//Grabberを設定
	{
		AM_MEDIA_TYPE mediaType;
		ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));

		//メディアの種類をRGB24に設定する
		mediaType.majortype	= MEDIATYPE_Video;
		mediaType.subtype = MEDIASUBTYPE_RGB24;
		//mediaType.formattype = FORMAT_VideoInfo;

		hr = pSG_->SetMediaType(&mediaType);
		ThrowExceptionForHR(hr);

		hr = pSG_->SetBufferSamples(TRUE);
		ThrowExceptionForHR(hr);
	}

    //フィルタグラフへのリンクを設定
    IBaseFilter* srcFilter = 0;
	hr = pFG_->AddSourceFilter(filename, filename, (IBaseFilter**)&srcFilter);
	ThrowExceptionForHR(hr);

	IBaseFilter* pSG_BF = 0;
	hr = pSG_->QueryInterface(IID_IBaseFilter, (void**)&pSG_BF);
	ThrowExceptionForHR(hr);
	
	//     うまくいっていない
	//ConfigSampleGrabber(pSG_);

	hr = pFG_->AddFilter(pSG_BF, L"Grabber");
	ThrowExceptionForHR(hr);

	DbgLog((LOG_TRACE, 0, "Dump"));
	DumpGraph(pFG_, 0);

	//レンダリングの流れを設定
	if(NULL != hWnd)
	{
/*
		//レンダリングするコントロールの設定
		hr = pFG_->QueryInterface(IID_IVideoWindow, (void**)&pVideoWindow_);
		ThrowExceptionForHR(hr);
		
		configVideoWindow(pVideoWindow_, hWnd);

		AutoRenderVideoStream(filename, pFG_, srcFilter, pSG_BF, NULL);
*/
		DShowUtil::VMR_TYPE t = (DShowUtil::VMR_TYPE)type;
		hr = CreateVMR(&pRenderer_, t);
		ThrowExceptionForHR(hr);
		ConfigSampleGrabber(pSG_);
		AutoRenderVideoStreamWindowless(filename, pFG_, srcFilter, pSG_BF, pRenderer_, hWnd, t);
	}
	else 
	{
#if 1
#if 0
		// AVI はいける
		hr = AutoRenderVideoStream(filename, pFG_, srcFilter, pSG_BF, NULL);
#else
		//if (FAILED(hr)) 
		{
			// AVIもいける? WMVはいけそう
			hr = CreateVMR(&pRenderer_, (DShowUtil::VMR_TYPE)type);
			ThrowExceptionForHR(hr);
			ConfigSampleGrabber(pSG_);
			AutoRenderVideoStream2(filename, pFG_, srcFilter, pSG_BF, pRenderer_);
		} 
#endif
#else
		hr = CreateVMR(pRenderer_);
		ThrowExceptionForHR(hr);
		AutoRenderVideoStreamReplace(filename, pFG_, srcFilter, pSG_BF, pRenderer_);


		ThrowExceptionForHR(hr);
#endif
		DbgLog((LOG_TRACE, 0, "Dump (3)"));
	}

	DumpGraph(pFG_, 0);

	//動画サイズを取得
	setSizeInfo(pSG_);

    if (0 == nFrameTime_) {
        double dframe_rate, dstream_length;
        DShowUtil::GetVideoFileInfo(filename, &dframe_rate, &dstream_length);
        nFrameTime_ = (int)dframe_rate;
    }

    if (0 == nFrameTime_) {
        hr = pMediaCtrl_->Run();
        long code;
        // 1sec 待って Frame rateを取得する
        hr = pMediaEvent_->WaitForCompletion(1000, &code);
        IQualProp* pQualProp;
        hr = pRenderer_->QueryInterface(IID_IQualProp, (void**)&pQualProp);
        int av_frame_rate = 0; // FPS * 100
        hr = pQualProp->get_AvgFrameRate(&av_frame_rate);
        if (0 != av_frame_rate) {
            nFrameTime_ = (1000 * 10000 * 100)/av_frame_rate;
        }
        hr = pMediaCtrl_->Stop();
        hr = pMediaSeeking_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
        LONGLONG dsTime = 0;
        hr = pMediaSeeking_->SetPositions(&dsTime,AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
    }
	SAFE_RELEASE(srcFilter);
	SAFE_RELEASE(pSG_BF);
}

VideoCaptureCore::~VideoCaptureCore(void)
{
	if (pBuf_) delete [] pBuf_;
	releaseQueriedInterfaces();

	SAFE_RELEASE(pVideoWindow_);
	SAFE_RELEASE(pRenderer_);
	SAFE_RELEASE(pSG_);
	SAFE_RELEASE(pFG_);

	if (pSampleCB_) delete pSampleCB_;
}

void VideoCaptureCore::configVideoWindow(IVideoWindow* pVW, HWND hWnd)
{
	HRESULT hr;

	// Set the output window
	hr = pVW->put_Owner((OAHWND)hWnd);
	ThrowExceptionForHR(hr);

	// Set the window style
	hr = pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	ThrowExceptionForHR(hr);

	// Make the window visible
	hr = pVW->put_Visible(FALSE);
	ThrowExceptionForHR(hr);

	// Position the playing location
	RECT rect;
	GetClientRect(hWnd, &rect);
	hr = pVW->SetWindowPosition( 0, 0, rect.right, rect.bottom);
	ThrowExceptionForHR(hr);
}


void VideoCaptureCore::setSizeInfo(ISampleGrabber* p)
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

void VideoCaptureCore::GetDurationW(LONGLONG* duration, GUID* format)
{
	HRESULT hr;
	hr = pMediaSeeking_->GetDuration(duration);
	ThrowExceptionForHR(hr);

	hr = pMediaSeeking_->GetTimeFormatW(format);
	ThrowExceptionForHR(hr);		
}

void VideoCaptureCore::GetCurrentPositionW(LONGLONG* pos, GUID* format)
{
	HRESULT hr;
	hr = pMediaSeeking_->GetCurrentPosition(pos);
	ThrowExceptionForHR(hr);

	hr = pMediaSeeking_->GetTimeFormatW(format);
	ThrowExceptionForHR(hr);
}

void VideoCaptureCore::queryInterfaces()
{
	HRESULT hr;

	// MediaEvent
	hr = pFG_->QueryInterface(IID_IMediaEvent, (LPVOID*)&pMediaEvent_);
	ThrowExceptionForHR(hr);

	// MediaCtrl
	hr = pFG_->QueryInterface(IID_IMediaControl, (LPVOID*)&pMediaCtrl_);
	ThrowExceptionForHR(hr);

	//  VideoFrameStep
	hr = pFG_->QueryInterface(IID_IVideoFrameStep, (LPVOID*)&pVideoFrameStep_);
	ThrowExceptionForHR(hr);

	// MediaSeeking
	hr = pFG_->QueryInterface(IID_IMediaSeeking, (LPVOID*)&pMediaSeeking_);
	ThrowExceptionForHR(hr);

	//  MediaFilter
	hr = pFG_->QueryInterface(IID_IMediaFilter, (LPVOID*)&pMediaFilter_);
	ThrowExceptionForHR(hr);
}

void VideoCaptureCore::releaseQueriedInterfaces()
{
	// MediaEvent
	SAFE_RELEASE(pMediaEvent_);
	//  VideoFrameStep
	SAFE_RELEASE(pVideoFrameStep_);
	// MediaSeeking
	SAFE_RELEASE(pMediaSeeking_);
	//  MediaFilter
	SAFE_RELEASE(pMediaFilter_);
	//  MediaControl
	SAFE_RELEASE(pMediaCtrl_);

}

void VideoCaptureCore::SetCallback(VideoCaptureCore::CBFUNC func)
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

bool VideoCaptureCore::SetTimeFormat(const GUID* format)
{
	HRESULT hr = pMediaSeeking_->SetTimeFormat(format);
	return FAILED(hr) ? false : true;
}

bool VideoCaptureCore::GetPosition(LONGLONG* duration)
{
	HRESULT hr = pMediaSeeking_->GetCurrentPosition(duration);
	return FAILED(hr) ? false : true;
}

bool VideoCaptureCore::GetDuration(LONGLONG* duration)
{
	HRESULT hr = pMediaSeeking_->GetDuration(duration);
	return FAILED(hr) ? false : true;
}

void VideoCaptureCore::GetCallbackCache(BYTE** ppBuf, int* pnSize, double* pTime)
{
	if (pSampleCB_) {
		BYTE* pBuf;
		pSampleCB_->LockBuffer(&pBuf, pnSize, pTime);
        if (*pnSize != nImageSize_) {
            nImageSize_ = *pnSize; 
            delete [] pBuf_;
            pBuf_ = new BYTE[nImageSize_];
        }
		memcpy(pBuf_, pBuf, nImageSize_);
		pSampleCB_->UnlockBuffer();
		*ppBuf = pBuf_ ;
	}
}

void VideoCaptureCore::GetCurrentCache(BYTE** ppBuf, long* pnSize)
{
	HRESULT hr;
	long nBuf;
	hr = pSG_->GetCurrentBuffer(&nBuf, NULL);
    if (FAILED(hr)) return;
    if (nBuf != nImageSize_) {
        nImageSize_ = nBuf; 
        delete [] pBuf_;
        pBuf_ = new BYTE[nBuf];
    }
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

void VideoCaptureCore::SetCtrl(HWND hWnd)
{
	HRESULT hr;
	hr = SetRendererWindow(pRenderer_, (DShowUtil::VMR_TYPE)vmr_type_, hWnd, true);
}