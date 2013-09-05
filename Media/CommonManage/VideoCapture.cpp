#include "StdAfx.h"

#include <iostream>
#include <string>
#include <msclr/lock.h>

#include "VideoCaptureCore.h"
#include "VideoCapture.h"

#if defined(_DEBUG) || defined(DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

using namespace System::Diagnostics;

namespace CommonManage{

    public class DsError
    {
	public:
        /// <summary>
        /// If hr has a "failed" status code (E_*), throw an exception.  Note that status
        /// messages (S_*) are not considered failure codes.  If DirectShow error text
        /// is available, it is used to build the exception, otherwise a generic com error
        /// is thrown.
        /// </summary>
        /// <param name="hr">The HRESULT to check</param>
        static void ThrowExceptionForHR(int hr, System::String^ str = L"")
        {
            // If a severe error has occurred
            if (hr < 0)
            {
                System::String^ s = GetErrorText(hr);

                // If a string is returned, build a com error from it
                if (s != nullptr)
                {
                    throw gcnew System::Runtime::InteropServices::COMException(s + str, hr);
                }
                else
                {
                    // No string, just use standard com error
                    System::Runtime::InteropServices::Marshal::ThrowExceptionForHR(hr);
                }
            }
        }
        static void ThrowExceptionForHR(int hr, int line)
		{
			ThrowExceptionForHR(hr, line.ToString());
		}
        ///<summary>Returns a string describing a DS error.
		//Works for both error codes (values < 0) and Status codes (values >= 0)
        ///</summary>
        ///<param name="hr">HRESULT for which to get description</param>
        ///<returns>The string, or null if no error text can be found</returns>
        static System::String^ GetErrorText(int hr)
        {
            // Make a buffer to hold the string
			wchar_t buf[MAX_ERROR_TEXT_LEN];

            // If a string is returned, build a com error from it
            if (AMGetErrorText(hr, buf, MAX_ERROR_TEXT_LEN) > 0)
            {
                return gcnew System::String(buf);
            }

            return nullptr;
        }
    };

	public enum EventCode : int
    {
        // EvCod.h
        Complete = 0x01, // EC_COMPLETE
        UserAbort = 0x02, // EC_USERABORT
        ErrorAbort = 0x03, // EC_ERRORABORT
        Time = 0x04, // EC_TIME
        Repaint = 0x05, // EC_REPAINT
        StErrStopped = 0x06, // EC_STREAM_ERROR_STOPPED
        StErrStPlaying = 0x07, // EC_STREAM_ERROR_STILLPLAYING
        ErrorStPlaying = 0x08, // EC_ERROR_STILLPLAYING
        PaletteChanged = 0x09, // EC_PALETTE_CHANGED
        VideoSizeChanged = 0x0a, // EC_VIDEO_SIZE_CHANGED
        QualityChange = 0x0b, // EC_QUALITY_CHANGE
        ShuttingDown = 0x0c, // EC_SHUTTING_DOWN
        ClockChanged = 0x0d, // EC_CLOCK_CHANGED
        Paused = 0x0e, // EC_PAUSED
        OpeningFile = 0x10, // EC_OPENING_FILE
        BufferingData = 0x11, // EC_BUFFERING_DATA
        FullScreenLost = 0x12, // EC_FULLSCREEN_LOST
        Activate = 0x13, // EC_ACTIVATE
        NeedRestart = 0x14, // EC_NEED_RESTART
        WindowDestroyed = 0x15, // EC_WINDOW_DESTROYED
        DisplayChanged = 0x16, // EC_DISPLAY_CHANGED
        Starvation = 0x17, // EC_STARVATION
        OleEvent = 0x18, // EC_OLE_EVENT
        NotifyWindow = 0x19, // EC_NOTIFY_WINDOW
        StreamControlStopped = 0x1A, // EC_STREAM_CONTROL_STOPPED
        StreamControlStarted = 0x1B, // EC_STREAM_CONTROL_STARTED
        EndOfSegment = 0x1C, // EC_END_OF_SEGMENT
        SegmentStarted = 0x1D, // EC_SEGMENT_STARTED
        LengthChanged = 0x1E, // EC_LENGTH_CHANGED
        DeviceLost = 0x1f, // EC_DEVICE_LOST
        SampleNeeded = 0x20, // EC_SAMPLE_NEEDED
        ProcessingLatency = 0x21, // EC_PROCESSING_LATENCY
        SampleLatency = 0x22, // EC_SAMPLE_LATENCY
        ScrubTime = 0x23, // EC_SCRUB_TIME
        StepComplete = 0x24, // EC_STEP_COMPLETE
        SkipFrames = 0x25, // EC_SKIP_FRAMES

