/**
* TEXBLoad.cpp
* Load and parse Texture Bank file
**/

#include "TEXB.h"
#include "TEXBPixel.h"
#include "xy2uv.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdlib>

#include <stdint.h>

#include <zlib.h>

uint8_t GetBytePerPixel(uint16_t TexbFlags)
{
	uint8_t iff=TexbFlags&7;
	switch(uint8_t(TexbFlags)>>6)
	{
		case 0:
		case 1:
		case 2:
			return 2;
		case 3:
		{
			switch(iff)
			{
			case 0:
				return 1;
			default:
				return iff;
			}
		}
		default:
			return 0;
	}
}

TextureBank* TextureBank::FromMemory(uint8_t* _mem,size_t _n)
{
	TextureBank* texb=NULL;
	char* temp_buffer=NULL;
	uint8_t prebuf[4];
	std::stringstream memory_buffer(std::string(reinterpret_cast<char*>(_mem),_n),std::ios::binary|std::ios::in);
	memory_buffer.seekg(0,std::ios_base::beg);

	// Find header
	temp_buffer=LIBTEXB_ALLOC(char,4);
	memory_buffer.read(temp_buffer,4);
	if(memcmp(temp_buffer,"TEXB",4))
	{
		LIBTEXB_FREE(temp_buffer);
		LIBTEXB_FAILWITH(EBADF);
	}
	memory_buffer.seekg(4,std::ios::cur);

	texb=new TextureBank();
	
	// TEXB Name without T prefix and extension
	uint16_t temp_short=0;
	memory_buffer.read(reinterpret_cast<char*>(prebuf),2);
	temp_short=prebuf[0]<<8|prebuf[1];
	temp_buffer=LIBTEXB_ALLOC(char,temp_short);
	memory_buffer.read(temp_buffer,temp_short);
	texb->Name=std::string(temp_buffer);
	texb->Name=texb->Name.substr(1,texb->Name.find(".texb")-1);	// Discard .texb
	
	// Width & Height
	uint16_t tWidth,tHeight;
	memory_buffer.read(reinterpret_cast<char*>(prebuf),4);
	texb->RawImageWidth=tWidth=(prebuf[0]<<8)|prebuf[1];
	texb->RawImageHeight=tHeight=(prebuf[2]<<8)|prebuf[3];

	// Flags
	uint16_t TexbFlags=0;
	memory_buffer.read(reinterpret_cast<char*>(prebuf),2);
	TexbFlags=(prebuf[0]<<8)|prebuf[1];
	texb->_Flags=TexbFlags;

	// Vertex & Index count
	uint16_t VertexCount=0;
	uint16_t IndexCount=0;
	uint16_t ImageCount=0;
	std::vector<TextureImage*> imageListTemp;
	memory_buffer.read(reinterpret_cast<char*>(prebuf),4);
	VertexCount=(prebuf[0]<<8)|prebuf[1];
	IndexCount=(prebuf[2]<<8)|prebuf[3];
	memory_buffer.read(reinterpret_cast<char*>(prebuf),2);
	ImageCount=(prebuf[0]<<8)|prebuf[1];

	LIBTEXB_FREE(temp_buffer);
	for(unsigned int i=0;i<ImageCount;i++)
	{
		TextureImage* timg=new TextureImage;
		timg->from_texb=texb;

		// Texture Image name
		memory_buffer.seekg(6,std::ios::cur);	// skip TIMG and it's size.
		memory_buffer.read(reinterpret_cast<char*>(prebuf),2);
		temp_short=(prebuf[0]<<8)|prebuf[1];
		temp_buffer=LIBTEXB_ALLOC(char,temp_short);
		memory_buffer.read(temp_buffer,temp_short);
		timg->Name=std::string(temp_buffer);
		timg->Name=timg->Name.substr(1,timg->Name.find(".png.imag")-1);	// Discard .png.imag

		// Vertex & index information
		uint16_t Subimgs=0;
		memory_buffer.read(reinterpret_cast<char*>(&Subimgs),2);
		if(Subimgs==0xffff)
		{
			memory_buffer.read(reinterpret_cast<char*>(prebuf),2);
			temp_short=(prebuf[0]<<8)|prebuf[1];
			for(unsigned int j=0;j<temp_short;j++)
			{
				// Based from extb.c, it just skips the attribute.
				uint8_t key_and_type[2];
				memory_buffer.read(reinterpret_cast<char*>(key_and_type),2);
				switch(key_and_type[1])
					{
					case 0:
					case 1:
					{
						memory_buffer.seekg(4,std::ios::cur);
						break;
					}
					case 2:
					{
						uint16_t len;
						memory_buffer.read(reinterpret_cast<char*>(&len),2);
						memory_buffer.seekg(len,std::ios::cur);
						break;
					}
					default:
					{
						// Based from extb.c, this is unknown and the program just throws assert failed. Delete all memory and exit
						for(std::vector<TextureImage*>::iterator k=imageListTemp.begin();k!=imageListTemp.end();k++)
							LIBTEXB_FREE((*k));
						LIBTEXB_FREE(temp_buffer);
						LIBTEXB_FREE(timg);
						LIBTEXB_FREE(texb);
						LIBTEXB_FAILWITH(EBADF);
					}
				}
			}
			memory_buffer.read(reinterpret_cast<char*>(prebuf),2);
		}

		Subimgs=(prebuf[0]<<8)|prebuf[1];
		if(Subimgs>1)
		{
			// Currently not supported. It breaks the TextureImage structure
			TextureImage* t;
			for(std::vector<TextureImage*>::iterator k=imageListTemp.begin();k!=imageListTemp.end();k++)
			{
				t=*k;
				LIBTEXB_FREE(t);
			}
			LIBTEXB_FREE(temp_buffer);
			LIBTEXB_FREE(timg);
			LIBTEXB_FREE(texb);
			LIBTEXB_FAILWITH(EBADF);
		}
		uint8_t verts=memory_buffer.get();
		uint8_t idxs=memory_buffer.get();

		memory_buffer.read(reinterpret_cast<char*>(prebuf),4);
		timg->Width=(prebuf[0]<<8)|prebuf[1];
		timg->Height=(prebuf[2]<<8)|prebuf[3];

		/*
		memory_buffer.read(reinterpret_cast<char*>(&cx),2);
		memory_buffer.read(reinterpret_cast<char*>(&cy),2);
		*/
		memory_buffer.seekg(4,std::ios::cur);

		uint32_t vsize=verts*sizeof(uint32_t)*4;
		uint32_t* vibuf=reinterpret_cast<uint32_t*>(LIBTEXB_ALLOC(uint8_t,vsize+idxs));
		for(unsigned int l=0;l<verts*4;l++)
		{
			memory_buffer.read(reinterpret_cast<char*>(prebuf),4);
			vibuf[l]=(prebuf[0]<<24)|(prebuf[1]<<16)|(prebuf[2]<<8)|prebuf[3];
		}

		memory_buffer.read(reinterpret_cast<char*>(vibuf)+vsize,idxs);
		LIBTEXB_FREE(temp_buffer);
		imageListTemp.push_back(timg);
		texb->VertexIndexUVs.push_back(vibuf);
	}
	uint32_t dataSize=_n-memory_buffer.tellg();
	uint8_t* rawData=LIBTEXB_ALLOC(uint8_t,dataSize);
	if(TexbFlags&0x8)
	{
		// Read 4 more byte to get the compression type.
		uint32_t compressType=0;
		memory_buffer.read(reinterpret_cast<char*>(&compressType),4);
		if(compressType==0)
		{
			// deflate compression.
			z_stream zstate;
			memset(&zstate,0,sizeof(z_stream));
			if(inflateInit(&zstate)!=Z_OK)
			{
				// Cannot initialize ZLib
				goto FailAndExit;
			}
			
			uint32_t infsize=tWidth*tHeight*GetBytePerPixel(TexbFlags);
			uint8_t* inf=LIBTEXB_ALLOC(uint8_t,infsize);
			
			zstate.avail_in=dataSize;
			zstate.next_in=rawData;
			zstate.avail_out=infsize;
			zstate.next_out=inf;
			memory_buffer.read(reinterpret_cast<char*>(rawData),dataSize-4);

			int ret=inflate(&zstate,Z_NO_FLUSH);
			if(ret!=Z_OK && ret!=Z_STREAM_END)
			{
				// Cannot initialize zlib
				LIBTEXB_FREE(inf);
				LIBTEXB_FREE(rawData);
				goto FailAndExit;
			}
			inflateEnd(&zstate);
			LIBTEXB_FREE(rawData);
			rawData=inf;
			dataSize=infsize;
		}
		else
		{
			// Upps, I don't know the kind of the compression.
			FailAndExit:
			for(std::vector<TextureImage*>::iterator k=imageListTemp.begin();k!=imageListTemp.end();k++)
				LIBTEXB_FREE((*k));
			LIBTEXB_FREE(texb);
			LIBTEXB_FAILWITH(EBADF);
		}
	}
	else
		memory_buffer.read(reinterpret_cast<char*>(rawData),dataSize);
	texb->RawImage=LIBTEXB_ALLOC(uint8_t,tWidth*tHeight*4);
	convert_map(rawData,tWidth,tHeight,TexbFlags,texb->RawImage);
	texb->ImageList_Id=imageListTemp;
	LIBTEXB_FREE(rawData);

	for(uint32_t i=0;i<texb->ImageList_Id.size();i++)
	{
		TextureImage* timg=texb->ImageList_Id[i];
		texb->ImageList_Names[timg->Name]=i;
		
		uint32_t* Vrtx=texb->VertexIndexUVs[i];
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
		uint32_t* rawBmp=NULL;
		uint32_t* texbBmp=reinterpret_cast<uint32_t*>(texb->RawImage);

		// Check UV
		for(uint32_t j=0;j<4;j++)
		{
			if(t[j].U>1.0) t[j].U = 1.0;
			if(t[j].V>1.0) t[j].V = 1.0;
		}

		uint32_t min_x = std::min(v[0].X, std::min(v[1].X, std::min(v[2].X, v[3].X)));
		uint32_t min_y = std::min(v[0].Y, std::min(v[1].Y, std::min(v[2].Y, v[3].Y)));
		uint32_t max_x = std::max(v[0].X, std::max(v[1].X, std::max(v[2].X, v[3].X)));
		uint32_t max_y = std::max(v[0].Y, std::max(v[1].Y, std::max(v[2].Y, v[3].Y)));

		rawBmp=LIBTEXB_ALLOC(uint32_t, timg->Width * timg->Height);

		memset(rawBmp, 0, timg->Width * timg->Height * 4);
		for(uint32_t y=min_y; y < max_y; y++)
		{
			for(uint32_t x = min_x; x < max_x; x++)
			{
				UVPoint uv=xy2uv(x, y, v[0], v[1], v[2], v[3], t[0], t[1], t[2], t[3]);
				rawBmp[x + y * timg->Width] = texbBmp[uint32_t(uv.U * tWidth + 0.5) + uint32_t(uv.V * tHeight + 0.5) * tWidth];
			}
		}

		timg->RawImage=reinterpret_cast<uint8_t*>(rawBmp);
	}

	return texb;
}

