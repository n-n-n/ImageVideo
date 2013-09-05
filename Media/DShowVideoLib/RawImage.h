#ifndef _RAW_IMAGE_LIB_H_
#define _RAW_IMAGE_LIB_H_
/*
 
��������:
- BMP(�t���J���[�A256�F, 16�r�b�g), GIF, TIF, PNG, JPG�ɑΉ����Ă���
- ������Unicode�������s���邽�߁A�t�@�C������ASCII�ȊO�̏ꍇ�ɂ́A
�g�p�O�Ƀ��P�[���ݒ肪�K�v�ɂȂ�ꍇ������
- �Ή�����摜�t�@�C���œǂݍ��݂Ɏ��s����ꍇ�A
 �摜�t�@�C���Ƀt�H�[�}�b�g���K�؂ɐݒ肳��Ă��Ȃ��ꍇ������B
 �y�C���g���̉摜�ҏW�A�v���P�[�V�����Ńt�H�[�}�b�g���w�肵�čĕۑ������
 ��������ꍇ������
*/


/* 
  �摜raw data�p�̍\���� 
*/
typedef struct __raw_image_t {
	unsigned char* raw; /* raw data ��pointer */
	unsigned int width;
	unsigned int height;
	unsigned int depth;
} raw_image_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 
Abs: �摜�t�@�C����ǂݍ����raw_image_t��raw data���쐬����B�t�@�C�����̓}���`�o�C�g����
Desc: �t�@�C���摜�t�@�C����ǂݍ���, depth=3��raw data���p�\���̂ɍ쐬����B
�쐬���s�̏ꍇ�ɂ�NULL���Ԃ�
*/
raw_image_t* RawImage_CreateDepth3(const char* filename);

/* 
Abs: �摜�t�@�C����ǂݍ����raw_image_t��raw data���쐬����B�t�@�C�����̓��C�h(Unicode)����
Desc: �t�@�C���摜�t�@�C����ǂݍ���, depth=3��raw data���p�\���̂ɍ쐬����B
�쐬���s�̏ꍇ�ɂ�NULL���Ԃ�
*/
raw_image_t* RawImage_CreateDepth3W(const wchar_t* filename);

/* 
Abs: raw_image_t���������
Desc: CreateRawImage*�ō쐬������p�\���̂�������� 
*/
void RawImage_Delete(raw_image_t* p);

raw_image_t* RawImage_CreateDirty(int w, int h, int d);
raw_image_t* RawImage_CreateBlank(int w, int h, int d);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RAW_IMAGE_LIB_H_ */
