/**
* TIMG.cpp
* TextureImage class implementation
**/

#include "TEXB.h"
#include "TEXBPixel.h"

#include <cstdlib>
#include <cstring>

TextureImage::TextureImage(uint32_t width,uint32_t height,uint8_t* raw_image)
: TextureImage(width, height, TEXB_CHANNEL_KIND_RGBA, TEXB_PIXEL_FORMAT_BYTE, raw_image)
{ }

TextureImage::TextureImage(uint32_t width, uint32_t height, TEXB_CHANNEL_KIND channelkind, TEXB_PIXEL_FORMAT pixfmt, uint8_t *pixels = nullptr)
: from_texb(nullptr)
, Width(width)
, Height(height)
, Name("")
, ChannelKind(channelkind)
, PixelFormat(pixfmt)
{
	size_t srcSize = width * height * GetBytePerPixel((uint8_t) channelkind, (uint8_t) pixfmt);
	size_t size = width * height * 4;

	RawImage = new uint8_t[size];

	if (pixels)
		convert_map(pixels, width, height, channelkind, pixfmt, RawImage);
	else
		memset(RawImage, 255, size);
}

TextureImage::~TextureImage()
{
	LIBTEXB_FREE(RawImage);
}

TextureImage* TextureImage::Clone()
{
	TextureImage* timg = new TextureImage(Width, Height, ChannelKind, PixelFormat, RawImage);
	timg->Name=Name;
	return timg;
}