TextureBank* TextureBank::FromFile(std::string Filename)
{
	TextureBank* texb;
	FILE* file;
	uint8_t* buffer;

	file=fopen(Filename.c_str(),"rb");
	if(file==NULL)
		LIBTEXB_FAILEXIT;

	buffer=LIBTEXB_ALLOC(uint8_t,4);
	fread(buffer,4,1,file);
	if(memcmp(buffer,"TEXB",4)!=0)
	{
		// Not a TEXB file
		fclose(file);
		LIBTEXB_FREE(buffer);
		LIBTEXB_FAILWITH(EBADF);
	}

	size_t texb_size=0;
	fread(buffer,4,1,file);
	texb_size=((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|buffer[3])+8;

	LIBTEXB_FREE(buffer);
	buffer=LIBTEXB_ALLOC(uint8_t,texb_size);	// TEXB magic number + 32-bit size = 8
	fseek(file,0,SEEK_SET);
	if(fread(buffer,1,texb_size,file)!=texb_size)
	{
		fclose(file);
		LIBTEXB_FREE(buffer);
		LIBTEXB_FAILEXIT;
	}

	fclose(file);
#ifndef LIBTEXB_NO_EXCEPTION
	try {
#endif
	texb=FromMemory(buffer,texb_size);
#ifndef LIBTEXB_NO_EXCEPTION
	} catch(int& v) {
		LIBTEXB_FREE(buffer);
		throw v;
	}
#endif

	LIBTEXB_FREE(buffer);
	return texb;
}
