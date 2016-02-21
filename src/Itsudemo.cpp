/**
* Itsudemo.cpp
* The main program.
**/

#include "TEXB.h"
#include "CompilerName.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <cstring>
#include <cstdlib>

#include <stdint.h>
#include <tclap/CmdLine.h>
#include <lodepng.h>
#include <zlib.h>


struct AppendStringVisitor:public TCLAP::Visitor
{
	std::string append;
	std::string* append_where;
	AppendStringVisitor(std::string append_which,std::string* where_append):Visitor()
	{
		append=append_which;
		append_where=where_append;
	}
	void visit()
	{
		append_where->append(append);
	}
};

//int main_interactive();

std::vector<uint8_t> getRawImageFromTEXB(TextureBank* texb,std::string& name,uint32_t& w,uint32_t& h)
{
	if(texb->Name==name)
	{
		w=texb->Width;
		h=texb->Height;
		return texb->FetchRaw();
	}
	TextureImage* timg=texb->FetchImage(name);
	w=timg->Width;
	h=timg->Height;
	return std::vector<uint8_t>(timg->RawImage,timg->RawImage+w*h*4);
}

inline const char* getBasename(const char* path)
{
	const char* a=strrchr(path,'/');
	const char* b=strrchr(path,'\\');
	return a==b?path:std::max(a,b);
}

std::string getDirectory(const char* path)
{
	const char* basename=getBasename(path);
	if(basename>path)
		return std::string(path,0,basename-path+1);

	return std::string();
}

void show_information_once(TextureBank* texb,const std::string path)
{
	static bool ever_here=false;
	if(ever_here) return;
	
	std::vector<TextureImage*> timg_list=texb->FetchAll();
	TextureImage* temp_timg=NULL;

	std::cerr << "File: " << path << std::endl << "Size: " << texb->Width << "x" << texb->Height << " pixels" << std::endl;
	std::cerr << "Image(s): " << timg_list.size() << std::endl;
	for(std::vector<TextureImage*>::iterator i=timg_list.begin();i!=timg_list.end();i++)
	{
		temp_timg=*i;
		std::cerr << "    " << temp_timg->Name << ": " << temp_timg->Width << "x" << temp_timg->Height << " pixels" << std::endl;
	}
	
	ever_here=true;
}

void dumpTEXB(TextureBank* texb,const std::string path)
{
	uint32_t temp_int=0;
	std::vector<TextureImage*> timg_list=texb->FetchAll();
	TextureImage* temp_timg=NULL;
	std::string temp_string;
	const char* charPath=path.c_str();
	const char* basename=getBasename(charPath);
	std::string fixedPath=getDirectory(charPath);

	show_information_once(texb,path);
	temp_string=std::string(getBasename(texb->Name.c_str())+1)+".png";

	std::cerr << std::endl;
	std::cerr << "Writing: " << temp_string << std::endl;
	temp_int=lodepng::encode(fixedPath+temp_string,texb->FetchRaw(),texb->Width,texb->Height);
	if(temp_int!=0)
		std::cerr << "         " << lodepng_error_text(temp_int) << std::endl;

	for(std::vector<TextureImage*>::iterator i=timg_list.begin();i!=timg_list.end();i++)
	{
		temp_timg=*i;
		temp_string=std::string(strrchr(temp_timg->Name.c_str(),'/')+1)+".png";
		std::cerr << "Writing: " << temp_string << std::endl;
		temp_int=lodepng::encode(
			fixedPath+temp_string,
			std::vector<uint8_t>(
				reinterpret_cast<char*>(temp_timg->RawImage),
				reinterpret_cast<char*>(temp_timg->RawImage)+temp_timg->Width*temp_timg->Height*4
			),
			temp_timg->Width,
			temp_timg->Height
		);
		if(temp_int!=0)
			std::cerr << "         " << lodepng_error_text(temp_int) << std::endl;
	}
}

bool parse_timg_path(const std::string& from,std::string* to,bool path_necessary=false,const std::string& default_suffix=".png")
{
	const char* a=from.c_str();
	const char* b=strchr(a,':');
	
	if(b)
	{
		to[0]=std::string(a,uint32_t(b-a));
		to[1]=std::string(b+1);
	}
	else
	{
		if(path_necessary) return false;
		const char* c=strrchr(a,'/');
		c=c==NULL?a:c+1;
		to[0]=a;
		to[1]=std::string(c)+default_suffix;
	}
	return true;
}

