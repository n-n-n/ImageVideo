#pragma once

namespace DShowUtil
{
	/* HRESULT �G���[���O�� */
	void ThrowExceptionForHR(HRESULT hr);

	/* �t�B���^�[���̎w��s�����擾�i�ŏ��̂���) */
	IPin* GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, GUID type = GUID_NULL);

	/* �t�B���^�[���Ȃ��� */
	void ConnectFilters(IFilterGraph2* pFG, IBaseFilter* pOut, IBaseFilter* pIn, GUID type = GUID_NULL); 

	HRESULT AutoRenderVideoStream(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst);

	HRESULT AutoRenderVideoStreamReplace(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst);
	
	HRESULT AutoRenderVideoStream2(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst);

	void ConfigSampleGrabber(ISampleGrabber* pSG);

	enum VMR_TYPE
	{
		VMR7 = 0,
		VMR9 = 1,
	};
	HRESULT AutoRenderVideoStreamWindowless(const wchar_t* file, IFilterGraph2* pFG, IBaseFilter* pSrc, IBaseFilter* pMiddle, IBaseFilter* pDst, HWND hWnd, VMR_TYPE type);

	HRESULT CreateVMR(IBaseFilter** ppBF, VMR_TYPE type);

	HRESULT SetRendererWindow(IBaseFilter* pRenderer, VMR_TYPE type, HWND hWnd, bool bAspectRatio);

    HRESULT GetVideoFileInfo(const wchar_t* file, double* pd_frame_rate, double* pd_stream_length);
}
