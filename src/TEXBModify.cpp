/**
* TEXBModify.cpp
* Modification of TextureBank class
**/

#include "TEXB.h"
#include "xy2uv.h"

#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include <cerrno>
#include <cstdlib>
#include <cstring>

TextureBank::TextureBank(uint32_t _Width,uint32_t _Height):Width(RawImageWidth),Height(RawImageHeight),Flags(_Flags)
{
	uint32_t rawimage_size=_Width*_Height*4;
	RawImageWidth=_Width;
	RawImageHeight=_Height;
	RawImage=LIBTEXB_ALLOC(uint8_t,rawimage_size);	// 4-byte/pixel
	_Flags=0;

	memset(RawImage,0,rawimage_size);
}

TextureBank::~TextureBank()
{
	for(uint32_t i=0;i<this->ImageList_Id.size();i++)
	{
		TextureImage* a=this->ImageList_Id[i];
		delete this->VertexIndexUVs[i];
		if(a->from_texb==this)
			delete a;
	}
}

TextureBank* TextureBank::Clone()
{
	TextureBank* texb=new TextureBank;
	size_t memsize=Width*Height*4;

	texb->RawImageWidth=Width;
	texb->RawImageHeight=Height;
	texb->RawImage=LIBTEXB_ALLOC(uint8_t,memsize);
	texb->Name=Name;
	memcpy(texb->RawImage,RawImage,memsize);

	memsize=ImageList_Id.size();
	for(uint32_t i=0;i<memsize;i++)
	{
		TextureImage* timg=ImageList_Id[i]->Clone();
		FixedVertexIndexFormat *VrtxMem = new FixedVertexIndexFormat(*VertexIndexUVs[i]);
		timg->from_texb=texb;
		texb->ImageList_Id.push_back(timg);
		texb->ImageList_Names[timg->Name]=i;
		texb->VertexIndexUVs.push_back(VrtxMem);

		memcpy(VrtxMem,VertexIndexUVs[i],70);
	}

	return texb;
}

int32_t TextureBank::ReplaceImage(TextureImage* Image)
{
	std::map<std::string,size_t>::iterator i=ImageList_Names.find(Image->Name);
	if(i!=ImageList_Names.end())
		return ReplaceImage(Image,i->second);

	return EINVAL;
}

int32_t TextureBank::ReplaceImage(TextureImage* Image,size_t Index)
{
	if(Index>=ImageList_Id.size()) return ERANGE;

	TextureImage* target=ImageList_Id[Index];
	if(target->Width!=Image->Width || target->Height!=Image->Height || target->Name!=Image->Name)
		return EINVAL;

	if(target->from_texb==this) delete target;
	ImageList_Id[Index]=Image;

	// Copy raw TIMG image to raw TEXB image
	FixedVertexIndexFormat* Vrtx=VertexIndexUVs[Index];
	uint32_t* texbBmp=reinterpret_cast<uint32_t*>(RawImage);
	uint32_t* rawBmp=reinterpret_cast<uint32_t*>(Image->RawImage);
	Point v[4]={
		{Vrtx->Vertices[0].X/65536,Vrtx->Vertices[0].Y/65536},
		{Vrtx->Vertices[1].X/65536,Vrtx->Vertices[1].Y/65536},
		{Vrtx->Vertices[2].X/65536,Vrtx->Vertices[2].Y/65536},
		{Vrtx->Vertices[3].X/65536,Vrtx->Vertices[3].Y/65536}
	};
	UVPoint t[4]={
		{Vrtx->Vertices[0].U/65536.0,Vrtx->Vertices[0].V/65536.0},
		{Vrtx->Vertices[1].U/65536.0,Vrtx->Vertices[1].V/65536.0},
		{Vrtx->Vertices[2].U/65536.0,Vrtx->Vertices[2].V/65536.0},
		{Vrtx->Vertices[3].V/65536.0,Vrtx->Vertices[3].V/65536.0}
	};

	for(uint32_t y=0;y<Image->Height;y++)
	{
		for(uint32_t x=0;x<Image->Width;x++)
		{
			UVPoint uv=xy2uv(x,y,v[0],v[1],v[2],v[3],t[0],t[1],t[2],t[3]);
			texbBmp[uint32_t(uv.U*Width+0.5)+uint32_t(uv.V*Height+0.5)*Width]=rawBmp[x+y*Image->Width];
		}
	}

	return 0;
}

void TextureBank::ReplaceImage(TextureImage *image, size_t index, bool _)
{

}

int32_t TextureBank::DefineImage(const Point* Vertexes,const UVPoint* UVs,std::string Name,uint32_t* Index)
{
	if(Index==NULL) return EINVAL;

	uint32_t ImgWidth = Vertexes[2].X - Vertexes[0].X;
	uint32_t ImgHeight = Vertexes[2].Y - Vertexes[0].Y;
	uint32_t Left = uint32_t(UVs[0].U * Width);
	uint32_t Top = uint32_t(UVs[0].V * Height);
	uint32_t Right = uint32_t(UVs[2].U * Width);
	uint32_t Bottom = uint32_t(UVs[2].V * Height);

	size_t NewIndex = DefineImage(Name, Top, Left, Bottom, Right, ImgWidth, ImgHeight);
	*Index = (uint32_t) NewIndex;
	return 0;

	return 0;
}

