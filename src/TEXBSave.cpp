/**
* TEXBSave.cpp
* Saves TextureBank to TEXB
**/

#include <string>
#include <sstream>
#include <vector>

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <stdint.h>

#include "TEXB.h"
#include "zlib.h"

inline void ToBE(uint8_t *dest, uint32_t v)
{
	dest[0]=v>>24;
	dest[1]=(v>>16)&255;
	dest[2]=(v>>8)&255;
	dest[3]=v&255;
}

int32_t TextureBank::SaveToMemory(uint8_t*& Memory,size_t* MemorySize,uint32_t CompressLevel)
{
	if(MemorySize==NULL) return EINVAL;
	if(CompressLevel>9) CompressLevel=9;

	uint16_t temp_short=0;
	uint32_t temp_int=0;
	uint8_t* temp_buffer;
	uint8_t prebuf[4];

	std::stringstream sstream(std::ios::binary|std::ios::out);
	sstream.write("TEXB",4);
	sstream.write("\0\0\0\0",4);

	size_t nameSize=Name.length()+7;
	temp_short=uint16_t((nameSize+1)/2*2);
	prebuf[0]=temp_short>>8;
	prebuf[1]=temp_short&255;
	sstream.write(reinterpret_cast<char*>(prebuf),2);
	sstream.put('T');
	sstream.write((Name+".texb").c_str(),nameSize-1);
	if(nameSize%2==1) sstream.put(0);

	prebuf[0]=RawImageWidth>>8;
	prebuf[1]=RawImageWidth&255;
	prebuf[2]=RawImageHeight>>8;
	prebuf[3]=RawImageHeight&255;
	sstream.write(reinterpret_cast<char*>(prebuf),4);

	prebuf[0]=0;
	prebuf[1]=CompressLevel>0?204:196;
	sstream.write(reinterpret_cast<char*>(prebuf),2);

	size_t tImgs=ImageList_Id.size();
	temp_short=uint16_t(tImgs*4);
	prebuf[0]=temp_short>>8;
	prebuf[1]=temp_short&255;
	temp_short=uint16_t(tImgs*6);
	prebuf[2]=temp_short>>8;
	prebuf[3]=temp_short&255;
	sstream.write(reinterpret_cast<char*>(prebuf),4);
	prebuf[0]=(tImgs>>8)&255;
	prebuf[1]=tImgs&255;
	sstream.write(reinterpret_cast<char*>(prebuf),2);

	for(uint32_t i=0;i<ImageList_Id.size();i++)
	{
		TextureImage* cur=ImageList_Id[i];
		sstream.write("TIMG\0\0",6);
		size_t curStreamSize=sstream.tellp();
		nameSize=cur->Name.length()+11;
		temp_short=uint16_t((nameSize+1)/2*2);
		prebuf[0]=temp_short>>8;
		prebuf[1]=temp_short&255;
		sstream.write(reinterpret_cast<char*>(prebuf),2);
		sstream.put('I');
		sstream.write((cur->Name+".png.imag").c_str(),nameSize-1);
		if(nameSize%2==1) sstream.put(0);

		sstream.write("\xFF\xFF\x00\x01\x00\x00\x00\x00\x00\x01\x00\x01\x04\x06",14);
		prebuf[0]=cur->Width>>8;
		prebuf[1]=cur->Width&255;
		prebuf[2]=cur->Height>>8;
		prebuf[3]=cur->Height&255;
		sstream.write(reinterpret_cast<char*>(prebuf),4);
		sstream.write("\0\0\0\0",4);

		FixedVertexIndexFormat *vrtxCur = VertexIndexUVs[i];

		for(uint32_t j=0;j<4;j++)
		{
			ToBE(prebuf, vrtxCur->Vertices[j].X);
			sstream.write(reinterpret_cast<char*>(prebuf),4);
			ToBE(prebuf, vrtxCur->Vertices[j].Y);
			sstream.write(reinterpret_cast<char*>(prebuf),4);
			ToBE(prebuf, vrtxCur->Vertices[j].U);
			sstream.write(reinterpret_cast<char*>(prebuf),4);
			ToBE(prebuf, vrtxCur->Vertices[j].V);
			sstream.write(reinterpret_cast<char*>(prebuf),4);
		}
		sstream.write(reinterpret_cast<char*>(vrtxCur)+64,6);

		temp_int=(uint32_t)sstream.tellp();
		sstream.seekp(curStreamSize-2,std::ios::beg);
		temp_short=uint16_t(temp_int-=(uint32_t)curStreamSize);
		prebuf[0]=temp_short>>8;
		prebuf[1]=temp_short&255;
		sstream.write(reinterpret_cast<char*>(prebuf),2);
		sstream.seekp(0,std::ios::end);
	}

	if(CompressLevel>0)
	{
		z_stream zstate;
		memset(&zstate,0,sizeof(z_stream));

		temp_int=deflateInit(&zstate,CompressLevel);
		if(temp_int==Z_MEM_ERROR) return ENOMEM;

		temp_int=RawImageWidth*RawImageHeight*4;
		temp_buffer=LIBTEXB_ALLOC(uint8_t,temp_int);
		zstate.avail_in = temp_int;
		zstate.next_in = RawImage;
		zstate.avail_out = temp_int;
		zstate.next_out = temp_buffer;

		deflate(&zstate,Z_FINISH);
		deflateEnd(&zstate);

		temp_int-=zstate.avail_out;
		sstream.write("\0\0\0\0",4);
		sstream.write(reinterpret_cast<char*>(temp_buffer),temp_int);
		LIBTEXB_FREE(temp_buffer);
	}
	else
	{
		temp_int=RawImageWidth*RawImageHeight*4;
		sstream.write(reinterpret_cast<char*>(RawImage),temp_int);
	}
	
	temp_int=static_cast<int>(sstream.tellp())-8;
	prebuf[0]=temp_int>>24;
	prebuf[1]=(temp_int>>16)&255;
	prebuf[2]=(temp_int>>8)&255;
	prebuf[3]=temp_int&255;
	sstream.seekp(4,std::ios::beg);
	sstream.write(reinterpret_cast<char*>(prebuf),4);

	sstream.seekp(0,std::ios::end);
	temp_int=(uint32_t)sstream.tellp();
	temp_buffer=LIBTEXB_ALLOC(uint8_t,temp_int);
	memcpy(temp_buffer,sstream.str().c_str(),temp_int);
	Memory=temp_buffer;
	*MemorySize=temp_int;

	return 0;
}

inline int32_t TextureBank::SaveToMemory(uint8_t** Memory,size_t* MemorySize,uint32_t CompressLevel)
{
	return SaveToMemory(*Memory,MemorySize,CompressLevel);
}

int32_t TextureBank::SaveToFile(std::string Filename,uint32_t CompressLevel)
{
	FILE* fptr;
	uint8_t* temp_buffer;
	size_t bufsize;
	uint32_t ret;

	fptr=fopen(Filename.c_str(),"wb");
	if(fptr==NULL) return errno;

	ret=SaveToMemory(&temp_buffer,&bufsize,CompressLevel);
	if(ret==0)
	{
		fwrite(temp_buffer,1,bufsize,fptr);
		fclose(fptr);
	}

	LIBTEXB_FREE(temp_buffer);
	return ret;
}
