#pragma once
#include <windows.h>
#include "ManageRawImage.h"
#pragma comment(lib,"DshowVideoLib.lib")

class VideoCaptureCore;
class CSampleCB;

namespace CommonManage
{
	inline bool IsEqual(GUID* a, const GUID* b) {
		return ( 0 == memcmp(a, b, sizeof(GUID)) ? true : false);
	}

    inline System::Guid ToGuid( _GUID& guid ) {
        return System::Guid( guid.Data1, guid.Data2, guid.Data3, 
			guid.Data4[ 0 ], guid.Data4[ 1 ], 
			guid.Data4[ 2 ], guid.Data4[ 3 ], 
			guid.Data4[ 4 ], guid.Data4[ 5 ], 
			guid.Data4[ 6 ], guid.Data4[ 7 ] );
	}

    inline _GUID ToGUID(System::Guid& guid ) {
        array<System::Byte>^ guidData = guid.ToByteArray();
        pin_ptr<System::Byte> data = &(guidData[ 0 ]);

		return *(_GUID *)data;
	}

    ref class RawImage;

	public ref class VideoCapture
	{
	public:
		enum class VMR {
			VMR7,
			VMR9,
		};
        VideoCapture(System::String^ fileName, System::Windows::Forms::Control^ hWin, VMR vmr);
		virtual ~VideoCapture(void);
		!VideoCapture(void);

		delegate void CompleteEventHandler(void);
		delegate void StepEventHandler(void);
		delegate void SampleEventHandler(double sampleTime, BYTE* pBuf, int nBuf);

		event CompleteEventHandler^ CompleteEvent;
		event StepEventHandler^ StepEvent;
		SampleEventHandler^ SampleEvent;

		enum class MediaState
		{
			Paused,
			Stopped,
			Playing,
			Exiting,
			Stepping,
			EndOfVideo,
		};

		RawImage^ SnapShot();
		RawImage^ SnapShotRect(unsigned int x, unsigned int y, unsigned int widht, unsigned int height);

        void SnapShotWithTime(RawImage^% img, System::Int64 &time);

		void ResetSampleEvent();

		//再生
		void Play();

		//nフレーム分ステップ
		void Step(int n);

		//dTime[ms]へジャンプ
		void SkipTo(System::Int64 dTime);

		//一時停止
		void Pause();

		//巻き戻し
		void Rewind();

		//停止
		void Stop();

		//再生速度の設定
		void SetSpeed(double ratio);

		//データの開放
		void ReleaseMediaFilters();

		//自動で停止する時間を設定
		void SetPauseTime(double dTime);

		property int Width{
			int get();
		}
		property int Height{
			int get();
		}
		property int Stride{
			int get();
		}
		property int ImageSize{
			int get();
		}

		property MediaState State{
			MediaState get();
		}

		//単位[秒]
        property System::Int64 CurrentPos{
            System::Int64 get();
		}
/*
		property Int64 CurrentFramePos
		{
			Int64 get();
		}
*/
        property System::Int64 Duration
		{
            System::Int64 get();
		}
/*
		property Int64 DurationFrame
		{
			Int64 get();
		}
*/
		void SetOneShot(bool flag);
		void WaitForEnd();
		void DisableClock();
		void SetRenderingSize(System::Drawing::Rectangle rect);

		bool SetFrameInterval(System::Int64 nStartFrame, System::Int64 nEndFrame);
		bool SetInterval(System::Int64 nStart, System::Int64 nEnd);

		int GetState();

		bool SetRate(double rate);

		//フレームレート[fps]
		property double Fps{
			double get();
		}

		property bool IsFrameEnable {
			bool get();
		}

        void SetCtrl(System::Windows::Forms::Control^ ctrl);

	protected:
		//１フレームの時間[100ns] 100 x 10^{-9}s = 10^{-7} s = 10^{-4} * 10^{-3} = 10^{-4} ms
		const static int TIME_UNIT_MSEC = 10000; 

		MediaState m_mediaState;
		System::Threading::Thread^ m_eventThread;

        System::Threading::ManualResetEvent^ m_manualResetEvent;

		raw_image_t* CreateRawImage(BYTE* ip, int nSize);
		raw_image_t* CreateRawImageRect(BYTE* ip, int nSize, 
			unsigned int x, unsigned int y, unsigned int width, unsigned int height);
		void EventHandler();

		System::Diagnostics::Stopwatch^ sw_;

		VideoCaptureCore *pCore_;
	};
}