        TimeCodeAvailable = 0x30, // EC_TIMECODE_AVAILABLE
        ExtDeviceModeChange = 0x31, // EC_EXTDEVICE_MODE_CHANGE
        StateChange = 0x32, // EC_STATE_CHANGE

        PleaseReOpen = 0x40, // EC_PLEASE_REOPEN
        Status = 0x41, // EC_STATUS
        MarkerHit = 0x42, // EC_MARKER_HIT
        LoadStatus = 0x43, // EC_LOADSTATUS
        FileClosed = 0x44, // EC_FILE_CLOSED
        ErrorAbortEx = 0x45, // EC_ERRORABORTEX
        EOSSoon = 0x046, // EC_EOS_SOON
        ContentPropertyChanged = 0x47, // EC_CONTENTPROPERTY_CHANGED
        BandwidthChange = 0x48, // EC_BANDWIDTHCHANGE
        VideoFrameReady = 0x49, // EC_VIDEOFRAMEREADY

        GraphChanged = 0x50, // EC_GRAPH_CHANGED
        ClockUnset = 0x51, // EC_CLOCK_UNSET
        VMRRenderDeviceSet = 0x53, // EC_VMR_RENDERDEVICE_SET
        VMRSurfaceFlipped = 0x54, // EC_VMR_SURFACE_FLIPPED
        VMRReconnectionFailed = 0x55, // EC_VMR_RECONNECTION_FAILED
        PreprocessComplete = 0x56, // EC_PREPROCESS_COMPLETE
        CodecApiEvent = 0x57, // EC_CODECAPI_EVENT

        // DVDevCod.h
        DvdDomainChange = 0x101, // EC_DVD_DOMAIN_CHANGE
        DvdTitleChange = 0x102, // EC_DVD_TITLE_CHANGE
        DvdChapterStart = 0x103, // EC_DVD_CHAPTER_START
        DvdAudioStreamChange = 0x104, // EC_DVD_AUDIO_STREAM_CHANGE
        DvdSubPicictureStreamChange = 0x105, // EC_DVD_SUBPICTURE_STREAM_CHANGE
        DvdAngleChange = 0x106, // EC_DVD_ANGLE_CHANGE
        DvdButtonChange = 0x107, // EC_DVD_BUTTON_CHANGE
        DvdValidUopsChange = 0x108, // EC_DVD_VALID_UOPS_CHANGE
        DvdStillOn = 0x109, // EC_DVD_STILL_ON
        DvdStillOff = 0x10a, // EC_DVD_STILL_OFF
        DvdCurrentTime = 0x10b, // EC_DVD_CURRENT_TIME
        DvdError = 0x10c, // EC_DVD_ERROR
        DvdWarning = 0x10d, // EC_DVD_WARNING
        DvdChapterAutoStop = 0x10e, // EC_DVD_CHAPTER_AUTOSTOP
        DvdNoFpPgc = 0x10f, // EC_DVD_NO_FP_PGC
        DvdPlaybackRateChange = 0x110, // EC_DVD_PLAYBACK_RATE_CHANGE
        DvdParentalLevelChange = 0x111, // EC_DVD_PARENTAL_LEVEL_CHANGE
        DvdPlaybackStopped = 0x112, // EC_DVD_PLAYBACK_STOPPED
        DvdAnglesAvailable = 0x113, // EC_DVD_ANGLES_AVAILABLE
        DvdPlayPeriodAutoStop = 0x114, // EC_DVD_PLAYPERIOD_AUTOSTOP
        DvdButtonAutoActivated = 0x115, // EC_DVD_BUTTON_AUTO_ACTIVATED
        DvdCmdStart = 0x116, // EC_DVD_CMD_START
        DvdCmdEnd = 0x117, // EC_DVD_CMD_END
        DvdDiscEjected = 0x118, // EC_DVD_DISC_EJECTED
        DvdDiscInserted = 0x119, // EC_DVD_DISC_INSERTED
        DvdCurrentHmsfTime = 0x11a, // EC_DVD_CURRENT_HMSF_TIME
        DvdKaraokeMode = 0x11b, // EC_DVD_KARAOKE_MODE
        DvdProgramCellChange = 0x11c, // EC_DVD_PROGRAM_CELL_CHANGE
        DvdTitleSetChange = 0x11d, // EC_DVD_TITLE_SET_CHANGE
        DvdProgramChainChange = 0x11e, // EC_DVD_PROGRAM_CHAIN_CHANGE
        DvdVOBU_Offset = 0x11f, // EC_DVD_VOBU_Offset
        DvdVOBU_Timestamp = 0x120, // EC_DVD_VOBU_Timestamp
        DvdGPRM_Change = 0x121, // EC_DVD_GPRM_Change
        DvdSPRM_Change = 0x122, // EC_DVD_SPRM_Change
        DvdBeginNavigationCommands = 0x123, // EC_DVD_BeginNavigationCommands
        DvdNavigationCommand = 0x124, // EC_DVD_NavigationCommand

