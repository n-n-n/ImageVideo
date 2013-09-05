#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "Gdiplus.lib")
#include <string>

#include "RawImage.h"

#if defined(_DEBUG) || defined(DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


static void copy_data(BYTE* p, Gdiplus::Rect *pRectrect, Gdiplus::BitmapData* pBitmapData, Gdiplus::ColorPalette* pPalette)
{
	switch(pBitmapData->PixelFormat) {
	case PixelFormat24bppRGB:
		for (size_t i = 0; i < pBitmapData->Height; i++) {
			memcpy(p + i * pBitmapData->Width * 3, static_cast<BYTE*>(pBitmapData->Scan0) + i * pBitmapData->Stride, 3 * pBitmapData->Width);
		}
		break;
	case PixelFormat32bppRGB:
	case PixelFormat32bppARGB:
		for (size_t i = 0; i < pBitmapData->Height; i++) {
			for (size_t j = 0; j < pBitmapData->Width; j++) {
				/* RGB 3ƒoƒCƒg–ˆ‚É‘‚«ž‚Ý */
				memcpy(p + i * pBitmapData->Width * 3 + 3 * j, static_cast<BYTE*>(pBitmapData->Scan0) + i * pBitmapData->Stride + 4 * j, 3);
			}
		}
		break;
	case PixelFormat8bppIndexed:
		for (size_t i = 0; i < pBitmapData->Height; i++) {
			for (size_t j = 0; j < pBitmapData->Width; j++) {
				BYTE val = static_cast<BYTE*>(pBitmapData->Scan0)[i * pBitmapData->Stride + j];
				DWORD dwColor = pPalette->Entries[val];
				memcpy(p + i * pBitmapData->Width * 3 + 3 * j, &dwColor, 3);
			}
		}
		break;
	case PixelFormat4bppIndexed:
		for (size_t i = 0; i < pBitmapData->Height; i++) {
			size_t j;
			BYTE val, val0, val1;
			DWORD dwColor;
			for (j = 0; j < pBitmapData->Stride - 1; j++) {
				val = static_cast<BYTE*>(pBitmapData->Scan0)[i * pBitmapData->Stride + j];
				val0 = 0x0F & val;
				val1 = 0x0F & (val >> 4);

				dwColor = pPalette->Entries[val1];
				memcpy(p + i * pBitmapData->Width * 3 + 3 * 2 * j, &dwColor, 3);

				dwColor = pPalette->Entries[val0];
				memcpy(p + i * pBitmapData->Width * 3 + 3 * (2 * j + 1), &dwColor, 3);
			}

			val = static_cast<BYTE*>(pBitmapData->Scan0)[i * pBitmapData->Stride + j];
			val0 = 0x0F & val;
			val1 = 0x0F & (val >> 4);

			dwColor = pPalette->Entries[val1];
			memcpy(p + i * pBitmapData->Width * 3 + 3 * 2 * j, &dwColor, 3);

			if ((2 * j + 1) < pBitmapData->Width) {
				dwColor = pPalette->Entries[val0];
				memcpy(p + i * pBitmapData->Width * 3 + 3 * (2 * j + 1), &dwColor, 3);
			}
		}
		break;
	default:
		break;
	}
}
raw_image_t* RawImage_CreateDepth3(const char* filename)
{
	/* file name */
	std::string strFile(filename);
	wchar_t wstrFile[_MAX_PATH];
	size_t wstrFileNum;
	mbstowcs_s(&wstrFileNum, wstrFile, _MAX_PATH, strFile.c_str(), strFile.size());
	return RawImage_CreateDepth3W(wstrFile);
}

raw_image_t* RawImage_CreateDepth3W(const wchar_t* filename)
{
	raw_image_t *p = 0;

	ULONG_PTR nToken;

	Gdiplus::GdiplusStartupInput gdiStartupInput;
	Gdiplus::GdiplusStartup(&nToken, &gdiStartupInput, NULL);
	Gdiplus::Bitmap* pBitmap = 0;

	pBitmap = Gdiplus::Bitmap::FromFile(filename, TRUE);

	if (0 != pBitmap) {

		Gdiplus::ColorPalette* pPalette = 0;
		Gdiplus::PixelFormat format = pBitmap->GetPixelFormat();

		switch(format) {
			case PixelFormat24bppRGB:
			case PixelFormat32bppRGB:
			case PixelFormat32bppARGB:
				break;
			case PixelFormat8bppIndexed:
			case PixelFormat4bppIndexed:
				{
					int palette_size = pBitmap->GetPaletteSize();
					pPalette = (Gdiplus::ColorPalette*)malloc(palette_size);
					pBitmap->GetPalette(pPalette, palette_size);
					break;
				}
			default:
				goto EXIT;
				break;
		}

		Gdiplus::Rect rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
		Gdiplus::BitmapData bitmapData;

		pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, pBitmap->GetPixelFormat(), &bitmapData);
		{
			p = RawImage_CreateDirty(bitmapData.Height, bitmapData.Width, 3);
			copy_data(p->raw, &rect, &bitmapData, pPalette);
			pBitmap->UnlockBits(&bitmapData);
		}
		delete pBitmap;
		if (pPalette) free(pPalette);
	}
EXIT:
	Gdiplus::GdiplusShutdown(nToken);
	return p;
}

void RawImage_Delete(raw_image_t* ptr)
{
	if (ptr) {
		delete [] ptr->raw;
	}
	delete ptr;
    ptr = 0;
}

#if 0
static int full_size = 0; // thread unsafe
static int count = 0;
#endif

raw_image_t* RawImage_CreateDirty(int w, int h, int d)
{
    raw_image_t* ret =  new raw_image_t;
    int size =  w * h * d;
    ret->width  = w;
    ret->height = h;
    ret->depth  = d;
    ret ->raw = new unsigned char[size];
#if 0
    full_size += size;
   _RPT3(_CRT_WARN, "%d mem %dKB (%dKB)\n", count++, size/1024, full_size/1024);
#endif
    return ret;
}
raw_image_t* RawImage_CreateBlank(int w, int h, int d)
{
    raw_image_t* ret = RawImage_CreateDirty(w, h, d);
    memset(ret->raw, 0x00, sizeof(unsigned char) * w * h * d);
    return ret;
}

