#pragma once

#include <dshow.h>

struct ISampleGrabber;

class CaptureCore
{
public:

    enum VMR_TYPE {
        VMR7 = 0,
        VMR9 = 1,
    };

    CaptureCore(HWND, VMR_TYPE);
    ~CaptureCore(void);

    typedef void (__stdcall *CBFUNC)(double, BYTE*, long);
    void SetCallback(CBFUNC func);

    void GetCallbackCache(BYTE** pBuf, int *nSize, double *time);
    void GetCurrentCache(BYTE** pBuf, long *nSize);

    void SetCtrl(HWND hWnd);

    inline int Width() { return nWidth_;}
    inline int Height() { return nHeight_;}
    inline int Stride() { return nStride_;}
    inline __int64 FrameTime() { return nFrameTime_;}
    inline int ImageSize() { return nImageSize_; }

protected:

    IFilterGraph2* pFG_;
    IBaseFilter* pRenderer_;

    void beginInit(IBaseFilter** pSG_BF);
    void rendererInit();
    void endInit(IBaseFilter* pSource, IBaseFilter* pSG_BF);

    HWND hWnd_;

    int nWidth_;
    int nHeight_;
    int nStride_;
    int nImageSize_;
    __int64 nFrameTime_;
   void setSize() { setSizeInfo(pSG_);}
private:
    ISampleGrabber* pSG_;
    class CSampleCB;
    CSampleCB* pSampleCB_;

    void setSizeInfo(ISampleGrabber* p);

    BYTE* pBuf_;
    VMR_TYPE vmr_type_;
};