        // AudEvCod.h
        SNDDEVInError = 0x200, // EC_SNDDEV_IN_ERROR
        SNDDEVOutError = 0x201, // EC_SNDDEV_OUT_ERROR

        WMTIndexEvent = 0x0251, // EC_WMT_INDEX_EVENT
        WMTEvent = 0x0252, // EC_WMT_EVENT

        Built = 0x300, // EC_BUILT
        Unbuilt = 0x301, // EC_UNBUILT

        // Sbe.h
        StreamBufferTimeHole = 0x0326, // STREAMBUFFER_EC_TIMEHOLE
        StreamBufferStaleDataRead = 0x0327, // STREAMBUFFER_EC_STALE_DATA_READ
        StreamBufferStaleFileDeleted = 0x0328, // STREAMBUFFER_EC_STALE_FILE_DELETED
        StreamBufferContentBecomingStale = 0x0329, // STREAMBUFFER_EC_CONTENT_BECOMING_STALE
        StreamBufferWriteFailure = 0x032a, // STREAMBUFFER_EC_WRITE_FAILURE
        StreamBufferReadFailure = 0x032b, // STREAMBUFFER_EC_READ_FAILURE
        StreamBufferRateChanged = 0x032c, // STREAMBUFFER_EC_RATE_CHANGED
    };

}

namespace CommonManage
{

    VideoCapture::VideoCapture(System::String^ fileName, System::Windows::Forms::Control^ hWin, VMR type) : 
	pCore_(NULL)
	{
		std::wstring wstr;
		{
            System::IntPtr mptr = System::Runtime::InteropServices::Marshal::StringToHGlobalUni(fileName);
			wstr = static_cast<const wchar_t*>(mptr.ToPointer());
            System::Runtime::InteropServices::Marshal::FreeHGlobal(mptr);
		}

		try{
			HWND hWnd;
			if (nullptr != hWin) {
				hWnd = static_cast<HWND>(hWin->Handle.ToPointer());
			} else {
				hWnd = NULL;
			}
			if (type == VMR::VMR9) {
				pCore_ = new VideoCaptureCore(wstr.c_str(), hWnd, VideoCaptureCore::VMR9);
			} else {
				pCore_ = new VideoCaptureCore(wstr.c_str(), hWnd, VideoCaptureCore::VMR7);
			}
			OAEVENT hEvent;

			int hr;
			hr = pCore_->pMediaEvent_->GetEventHandle(&hEvent);
			DsError::ThrowExceptionForHR(hr, __LINE__);
			
            m_manualResetEvent = gcnew System::Threading::ManualResetEvent(false);
			//引数はfalseか？
			m_manualResetEvent->SafeWaitHandle 
                = gcnew Microsoft::Win32::SafeHandles::SafeWaitHandle(static_cast<System::IntPtr>(hEvent), false);

			m_mediaState = MediaState::Paused;

            m_eventThread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(this, &VideoCapture::EventHandler));
            m_eventThread->Name = gcnew System::String("Media Event Thread");
			m_eventThread->Start();

		}
        catch(System::Exception^ e)
		{
			ReleaseMediaFilters();
			throw e;
		}

