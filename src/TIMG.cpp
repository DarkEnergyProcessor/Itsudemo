/**
* TIMG.cpp
* TextureImage class implementation
**/

#include "TEXB.h"

#include <cstdlib>
#include <cstring>

TextureImage::TextureImage(uint32_t width,uint32_t height,uint8_t* raw_image)
{
	uint32_t size=width*height*4;
	Width=width;
	Height=height;
	from_texb=NULL;
	RawImage=LIBTEXB_ALLOC(uint8_t,size);

	if(raw_image==NULL)
		memset(RawImage,255,size);
	else
		memcpy(RawImage,raw_image,size);
}

TextureImage::~TextureImage()
{
	LIBTEXB_FREE(RawImage);
}

TextureImage* TextureImage::Clone()
{
	TextureImage* timg=new TextureImage;
	timg->from_texb=NULL;
	timg->Width=Width;
	timg->Height=Height;
	timg->RawImage=LIBTEXB_ALLOC(uint8_t,Width*Height*4);
	timg->Name=Name;
	memcpy(timg->RawImage,RawImage,Width*Height*4);

	return timg;
}
