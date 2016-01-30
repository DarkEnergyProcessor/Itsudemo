#include <iostream>

#include "TEXB.h"

int main() {
	TextureBank texb(256,256);
	texb.Name="mytest/test123/texture";
	uint32_t temp_int;
	/*
	Point vrtx[4]={
		{0,0},
		{128,0},
		{128,64},
		{0,64}
	};
	UVPoint uv[4]={
		{0.0,0.0},
		{128.0/256.0,0.0},
		{128.0/256.0,64.0/256.0},
		{0.0,64.0/256.0}
	};
	texb.DefineImage(vrtx,uv,"mytest/test123/ok",&temp_int);*/
	Point where_and_size[2]={
		{16,16},	// Start pixel
		{128,64}	// Image size
	};
	texb.DefineImage(where_and_size,"mytest/test123/ok",&temp_int);
	TextureImage* timg=texb[temp_int];
	memset(timg->RawImage,255,timg->Width*timg->Height*4);
	texb.ReflectChanges();

	texb.SaveToFile("mytest.texb");
	return 0;
}