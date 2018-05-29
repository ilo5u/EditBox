#pragma once
#include<string>
class Read_Text_Failed
{
public:
	Read_Text_Failed(const std::string& Str)
	{
		s = Str;
	}
	std::string what() { return s; }
private:
	std::string s;
};

class No_File_Name :public std::logic_error
{
public:
	No_File_Name(const std::string& Str) :std::logic_error(Str) {  }
};