int main(int argc,char* argv[])
{
	if(argc<2)
	{
		// TODO: Interactive mode
		//std::cerr << "Currently interactive mode is not supported." << std::endl;
		//return main_interactive();
		std::cerr << "   Type --help for command-line usage.\n" << std::endl;
		std::cerr << "   Itsudemo. TEXB Manipulation tool\n\n   Copyright (c) 2037 Dark Energy Processor Corporation\n" << std::endl;
		return 0;
	}
	std::string VersionString("1.0.1\nCopyright (c) 2037 Dark Energy Processor Corporation\nCompiled with ");
	VersionString.append(CompilerName());
	using namespace TCLAP;
	std::string CmdLineOrder;
	AppendStringVisitor AppendE("e",&CmdLineOrder);
	AppendStringVisitor AppendN("n",&CmdLineOrder);
	AppendStringVisitor AppendR("r",&CmdLineOrder);
	CmdLine CommandLine("Itsudemo. TEXB Manipulation tool\nCopyright (c) 2037 Dark Energy Processor Corporation",' ',VersionString);
	SwitchArg SwitchA("a","file-info","Prints TEXB information to stdout",CommandLine,false);
	ValueArg<uint32_t> SwitchC("c","compress-level","Sets compress level when writing TEXB. 0 - No compression, 9 - Best compression",false,6,"0-9",CommandLine);
	SwitchArg SwitchD("d","dump-all","Dump all images, including the texture to PNG in the TEXB directory",CommandLine,false);
	MultiArg<std::string> SwitchE("e","extract-image","Extract specificed TIMG image in TEXB.",false,"timg name>:<png out path",CommandLine,&AppendE);
	SwitchArg SwitchI("i","dump-images","Dump all images, excluding the texture bank raw image.",CommandLine,false);
	MultiArg<std::string> SwitchN("n","rename","Rename internal name of TIMG in TEXB.",false,"timg name>:<timg new name",CommandLine,&AppendN);
	ValueArg<std::string> SwitchO("o","output","Specify output of the TEXB file. Defaults to input file which means overwrite it",false,"","output texb",CommandLine);
	MultiArg<std::string> SwitchR("r","replace","Replace TIMG with PNG in TEXB.",false,"timg name>:<png in path",CommandLine,&AppendR);
	SwitchArg SwitchT("t","dump-texb","Dump texture bank raw image only.",CommandLine,false);
	UnlabeledValueArg<std::string> TEXBInput("input","Input TEXB File location",true,"","input texb",CommandLine);

	try
	{
		CommandLine.parse(argc,argv);
	}
	catch(ArgException& e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
		return -1;
	}

	TextureBank* texb=NULL;
	TextureImage* timg=NULL;
	uint32_t temp_int=0;
	char isTexbModified=0;
	std::string inputPath=TEXBInput.getValue();

	try
	{
		texb=TextureBank::FromFile(inputPath);
	}
	catch(int err)
	{
		std::cerr << "Cannot open " << inputPath.c_str() << ": " << strerror(err) << std::endl;
		return err;
	}

	if(SwitchA.getValue())
	{
		std::vector<TextureImage*> timg_list=texb->FetchAll();
		TextureImage* temp_timg=NULL;

		show_information_once(texb,inputPath);
		std::cerr << std::endl;
	}

	if(SwitchD.getValue())
	{
		dumpTEXB(texb,inputPath);
		std::cerr << std::endl;
		return 0;
	}
	else if(SwitchI.getValue())
	{
		show_information_once(texb,inputPath);

		std::string temp_string;
		std::vector<TextureImage*> timg_list=texb->FetchAll();
		std::string fixedPath="";
		const char* charPath=inputPath.c_str();
		const char* basename=getBasename(charPath);
		if(basename>charPath)
			fixedPath=std::string(inputPath,0,basename-charPath+1);

		for(std::vector<TextureImage*>::iterator i=timg_list.begin();i!=timg_list.end();i++)
		{
			temp_string=std::string(strrchr((*i)->Name.c_str(),'/')+1)+".png";
			std::cerr << "Writing: " << temp_string << std::endl;
			temp_int=lodepng::encode(
				fixedPath+temp_string,
				std::vector<uint8_t>(
					reinterpret_cast<char*>((*i)->RawImage),
					reinterpret_cast<char*>((*i)->RawImage)+(*i)->Width*(*i)->Height*4
				),
				(*i)->Width,
				(*i)->Height
			);
			if(temp_int!=0)
				std::cerr << "         " << lodepng_error_text(temp_int) << std::endl;
		}
		std::cerr << std::endl;

		return 0;
	}
	else if(SwitchT.getValue())
	{
		show_information_once(texb,inputPath);

		std::string temp_string=std::string(getBasename(texb->Name.c_str())+1)+".png";
		std::string fixedPath=getDirectory(inputPath.c_str());
		std::cerr << "Writing: " << temp_string << std::endl;
		temp_int=lodepng::encode(fixedPath+temp_string,texb->FetchRaw(),texb->Width,texb->Height);
		if(temp_int!=0)
			std::cerr << "         " << lodepng_error_text(temp_int) << std::endl;
		std::cerr << std::endl;

		return 0;
	}

	std::string TEXBOutput;
	const std::vector<std::string> ExtractList=SwitchE.getValue();
	const std::vector<std::string> RenameList=SwitchN.getValue();
	const std::vector<std::string> ReplaceList=SwitchR.getValue();
	int indexE=(-1);
	int indexN=(-1);
	int indexR=(-1);
	uint32_t temp_int2=0;
	uint32_t temp_int3=0;
	std::vector<uint8_t> rawImage;
	
	if(SwitchO.isSet())
		TEXBOutput=SwitchO.getValue();
	else
		TEXBOutput=inputPath;

	for(uint32_t i=0;i<CmdLineOrder.size();i++)
	{
		std::string timg_n_path[2];
		char type=CmdLineOrder.at(i);
		
		if(type=='e')
		{
			indexE++;
			parse_timg_path(ExtractList[indexE],timg_n_path);

			try
			{
				rawImage=getRawImageFromTEXB(texb,timg_n_path[0],temp_int2,temp_int3);
			}
			catch(int)
			{
				std::cerr << "Extract: Cannot find " << timg_n_path[0] << "in " << texb->Name << std::endl;
				continue;
			}
			std::cerr << "Extract: Writing " << timg_n_path[0] << " to " << timg_n_path[1] << std::endl;
			temp_int=lodepng::encode(timg_n_path[1],rawImage,temp_int2,temp_int3);
			if(temp_int!=0)
				std::cerr << "Extract: Cannot write " << timg_n_path[1] << ": " << lodepng_error_text(temp_int) << std::endl;
			rawImage=std::vector<uint8_t>();
		}
		else if(type=='n')
		{
			indexN++;
			if(parse_timg_path(RenameList[indexN],timg_n_path,true)==false)
			{
				std::cerr << "Rename: Cannot rename '" << RenameList[indexN] << "': missing 'timg new name'" << std::endl;
				continue;
			}

			if(texb->Name==timg_n_path[0])
				texb->Name=timg_n_path[1];
			else
			{
				try
				{
					timg=texb->FetchImage(timg_n_path[0]);
				}
				catch(int)
				{
					std::cerr << "Rename: Cannot find " << timg_n_path[0] << " in " << texb->Name << std::endl;
					continue;
				}

				std::cerr << "Rename: Renaming " << timg_n_path[0] << " to " << timg_n_path[1] << std::endl;
				timg->Name=std::string(timg_n_path[1]);
			}
			isTexbModified|=1;
		}
		else if(type=='r')
		{
			indexR++;
			parse_timg_path(ReplaceList[indexR],timg_n_path);

			if(texb->Name==timg_n_path[0])
			{
				uint8_t* rw=texb->RawImage;
				temp_int=lodepng::decode(rawImage,temp_int2,temp_int3,timg_n_path[1]);
				if(temp_int!=0)
				{
					std::cerr << "Replace: Cannot read " << timg_n_path[1] << ": " << lodepng_error_text(temp_int) << std::endl;
					rawImage=std::vector<uint8_t>();
					continue;
				}
				if(texb->Width!=temp_int2 || texb->Height!=temp_int3)
				{
					std::cerr << "Replace: Cannot write " << timg_n_path[0] << ": Image size mismatch." << std::endl <<
						"         Expected " << texb->Width << "x" << texb->Height << " pixels, got " <<
						temp_int2 << "x" << temp_int3 << " pixels." << std::endl;
					rawImage=std::vector<uint8_t>();
					continue;
				}
				std::cerr << "Replace: Replacing " << timg_n_path[0] << " image from " << timg_n_path[1] << std::endl;
				memcpy(texb->RawImage,&rawImage[0],temp_int2*temp_int3*4);
				isTexbModified|=4;
			}
			else
			{
				try
				{
					timg=texb->FetchImage(timg_n_path[0]);
				}
				catch(int)
				{
					std::cerr << "Replace: Cannot find " << timg_n_path[0] << " in " << texb->Name << std::endl;
					continue;
				}

				temp_int=lodepng::decode(rawImage,temp_int2,temp_int3,timg_n_path[1]);
				if(temp_int!=0)
				{
					std::cerr << "Replace: Cannot read " << timg_n_path[1] << ": " << lodepng_error_text(temp_int) << std::endl;
					rawImage=std::vector<uint8_t>();
					continue;
				}
				if(timg->Width!=temp_int2 || timg->Height!=temp_int3)
				{
					std::cerr << "Replace: Cannot write " << timg_n_path[0] << ": Image size mismatch." << std::endl <<
						"         Expected " << timg->Width << "x" << timg->Height << " pixels, got " <<
						temp_int2 << "x" << temp_int3 << " pixels." << std::endl;
					rawImage=std::vector<uint8_t>();
					continue;
				}
				
				std::cerr << "Replace: Replacing " << timg_n_path[0] << " image from " << timg_n_path[1] << std::endl;
				memcpy(timg->RawImage,&rawImage[0],temp_int2*temp_int3*4);
			}
			rawImage=std::vector<uint8_t>();
			isTexbModified|=2;
		}
		else continue;	// We should never reach here
	}

	// Reflect changes.
	if(isTexbModified>1 && (isTexbModified&4)==0)
		texb->ReflectChanges();
	if(isTexbModified)
		texb->SaveToFile(TEXBOutput,SwitchC.getValue());

	delete texb;
	return 0;
}
