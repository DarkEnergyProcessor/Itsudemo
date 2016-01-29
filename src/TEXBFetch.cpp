/**
* TEXBFetch.cpp
* TextureBank::Fetch* implementation
**/

#include "TEXB.h"

#include <string>
#include <vector>

#include <cerrno>

#include <stdint.h>

TextureImage* TextureBank::FetchImage(std::string ImageName)
{
	std::map<std::string,uint32_t>::iterator i=ImageList_Names.find(ImageName);
	if(i==this->ImageList_Names.end())
		LIBTEXB_FAILWITH(ENOENT);

	return ImageList_Id[i->second];
}

TextureImage* TextureBank::FetchImage(uint32_t Index)
{
	if(Index<this->ImageList_Id.size())
		return (this->ImageList_Id)[Index];
	else
		LIBTEXB_FAILWITH(ERANGE);
}

std::vector<TextureImage*> TextureBank::FetchAll()
{
	std::vector<TextureImage*> i=this->ImageList_Id;
	return i;
}

std::vector<uint8_t> TextureBank::FetchRaw()
{
	return std::vector<uint8_t>(this->RawImage,this->RawImage+this->RawImageWidth*this->RawImageHeight*4);
}
