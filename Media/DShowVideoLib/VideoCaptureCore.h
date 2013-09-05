#pragma once

#include <dshow.h>

#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include "qedit.h"

class VideoCaptureCore
{
public:

	enum VMR_TYPE {
		VMR7 = 0,
		VMR9 = 1,
	};

	VideoCaptureCore(const wchar_t* filename, HWND hWnd, VMR_TYPE type);
	~VideoCaptureCore(void);

	void GetDurationW(LONGLONG* duration, GUID* format);
	void GetCurrentPositionW(LONGLONG* pos, GUID* format);

	typedef void (__stdcall *CBFUNC)(double, BYTE*, long);
	void SetCallback(CBFUNC func);

	bool SetTimeFormat(const GUID* format);
	bool GetPosition(LONGLONG* duration);
	bool GetDuration(LONGLONG* duration);

	void GetCallbackCache(BYTE** pBuf, int *nSize, double *time);
	void GetCurrentCache(BYTE** pBuf, long *nSize);

	void SetCtrl(HWND hWnd);

//protected:

	IMediaSeeking* pMediaSeeking_;
	IMediaFilter* pMediaFilter_;
	IVideoWindow* pVideoWindow_;

	IMediaEvent*  pMediaEvent_;
	IMediaControl* pMediaCtrl_;
	
	IVideoFrameStep* pVideoFrameStep_;

	int nWidth_;
	int nHeight_;
	int nStride_;
	int nImageSize_;
	long long nFrameTime_;

private:
	class CSampleCB;
	CSampleCB* pSampleCB_;

	ISampleGrabber* pSG_;
	IFilterGraph2*	pFG_;
	IBaseFilter* pRenderer_;

	BYTE* pBuf_;

	void configVideoWindow(IVideoWindow* p, HWND hWnd);
	void setSizeInfo(ISampleGrabber* p);

	void queryInterfaces();
	void releaseQueriedInterfaces();

	VMR_TYPE vmr_type_;
};
