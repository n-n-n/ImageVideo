#include "stdafx.h"
#include <windows.h>
#include "VideoCaptureCore.h"
#include "VideoCapture.h"


namespace CommonManage
{
	bool VideoCapture::IsFrameEnable::get()
	{
		return pCore_->SetTimeFormat(&TIME_FORMAT_FRAME);
	}
/*
	Int64 VideoCapture::DurationFrame::get()
	{
		LONGLONG dur;

		if (!pCore_->SetTimeFormat(&TIME_FORMAT_FRAME)) {
			return -1;
		}

		if (pCore_->GetDuration(&dur)) {
			return dur;
		} else {
			return -1;
		}
	}
*/
	System::Int64 VideoCapture::Duration::get()
	{
		LONGLONG dur;

		if (!pCore_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME)) {
			return -1;
		}

		if (pCore_->GetDuration(&dur)) {
			return dur/TIME_UNIT_MSEC;
		} else {
			return -1;
		}
	}
/*
	Int64 VideoCapture::CurrentFramePos::get()
	{
		LONGLONG pos;

		if (!pCore_->SetTimeFormat(&TIME_FORMAT_FRAME)) {
			return -1;
		}

		if (pCore_->GetPosition(&pos)) {
			return pos;
		} else {
			return -1;
		}

		return pos;
	}
*/
	System::Int64 VideoCapture::CurrentPos::get()
	{
		LONGLONG pos;

		if (!pCore_->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME)) {
			pos = -1;
		}

		if (pCore_->GetPosition(&pos)) {
			pos = pos/TIME_UNIT_MSEC;
		} else {
			pos = -1;
		}

		return pos;
	}

	int VideoCapture::Width::get()
	{
		return pCore_->nWidth_;
	}

	int VideoCapture::Height::get()
	{
		return pCore_->nHeight_;
	}

	int VideoCapture::Stride::get()
	{
		return pCore_->nStride_;
	}

	int VideoCapture::ImageSize::get()
	{
		return pCore_->nImageSize_;
	}

	VideoCapture::MediaState VideoCapture::State::get()
	{
		return m_mediaState;
	}

	double VideoCapture::Fps::get()
	{
		return (double)(1000 * TIME_UNIT_MSEC)/pCore_->nFrameTime_;
	}
}
