#pragma once
#include <string>
using namespace std;
class CCreateDump
{
public:
	CCreateDump();
	~CCreateDump(void);
	static CCreateDump* Instance();
	static long __stdcall UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo);
	//����Dump�ļ����쳣ʱ���Զ����ɡ����Զ�����.dmp�ļ�����׺
	void DeclarDumpFile(std::string dmpFileName = "");
private:
	static std::string    strDumpFile;
	static CCreateDump*    __instance;
};