/******************************************************************************
*                                                                             *
* bnb.h -- Binary Beowolf Network Engine. Sockets JBNB class definition       *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2010. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _BNB_
#define _BNB_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Common
#include "CRC.h"
#include "crypto\huge-number.h"
#include "crypto\ecrypt.h"
// ECrypt
extern "C"
{
#include "crypto\skein\skein.h"
}

// Project
#include "netengine.h"
#include "iomem.h"
#include "bnbp.h"

#pragma endregion

//-----------------------------------------------------------------------------

// default encryption algorithm
#define ECRYPT_BINDEFAULT      "HC-256"

//-----------------------------------------------------------------------------

namespace netengine
{
	class Header;
	class JBTransaction;
	class JBLink;
	class JBNB;

	class Header
	{
	public:

		static unsigned getSizeDimension(size_t val) throw();
		static unsigned getDimensionIndex(unsigned val) throw();
		static unsigned getIndexDimension(unsigned val) throw();
		static size_t getHeaderSize(const BNP_HDR2& hdr2) throw();
		static size_t isComplete(const char* ptr, size_t size) throw();

		Header(WORD msg, WORD ti = 0) throw();
		void init(WORD msg, WORD ti = 0);

		int    read(const char*& ptr) throw();
		size_t write(char*& ptr) const throw();
		size_t getSize() const throw();
		CRC16  realCRC() const throw();

		__declspec(property(get=getnativeAction,put=setnativeAction)) WORD nativeAction;
		WORD __fastcall getnativeAction() const;
		void __fastcall setnativeAction(WORD val);

		__declspec(property(get=getnativeMessage,put=setnativeMessage)) WORD nativeMessage;
		WORD __fastcall getnativeMessage() const;
		void __fastcall setnativeMessage(WORD val);

	protected:

		JPROPERTY_R(CRC16, crc);
		JPROPERTY_RW(WORD, message);
		JPROPERTY_RW(WORD, trnid);
		JPROPERTY_R(DWORD, sizeuncompr);
		JPROPERTY_R(DWORD, sizecompr);
	};

	class JBTransaction : public JTransaction, public Header
	{
	public:

		explicit JBTransaction(WORD message = 0, WORD ti = 0) throw();

		size_t getSize() const throw();
		void prepare() throw();

		// Checks transaction content on possibility processing without identification
		bool isPrimary() throw();

		// stream IO
		int  read(const char*& ptr);
		void serialize(JPtr<JLink> link); // write to send buffer
		bool unserialize(JPtr<JLink> link); // read from recv buffer

		// compression
		void setdataUncompr(const std::string& str) throw();
		void setdataCompr(const std::string& str, DWORD ucsz) throw();
		bool compress(int level) throw();
		bool uncompress() throw();

		// encryption
		void encrypt(ecrypt::JCipher* ctx, const u8* iv) throw();
		void decrypt(ecrypt::JCipher* ctx, const u8* iv) throw();
		void restore(const std::string& content, bool enctypted);

		CRC16  realCRC() const throw();
		void   updateCRC() throw();

	protected:

		JPROPERTY_RREF_CONST(std::string, data);
	}; // class JBTransaction

	class JBLink : public JLink
	{
	public:

		friend class JBTransaction;

		static JPtr<JBLink> get( id_type id )
		{
			IDS::const_iterator i = s_IDs.find(id);
			return i != s_IDs.end() ? jdynamic_cast<JBLink, JLink>(i->second) : 0;
		}

		explicit JBLink();
		JBLink(SOCKET sock, const sockaddr_in& addr);

		// Security methods
		const char* getEncryptorName() const;
		void setEncryptor(const char* ciphername);
		const char* getDecryptorName() const;
		void setDecryptor(const char* ciphername);
		void keysetup(huge::number* key);

	protected:

		void Clear(); // inits all data members

	protected:

		// Security data
		uint_t m_keysize;
		JPtr<JArrayRef<u08b_t>> m_key;
		JPtr<ecrypt::JCipher> ctxDecryptor;
		JPtr<ecrypt::JCipher> ctxEncryptor;
		u64b_t m_uDecryptCount;
		u64b_t m_uEncryptCount;
		ecrypt::Nonce256 m_nonceDecrypt;
		ecrypt::Nonce256 m_nonceEncrypt;
	}; // class JBLink

	class JBNB : public JEngine
	{
	public:

		typedef fastdelegate::FastDelegateList2<SOCKET, io::mem&, void> TrnCommand;
		typedef fastdelegate::FastDelegateList4<SOCKET, WORD, io::mem&, std::ostream&, void> TrnQuest;
		typedef fastdelegate::FastDelegateList3<SOCKET, WORD, io::mem&, void> TrnReply;
		typedef fastdelegate::FastDelegateList2<SOCKET, io::mem&, void> TrnNotify;
		typedef std::map<WORD, TrnCommand> MapTrnCommand;
		typedef std::map<WORD, TrnQuest> MapTrnQuest;
		typedef std::map<WORD, TrnReply> MapTrnReply;
		typedef std::map<WORD, TrnNotify> MapTrnNotify;

		JBNB();

		// --- Service ---

		void Init();
		void Done();

	public:

		virtual DWORD getMinVersion() const {return MAKEVERSIONNUM(1, 0, 0, 0);}
		virtual DWORD getCurVersion() const {return MAKEVERSIONNUM(1, 0, 0, 0);}

		// Security
		static bool CALLBACK isSupportedCipher(const char* ciphername);
		static JPtr<ecrypt::JCipher> CALLBACK CipherFactory(const char* ciphername);
		// encryption cipher name
		virtual const char* getEncryptorName() const {return ECRYPT_BINDEFAULT;}

		// --- Sending ---

		// Sends transaction with given message and data
		bool MakeAndPushTrn(SOCKET sock, WORD message, WORD trnid, const std::string& str, size_t ssi = 0) throw();
		// Makes transaction with given string, base algorithm
		JPtr<JBTransaction> MakeTrn(WORD message, WORD trnid, const std::string& str) const throw();
		// Makes transaction with given long value
		JPtr<JBTransaction> MakeTrn(WORD message, WORD trnid, LONG_PTR val) const throw();
		// Makes transaction with given memory data block
		JPtr<JBTransaction> MakeTrn(WORD message, WORD trnid, const void* ptr, int size) const throw();

	protected:

		// --- Init/done ---

		virtual void LoadState() {}
		virtual void SaveState() {}
		virtual void InitLogs() {}

		// --- Transactions work ---

		// Create link object for this engine
		JPtr<JLink> createLink() const;
		JPtr<JLink> createLink(SOCKET sock, const sockaddr_in& addr) const;
		// Creates transaction for this protocol
		JPtr<JTransaction> createTrn() const {return new JBTransaction();}
		// Extracts and dispatch transactions from stream to objects
		bool DispatchTrn(SOCKET sock, JTransaction* jpTrn);
		// Parse all incoming transactions on given messages
		virtual void ParseTransaction(SOCKET sock, WORD message, WORD trnid, io::mem is);

		// --- Beowolf Network Protocol Messages reciving ---

		void Recv_Cmd_NULL(SOCKET sock, io::mem& is);
		void Recv_Quest_NULL(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os);
		void Recv_Reply_NULL(SOCKET sock, WORD trnid, io::mem& is);
		void Recv_Notify_NULL(SOCKET sock, io::mem& is);
		void Recv_Quest_Ping(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os);
		void Recv_Reply_Ping(SOCKET sock, WORD trnid, io::mem& is);
		virtual void Recv_Quest_Identify(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os);
		virtual void Recv_Reply_Identify(SOCKET sock, WORD trnid, io::mem& is);
		void Recv_Cmd_Disconnect(SOCKET sock, io::mem& is);

		// --- Beowolf Network Protocol Messages sending ---

		JPtr<JBTransaction> Make_Quest_Ping() const;
		JPtr<JBTransaction> Make_Quest_Identify(const char* ciphername) const;
		void Form_Reply_Identify(std::ostream& os, LRESULT result, const char* ciphername) const;
		JPtr<JBTransaction> Make_Cmd_Disconnect() const;

		// --- Events responders ---

		void OnHook(JNode* src);
		void OnUnhook(JNode* src);
		// Register/unregister transactions parsers
		void RegHandlers(JNode* src);
		void UnregHandlers(JNode* src);

		void OnLinkEstablished(SOCKET sock);

		virtual void OnTrnProcess(SOCKET sock, WORD message, WORD trnid, io::mem is) {}
		virtual void OnTrnIgnore(SOCKET sock, WORD message, WORD trnid);
		virtual void OnTrnBadCRC(SOCKET sock);

	public:

		static CHAR szSignature[16];
		static int s_nCompression;

		// --- Socket events ---

		fastdelegate::FastDelegateList4<SOCKET, WORD, WORD, io::mem>
			EvTrnProcess; // Alternative opportunity to process transaction outside of engine
		fastdelegate::FastDelegateList3<SOCKET, WORD, WORD>
			EvTrnIgnore; // Message is can not be processed
		fastdelegate::FastDelegateList1<SOCKET>
			EvTrnBadCRC;

	protected:

		JPROPERTY_RREF_CONST(std::tstring, passwordNet);

		// --- Transactions parsers maps ---

		MapTrnCommand m_mTrnCommand;
		MapTrnQuest   m_mTrnQuest;
		MapTrnReply   m_mTrnReply;
		MapTrnNotify  m_mTrnNotify;
	}; // class JBNB
}; // netengine

//-----------------------------------------------------------------------------

#endif // _BNB_