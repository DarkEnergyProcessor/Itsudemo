/**
* TIMG.cpp
* TextureImage class implementation
**/

#include "TEXB.h"

#include <cstdlib>
#include <cstring>

TextureImage::TextureImage(uint32_t width,uint32_t height,uint8_t* raw_image)
{
	if(raw_image==NULL)
	{
		uint32_t size=width*height*4;
		raw_image=LIBTEXB_ALLOC(uint8_t,size);
		memset(raw_image,255,size);
	}

	Width=width;
	Height=height;
	RawImage=raw_image;
	from_texb=NULL;
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
