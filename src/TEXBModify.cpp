/**
* TEXBModify.cpp
* Modification of TextureBank class
**/

#include "TEXB.h"
#include "xy2uv.h"

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
		uint32_t* b=this->VertexIndexUVs[i];
		if(a->from_texb==this)
			delete a;
		LIBTEXB_FREE(b);
	}
}

TextureBank* TextureBank::Clone()
{
	TextureBank* texb=new TextureBank;
	uint32_t memsize=Width*Height*4;

	texb->RawImageWidth=Width;
	texb->RawImageHeight=Height;
	texb->RawImage=LIBTEXB_ALLOC(uint8_t,memsize);
	texb->Name=Name;
	memcpy(texb->RawImage,RawImage,memsize);

	memsize=ImageList_Id.size();
	for(uint32_t i=0;i<memsize;i++)
	{
		TextureImage* timg=ImageList_Id[i]->Clone();
		uint32_t* VrtxMem=reinterpret_cast<uint32_t*>(LIBTEXB_ALLOC(uint8_t,70));
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
	std::map<std::string,uint32_t>::iterator i=ImageList_Names.find(Image->Name);
	if(i!=ImageList_Names.end())
		return ReplaceImage(Image,i->second);

	return EINVAL;
}

int32_t TextureBank::ReplaceImage(TextureImage* Image,uint32_t Index)
{
	if(Index>=ImageList_Id.size()) return ERANGE;

	TextureImage* target=ImageList_Id[Index];
	if(target->Width!=Image->Width || target->Height!=Image->Height || target->Name!=Image->Name)
		return EINVAL;

	if(target->from_texb==this) delete target;
	ImageList_Id[Index]=Image;

	// Copy raw TIMG image to raw TEXB image
	uint32_t* Vrtx=VertexIndexUVs[Index];
	uint32_t* texbBmp=reinterpret_cast<uint32_t*>(RawImage);
	uint32_t* rawBmp=reinterpret_cast<uint32_t*>(Image->RawImage);
	Point v[4]={
		{Vrtx[0]/65536,Vrtx[1]/65536},
		{Vrtx[4]/65536,Vrtx[5]/65536},
		{Vrtx[8]/65536,Vrtx[9]/65536},
		{Vrtx[12]/65536,Vrtx[13]/65536}
	};
	UVPoint t[4]={
		{Vrtx[2]/65536.0,Vrtx[3]/65536.0},
		{Vrtx[6]/65536.0,Vrtx[7]/65536.0},
		{Vrtx[10]/65536.0,Vrtx[11]/65536.0},
		{Vrtx[14]/65536.0,Vrtx[15]/65536.0}
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

int32_t TextureBank::DefineImage(const Point* Vertexes,const UVPoint* UVs,std::string Name,uint32_t* Index)
{
	if(Index==NULL) return EINVAL;
	
	TextureImage* timg=new TextureImage();
	timg->Width=Vertexes[2].X;
	timg->Height=Vertexes[2].Y;
	timg->from_texb=this;
	timg->Name=Name;
	uint32_t* Vertex=reinterpret_cast<uint32_t*>(LIBTEXB_ALLOC(uint8_t,70));
	uint8_t* RawMem=LIBTEXB_ALLOC(uint8_t,timg->Width*timg->Height*4);
	timg->RawImage=RawMem;
	memcpy(&Vertex[16],"\x00\x01\x03\x03\x01\x02",6);
	memset(RawMem,0,timg->Width*timg->Height*4);

	for(uint32_t i=0;i<4;i++)
	{
		Vertex[i*4]=Vertexes[i].X*65536;
		Vertex[i*4+1]=Vertexes[i].Y*65536;
		Vertex[i*4+2]=uint32_t(UVs[i].U*65536);
		Vertex[i*4+3]=uint32_t(UVs[i].V*65536);
	}
	
	*Index=ImageList_Id.size();
	VertexIndexUVs.push_back(Vertex);
	ImageList_Id.push_back(timg);
	ImageList_Names[Name]=*Index;

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

void TextureBank::ReflectChanges()
{
	for(uint32_t i=0;i<ImageList_Id.size();i++)
	{
		TextureImage* timg=ImageList_Id[i];
		uint32_t* Vrtx=VertexIndexUVs[i];
		uint32_t* texbBmp=reinterpret_cast<uint32_t*>(RawImage);
		uint32_t* rawBmp=reinterpret_cast<uint32_t*>(timg->RawImage);
		Point v[4]={
			{Vrtx[0]/65536,Vrtx[1]/65536},
			{Vrtx[4]/65536,Vrtx[5]/65536},
			{Vrtx[8]/65536,Vrtx[9]/65536},
			{Vrtx[12]/65536,Vrtx[13]/65536}
		};
		UVPoint t[4]={
			{Vrtx[2]/65536.0,Vrtx[3]/65536.0},
			{Vrtx[6]/65536.0,Vrtx[7]/65536.0},
			{Vrtx[10]/65536.0,Vrtx[11]/65536.0},
			{Vrtx[14]/65536.0,Vrtx[15]/65536.0}
		};

		for(uint32_t y=0;y<timg->Height;y++)
		{
			for(uint32_t x=0;x<timg->Width;x++)
			{
				UVPoint uv=xy2uv(x,y,v[0],v[1],v[2],v[3],t[0],t[1],t[2],t[3]);
				texbBmp[uint32_t(uv.U*Width+0.5)+uint32_t(uv.V*Height+0.5)*Width]=rawBmp[x+y*timg->Width];
			}
		}
	}
}
