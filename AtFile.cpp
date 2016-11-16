#include "stdafx.h"
#include "AtFile.h"
#include <fstream>
#include <windows.h>
#include <vector>
#include <assert.h>

#include "filedir.h"
//修复了一个紧急BUG
using namespace std;

namespace at_base {

/*******************CAtFile类***************************/

CAtFile::CAtFile() : m_hFile(INVALID_ATHANDLE)
{
}

CAtFile::~CAtFile()
{
	if (m_hFile != INVALID_ATHANDLE)
	{
		Close();
	}
}

bool GetDzhDirPath(const char* cFilePath, std::string& scrDzhPath);
bool GetStandardDir(const char* cDir, std::string& strDir);
bool GetStandardFilePath(const char* cFilePath, std::string& strFilePath);
bool GetWindowsDirPath(std::string& scrDzhPath, std::string& desWindowsPath);
void StringSplit(std::string s,char splitchar,vector<string>& vec);
bool ImportFile_A(const char* ScrFileName, const char* DesFileName);
bool ExportFile_A(const char* ScrFileName, const char* DesFileName);

/********************************CAtFile类基本接口实现**********************************************/

bool CAtFile::Open(const char* FileName, size_t nOpenFlags)
{
	return true;
}

ULONGLONG CAtFile::Read(void* lpBuf, ULONGLONG nCount)
{
	assert(this);
	assert(lpBuf != NULL);
	assert(m_hFile != INVALID_ATHANDLE);
	ULONGLONG tReadNum=0;
	return tReadNum;
}

void CAtFile::Write(void* lpBuf, ULONGLONG nCount)
{
	assert(this);
	assert(lpBuf != NULL);
	assert(m_hFile != INVALID_ATHANDLE);

	if (nCount == 0)
		return ;
}

bool CAtFile::Rename(const char* oldName, const char* newName)
{
	return true;
}


bool CAtFile::Remove(const char* FileName)
{
	return true;
}

bool CAtFile::GetStatus(const char* FileName, CAtFileStatus& rStatus)
{
	return true;
}

ULONGLONG CAtFile::GetPosition() const
{
	assert(this);
	assert(m_hFile != INVALID_ATHANDLE);

	ULONGLONG tPosition=0;
	return tPosition;
}

ULONGLONG CAtFile::SeekToEnd()
{
	assert(this);
	return  Seek(0, CAtFile::end);
}

void CAtFile::SeekToBegin()
{
	assert(this);
	Seek(0, CAtFile::begin);
}

ULONGLONG CAtFile::Seek(LONGLONG lOff, int nFrom)
{
	return 0;
}

ULONGLONG CAtFile::GetLength() const
{
	return 0;
}

void CAtFile::SetLength(ULONGLONG dwNewLen)
{
}

std::string CAtFile::GetFileName() const
{
	std::string strFileName;
	std::string::size_type uiPositon = m_strFileName.find_last_of("\\");
	if(uiPositon == m_strFileName.npos) return NULL;
	strFileName = m_strFileName.substr(uiPositon+1, m_strFileName.size());
	return strFileName;
}

std::string CAtFile::GetFileTitle() const
{
	std::string strFileTitle;
	std::string::size_type uiPositonA = m_strFileName.find_last_of("\\");
	if(uiPositonA == m_strFileName.npos) return NULL;
	std::string::size_type uiPositonB = m_strFileName.find_last_of(".");
	if(uiPositonB == m_strFileName.npos)
	{
		strFileTitle = m_strFileName.substr(uiPositonA+1, m_strFileName.size());
	}
	else
	{
		strFileTitle = m_strFileName.substr(uiPositonA+1, uiPositonB-uiPositonA-1);
	}
	return strFileTitle;
}

std::string CAtFile::GetFilePath() const
{
	std::string strFilePath;
	std::string::size_type uiPositon = m_strFileName.find_last_of("\\");
	if(uiPositon == m_strFileName.npos) return NULL;
	strFilePath = m_strFileName.substr(0, uiPositon+1);
	return strFilePath;
}

void CAtFile::Close()
{
	assert(this);
	m_hFile = INVALID_ATHANDLE;
	m_strFileName.clear();
}

bool ImportFile_A(const char* ScrFileName, const char* DesFileName)
{
	assert(ScrFileName != NULL);
	assert(DesFileName != NULL);
	CAtFile dzhFile;
	ASSERT_END(dzhFile.Open(DesFileName, CAtFile::modeCreate|CAtFile::modeWrite));

#ifdef WIN32
	HANDLE hFile=CreateFileA(ScrFileName ,GENERIC_READ, FILE_SHARE_READ,		//创建目的文件
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_END(hFile != INVALID_HANDLE_VALUE);

	DWORD dwFileSizeHigh;
	__int64 uiFileSize = GetFileSize(hFile, &dwFileSizeHigh);
	uiFileSize += (((__int64) dwFileSizeHigh) << 32);
	HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY,			//以文件映射的方式读取源文件
		(DWORD)(uiFileSize>>32), (DWORD)(uiFileSize& 0xFFFFFFFF), NULL);
	if (uiFileSize == 0)
	{
		CloseHandle(hFile);
		dzhFile.Close();
		return true;
	}
	ASSERT_END(hFileMap != NULL);

	size_t uiBuffSize = 256*1024;
	ULONGLONG ullTimes = uiFileSize / uiBuffSize;
	size_t	uiRemaid = uiFileSize % uiBuffSize;
	__int64 qwOffSet = 0;
	while (ullTimes > 0) 
	{
		PVOID pvFile = (PBYTE) MapViewOfFile(hFileMap, FILE_MAP_READ, 
			(DWORD)(qwOffSet>>32), (DWORD)(qwOffSet& 0xFFFFFFFF), uiBuffSize); 
		dzhFile.Write(pvFile, uiBuffSize);
		UnmapViewOfFile(pvFile);
		qwOffSet += uiBuffSize;
		ullTimes--;
	}
	if (uiRemaid > 0)
	{
		PVOID pvFile = (PBYTE) MapViewOfFile(hFileMap, FILE_MAP_READ, 
			(DWORD)(qwOffSet>>32), (DWORD)(qwOffSet& 0xFFFFFFFF), uiRemaid); 
		dzhFile.Write(pvFile, uiRemaid);
		UnmapViewOfFile(pvFile);
	}
	CloseHandle(hFileMap);
	CloseHandle(hFile);

#else

	ifstream inFile(ScrFileName, ios::binary|ios::in);
	ASSERT_END(inFile);
	inFile.seekg(0, ios::end);
	ULONGLONG uiFileSize = inFile.tellg().seekpos();

	std::string strData;
	size_t uiBuffSize = 256*1024;
	strData.resize(uiBuffSize);
	inFile.seekg(0, ios::beg);
	dzhFile.SeekToBegin();
	ULONGLONG ullTimes = uiFileSize / uiBuffSize;
	size_t	uiRemaid = uiFileSize % uiBuffSize;
	while (ullTimes > 0) 
	{
		inFile.read((char*)strData.data(), strData.size());
		dzhFile.Write((void*)strData.data(), strData.size());
		inFile.seekg(uiBuffSize, ios::cur);
		ullTimes--;
	}
	if (uiRemaid > 0)
	{
		strData.resize(uiRemaid);
		inFile.read((char*)strData.data(), strData.size());
		dzhFile.Write((void*)strData.data(), strData.size());
	}
	inFile.close();

#endif

	dzhFile.Close();
	return true;
}

bool ExportFile_A(const char* ScrFileName, const char* DesFileName)
{
	ASSERT_END(ScrFileName != NULL);
	ASSERT_END(DesFileName != NULL);
	CAtFile dzhFile;
	if (!dzhFile.Open(ScrFileName, CAtFile::modeRead))
	{
		return false;
	}
	ULONGLONG uiFileSize = dzhFile.GetLength();

	std::string strDesFileName(DesFileName);

	std::string::size_type uiPositonA = strDesFileName.find_last_of("\\");
	std::string::size_type uiPositonB = strDesFileName.find_last_of("/");
	if (uiPositonA== strDesFileName.npos) uiPositonA = 0;
	if (uiPositonB == strDesFileName.npos) uiPositonB = 0;
	std::string::size_type uiPositon = uiPositonA > uiPositonB ? uiPositonA : uiPositonB;

	if (uiPositon != 0)	//若目的文件目录不存在，创建目录
	{
		std::string strDir = strDesFileName.substr(0, uiPositon+1);
		if (!DirHelper::IsDirValid(strDir.c_str()))
		{
			DirHelper::CreateDir(strDir.c_str());
		}
	}

	ofstream outFile(DesFileName, ios::binary|ios::out|ios::trunc);
	if (!outFile)
	{
		return false;
	}

	std::string strData;
	size_t uiBuffSize = 256*1024;
	strData.resize(uiBuffSize);
	outFile.seekp(0, ios::beg);
	dzhFile.SeekToBegin();
	ULONGLONG ullTimes = uiFileSize / uiBuffSize;
	size_t	uiRemaid = uiFileSize % uiBuffSize;
	while (ullTimes > 0) 
	{
		dzhFile.Read((void*)strData.data(), strData.size());
		outFile.write((char*)strData.data(), strData.size());
		outFile.seekp(0, ios::end);
		ullTimes--;
	}
	if (uiRemaid > 0)
	{
		strData.resize(uiRemaid);
		dzhFile.Read((void*)strData.data(), strData.size());
		outFile.write((char*)strData.data(), strData.size());
	}
	outFile.close();
	dzhFile.Close();
	return true;
}

bool GetStandardDir(const char* cDir, std::string& strDir)	//转换为标准目录形式，如绝对目录"D:\\we\\dra\\",相对目录"we\\dra\\"
{
	ASSERT_END(cDir != NULL);
	strDir = cDir;
	std::string::size_type uiPositon = strDir.find("/");	//把所有"/"转化为"\\"
	while (uiPositon != strDir.npos) 
	{
		strDir = strDir.replace(uiPositon, 1, 1, '\\');
		uiPositon = strDir.find("/");
	}
	uiPositon = strDir.find(":");
	if (uiPositon != strDir.npos)	//如果是绝对路径目录，把逻辑盘名称大写
	{
		for (size_t i = 0; i < uiPositon; i++)
		{
			if(strDir[i] >= 'a' && strDir[i] <= 'z')
				strDir[i] = strDir[i] - 32;
		}
	}
	else	//如果是相对路径目录，去掉最前面的".\\"和"\\"
	{
		if (strDir.find_first_of(".") == 0 && strDir.find_first_of("\\") == 1)
		{
			strDir = strDir.substr(2, strDir.size());
		}
		else if (strDir.find_first_of("\\") == 0)
		{
			strDir = strDir.substr(1, strDir.size());
		} 
	}
	//如果目录不是以"\\"结尾，则末尾添加"\\"
	if (strlen(cDir) != 0 && strDir.substr(strDir.size()-1, strDir.size()).compare("\\") != 0)
	{
		strDir.append("\\");
	}
	return true;
}

//转换为标准路径形式，如绝对目录"D:\\we\\dra\\dzh.ini",相对目录"we\\dra\\dzh.ini"
bool GetStandardFilePath(const char* cFilePath, std::string& strFilePath)
{
	ASSERT_END(cFilePath != NULL);
	strFilePath = cFilePath;

	std::string::size_type uiPositon = strFilePath.find("/");	//把所有"/"转化为"\\"
	while (uiPositon != strFilePath.npos) 
	{
		strFilePath = strFilePath.replace(uiPositon, 1, 1, '\\');
		uiPositon = strFilePath.find("/");
	}
	uiPositon = strFilePath.find(":");
	if (uiPositon != strFilePath.npos)	//如果是绝对路径目录，把逻辑盘名称大写
	{
		for (size_t i = 0; i < uiPositon; i++)
		{
			if(strFilePath[i] >= 'a' && strFilePath[i] <= 'z')
				strFilePath[i] = strFilePath[i] - 32;
		}
	}
	else	//如果是相对路径目录，去掉最前面的".\\"和"\\"
	{
		if (strFilePath.find_first_of(".") == 0 && strFilePath.find_first_of("\\") == 1)
		{
			strFilePath = strFilePath.substr(2, strFilePath.size());
		}
		else if (strFilePath.find_first_of("\\") == 0)
		{
			strFilePath = strFilePath.substr(1, strFilePath.size());
		} 
	}
	return true;
}

//转换为以dzh运行目录作为基目录的标准相对路径形式，若路径为相对路径，则默认为基于dzh运行目录的相对路径
//若路径为在dzh运行目录下的绝对路径，则转化为dzh相对路径，若绝对路径不是dzh运行目录下的绝对路径，保持路径不变
bool GetDzhDirPath(const char* cFilePath, std::string& scrDzhPath)
{
	ASSERT_END(cFilePath != NULL);
	scrDzhPath = cFilePath;
	std::string strDzhDir;
	if (!DirHelper::GetExeDir(strDzhDir))	//获取dzh运行目录下的绝对路径
		return false;
	if (strDzhDir.compare(scrDzhPath.substr(0, strDzhDir.size())) == 0)
	{
		scrDzhPath = scrDzhPath.substr(strDzhDir.size(), scrDzhPath.size()).c_str();
	}
	return true;
}

//dzh路径转换为Windows绝对路径形式
bool GetWindowsDirPath(std::string& scrDzhPath, std::string& desWindowsPath)
{
	desWindowsPath.clear();
	std::string strDzhDir;
	if (!DirHelper::GetExeDir(strDzhDir))
		return false;
	std::string::size_type uiPositon = scrDzhPath.find(":");
	if (uiPositon == scrDzhPath.npos) //若路径为基于dzh运行目录的相对路径，则转为绝对路径
	{
		desWindowsPath.append(strDzhDir);
	}
	desWindowsPath.append(scrDzhPath);
	return true;
}

void StringSplit(std::string s,char splitchar,vector<string>& vec)
{
	if(vec.size()>0)//保证vec是空的
		vec.clear();
	int length = s.length();
	int start=0;
	for(int i=0;i<length;i++)
	{
		if(s[i] == splitchar && i == 0)//第一个就遇到分割符
		{
			start += 1;
		}
		else if(s[i] == splitchar)
		{
			vec.push_back(s.substr(start,i - start));
			start = i+1;
		}
		else if(i == length-1)//到达尾部
		{
			vec.push_back(s.substr(start,i+1 - start));
		}
	}
}

} //namespace dzh_base