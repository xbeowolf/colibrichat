/******************************************************************************
*                                                                             *
* profile.h -- Windows register support functions for program data exchange   *
*              Structured data interface                                      *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2006-2007. All rights reserved.  *
*                                                                             *
******************************************************************************/

#pragma once
#ifndef _PROFILE_
#define _PROFILE_

//-----------------------------------------------------------------------------

namespace profile
{
	HKEY CALLBACK getKey();
	HKEY CALLBACK setKey(const std::tstring& registryKey, const std::tstring& profileName);
	HKEY CALLBACK getAppKey();
	HKEY CALLBACK getSectionKey(const std::tstring& section);
	UINT CALLBACK getInt(const std::tstring& section, const std::tstring& entry, UINT nDefault);
	BOOL CALLBACK getString(const std::tstring& section, const std::tstring& entry, LPTSTR lpszString, UINT count);
	BOOL CALLBACK getBinary(const std::tstring& section, const std::tstring& entry, LPBYTE pData, UINT nBytes);
	BOOL CALLBACK setInt(const std::tstring& section, const std::tstring& entry, UINT nValue);
	BOOL CALLBACK setString(const std::tstring& section, const std::tstring& entry, const std::tstring& value);
	BOOL CALLBACK setBinary(const std::tstring& section, const std::tstring& entry, const BYTE* pData, UINT nBytes);

	std::tstring CALLBACK getString(const std::tstring& section, const std::tstring& entry, const std::tstring& szDefault = TEXT(""));

	void CALLBACK BinToStr(const BYTE* data, TCHAR* str, size_t size);
	void CALLBACK StrToBin(const TCHAR* str, BYTE* data, size_t size);

	class JEntry;
	typedef std::basic_string<TCHAR> TString;
	typedef std::vector<BYTE> TBin;
	typedef std::map<std::basic_string<TCHAR>, JPtr<JEntry>> TList;

	class JEntry : public JClass
	{
	public:

		enum EType
		{
			edtNull,
			edtBool,
			edtWord,
			edtInt,
			edtDword, edtUint = edtDword,
			edtFloat,
			edtString, edtText = edtString,
			edtBin,
			edtFolder, edtList = edtFolder
		};

	public:

		CALLBACK JEntry();
		virtual CALLBACK ~JEntry();

		void CALLBACK Clear();

		EType CALLBACK getType() const {return type;}
		TString CALLBACK ToString() const;

		__declspec(property(get=getBool,put=putBool)) bool valBool;
		bool CALLBACK getBool() const {return data.b;}
		bool CALLBACK putBool(bool value) {return setBool() = value;}
		bool& CALLBACK setBool();
		__declspec(property(get=getWord,put=putWord)) WORD valWord;
		WORD CALLBACK getWord() const {return data.w;}
		WORD CALLBACK putWord(WORD value) {return setWord() = value;}
		WORD& CALLBACK setWord();
		__declspec(property(get=getInt,put=putInt)) int valInt;
		int CALLBACK getInt() const {return data.i;}
		int CALLBACK putInt(int value) {return setInt() = value;}
		int& CALLBACK setInt();
		__declspec(property(get=getDword,put=putDword)) DWORD valDword;
		DWORD CALLBACK getDword() const {return data.dw;}
		DWORD CALLBACK putDword(DWORD value) {return setDword() = value;}
		DWORD& CALLBACK setDword();
		__declspec(property(get=getFloat,put=putFloat)) float valFloat;
		float CALLBACK getFloat() const {return data.f;}
		float CALLBACK putFloat(float value) {return setFloat() = value;}
		float& CALLBACK setFloat();
		__declspec(property(get=getString,put=putString)) TString& valString;
		TString& CALLBACK getString() const {_ASSERT(type == edtString); return *data.str;}
		TString& CALLBACK putString(const TString& value) {return setString() = value;}
		TString& CALLBACK setString();
		__declspec(property(get=getBin,put=putBin)) TBin& valBin;
		UINT CALLBACK getBin(void* ptr, UINT size);
		TBin& CALLBACK getBin() const {_ASSERT(type == edtBin); return *data.bin;}
		TBin& CALLBACK putBin(const TBin& value) {return setBin() = value;}
		void CALLBACK putBin(const void* ptr, UINT size);
		TBin& CALLBACK setBin();
		__declspec(property(get=getList,put=putList)) TList& valList;
		TList& CALLBACK getList() const {_ASSERT(type == edtFolder); return *data.list;}
		TList& CALLBACK putList(const TList& value) {return setList() = value;}
		TList& CALLBACK setList();

		void CALLBACK reinterpretDwordToFloat() {if (type == edtDword) type = edtFloat;}
		void CALLBACK reinterpretFloatToDword() {if (type == edtFloat) type = edtDword;}
		JEntry* CALLBACK operator [] (const TCHAR* path);

	private:

		union
		{
			bool b;
			WORD w;
			int i;
			DWORD dw;
			float f;
			TString* str;
			TBin* bin;
			TList* list;
		} data;

		EType type;
	};

	class IContext
	{
	public:

		bool CALLBACK Read(JEntry& entry);
		bool CALLBACK Write(const JEntry& entry);

	protected:

		virtual void CALLBACK OpenRead() = 0;
		virtual void CALLBACK ReadEntry(JEntry& entry) = 0;
		virtual void CALLBACK CloseRead() = 0;
		virtual void CALLBACK OpenWrite() = 0;
		virtual void CALLBACK WriteEntry(const JEntry& entry) = 0;
		virtual void CALLBACK CloseWrite() = 0;
	};

	class CStream : public IContext
	{
	public:

		static size_t FormatStrLen;
		static bool UseLexemUint, UseLexemText, UseLexemList, UseLexemEquTypeLen;

		CALLBACK CStream();
		virtual CALLBACK ~CStream();

	protected:

		virtual void CALLBACK ReadEntry(JEntry& entry);
		virtual void CALLBACK WriteEntry(const JEntry& entry);

		std::basic_istream<TCHAR> *is;
		std::basic_ostream<TCHAR> *os;
		bool unicode;

		void CALLBACK SkipSpace();
		TString CALLBACK CStream::ReadLexem();
		void CALLBACK CStream::setString(const TCHAR* str, bool quot = true);

		int writelevel;
		TCHAR ch;
	};

	class CFile : public CStream
	{
	public:
		CALLBACK CFile(const TCHAR* file);

	protected:

		virtual void CALLBACK OpenRead();
		virtual void CALLBACK CloseRead();
		virtual void CALLBACK OpenWrite();
		virtual void CALLBACK CloseWrite();

	private:
		TString sFile;
	};

	class CStrBuffer : public CStream
	{
	public:
		CALLBACK CStrBuffer(TString& str);

	protected:

		virtual void CALLBACK OpenRead() {is->seekg(0);}
		virtual void CALLBACK CloseRead() {}
		virtual void CALLBACK OpenWrite() {os->seekp(0);}
		virtual void CALLBACK CloseWrite() {}

	private:
		TString& string;
	};
};

//-----------------------------------------------------------------------------

#endif // _PROFILE_
