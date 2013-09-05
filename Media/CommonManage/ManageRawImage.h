#pragma once
#include "RawImage.h"

namespace CommonManage 
{
public ref class RawImage
{
public:
    RawImage(void);
    virtual ~RawImage(void);
    !RawImage(void);

    static RawImage^ LoadFromFile(System::String^ filename);
    static RawImage^ LoadRect(RawImage^ img, System::Drawing::Rectangle rect);
    static RawImage^ LoadFromRawImage(raw_image_t* p);
    static RawImage^ LoadFromRawImageNoCopy(raw_image_t* p);
    static RawImage^ LoadFromBMP(System::Drawing::Bitmap^ bmp, System::Drawing::Rectangle rect);
    static RawImage^ LoadRectMonoDoubleSize(RawImage^ img, System::Drawing::Rectangle rect);
    static RawImage^ CreateEmptyRawImage(int width, int height, int depth);

    void SaveToFile(System::String^ filename, System::Drawing::Imaging::ImageFormat^ format);
    void SaveToFile(System::String^ filename, System::Drawing::Imaging::ImageFormat^ format, System::Drawing::Rectangle rect);
    void Release(void);

    System::Drawing::Bitmap^ ToBMP();

    raw_image_t* CopyToRawImage();

    RawImage^ Clone() {
        raw_image_t* p = this->CopyToRawImage();
        return LoadFromRawImageNoCopy(p);
    }

	property int Width {
		int get(){ return (pImg ? pImg->width : 0); }
	}
    property int Height {
		int get(){ return (pImg ? pImg->height : 0);}
    }
    property unsigned char* Image {
		unsigned char* get(){ return (pImg ? pImg->raw : 0);}
	}
	property int Depth {
		int get(){ return (pImg ? pImg->depth : 0);}
    }
    property int Size {
		int get(){ return (pImg ? pImg->depth * pImg->height * pImg->width : 0);}
    }

protected:
    raw_image_t *pImg;
};
}