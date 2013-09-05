#ifndef _RAW_IMAGE_LIB_H_
#define _RAW_IMAGE_LIB_H_
/*
 
制限事項:
- BMP(フルカラー、256色, 16ビット), GIF, TIF, PNG, JPGに対応している
- 内部でUnicode処理が行われるため、ファイル名がASCII以外の場合には、
使用前にロケール設定が必要になる場合がある
- 対応する画像ファイルで読み込みに失敗する場合、
 画像ファイルにフォーマットが適切に設定されていない場合がある。
 ペイント等の画像編集アプリケーションでフォーマットを指定して再保存すると
 成功する場合がある
*/


/* 
  画像raw data用の構造体 
*/
typedef struct __raw_image_t {
	unsigned char* raw; /* raw data のpointer */
	unsigned int width;
	unsigned int height;
	unsigned int depth;
} raw_image_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 
Abs: 画像ファイルを読み込んでraw_image_tとraw dataを作成する。ファイル名はマルチバイト文字
Desc: ファイル画像ファイルを読み込み, depth=3のraw dataを専用構造体に作成する。
作成失敗の場合にはNULLが返る
*/
raw_image_t* RawImage_CreateDepth3(const char* filename);

/* 
Abs: 画像ファイルを読み込んでraw_image_tとraw dataを作成する。ファイル名はワイド(Unicode)文字
Desc: ファイル画像ファイルを読み込み, depth=3のraw dataを専用構造体に作成する。
作成失敗の場合にはNULLが返る
*/
raw_image_t* RawImage_CreateDepth3W(const wchar_t* filename);

/* 
Abs: raw_image_tを解放する
Desc: CreateRawImage*で作成した専用構造体を解放する 
*/
void RawImage_Delete(raw_image_t* p);

raw_image_t* RawImage_CreateDirty(int w, int h, int d);
raw_image_t* RawImage_CreateBlank(int w, int h, int d);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RAW_IMAGE_LIB_H_ */
