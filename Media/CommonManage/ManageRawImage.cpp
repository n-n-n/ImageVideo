#include "StdAfx.h"
#include <string>
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <math.h>

#include "ManageRawImage.h"

#if defined(_DEBUG) || defined(DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

namespace CommonManage
{
RawImage^ RawImage::CreateEmptyRawImage(int width, int height, int depth) 
{
    RawImage^ pImg = gcnew RawImage;
    pImg->pImg = RawImage_CreateBlank(width, height, depth);
    System::GC::AddMemoryPressure(sizeof(raw_image_t) + pImg->Size);
    return pImg;
}

RawImage::RawImage(void): pImg(0)
{
}

RawImage::~RawImage(void)
{
    this->!RawImage();
}

RawImage::!RawImage(void)
{
    Release();
}

void RawImage::SaveToFile(System::String^ filename, System::Drawing::Imaging::ImageFormat^ format)
{
    System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(Width, Height, System::Drawing::Imaging::PixelFormat::Format24bppRgb);
    System::Drawing::Imaging::BitmapData^ data;

    System::Drawing::Rectangle rect(0, 0, Width, Height);

    // 画像データのコピー
    try{
        data = bmp->LockBits(rect, 
            System::Drawing::Imaging::ImageLockMode::WriteOnly, bmp->PixelFormat);
        memset(static_cast<unsigned char*>(data->Scan0.ToPointer()), 0x00, data->Stride * data->Height);
        for (int i = 0; i < data->Height; i++) {
            memcpy(static_cast<unsigned char*>(data->Scan0.ToPointer()) + i * data->Stride, Image + i * bmp->Width * 3, 3 * bmp->Width);
        }
    } finally{
        bmp->UnlockBits(data);
    }

    bmp->Save(filename, format);
}

void RawImage::SaveToFile(System::String^ filename, System::Drawing::Imaging::ImageFormat^ format, System::Drawing::Rectangle rect)
{
    System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(rect.Width, rect.Height, System::Drawing::Imaging::PixelFormat::Format24bppRgb);
    System::Drawing::Imaging::BitmapData^ data;
    System::Drawing::Rectangle lockRect(0, 0, rect.Width, rect.Height);
    int src_offset = (rect.X + rect.Y * Width) * 3;
    // 画像データのコピー
    try{
        data = bmp->LockBits(lockRect, System::Drawing::Imaging::ImageLockMode::WriteOnly, bmp->PixelFormat);
        memset(static_cast<unsigned char*>(data->Scan0.ToPointer()), 0x00, data->Stride * data->Height);
        for (int i = 0; i < data->Height; i++) {
            memcpy(static_cast<unsigned char*>(data->Scan0.ToPointer()) + i * data->Stride, Image + i * Width * 3 + src_offset, data->Stride);
        }
    } finally{
        bmp->UnlockBits(data);
    }

    bmp->Save(filename, format);
}


void RawImage::Release(void)
{
    if( 0 == pImg) return;

    System::GC::RemoveMemoryPressure(sizeof(raw_image_t) + Size);
    RawImage_Delete(pImg);
    pImg = 0;
}

System::Drawing::Bitmap^ RawImage::ToBMP()
{
    if (Width == 0 || Height == 0) return nullptr;
    System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(Width, Height, System::Drawing::Imaging::PixelFormat::Format24bppRgb);
    System::Drawing::Imaging::BitmapData^ data;

    System::Drawing::Rectangle rect(0, 0, Width, Height);

    // 画像データのコピー
    try {
        data = bmp->LockBits(rect, 
            System::Drawing::Imaging::ImageLockMode::WriteOnly,
            bmp->PixelFormat);
        memset(static_cast<unsigned char*>(data->Scan0.ToPointer()), 0x00, data->Stride * data->Height);
        for (int i = 0; i < data->Height; i++) {
            memcpy(static_cast<unsigned char*>(data->Scan0.ToPointer()) + i * data->Stride, this->pImg->raw + i * Width * 3, 3 * Width);
        }
    } finally {
        bmp->UnlockBits(data);
    }

    return bmp;
}

RawImage^ RawImage::LoadFromFile(System::String^ filename)
{
    RawImage^ img = gcnew RawImage();

    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
    img->pImg = RawImage_CreateDepth3W(wstr.c_str());
    System::GC::AddMemoryPressure(sizeof(raw_image_t) + img->Size);

    return img;
}

RawImage^ RawImage::LoadRect(RawImage ^img, System::Drawing::Rectangle rect)
{
    RawImage^ outImg = CreateEmptyRawImage(rect.Width, rect.Height, img->Depth);

    for(int x = 0; x < rect.Width && x + rect.X < img->Width; ++x)
    {
        for(int y = 0; y < rect.Height && y + rect.Y < img->Height; ++y)
        {
            if(x + rect.X >= 0 && x + rect.X < img->Width
                && 0 <= y + rect.Y && y + rect.Y < img->Height)
            {
                memcpy(outImg->Image + y * outImg->Width * outImg->Depth + x * outImg->Depth,
                    img->Image + (y + rect.Y) * img->Width * img->Depth + (x + rect.X) * img->Depth,
                    img->Depth);
            }
        }
    }

    return outImg;
}

#define  CELLING_255(x) ((((x) < 0x00 ? 0x00 : (x)) > 0xFF)  ?  0xFF : (x))

static void Bilinear(BYTE *dst, BYTE *src, size_t sw, size_t sh)
{
    size_t idx = 0;
    size_t dw = 2 * sw;
    size_t dh = 2 * sh;
    size_t s_size = sw * sh;
    for(size_t dy = 0; dy < dh; ++dy) {
        float sy = (float)dy * 0.5f;
        int by = (int)floor((double)sy);
        float y1 = by-sy+1.0f;
        float y2 = sy - by;
        for(size_t dx = 0; dx < dw; ++dx, ++idx) {
            float sx = (float)dx * 0.5f;
            int bx = (int)floor((double)sx);
            float x1 = bx-sx+1.0f; 
            float x2 = sx-bx;
            size_t base1 = by * sw + bx;
            size_t base2 = base1 + sw;
            if (base2 < s_size) {
                dst[idx] = CELLING_255((int)(
                    y1 * (x1*(float)src[base1] + x2*(float)src[base1+1]) +
                    y2 * (x1*(float)src[base2] + x2*(float)src[base2+1])
                    ));
            } else {
                dst[idx] = CELLING_255((int)(x1*(float)src[base1] + x2*(float)src[base1+1]));
            }
        }
    }
}

#define	RGB2G(r, g, b) (UINT8)((11L * (UINT32)(r) + ((UINT32)(g) << 4) + 5L * (UINT32)(b)) >> 5)

RawImage^ RawImage::LoadRectMonoDoubleSize(RawImage ^img, System::Drawing::Rectangle rect)
{
    RawImage^ ret = CreateEmptyRawImage(rect.Width * 2, rect.Height * 2, 1);
    BYTE* p = new BYTE[rect.Width * rect.Height]; // work
    if (1 == img->Depth) 
    {
        for(int x = 0; x < rect.Width && x + rect.X < img->Width; ++x) {
            for(int y = 0; y < rect.Height && y + rect.Y < img->Height; ++y) {

                if(x + rect.X >= 0 && x + rect.X < img->Width
                    && 0 <= y + rect.Y && y + rect.Y < img->Height)
                {
                    *(p + y * rect.Width + x) = *(img->Image + (y + rect.Y) * img->Width * img->Depth + (x + rect.X));
                }
            }
        }
    } 
    else if (3 == img->Depth)
    {
        for(int x = 0; x < rect.Width && x + rect.X < img->Width; ++x) {
            for(int y = 0; y < rect.Height && y + rect.Y < img->Height; ++y) {

                if(x + rect.X >= 0 && x + rect.X < img->Width
                    && 0 <= y + rect.Y && y + rect.Y < img->Height)
                {
                    BYTE* address = img->Image + (y + rect.Y) * img->Width * 3 + (x + rect.X) * 3;
                    *(p + y * rect.Width + x) = RGB2G(*(address + 2), *(address + 1), *(address));
                }
            }
        }
    }
    Bilinear(ret->Image, p, rect.Width, rect.Height);
    delete [] p;
    return ret;
}
#if 0
RawImage^ RawImage::LoadFromImage(RawImage^ img,Drawing::Rectangle srcRect, double scale) 
{
    if( scale <= 0 ) return nullptr;

    //とりあえずNearest Neighbor
    RawImage^ pOutImg = RawImage::CreateEmptyRawImage(static_cast<int>(srcRect.Width*scale), static_cast<int>(srcRect.Height*scale), img->Depth);

    unsigned char* pOut = pOutImg->Image;
    unsigned char* pIn = img->Image;

    int outStride = pOutImg->Depth * pOutImg->Width;
    int inStride  = img->Depth * img->Width;
    int depth =img->Depth;
    for (int y = 0; y < pOutImg->Height; ++y ) 
    {
        for (int x = 0; x < pOutImg->Width; ++x ) 
        {
            int nInX = static_cast<int>((x+0.5)/scale);
            int nInY = static_cast<int>((y+0.5)/scale);
            if(nInX + srcRect.X >= 0 && nInX + srcRect.X < img->Width
                && 0 <= nInY + srcRect.Y && nInY + srcRect.Y < img->Height)
            {
                for(int i = 0; i < depth; ++i)
                {
                    (*pOut)//pOut[y * outStride + x * depth + i]
                        = pIn[(nInY + srcRect.Y) * inStride + (nInX + srcRect.X) * depth + i];
                    ++pOut;
                }
                /*
                memcpy(
                pOut + y * outStride + x * depth,
                pIn + (nInY + srcRect.Y) * inStride + (nInX + srcRect.X) * depth,
                depth);
                */
            }
        }
    }
    return pOutImg;
}
#endif
RawImage^ RawImage::LoadFromRawImage(raw_image_t* p)
{
    RawImage^ img = CreateEmptyRawImage(p->width,p->height,p->depth);
    memcpy(img->Image, p->raw, img->Size);
    return img;
}

RawImage^ RawImage::LoadFromRawImageNoCopy(raw_image_t* p)
{
    RawImage^ img = gcnew RawImage();
    img->pImg = p;
    System::GC::AddMemoryPressure(sizeof(raw_image_t) + img->Size);
    return img;
}

#if 0
RawImage^ RawImage::LoadFromImage(RawImage^ img, double scale) 
{
    if(scale <= 0) return nullptr;

    //とりあえずNearest Neighbor
    RawImage^ pOutImg = RawImage::CreateEmptyRawImage(static_cast<int>(img->Width*scale), static_cast<int>(img->Height*scale), img->Depth);

    for (int y = 0; y < pOutImg->Height; ++y ) 
    {
        for (int x = 0; x < pOutImg->Width; ++x ) 
        {
            int nInX = static_cast<int>((x+0.5)/scale);
            int nInY = static_cast<int>((y+0.5)/scale);
            memcpy(pOutImg->Image + y*pOutImg->Depth*pOutImg->Width + x*pOutImg->Depth, img->Image + nInY*img->Depth*img->Width + nInX * img->Depth, img->Depth);
        }
    }

    return pOutImg;
}

#endif
RawImage^ RawImage::LoadFromBMP(System::Drawing::Bitmap^ bitmap, System::Drawing::Rectangle rect) 
{
    RawImage^ img = CreateEmptyRawImage(rect.Width,rect.Height,3);

    System::Drawing::Imaging::BitmapData^ data = bitmap->LockBits(rect, System::Drawing::Imaging::ImageLockMode::ReadOnly, bitmap->PixelFormat);

    try {
        memset(img->Image, 0x00, img->Width * img->Height * img->Depth);

        for(int i = 0; i < img->Height; ++i) {
            memcpy(img->Image + i * img->Width * img->Depth, static_cast<char*>(data->Scan0.ToPointer()) + i * data->Stride, data->Width * 3);
        }
    } finally {
        bitmap->UnlockBits(data);
    }
    return img;
}

raw_image_t* RawImage::CopyToRawImage()
{
    raw_image_t* p = RawImage_CreateDirty(Width, Height, Depth);
    memcpy( p->raw, Image, Size);
    System::GC::AddMemoryPressure(sizeof(raw_image_t) + Size);
    return p;
}

}