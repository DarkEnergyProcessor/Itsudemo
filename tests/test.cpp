/**
* test.cpp
* Testing purpose
**/

#include <iostream>	// All in one in MSVC
#include <cstdio>
#include <cstdlib>
using namespace std;

#include <lodepng.h>

#include "TEXB.h"

int main(int argc,char* argv[])
{
	if(argc<2)
	{
		puts("Usage: <program> <input texb>");
		return 1;
	}

	TextureBank* texb;

	try
	{
		texb=TextureBank::FromFile(argv[1]);

		if(texb==NULL)
		{
			puts(strerror(errno));
			return 1;
		}
	}
	catch(int err)
	{
		puts(strerror(err));
		return 1;
	}

	std::string temp_string=std::string(strrchr(texb->Name.c_str(),'/')+1)+".png";

	printf("File: %s\nSize: %u x %u\nWriting %s\n",argv[1],texb->Width,texb->Height,temp_string.c_str());
	lodepng::encode(temp_string,texb->FetchRaw(),texb->Width,texb->Height);
	std::vector<TextureImage*> timg_list=texb->FetchAll();
	for(std::vector<TextureImage*>::iterator i=timg_list.begin();i!=timg_list.end();i++)
	{
		TextureImage* timg=*i;
		temp_string=std::string(strrchr(timg->Name.c_str(),'/')+1);

		printf("Writing %s.png\n",temp_string.c_str());
		lodepng::encode(temp_string+".png",timg->RawImage,timg->Width,timg->Height);
	}
	
	delete texb;
	return 0;
}