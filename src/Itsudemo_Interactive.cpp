/**
* Itsudemo_Interactive.cpp
* Itsudemo interactive command parser
**/

#include "TEXB.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

#include <stdint.h>
#include <lodepng.h>

struct InteractiveVariables
{
	uint32_t Type;	// 0=int; 1=TextureBank; 2=TextureImage; 3 = std::string
	int32_t Value;
	TextureBank* Bank;
	TextureImage* Image;
	std::string String;
};

const char* VariableTypename[]={"Integer","TextureBank","TextureImage","String"};

// Forward declaration. Declared in Itsudemo.cpp

void dumpTEXB(TextureBank* texb,const std::string path);

std::vector<std::string> split_string(const std::string split)
{
	std::stringstream ss;
	std::vector<std::string> splitted;
	

	for(uint32_t i=0;i<split.length();i++)
	{
		char c=split[i];
		if(c==' ')
		{
			splitted.push_back(std::string(ss.str()));
			ss.str("");
			ss.clear();
		}
		else if(c=='\"')
		{
			i++;
			while(split[i]!='\"')
			{
				ss << split[i];
				i++;
			}
		}
		else
			ss << split[i];
	}
	if(ss.rdbuf()->in_avail()>0)
		splitted.push_back(std::string(ss.str()));

	return splitted;
}

bool isValidVariable(const std::string str)
{
	if(str.length()>0 && !(str[0]>='0' && str[0] <='9'))
	{
		for(uint32_t i=0;i<str.length();i++)
		{
			char c=str[i];
			if((c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_')
				continue;
			return false;
		}
		return true;
	}
	return false;
}

template<typename T> bool getVectorIndex(std::vector<T>& v,uint32_t index,T& to)
{
	try
	{
		T& a=v[index];
		to=a;
		return true;
	}
	catch(...)
	{
		return false;
	}
}

int main_interactive()
{
	std::vector<std::string> DelimitedCommand;
	std::map<std::string,InteractiveVariables*> Variables;

	std::cout << "Itsudemo interactive mode" << std::endl << "Expect bugs. It still experimental" << std::endl;
	
	for(std::string TempString;std::getline(std::cin,TempString);)
	{
		DelimitedCommand=split_string(TempString);
		if(DelimitedCommand.size()==0) continue;

		std::transform(DelimitedCommand[0].begin(),DelimitedCommand[0].end(),DelimitedCommand[0].begin(),::tolower);

		// Commands
		// dump @<path to texb> OR dump <texb variable>
		if(DelimitedCommand[0]=="dump")
		{
			std::string var;
			TextureBank* texb=NULL;
			if(getVectorIndex(DelimitedCommand,1,var))
			{
				if(var[0]=='@')
				{
					// Path to TEXB
					var=var.substr(1);

					try
					{
						texb=TextureBank::FromFile(var);
					}
					catch(int e)
					{
						std::cerr << "Error: Cannot open " << var << ": " << strerror(e) << std::endl;
						continue;
					}

					dumpTEXB(texb,var);
					delete texb;
				}
				else
				{
					// Variable
					if(isValidVariable(var))
					{
						InteractiveVariables* v;
						try
						{
							v=Variables[var];
						}
						catch(std::out_of_range )
						{
							std::cerr << "Error: Undefined variable '" << var << "'" << std::endl;
							continue;
						}
						if(v->Type==1)
						{
							dumpTEXB(v->Bank,"");
						}
						else
						{
							std::cerr << "Error: Expected 'TextureBank' got '" << VariableTypename[v->Type] << "'" << std::endl;
							continue;
						}
					}
					else
					{
						std::cerr << "Error: Not a valid variable" << std::endl;
						continue;
					}
				}
			}
			else
			{
				std::cerr << "Error: Arg #2 missing. Expected TEXB or path" << std::endl;
				continue;
			}
		}
		else if(DelimitedCommand[0]=="quit" || DelimitedCommand[0]=="exit")
			return 0;
		else
			std::cerr << "Error: Unknown command '" << TempString << "'" << std::endl;
	}
	return 1;
}