		sw_ = gcnew System::Diagnostics::Stopwatch();
	}

	VideoCapture::~VideoCapture(void)
	{
		this->!VideoCapture();
	}

	VideoCapture::!VideoCapture(void)
	{
		ReleaseMediaFilters();
	}

	void VideoCapture::ReleaseMediaFilters()
	{
		if(m_mediaState != MediaState::Exiting)
		{
			m_mediaState = MediaState::Exiting;
        }

		if( nullptr != m_manualResetEvent ) {
			m_manualResetEvent->Set();
		}
		
        if( nullptr != m_eventThread)
		{
            m_eventThread->Join();
			delete m_eventThread;
			m_eventThread = nullptr;
		}


        if( nullptr != m_manualResetEvent )
		{

			if(nullptr != m_manualResetEvent->SafeWaitHandle)
			{
				delete m_manualResetEvent->SafeWaitHandle;
				m_manualResetEvent->SafeWaitHandle = nullptr;
			}

			delete m_manualResetEvent;
			m_manualResetEvent = nullptr;
		}


		{
			msclr::lock l(this);
			if (pCore_) {
				delete pCore_;
				pCore_ = 0;
			}
		}
	}

	void VideoCapture::EventHandler()
	{
		int hr;
		LONG ec;
		LONG_PTR p1,p2;
        Debug::WriteLine("Enter Event Thread");
		//イベント処理
		while(1) 
		{
			try{

				//次のイベントが来るまで待機
				m_manualResetEvent->WaitOne(-1, true);

				if(m_mediaState == MediaState::Exiting)
				{
					break;
				}
				else
				{
					if (S_OK == pCore_->pMediaEvent_->GetEvent(&ec, &p1, &p2, 0))
					{
						// If the clip is finished playing
						switch(ec)
						{
						case EC_COMPLETE:
							hr = pCore_->pMediaCtrl_->Stop();
							DsError::ThrowExceptionForHR(hr);
							m_mediaState = MediaState::EndOfVideo;
							CompleteEvent();
							break;
						case EC_PAUSED:
							break;
						case EC_STREAM_CONTROL_STOPPED:
							m_mediaState = MediaState::Stopped;
							break;
						case EC_STEP_COMPLETE:
							m_mediaState = MediaState::Paused;
							StepEvent();
							break;
						}

						// Release any resources the message allocated
						hr = pCore_->pMediaEvent_->FreeEventParams(ec, p1, p2);
						DsError::ThrowExceptionForHR(hr);
					}

					// If the error that exited the loop wasn't due to running out of events
					if (hr != E_ABORT)
					{
						DsError::ThrowExceptionForHR(hr);
					}
				}
			}
            catch(System::Exception^ e)
			{
				System::Windows::Forms::MessageBox::Show(e->ToString());
			}
		}
        Debug::WriteLine("Leave Event Thread");
	}

    void VideoCapture::SnapShotWithTime(RawImage^% img, System::Int64 &time)
	{
		img = nullptr;

		try
		{
			raw_image_t* p = 0;
			double dtime = 0;
			BYTE* src;
			int nSize;

			pCore_->GetCallbackCache(&src, &nSize, &dtime);
			if (dtime < 0) {
				return;
			}

			//IntPtrをraw_image型に変換
			p = CreateRawImage(src, nSize);
			
			//raw_image型をRawImageクラスに変換
			img = RawImage::LoadFromRawImageNoCopy(p);

            time = (System::Int64)dtime;
		}
		catch(std::bad_alloc &e)
		{
            System::Diagnostics::Debug::WriteLine(gcnew System::String(e.what()));
            System::GC::Collect();
            System::GC::WaitForPendingFinalizers();
			time = 0;
			img = nullptr;
		}
        catch(System::Exception ^e)
		{
			System::Diagnostics::Debug::WriteLine(e->Message);
			time = 0;
			img = nullptr;
		}
	}

	/**************************************************************/
	RawImage^ VideoCapture::SnapShot() 
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
			p = CreateRawImage(src, nSize);
			
			//raw_image型をRawImageクラスに変換
			img = RawImage::LoadFromRawImageNoCopy(p);
