/**
* TEXBSave.cpp
* Saves TextureBank to TEXB
**/

#include "TEXB.h"

#include <string>
#include <sstream>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <stdint.h>

#include <zlib.h>

// Byte-order function
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

int32_t TextureBank::SaveToMemory(uint8_t*& Memory,size_t* MemorySize,uint32_t CompressLevel)
{
	if(MemorySize==NULL) return EINVAL;
	if(CompressLevel>9) CompressLevel=9;

	uint16_t temp_short=0;
	uint32_t temp_int=0;
	uint8_t* temp_buffer;

	std::stringstream sstream(std::ios::binary|std::ios::out);
	sstream.write("TEXB",4);
	sstream.write("\0\0\0\0",4);

	uint32_t nameSize=Name.length()+7;
	temp_short=htons((nameSize+1)/2*2);
	sstream.write(reinterpret_cast<char*>(&temp_short),2);
	sstream.put('T');
	sstream.write((Name+".texb").c_str(),nameSize-1);
	if(nameSize%2==1) sstream.put(0);

	temp_short=htons(RawImageWidth);
	sstream.write(reinterpret_cast<char*>(&temp_short),2);
	temp_short=htons(RawImageHeight);
	sstream.write(reinterpret_cast<char*>(&temp_short),2);

	temp_short=htons(CompressLevel>0?204:196);
	sstream.write(reinterpret_cast<char*>(&temp_short),2);

	uint32_t TotalVertexLocation=sstream.tellp();
	uint16_t tImgs=ImageList_Id.size();
	uint16_t tVrtx=htons(tImgs*4);
	uint16_t tIndx=htons(tImgs*6);
	temp_short=htons(tImgs);
	sstream.write(reinterpret_cast<char*>(&tVrtx),2);
	sstream.write(reinterpret_cast<char*>(&tIndx),2);
	sstream.write(reinterpret_cast<char*>(&temp_short),2);

	for(uint32_t i=0;i<ImageList_Id.size();i++)
	{
		TextureImage* cur=ImageList_Id[i];
		sstream.write("TIMG\0\0",6);
		uint32_t curStreamSize=sstream.tellp();
		nameSize=cur->Name.length()+11;
		temp_short=htons((nameSize+1)/2*2);
		sstream.write(reinterpret_cast<char*>(&temp_short),2);
		sstream.put('I');
		sstream.write((cur->Name+".png.imag").c_str(),nameSize-1);
		if(nameSize%2==1) sstream.put(0);

		sstream.write("\xFF\xFF\x00\x01\x00\x00\x00\x00\x00\x01\x00\x01\x04\x06",14);
		temp_short=htons(cur->Width);
		sstream.write(reinterpret_cast<char*>(&temp_short),2);
		temp_short=htons(cur->Height);
		sstream.write(reinterpret_cast<char*>(&temp_short),2);
		sstream.write("\0\0\0\0",4);

		uint32_t* vrtxCur=VertexIndexUVs[i];

		for(uint32_t j=0;j<16;j++)
		{
			temp_int=htonl(vrtxCur[j]);
			sstream.write(reinterpret_cast<char*>(&temp_int),4);
		}
		sstream.write(reinterpret_cast<char*>(vrtxCur)+64,6);

		temp_int=sstream.tellp();
		sstream.seekp(curStreamSize-2,std::ios::beg);
		temp_int-=curStreamSize;
		temp_short=htons(temp_int);
		sstream.write(reinterpret_cast<char*>(&temp_short),2);
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
	
	temp_int=htonl(static_cast<uint32_t>(sstream.tellp())-8);
	sstream.seekp(4,std::ios::beg);
	sstream.write(reinterpret_cast<char*>(&temp_int),4);

	sstream.seekp(0,std::ios::end);
	temp_int=sstream.tellp();
	temp_buffer=LIBTEXB_ALLOC(uint8_t,temp_int);
	memcpy(temp_buffer,sstream.str().c_str(),temp_int);
	Memory=temp_buffer;
	*MemorySize=temp_int;

	return 0;
}

int32_t TextureBank::SaveToMemory(uint8_t** Memory,size_t* MemorySize,uint32_t CompressLevel)
{
	return SaveToMemory(*Memory,MemorySize,CompressLevel);
}

int32_t TextureBank::SaveToFile(std::string Filename,uint32_t CompressLevel)
{
	FILE* fptr;
	uint8_t* temp_buffer;
	uint32_t bufsize;
	uint32_t ret;

	fptr=fopen(Filename.c_str(),"wb");
	if(fptr==NULL) return errno;

	ret=SaveToMemory(temp_buffer,&bufsize,CompressLevel);
	if(ret==0)
	{
		fwrite(temp_buffer,1,bufsize,fptr);
		fclose(fptr);
	}

	LIBTEXB_FREE(temp_buffer);
	return ret;
}
