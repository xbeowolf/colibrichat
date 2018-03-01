/******************************************************************************
*                                                                             *
* profile.cpp -- Windows register support functions for program data          *
*                exchange and structured data implementation                  *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2006-2007. All rights reserved.  *
*                                                                             *
******************************************************************************/

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>

// Common
#include "Profile.h"

#pragma endregion

using namespace profile;

//-----------------------------------------------------------------------------

static std::tstring RegistryKey;
static std::tstring ProfileName;
static HKEY hHiveKey = HKEY_CURRENT_USER;

HKEY CALLBACK profile::getKey()
{
	return hHiveKey;
}

HKEY CALLBACK profile::setKey(const std::tstring& registryKey, const std::tstring& profileName)
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;

	if (registryKey.size()) RegistryKey = registryKey;
	if (profileName.size()) ProfileName = profileName;
	hHiveKey = (
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("software"), 0, KEY_WRITE|KEY_READ, &hSoftKey) == ERROR_SUCCESS &&
		RegOpenKeyEx(hSoftKey, RegistryKey.c_str(), 0, KEY_WRITE|KEY_READ, &hCompanyKey) == ERROR_SUCCESS &&
		RegOpenKeyEx(hCompanyKey, ProfileName.c_str(), 0, KEY_WRITE|KEY_READ, &hAppKey) == ERROR_SUCCESS)
		? HKEY_LOCAL_MACHINE
		: HKEY_CURRENT_USER;
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);
	if (hAppKey != NULL)
		RegCloseKey(hAppKey);
	return hHiveKey;
}

// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CALLBACK profile::getAppKey()
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;

	if (RegOpenKeyEx(hHiveKey, TEXT("software"), 0, KEY_WRITE | KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, RegistryKey.c_str(), 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, ProfileName.c_str(), 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CALLBACK profile::getSectionKey(const std::tstring& section)
{
	HKEY hSectionKey = NULL;
	HKEY hAppKey = getAppKey();
	if (hAppKey == NULL) return NULL;
	if (section.empty()) return hAppKey;

	DWORD dw;
	RegCreateKeyEx(hAppKey, section.c_str(), 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

UINT CALLBACK profile::getInt(const std::tstring& section, const std::tstring& entry, UINT nDefault)
{
	HKEY hSecKey = getSectionKey(section);
	if (hSecKey == NULL) return nDefault;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwCount = sizeof(DWORD);
	LONG lResult = RegQueryValueEx(hSecKey, entry.c_str(), NULL, &dwType,
		(LPBYTE)&dwValue, &dwCount);
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS) return (UINT)dwValue;
	else return nDefault;
}

BOOL CALLBACK profile::getString(const std::tstring& section, const std::tstring& entry, LPTSTR lpszString, UINT count)
{
	HKEY hSecKey = getSectionKey(section);
	if (hSecKey == NULL) return FALSE;
	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)entry.c_str(), NULL, &dwType,
		NULL, &dwCount);
	if (lResult == ERROR_SUCCESS)
	{
		dwCount = min(dwCount, count*sizeof(TCHAR));
		lResult = RegQueryValueEx(hSecKey, (LPTSTR)entry.c_str(), NULL, &dwType,
			(LPBYTE)lpszString, &dwCount);
	}
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

std::tstring CALLBACK profile::getString(const std::tstring& section, const std::tstring& entry, const std::tstring& szDefault)
{
	std::tstring res;
	HKEY hSecKey = getSectionKey(section);
	if (hSecKey == NULL) return szDefault;
	DWORD dwType, dwCount;
	if (RegQueryValueEx(hSecKey, entry.c_str(), NULL, &dwType, NULL, &dwCount) == ERROR_SUCCESS)
	{
		TCHAR* tbuf = (TCHAR*)_alloca(dwCount);
		RegQueryValueEx(hSecKey, entry.c_str(), NULL, &dwType,
			(LPBYTE)tbuf, &dwCount);
		res = tbuf;
	} else res = szDefault;
	RegCloseKey(hSecKey);
	return res;
}

BOOL CALLBACK profile::getBinary(const std::tstring& section, const std::tstring& entry, LPBYTE pData, UINT nBytes)
{
	HKEY hSecKey = getSectionKey(section);
	if (hSecKey == NULL) return FALSE;

	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx(hSecKey, entry.c_str(), NULL, &dwType, NULL, &dwCount);
	if (dwCount > nBytes) return FALSE;
	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegQueryValueEx(hSecKey, entry.c_str(), NULL, &dwType, pData, &dwCount);
	}
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

BOOL CALLBACK profile::setInt(const std::tstring& section, const std::tstring& entry, UINT nValue)
{
	HKEY hSecKey = getSectionKey(section);
	if (hSecKey == NULL) return FALSE;
	LONG lResult = RegSetValueEx(hSecKey, entry.c_str(), NULL, REG_DWORD,
		(LPBYTE)&nValue, sizeof(nValue));
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

BOOL CALLBACK profile::setString(const std::tstring& section, const std::tstring& entry, const std::tstring& value)
{
	LONG lResult;
	if (entry.empty()) //delete whole section
	{
		HKEY hAppKey = getAppKey();
		if (hAppKey == NULL) return FALSE;
		lResult = ::RegDeleteKey(hAppKey, section.c_str());
		RegCloseKey(hAppKey);
	}
	else
	{
		HKEY hSecKey = getSectionKey(section.c_str());
		if (hSecKey == NULL) return FALSE;
		if (value.empty())
		{
			// necessary to cast away const below
			lResult = ::RegDeleteValue(hSecKey, entry.c_str());
		}
		else
		{
			lResult = RegSetValueEx(hSecKey, entry.c_str(), NULL, REG_SZ,
				(LPBYTE)value.c_str(), (DWORD)(value.length() + 1)*sizeof(TCHAR));
		}
		RegCloseKey(hSecKey);
	}
	return lResult == ERROR_SUCCESS;
}

BOOL CALLBACK profile::setBinary(const std::tstring& section, const std::tstring& entry, const BYTE* pData, UINT nBytes)
{
	LONG lResult;
	HKEY hSecKey = getSectionKey(section);
	if (hSecKey == NULL) return FALSE;
	lResult = RegSetValueEx(hSecKey, entry.c_str(), NULL, REG_BINARY, pData, nBytes);
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

static TCHAR CALLBACK HexToChar(BYTE hex)
{
	return hex < 10 ? '0' + hex : 'A' + hex - 10;
}

static BYTE CALLBACK CharToHex(TCHAR ch)
{
	return (ch <= '9' ? ch - '0' : (ch & 0x5F) - 'A' + 10) & 0x0F;
}

void CALLBACK profile::BinToStr(const BYTE* data, TCHAR* str, size_t size)
{
	for (UINT i = 0; i < size; i++)
	{
		str[2*i + 0] = HexToChar((data[i] >> 4) & 0x0F);
		str[2*i + 1] = HexToChar((data[i] >> 0) & 0x0F);
	}
}

void CALLBACK profile::StrToBin(const TCHAR* str, BYTE* data, size_t size)
{
	for (UINT i = 0; i < size; i++)
	{
		data[i] = (CharToHex(str[2*i + 0]) << 4) + (CharToHex(str[2*i + 1]) << 0);
	}
}

CALLBACK JEntry::JEntry()
{
	ZeroMemory(&data, sizeof(data));
	type = edtNull;
}

CALLBACK JEntry::~JEntry()
{
	Clear();
}

void CALLBACK JEntry::Clear()
{
	switch (type)
	{
	case edtString:
		delete data.str;
		break;

	case edtBin:
		delete data.bin;
		break;

	case edtFolder:
		delete data.list;
		break;
	}
	ZeroMemory(&data, sizeof(data));
	type = edtNull;
}

TString CALLBACK JEntry::ToString() const
{
	TCHAR buffer[128];
	switch (type)
	{
	case edtBool:
		StringCchCopy(buffer, _countof(buffer), data.b ? TEXT("true") : TEXT("false"));
		break;

	case edtWord:
		StringCchPrintf(buffer, _countof(buffer), TEXT("%u"), data.w);
		break;

	case edtInt:
		StringCchPrintf(buffer, _countof(buffer), TEXT("%li"), data.i);
		break;

	case edtDword:
		StringCchPrintf(buffer, _countof(buffer), TEXT("%lu"), data.dw);
		break;

	case edtFloat:
		StringCchPrintf(buffer, _countof(buffer), TEXT("%f"), data.f);
		break;

	default:
		buffer[0] = 0;
	}
	return buffer;
}

bool& CALLBACK JEntry::setBool()
{
	if (type != edtBool)
	{
		if (type != edtWord && type != edtInt && type != edtDword)
			Clear();
		else
			data.b = data.i != 0;
		type = edtBool;
	}
	return data.b;
}

WORD& CALLBACK JEntry::setWord()
{
	if (type != edtWord)
	{
		if (type != edtBool && type != edtInt && type != edtDword)
			Clear();
		type = edtWord;
	}
	return data.w;
}

int& CALLBACK JEntry::setInt()
{
	if (type != edtInt)
	{
		if (type != edtBool && type != edtWord && type != edtDword)
			Clear();
		type = edtInt;
	}
	return data.i;
}

DWORD& CALLBACK JEntry::setDword()
{
	if (type != edtDword)
	{
		if (type != edtBool && type != edtWord && type != edtInt)
			Clear();
		type = edtDword;
	}
	return data.dw;
}

float& CALLBACK JEntry::setFloat()
{
	if (type != edtFloat)
	{
		if (type != edtBool && type != edtWord && type != edtInt)
			Clear();
		else
			data.f = (float)data.i;
		type = edtFloat;
	}
	return data.f;
}

TString& CALLBACK JEntry::setString()
{
	if (type != edtString)
	{
		Clear();
		data.str = new TString();
		ASSERT(data.str);
		type = edtString;
	}
	return *data.str;
}

UINT CALLBACK JEntry::getBin(void* ptr, UINT size)
{
	if (type != edtBin) return 0;
	size = min((UINT)data.bin->size(), size);
	if (size) MoveMemory(ptr, &(*data.bin)[0], size);
	return size;
}

void CALLBACK JEntry::putBin(const void* ptr, UINT size)
{
	setBin();
	data.bin->clear();
	data.bin->resize(size);
	if (size) MoveMemory(&(*data.bin)[0], ptr, size);
}

TBin& CALLBACK JEntry::setBin()
{
	if (type != edtBin)
	{
		Clear();
		data.bin = new TBin();
		ASSERT(data.bin);
		type = edtBin;
	}
	return *data.bin;
}

TList& CALLBACK JEntry::setList()
{
	if (type != edtFolder)
	{
		Clear();
		data.list = new TList();
		ASSERT(data.list);
		type = edtFolder;
	}
	return *data.list;
}

JEntry* CALLBACK JEntry::operator [] (const TCHAR* path)
{
	int start, end = 0;
	JEntry* entry = this;
	while (*(path + end))
	{
		for (start = end; *(path + end) && *(path + end) != '\\' && *(path + end) != '/'; end++);
		TString name(path, start, end - start);
		entry->setList();
		JPtr<JEntry>& v = entry->valList[name];
		if (!v) v = new JEntry();
		entry = v;
		ASSERT(entry);
		if (*(path + end)) end++;
	}
	return entry;
}

static void CALLBACK WriteTab(std::basic_ostream<TCHAR>& os, int num)
{
	for (int i = 0; i < num; i++) os << '\t';
}

bool CALLBACK profile::IContext::Read(JEntry& entry)
{
	try
	{
		OpenRead();
		ReadEntry(entry);
		CloseRead();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool CALLBACK profile::IContext::Write(const JEntry& entry)
{
	try
	{
		OpenWrite();
		WriteEntry(entry);
		CloseWrite();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

size_t CStream::FormatStrLen = 50;
bool CStream::UseLexemUint = false;
bool CStream::UseLexemText = false;
bool CStream::UseLexemList = false;
bool CStream::UseLexemEquTypeLen = true;

CALLBACK CStream::CStream()
{
	is = 0;
	os = 0;
	unicode = true;
}

CALLBACK CStream::~CStream()
{
	delete is;
	delete os;
}

void CALLBACK CStream::SkipSpace()
{
	while (!is->eof() && ch <= TEXT(' ')) is->get(ch);
}

profile::TString CALLBACK CStream::ReadLexem()
{
	TString str = TEXT("");
	SkipSpace();
	if (is->eof()) return str;
	if (ch == TEXT('\"') || ch == TEXT('\'')) // read quoted string
	{
		while (!is->eof() && (ch == TEXT('\"') || ch == TEXT('\'')))
		{
			is->get(ch);
			while (!is->eof() && ch != TEXT('\"') && ch != TEXT('\''))
			{
				if (ch == '\\')
				{
					is->get(ch);
					switch (ch)
					{
					case 'a':
						ch = '\a';
						break;

					case 'b':
						ch = '\b';
						break;

					case 'f':
						ch = '\f';
						break;

					case 'n':
						ch = '\n';
						break;

					case 'r':
						ch = '\r';
						break;

					case 't':
						ch = '\t';
						break;

					case 'v':
						ch = '\v';
						break;
					}
				}
				str += ch;
				is->get(ch);
			}
			is->get(ch);
			SkipSpace();
		}
	}
	else
		if (IsCharAlpha(ch) || ch == TEXT('_')) // read literals
		{
			do
			{
				str += ch;
				is->get(ch);
			} while (!is->eof() && (IsCharAlphaNumeric(ch) || ch == TEXT('_')) ||
				ch == TEXT('\\') || ch == TEXT('/'));
		}
		else
			if ((ch >= TEXT('0') && ch <= TEXT('9')) ||
				(ch == TEXT('+') || ch == TEXT('-'))) // read number
			{
				bool wasDot = false;
				bool wasE   = false;
				bool wasSgn = false;
				bool wasExp = false;
				if (ch == TEXT('+') || ch == TEXT('-')) // sign of numeric
				{
					str += ch;
					is->get(ch);
				}
				do
				{
					if (ch >= TEXT('0') && ch <= TEXT('9')) // any digit
					{
						if (wasE) wasSgn = wasExp = true;
					}
					else
						if (ch == TEXT('.')) // dot in float
						{
							if (wasDot) break;
							wasDot = true;
						}
						else
							if (ch == TEXT('E') || ch == TEXT('e')) // "E" literal in float
							{
								if (wasE) break;
								wasDot = wasE = true;
							}
							else
								if (ch == TEXT('+') || ch == TEXT('-')) // sign of exponent
								{
									if (wasSgn || !wasE || wasExp) break;
									wasSgn = true;
								}
								else // non-numeric symbol
									break;
					str += ch;
					is->get(ch);
				} while (!is->eof());
			}
			else // othercase - any symbol
			{
				str += ch;
				is->get(ch);
			}
			return str;
}

void CALLBACK CStream::setString(const TCHAR* str, bool quot)
{
	size_t len = lstrlen(str), pos = 0, linepos;
	if (len > FormatStrLen) *os << std::endl;
	while (pos < len)
	{
		if (len > FormatStrLen) WriteTab(*os, writelevel + 1);
		*os << (quot ? TEXT("\"") : TEXT("'"));
		linepos = 0;
		while (pos < len && linepos < FormatStrLen)
		{
			if (str[pos] == '\\' || str[pos] == '\"' || str[pos] == '\'' || str[pos] < ' ')
			{
				*os << '\\';
				switch (str[pos])
				{
				case '\a':
					*os << 'a';
					break;

				case '\b':
					*os << 'b';
					break;

				case '\f':
					*os << 'f';
					break;

				case '\n':
					*os << 'n';
					break;

				case '\r':
					*os << 'r';
					break;

				case '\t':
					*os << 't';
					break;

				case '\v':
					*os << 'v';
					break;

				default:
					*os << str[pos];
				}
			}
			else
				*os << str[pos];
			pos++;
			linepos++;
		}
		*os << (quot ? TEXT("\"") : TEXT("'"));
		if (len > FormatStrLen) *os << std::endl;
	}
}

void CALLBACK CStream::ReadEntry(JEntry& entry)
{
	TString type, name, value;

	while (true)
	{
		// Read type of entry
		type = ReadLexem();
		if (type == TEXT("}") || is->eof()) return;
		// Read lexem name
		name = ReadLexem();
		if (is->eof()) throw 0;
		// Read "=" sign and value
		if (type != TEXT("folder") && type != TEXT("list"))
		{
			if (ReadLexem() != TEXT("=")) throw 0;
			if (is->eof()) throw 0;
			// Read context of lexem
			value = ReadLexem();
			if (is->eof()) throw 0;
			if (ReadLexem() != TEXT(";")) throw 0;
		}

		JEntry* subentry = entry[name.c_str()];
		if (type == TEXT("bool"))
			subentry->valBool = value == TEXT("true")
			? true
			: (value == TEXT("false") ? false : (_ttoi(value.c_str()) != 0));
		else if (type == TEXT("word"))
			subentry->valWord = (WORD)_ttoi(value.c_str());
		else if (type == TEXT("int"))
			subentry->valInt = _ttoi(value.c_str());
		else if (type == TEXT("dword") || type == TEXT("uint"))
			subentry->valDword = _ttoi(value.c_str());
		else if (type == TEXT("float") || type == TEXT("flt"))
			subentry->valFloat = (float)_tstof(value.c_str());
		else if (type == TEXT("string") || type == TEXT("text"))
			subentry->valString = value;
		else if (type == TEXT("bin"))
		{
			subentry->setBin();
			subentry->valBin.resize(value.length() / 2);
			StrToBin(value.data(), &subentry->valBin[0], value.length() / 2);
		}
		else if (TEXT("folder") || type == TEXT("list"))
		{
			subentry->setList();
			if (ReadLexem() != TEXT("{")) throw 0;
			ReadEntry(*subentry);
		}
		else throw 0;
	}
}

void CALLBACK CStream::WriteEntry(const JEntry& entry)
{
	writelevel++;
	for each (TList::value_type const& v in entry.valList)
	{
		WriteTab(*os, writelevel);
		switch (v.second->getType())
		{
		case JEntry::edtBool:
			*os << TEXT("bool") << TEXT(" ") << v.first << TEXT(" = ") << v.second->ToString() << TEXT(";");
			break;

		case JEntry::edtWord:
			*os << TEXT("word") << TEXT(" ") << v.first << TEXT(" = ") << v.second->ToString() << TEXT(";");
			break;

		case JEntry::edtInt:
			*os << (UseLexemEquTypeLen ? TEXT("int ") : TEXT("int")) <<
				TEXT(" ") << v.first << TEXT(" = ") << v.second->ToString() << TEXT(";");
			break;

		case JEntry::edtDword:
			*os << (UseLexemUint || UseLexemEquTypeLen ? TEXT("uint") : TEXT("dword")) <<
				TEXT(" ") << v.first << TEXT(" = ") << v.second->ToString() << TEXT(";");
			break;

		case JEntry::edtFloat:
			*os << (UseLexemEquTypeLen ? TEXT("flt ") : TEXT("float")) << TEXT(" ") << v.first << TEXT(" = ") << v.second->ToString() << TEXT(";");
			break;

		case JEntry::edtString:
			*os << (UseLexemText || UseLexemEquTypeLen ? TEXT("text") : TEXT("string")) <<
				TEXT(" ") << v.first << TEXT(" = ");
			setString(v.second->valString.c_str(), true);
			*os << TEXT(";");
			break;

		case JEntry::edtBin:
			{
				TString str;
				str.resize(v.second->valBin.size() * 2);
				BinToStr(&v.second->valBin[0], (TCHAR*)str.data(), v.second->valBin.size());

				*os << (UseLexemEquTypeLen ? TEXT("bin ") : TEXT("bin")) <<
					TEXT(" ") << v.first << TEXT(" = ");
				setString(str.c_str(), false);
				*os << TEXT(";");
				break;
			}

		case JEntry::edtFolder:
			*os << (UseLexemList || UseLexemEquTypeLen ? TEXT("list") : TEXT("folder")) <<
				TEXT(" ") << v.first << std::endl;
			WriteTab(*os, writelevel);
			*os << TEXT("{") << std::endl;
			WriteEntry(*v.second);
			WriteTab(*os, writelevel);
			*os << TEXT("}");
			break;
		}
		*os << std::endl;
	}
	writelevel--;
}

CALLBACK CFile::CFile(const TCHAR* file) : CStream(), sFile(file)
{
	is = new std::basic_ifstream<TCHAR>();
	ASSERT(is);
	os = new std::basic_ofstream<TCHAR>();
	ASSERT(os);
}

void CALLBACK CFile::OpenRead()
{
	// Open file
	dynamic_cast<std::basic_ifstream<TCHAR>*>(is)->open(sFile.c_str(), std::ios_base::binary | std::ios_base::in);
	if (!*dynamic_cast<std::basic_ifstream<TCHAR>*>(is)) throw 0;
	ch = 0;
}

void CALLBACK CFile::CloseRead()
{
	if (*dynamic_cast<std::basic_ifstream<TCHAR>*>(is))
		dynamic_cast<std::basic_ifstream<TCHAR>*>(is)->close();
}

void CALLBACK CFile::OpenWrite()
{
	// Open file
	dynamic_cast<std::basic_ofstream<TCHAR>*>(os)->open(sFile.c_str(), std::ios_base::out);
	if (!*dynamic_cast<std::basic_ofstream<TCHAR>*>(os)) throw 0;
	writelevel = -1;
}

void CALLBACK CFile::CloseWrite()
{
	if (*dynamic_cast<std::basic_ofstream<TCHAR>*>(os))
		dynamic_cast<std::basic_ofstream<TCHAR>*>(os)->close();
}

CALLBACK CStrBuffer::CStrBuffer(TString& str) : string(str)
{
	is = new std::basic_istringstream<TCHAR>(string);
	ASSERT(is);
	os = new std::basic_ostringstream<TCHAR>(string);
	ASSERT(os);
}

//-----------------------------------------------------------------------------

// The End.