#endif

		}
		catch(std::bad_alloc &e)
		{
            System::Diagnostics::Debug::WriteLine(gcnew System::String(e.what()));
            System::GC::Collect();
            System::GC::WaitForPendingFinalizers();
			return nullptr;
		}
        catch(System::Exception ^e)
		{
			System::Diagnostics::Debug::WriteLine(e->Message);
			return nullptr;
		}

		return img;
	}


	RawImage^ VideoCapture::SnapShotRect(unsigned int x, unsigned int y, 
		unsigned int width, unsigned int height) 
	{
		raw_image_t* p = 0;
		RawImage^ img = nullptr;
		if (x + width > Width || y + height > Height) return nullptr;

		try{
#ifndef GRABBER
			//バッファの読み込み
			long nSize = 0;
			BYTE* src = NULL;
			pCore_->GetCurrentCache(&src, &nSize);
			if (src == NULL || 0 == nSize) return nullptr;
			//IntPtrをraw_image型に変換
			p = CreateRawImageRect(src, nSize, x, y, width, height);
			
			//raw_image型をRawImageクラスに変換
			img = RawImage::LoadFromRawImageNoCopy(p);
#endif

		}
		catch(std::bad_alloc &e)
		{
            System::Diagnostics::Debug::WriteLine(gcnew System::String(e.what()));
            System::GC::Collect();
            System::GC::WaitForPendingFinalizers();
			return nullptr;
		}
        catch(System::Exception ^e)
		{
			System::Diagnostics::Debug::WriteLine(e->Message);
			return nullptr;
		}

		return img;
	}

    raw_image_t* VideoCapture::CreateRawImage(BYTE* p, int nSize) 
    {
        raw_image_t *pImage = 0;

        try
        {
            int dst_stride = Width * 3;
            if (nSize < dst_stride * Height) return NULL;

            pImage = RawImage_CreateBlank(Width, Height, 3);

            int src_stride = nSize/Height;

            for (unsigned int i = 0; i < pImage->height; i++) {
                memcpy(pImage->raw + (pImage->height - i - 1) * dst_stride,  p + i * src_stride, dst_stride);
            }
        }
        catch(std::bad_alloc& e)
        {
            if( 0 != pImage )
            {
                RawImage_Delete(pImage);
            }
            throw e;
        }

        return pImage;
    }


   raw_image_t* VideoCapture::CreateRawImageRect(BYTE* p, int nSize, 
	   unsigned int x, unsigned int y, unsigned int width, unsigned int height) 
    {
        raw_image_t *pImage = 0;
		if (nSize < (x + width) * (y + height) * 3)  return NULL;

		int dst_stride = width * 3;
        try
        {
            pImage = RawImage_CreateBlank(width, height, 3);

            int src_stride = nSize/Height;
			y = max(Height - height - y, 0);
            for (unsigned int i = 0; i < height; i++) {
                memcpy(pImage->raw + (pImage->height - i - 1) * dst_stride,  p + x * 3 + (y + i) * src_stride, dst_stride);
            }
        }
        catch(std::bad_alloc& e)
        {
            if( 0 != pImage )
            {
                RawImage_Delete(pImage);
            }
            throw e;
        }

        return pImage;
    }

    void VideoCapture::ResetSampleEvent()
    {
        if (NULL != pCore_ && nullptr != SampleEvent)
        {
            pCore_->SetCallback((VideoCaptureCore::CBFUNC)
                 System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(SampleEvent).ToPointer());
        }
    }

    void VideoCapture::Play()
    {
        int hr;
        if (NULL != pCore_ && nullptr != SampleEvent)
        {
            pCore_->SetCallback((VideoCaptureCore::CBFUNC)
                 System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(SampleEvent).ToPointer());
        }

        if(m_mediaState == MediaState::Stopped
            || m_mediaState == MediaState::Paused)
        {
            hr = pCore_->pMediaCtrl_->Run();
            DsError::ThrowExceptionForHR(hr);

            m_mediaState = MediaState::Playing;
        }
    }

    void VideoCapture::Step(int nSteps)
    {
        if(m_mediaState == MediaState::Playing
            || m_mediaState == MediaState::Paused
            || m_mediaState == MediaState::Stopped)
        {
            HRESULT hr;
            m_mediaState = MediaState::Stepping;
            hr = pCore_->pVideoFrameStep_->Step(nSteps, nullptr);
            DsError::ThrowExceptionForHR(hr);
        }
    }

    void VideoCapture::SkipTo(System::Int64 time)
    {
        int hr;
        hr = pCore_->pMediaSeeking_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
        if (hr < 0) return;

        LONGLONG dsTime = static_cast<__int64>(time * TIME_UNIT_MSEC);

        hr = pCore_->pMediaSeeking_->SetPositions(&dsTime,AM_SEEKING_AbsolutePositioning,
            NULL, AM_SEEKING_NoPositioning);
        DsError::ThrowExceptionForHR(hr);

        if(m_mediaState == MediaState::EndOfVideo)
            m_mediaState = MediaState::Paused;

    }

    void VideoCapture::Pause()
    {
        int hr;

        if(m_mediaState == MediaState::Playing)
        {
            hr = pCore_->pMediaCtrl_->Pause();
            DsError::ThrowExceptionForHR(hr);
            m_mediaState = MediaState::Paused;
        }
    }

    void VideoCapture::Stop()
    {
        int hr;

        if(m_mediaState == MediaState::Playing
            || m_mediaState == MediaState::Paused)
        {
            hr = pCore_->pMediaCtrl_->Stop();
            DsError::ThrowExceptionForHR(hr);

            m_mediaState = MediaState::Stopped;
        }
    }

    void VideoCapture::Rewind()
    {
        int hr;
        LONGLONG pos = 0;
        hr = pCore_->pMediaSeeking_->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
        DsError::ThrowExceptionForHR(hr);

        if(m_mediaState == MediaState::EndOfVideo)
            m_mediaState = MediaState::Paused;
    }

    void VideoCapture::SetSpeed(double ratio)
    {
        int hr;
        hr = pCore_->pMediaSeeking_->SetRate(ratio);
        DsError::ThrowExceptionForHR(hr);
    }

    void VideoCapture::SetPauseTime(double dTime)
    {
        int hr;
        LONGLONG dsTime = static_cast<__int64>(dTime * TIME_UNIT_MSEC);

        pCore_->pMediaSeeking_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

        hr = pCore_->pMediaSeeking_->SetPositions(
            NULL, AM_SEEKING_NoPositioning,
            &dsTime, AM_SEEKING_AbsolutePositioning);

        DsError::ThrowExceptionForHR(hr);
    }

    void VideoCapture::SetOneShot(bool flag)
    {
#ifdef GRABBER
        pCore_->pSG_->SetOneShot(flag);
#endif
    }

    void VideoCapture::WaitForEnd()
    {
        long code;
        pCore_->pMediaEvent_->WaitForCompletion( -1, &code);
    }

    void VideoCapture::DisableClock()
    {
        pCore_->pMediaFilter_->SetSyncSource(NULL);
    }

    void VideoCapture::SetRenderingSize(System::Drawing::Rectangle rect)
    {
        int hr;
        hr = pCore_->pVideoWindow_->SetWindowPosition(rect.X, rect.Y, rect.Width, rect.Height);
        DsError::ThrowExceptionForHR(hr);
    }

    bool VideoCapture::SetFrameInterval(System::Int64 nStartFrame, System::Int64 nEndFrame)
    {
        int hr;
        hr = pCore_->pMediaSeeking_->SetTimeFormat(&TIME_FORMAT_FRAME);
        if (hr < 0 ) return false;

        LONGLONG start = nStartFrame;
        LONGLONG end = nEndFrame;
        hr = pCore_->pMediaSeeking_->SetPositions(&start, AM_SEEKING_AbsolutePositioning, &end, AM_SEEKING_AbsolutePositioning);
        DsError::ThrowExceptionForHR(hr);
        return true;
    }

    bool VideoCapture::SetInterval(System::Int64 nStart, System::Int64 nEnd)
    {
        int hr;
        hr = pCore_->pMediaSeeking_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
        if (hr < 0 ) return false;

        LONGLONG start = TIME_UNIT_MSEC * nStart;
        LONGLONG end = TIME_UNIT_MSEC * nEnd;
        hr = pCore_->pMediaSeeking_->SetPositions(&start, AM_SEEKING_AbsolutePositioning, &end, AM_SEEKING_AbsolutePositioning);
        DsError::ThrowExceptionForHR(hr);
        return true;
    }

    int VideoCapture::GetState()
    {
        int hr;
        OAFilterState ret;
        hr = pCore_->pMediaCtrl_->GetState(-1, &ret);
        DsError::ThrowExceptionForHR(hr);
        return ret;
    }

    bool VideoCapture::SetRate(double rate)
    {
        int hr;
        hr = pCore_->pMediaSeeking_->SetRate(rate);
        return ( hr >=0 ? true : false);
    }

    void VideoCapture::SetCtrl(System::Windows::Forms::Control^ hWin)
    {
        HWND hWnd;
        if (nullptr != hWin) {
            hWnd = static_cast<HWND>(hWin->Handle.ToPointer());
            pCore_->SetCtrl(hWnd);
        }
    }
}