int32_t TextureBank::DefineImage(const Point* WhereWidthHeight,std::string Name,uint32_t* Index)
{
	Point v[4]={
		{0,0},
		{WhereWidthHeight[1].X,0},
		{WhereWidthHeight[1].X,WhereWidthHeight[1].Y},
		{0,WhereWidthHeight[1].Y}
	};
	// Applied MiraiNoHana TEXBModify.cpp patch.
	UVPoint t[4]={
		{double(WhereWidthHeight[0].X)/double(Width),double(WhereWidthHeight[0].Y)/double(Height)},
		{double(WhereWidthHeight[1].X+WhereWidthHeight[0].X)/double(Width),double(WhereWidthHeight[0].Y)/double(Height)},
		{double(WhereWidthHeight[1].X+WhereWidthHeight[0].X)/double(Width),double(WhereWidthHeight[1].Y+WhereWidthHeight[0].Y)/double(Height)},
		{double(WhereWidthHeight[0].X)/double(Width),double(WhereWidthHeight[1].Y+WhereWidthHeight[0].Y)/double(Height)}
	};
	return DefineImage(v,t,Name,Index);
}

size_t TextureBank::DefineImage(const std::string &name, uint32_t top, uint32_t left, uint32_t bottom, uint32_t right, uint32_t width, uint32_t height)
{
	TextureImage* timg = new TextureImage(width, height, TEXB_GET_CHANNEL_KIND(Flags), TEXB_GET_PIXEL_FORMAT(Flags));
	timg->from_texb = this;
	timg->Name = Name;

	FixedVertexIndexFormat *VertData = new FixedVertexIndexFormat();
	/*
		Layout
		0-----1
		|    /|
		|   / |
		|  /  |
		| /   |
		|/    |
		3-----2
	*/
	VertData->Indices = {0, 1, 3, 3, 1, 2};
	VertData->Vertices = {
		VertexFormat({0            , 0             , left * 65536 , top * 65536   }),
		VertexFormat({width * 65536, 0             , right * 65536, top * 65536   }),
		VertexFormat({width * 65536, height * 65536, right * 65536, bottom * 65536}),
		VertexFormat({0            , height * 65536, left * 65536 , bottom * 65536})
	};
	
	size_t Index = ImageList_Id.size();
	VertexIndexUVs.push_back(VertData);
	ImageList_Id.push_back(timg);
	ImageList_Names[Name] = Index;

	return Index;
}

void TextureBank::ReflectChanges()
{
	for(uint32_t i=0;i<ImageList_Id.size();i++)
	{
		TextureImage* timg=ImageList_Id[i];
		FixedVertexIndexFormat *Vi = VertexIndexUVs[i];
		uint32_t* texbBmp=reinterpret_cast<uint32_t*>(RawImage);
		uint32_t* rawBmp=reinterpret_cast<uint32_t*>(timg->RawImage);
		Point v[4] = {
			{Vi->Vertices[0].X / 65536, Vi->Vertices[0].Y / 65536},
			{Vi->Vertices[1].X / 65536, Vi->Vertices[1].Y / 65536},
			{Vi->Vertices[2].X / 65536, Vi->Vertices[2].Y / 65536},
			{Vi->Vertices[3].X / 65536, Vi->Vertices[3].Y / 65536},
		};
		UVPoint t[4] = {
			{Vi->Vertices[0].U / 65536.0, Vi->Vertices[0].V / 65536.0},
			{Vi->Vertices[1].U / 65536.0, Vi->Vertices[1].V / 65536.0},
			{Vi->Vertices[2].U / 65536.0, Vi->Vertices[2].V / 65536.0},
			{Vi->Vertices[3].U / 65536.0, Vi->Vertices[3].V / 65536.0},
		};

		uint32_t min_x = std::min(v[0].X, std::min(v[1].X, std::min(v[2].X, v[3].X)));
		uint32_t min_y = std::min(v[0].Y, std::min(v[1].Y, std::min(v[2].Y, v[3].Y)));
		uint32_t max_x = std::max(v[0].X, std::max(v[1].X, std::max(v[2].X, v[3].X)));
		uint32_t max_y = std::max(v[0].Y, std::max(v[1].Y, std::max(v[2].Y, v[3].Y)));

		for(uint32_t y = min_y; y < max_y; y++)
		{
			for(uint32_t x = min_x; x < max_x; x++)
			{
				UVPoint uv=xy2uv(x, y, v[0], v[1], v[2], v[3], t[0], t[1], t[2], t[3]);
				texbBmp[uint32_t(uv.U * Width + 0.5) + uint32_t(uv.V * Height + 0.5) * Width] = rawBmp[x + y * timg->Width];
			}
		}
	}
}
