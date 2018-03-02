// bnb.cpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// zlib
#include "zlib.h"

// Project
#include "bnb.h"
// ECrypt
#include "crypto\copy.h"
#include "crypto\hc-256.h"
#include "crypto\rabbit.h"
#include "crypto\salsa20.h"
#include "crypto\lex.h"
#include "crypto\grain-v1.h"
#include "crypto\grain-128.h"
#include "crypto\trivium.h"
#include "crypto\edon80.h"

#define BNP_UNCOMPRESS_ONCE

#pragma endregion

//-----------------------------------------------------------------------------

using namespace netengine;

//-----------------------------------------------------------------------------

#define DFLT_GENERATOR            "2" /* Default generator value */
#define DFLT_MODULUS              /* Default modulus value */ \
"f488fd584e49dbcd20b49de49107366b336c380d451d0f7c88b31c7c5b2d8ef6" \
"f3c923c043f0a55b188d8ebb558cb85d38d334fd7c175743a31d186cde33212c" \
"b52aff3ce1b1294018118d7c84a70a72d686c40319c807297aca950cd9969fab" \
"d00a509b0246d3083d66a45d419f9c7cbd894b221926baaba25ec355e92f78c7"

static huge::numptr gen(huge::number::from_string(DFLT_GENERATOR, 0, 16));
static huge::numptr mod(huge::number::from_string(DFLT_MODULUS, 0, 16));

//-----------------------------------------------------------------------------

namespace ecrypt {
	ECRYPTCLASS(copy, JCopy);
	ECRYPTCLASS(hc256, JHC256);
	ECRYPTCLASS(rabbit, JRabbit);
	ECRYPTCLASS(salsa20, JSalsa20);
	ECRYPTCLASS(lex, JLex);
	ECRYPTCLASS(grainv1, JGrainv1);
	ECRYPTCLASS(grain128, JGrain128);
	ECRYPTCLASS(trivium, JTrivium);
	ECRYPTCLASS(edon80, JEdon80);
}; // ecrypt

bool CALLBACK JBNB::isSupportedCipher(const char* ciphername)
{
	return !_stricmp(ciphername, ecrypt::copy::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::hc256::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::rabbit::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::salsa20::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::lex::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::grainv1::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::grain128::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::trivium::ECRYPT_NAME)
		|| !_stricmp(ciphername, ecrypt::edon80::ECRYPT_NAME);
}

JPtr<ecrypt::JCipher> CALLBACK JBNB::CipherFactory(const char* ciphername)
{
	if (!_stricmp(ciphername, ecrypt::copy::ECRYPT_NAME))
		return new ecrypt::JCopy;
	else if (!_stricmp(ciphername, ecrypt::hc256::ECRYPT_NAME))
		return new ecrypt::JHC256;
	else if (!_stricmp(ciphername, ecrypt::rabbit::ECRYPT_NAME))
		return new ecrypt::JRabbit;
	else if (!_stricmp(ciphername, ecrypt::salsa20::ECRYPT_NAME))
		return new ecrypt::JSalsa20;
	else if (!_stricmp(ciphername, ecrypt::lex::ECRYPT_NAME))
		return new ecrypt::JLex;
	else if (!_stricmp(ciphername, ecrypt::grainv1::ECRYPT_NAME))
		return new ecrypt::JGrainv1;
	else if (!_stricmp(ciphername, ecrypt::grain128::ECRYPT_NAME))
		return new ecrypt::JGrain128;
	else if (!_stricmp(ciphername, ecrypt::trivium::ECRYPT_NAME))
		return new ecrypt::JTrivium;
	else if (!_stricmp(ciphername, ecrypt::edon80::ECRYPT_NAME))
		return new ecrypt::JEdon80;
	else return 0;
}

//-----------------------------------------------------------------------------

//
// Header
//

unsigned Header::getSizeDimension(size_t val) throw()
{
	if ((val & 0x00000000UL) == val) return 0;
	else if ((val & 0x000000FFUL) == val) return 1;
	else if ((val & 0x0000FFFFUL) == val) return 2;
	else return 4;
}

unsigned Header::getDimensionIndex(unsigned val) throw()
{
	switch (val)
	{
	case 0: return 0;
	case 1: return 1;
	case 2: return 2;
	case 4: return 3;
	default: return 3; // no case to here, for compilation without warning
	}
}

unsigned Header::getIndexDimension(unsigned val) throw()
{
	_ASSERT(val < 4);
	switch (val)
	{
	case 0: return 0;
	case 1: return 1;
	case 2: return 2;
	case 3: return 4;
	default: return 4; // only for compilation without warning
	}
}

size_t Header::getHeaderSize(const BNP_HDR2& hdr2) throw()
{
	unsigned szd = getIndexDimension(hdr2.m_sizedim);
	return sizeof(crc16_t) + sizeof(BNP_HDR2) + (hdr2.m_hasTrnid ? sizeof(WORD) : 0) + szd + (hdr2.m_isCompressed ? szd : 0);
}

size_t Header::isComplete(const char* ptr, size_t size) throw()
{
	const char* ptr0 = ptr;

	if (size < sizeof(crc16_t)) return 0;
	ptr += sizeof(crc16_t);
	size -= sizeof(crc16_t);

	if (size < sizeof(BNP_HDR2)) return 0;
	BNP_HDR2 hdr2 = *((const BNP_HDR2*)ptr);
	ptr += sizeof(BNP_HDR2);
	size -= sizeof(BNP_HDR2);

	if (hdr2.m_hasTrnid) {
		if (size < sizeof(WORD)) return 0;
		ptr += sizeof(WORD);
		size -= sizeof(WORD);
	}

	DWORD compr, uncompr;
	switch (hdr2.m_sizedim)
	{
	case 0:
		compr = 0;
		uncompr = 0;
		break;
	case 1:
		if (size < sizeof(BYTE)) return 0;
		uncompr = *((const BYTE*)ptr);
		ptr += sizeof(BYTE);
		size -= sizeof(BYTE);
		if (hdr2.m_isCompressed) {
			if (size < sizeof(BYTE)) return 0;
			compr = *((const BYTE*)ptr);
			ptr += sizeof(BYTE);
			size -= sizeof(BYTE);
			ptr += compr;
			size -= compr;
		}
		else {
			compr = 0;
			ptr += uncompr;
			size -= uncompr;
		}
		break;
	case 2:
		if (size < sizeof(WORD)) return 0;
		uncompr = *((const WORD*)ptr);
		ptr += sizeof(WORD);
		size -= sizeof(WORD);
		if (hdr2.m_isCompressed) {
			if (size < sizeof(WORD)) return 0;
			compr = *((const WORD*)ptr);
			ptr += sizeof(WORD);
			size -= sizeof(WORD);
			ptr += compr;
			size -= compr;
		}
		else {
			compr = 0;
			ptr += uncompr;
			size -= uncompr;
		}
		break;
	case 3:
	default:
		if (size < sizeof(DWORD)) return 0;
		uncompr = *((const DWORD*)ptr);
		ptr += sizeof(DWORD);
		size -= sizeof(DWORD);
		if (hdr2.m_isCompressed) {
			if (size < sizeof(DWORD)) return 0;
			compr = *((const DWORD*)ptr);
			ptr += sizeof(DWORD);
			size -= sizeof(DWORD);
			ptr += compr;
			size -= compr;
		}
		else {
			compr = 0;
			ptr += uncompr;
			size -= uncompr;
		}
		break;
	}

	// Checkup readed uncompressed data size
	_ASSERT(compr <= uncompr + 12 + (uncompr + 500) / 1000);

	return (int)size >= 0 ? ptr - ptr0 : 0;
}

Header::Header(WORD msg, WORD ti) throw()
{
	init(msg, ti);
}

void Header::init(WORD msg, WORD ti)
{
	m_crc = 0xFFFF;
	m_message = msg;
	m_trnid = ti;
	m_sizeuncompr = 0;
	m_sizecompr = 0;

	if (!m_trnid && (NATIVEACTION(m_message) == BNPM_QUEST || NATIVEACTION(m_message) == BNPM_REPLY))
		m_trnid = -1;
}

int Header::read(const char*& ptr) throw()
{
	const char* ptr0 = ptr;

	m_crc = *((const crc16_t*)ptr);
	ptr += sizeof(crc16_t);

	BNP_HDR2 hdr2 = *((const BNP_HDR2*)ptr);
	ptr += sizeof(BNP_HDR2);

	m_message = BUILTMESSAGE(hdr2);

	if (hdr2.m_hasTrnid) {
		m_trnid = *((const WORD*)ptr);
		ptr += sizeof(WORD);

		if (!m_trnid) m_trnid = -1;
	}
	else {
		m_trnid = 0;
	}

	switch (hdr2.m_sizedim)
	{
	case 0:
		m_sizeuncompr = 0;
		m_sizecompr = 0;
		break;
	case 1:
		m_sizeuncompr = *((const BYTE*)ptr);
		ptr += sizeof(BYTE);
		if (hdr2.m_isCompressed) {
			m_sizecompr = *((const BYTE*)ptr);
			ptr += sizeof(BYTE);
		}
		else {
			m_sizecompr = 0;
		}
		break;
	case 2:
		m_sizeuncompr = *((const WORD*)ptr);
		ptr += sizeof(WORD);
		if (hdr2.m_isCompressed) {
			m_sizecompr = *((const WORD*)ptr);
			ptr += sizeof(WORD);
		}
		else {
			m_sizecompr = 0;
		}
		break;
	case 3:
	default:
		m_sizeuncompr = *((const DWORD*)ptr);
		ptr += sizeof(DWORD);
		if (hdr2.m_isCompressed) {
			m_sizecompr = *((const DWORD*)ptr);
			ptr += sizeof(DWORD);
		}
		else {
			m_sizecompr = 0;
		}
		break;
	}

	// Checkup readed uncompressed data size
	_ASSERT(m_sizecompr <= m_sizeuncompr + 12 + (m_sizeuncompr + 500) / 1000);

	return ptr - ptr0;
}

size_t Header::write(char*& ptr) const throw()
{
	const char* ptr0 = ptr;

	*((crc16_t*)ptr) = m_crc;
	ptr += sizeof(crc16_t);

	unsigned idxs = getDimensionIndex(getSizeDimension(max(m_sizeuncompr, m_sizecompr)));
	_ASSERT(idxs < 4);

	BNP_HDR2 hdr2;
	hdr2.m_sizedim = idxs;
	hdr2.m_hasTrnid = m_trnid != 0;
	hdr2.m_isQuest = (m_message & BNPM_QUEST) != 0;
	hdr2.m_isCompressed = m_sizecompr != 0;
	hdr2.m_isReserved = 0;
	hdr2.m_message = m_message;
	*((BNP_HDR2*)ptr) = hdr2;
	ptr += sizeof(BNP_HDR2);

	if (hdr2.m_hasTrnid) {
		*((WORD*)ptr) = m_trnid;
		ptr += sizeof(WORD);
	}

	switch (idxs)
	{
	case 0:
		break;
	case 1:
		*((BYTE*)ptr) = (BYTE)m_sizeuncompr;
		ptr += sizeof(BYTE);
		if (hdr2.m_isCompressed) {
			*((BYTE*)ptr) = (BYTE)m_sizecompr;
			ptr += sizeof(BYTE);
		}
		break;
	case 2:
		*((WORD*)ptr) = (WORD)m_sizeuncompr;
		ptr += sizeof(WORD);
		if (hdr2.m_isCompressed) {
			*((WORD*)ptr) = (WORD)m_sizecompr;
			ptr += sizeof(WORD);
		}
		break;
	case 3:
	default:
		*((DWORD*)ptr) = (DWORD)m_sizeuncompr;
		ptr += sizeof(DWORD);
		if (hdr2.m_isCompressed) {
			*((DWORD*)ptr) = (DWORD)m_sizecompr;
			ptr += sizeof(DWORD);
		}
		break;
	}

	return ptr - ptr0;
}

size_t Header::getSize() const throw()
{
	unsigned szd = getSizeDimension(max(m_sizeuncompr, m_sizecompr));
	return sizeof(crc16_t) + sizeof(BNP_HDR2) + (m_trnid ? sizeof(WORD) : 0) + szd + (m_sizecompr ? szd : 0);
}

crc16_t Header::realCRC() const throw()
{
	char buffer[16];
	char* ptr = buffer;
	size_t hdrsz = write(ptr);
	return CRC16CCITTtbl(buffer + sizeof(crc16_t), hdrsz - sizeof(crc16_t), 0xFFFF);
}

WORD __fastcall Header::getnativeAction() const
{
	return NATIVEACTION(m_message);
}

void __fastcall Header::setnativeAction(WORD val)
{
	m_message &= BNPM_MESSAGE;
	m_message |= NATIVEACTION(val);

	if (!m_trnid && (NATIVEACTION(m_message) == BNPM_QUEST || NATIVEACTION(m_message) == BNPM_REPLY))
		m_trnid = -1;
}

WORD __fastcall Header::getnativeMessage() const
{
	return NATIVEMESSAGE(m_message);
}

void __fastcall Header::setnativeMessage(WORD val)
{
	m_message &= BNPM_ACTION;
	m_message |= NATIVEMESSAGE(val);
}

//-----------------------------------------------------------------------------

//
// JBTransaction
//

JBTransaction::JBTransaction(WORD message, WORD ti) throw() :
	Header(message, ti)
{
}

size_t JBTransaction::getSize() const throw()
{
	return Header::getSize() + m_data.size();
}

void JBTransaction::prepare() throw()
{
	compress(JBNB::s_nCompression);
	if (!m_trnid && getnativeAction() == BNPM_QUEST) {
		m_trnid = JLink::getNextQuestId(); // generate transaction identifier
	}
	updateCRC();
}

bool JBTransaction::isPrimary() throw()
{
	return nativeMessage == BNPM_IDENTIFY;
}

// stream IO
int JBTransaction::read(const char*& ptr)
{
	int epos = Header::read(ptr);
	int dsize = m_sizecompr ? m_sizecompr : m_sizeuncompr;
	m_data = std::string(ptr, dsize);
	ptr += dsize;
	epos += dsize;
	return epos;
}

void JBTransaction::serialize(JPtr<JLink> link)
{
	size_t trnsz0, trnsz1, trnsz2;
	char hdrbuf[16], *ptr;
	size_t pos0 = link->aBufferSend.size();

	trnsz0 = sizeof(crc16_t) + sizeof(BNP_HDR2);
	// push header
	ptr = hdrbuf;
	trnsz1 = Header::write(ptr);
	link->aBufferSend.append(hdrbuf, trnsz1);
	// push data
	trnsz2 = trnsz1 + m_data.size();
	link->aBufferSend.append(m_data);

	if (!isPrimary()) {
		JPtr<JBLink> blink = jdynamic_cast<JBLink, JLink>(link);
		_ASSERT(blink);
		_ASSERT(blink->ctxEncryptor);
		Skein_256_Ctxt_t ctx;
		blink->m_uEncryptCount++;
		Skein_256_InitExt(&ctx, 256, blink->m_uEncryptCount, *blink->m_key, blink->m_keysize);
		Skein_256_Final_Pad(&ctx, blink->m_nonceEncrypt.p32.a);
		blink->ctxEncryptor->ivsetup(blink->m_nonceEncrypt.p32.a);

		ptr = (char*)link->aBufferSend.data() + pos0;
		blink->ctxEncryptor->encrypt_bytes((const u8*)(ptr + 0), (u8*)(ptr + 0), (u32)(trnsz0 - 0));
		blink->ctxEncryptor->encrypt_bytes((const u8*)(ptr + trnsz0), (u8*)(ptr + trnsz0), (u32)(trnsz1 - trnsz0));
		blink->ctxEncryptor->encrypt_bytes((const u8*)(ptr + trnsz1), (u8*)(ptr + trnsz1), (u32)(trnsz2 - trnsz1));
	}
}

bool JBTransaction::unserialize(JPtr<JLink> link)
{
	const char* ptr0 = (char*)link->aBufferRecv.c_str();
	const char* ptr = ptr0;
	size_t bufsize = link->aBufferRecv.size();

	size_t trnsz0, trnsz1, trnsz2;
	char hdrbuf[16];

	JPtr<JBLink> blink = jdynamic_cast<JBLink, JLink>(link);
	_ASSERT(blink);

	// Translate CRC & BNP_HDR2
	trnsz0 = sizeof(crc16_t) + sizeof(BNP_HDR2);
	if (trnsz0 > bufsize - (ptr - ptr0)) return false;
	if (!blink->bHalfTrnRecv && blink->ctxDecryptor) { // setup IV-key once for each transaction
		Skein_256_Ctxt_t ctx;
		blink->m_uDecryptCount++;
		Skein_256_InitExt(&ctx, 256, blink->m_uDecryptCount, *blink->m_key, blink->m_keysize);
		Skein_256_Final_Pad(&ctx, blink->m_nonceDecrypt.p32.a);

		blink->bHalfTrnRecv = true;
	}
	if (blink->ctxDecryptor) { // setup IV to begining of transaction
		blink->ctxDecryptor->ivsetup(blink->m_nonceDecrypt.p32.a);
	}
	memcpy(hdrbuf, ptr, trnsz0);
	if (blink->ctxDecryptor) {
		blink->ctxDecryptor->decrypt_bytes((const u8*)hdrbuf, (u8*)hdrbuf, (u32)(trnsz0 - 0));
	}
	// Translate full header
	trnsz1 = Header::getHeaderSize(*((const BNP_HDR2*)(hdrbuf + sizeof(crc16_t))));
	if (trnsz1 > bufsize - (ptr - ptr0)) return false;
	memcpy(hdrbuf + trnsz0, ptr + trnsz0, trnsz1 - trnsz0);
	if (blink->ctxDecryptor) {
		blink->ctxDecryptor->decrypt_bytes((const u8*)hdrbuf + trnsz0, (u8*)hdrbuf + trnsz0, (u32)(trnsz1 - trnsz0));
	}
	// Translate full transaction
	trnsz2 = Header::isComplete(hdrbuf, bufsize - (ptr - ptr0));
	if (!trnsz2) return false;
	memcpy((char*)ptr, hdrbuf, trnsz1); // use already decrypted header
	if (blink->ctxDecryptor) {
		blink->ctxDecryptor->decrypt_bytes((const u8*)ptr + trnsz1, (u8*)ptr + trnsz1, (u32)(trnsz2 - trnsz1));
	}

	read(ptr);

	blink->bHalfTrnRecv = false;

	// Delete traslated data from buffer
	link->aBufferRecv.erase(0, ptr - ptr0);
	return true;
}

void JBTransaction::setdataUncompr(const std::string& str) throw()
{
	m_data = str;
	m_sizeuncompr = (DWORD)m_data.size();
	m_sizecompr = 0;
}

void JBTransaction::setdataCompr(const std::string& str, DWORD ucsz) throw()
{
	m_data = str;
	m_sizeuncompr = ucsz;
	m_sizecompr = (DWORD)m_data.size();
	// Checkup for correct data
	_ASSERT(m_sizecompr <= (m_sizeuncompr + 12 + (m_sizeuncompr + 500) / 1000));
}

bool JBTransaction::compress(int level) throw()
{
	if (m_sizecompr) return true;
	if (!level || m_data.size() < 12 + 8) return false;

	// Make compression
#ifndef BNP_COMPRESS_ONCE
	std::string& uncompr = m_data;
	std::string compr;
	uLong uncomprLen = (uLong)m_data.size();
	uLong comprLen = 0;

	z_stream stream;
	int err;

	stream.zalloc = (alloc_func)Z_NULL;
	stream.zfree = (free_func)Z_NULL;
	stream.opaque = (voidpf)Z_NULL;

	stream.next_in = (Bytef*)uncompr.data();
	stream.avail_in = uncomprLen;
	stream.next_out = (Bytef*)compr.data();
	stream.avail_out = comprLen;

	err = deflateInit(&stream, level);
	if (err != Z_OK) return false;
	while (err == Z_OK && stream.total_in < uncomprLen) {
		if (!stream.avail_out) {
			uLong offset = comprLen - stream.avail_out;
			comprLen = (comprLen / BNP_COMPRESS_STEP + 1)*BNP_COMPRESS_STEP;
			compr.resize(comprLen);
			stream.next_out = (Bytef*)(compr.data() + offset);
			stream.avail_out = comprLen - offset;
		}
		err = deflate(&stream, Z_NO_FLUSH);
	}

	err = deflate(&stream, Z_FINISH);
#ifdef _DEBUG
	int finishiter = 0;
#endif
	while (err != Z_STREAM_END) {
		uLong offset = comprLen - stream.avail_out;
		comprLen = (comprLen / BNP_COMPRESS_STEP + 1)*BNP_COMPRESS_STEP;
		compr.resize(comprLen);
		stream.next_out = (Bytef*)(compr.data() + offset);
		stream.avail_out = comprLen - offset;

		err = deflate(&stream, Z_FINISH);
#ifdef _DEBUG
		finishiter++;
		_ASSERT(finishiter < 10); // ensure that not cycled
#endif
	}

	comprLen = stream.total_out; // set to real size
	err = deflateEnd(&stream);
	_ASSERT(err == Z_OK);
#else
	std::string& uncompr = m_data;
	std::string compr;
	uLong uncomprLen = (uLong)m_data.length();
	uLong comprLen = uncomprLen + 12 + (uncomprLen + 500) / 1000;

	compr.resize(comprLen); // resize to possible size
	int err = compress2((Bytef*)compr.data(), &comprLen, (const Bytef*)uncompr.data(), uncomprLen, level);
#endif // BNP_COMPRESS_ONCE
	if (err == Z_OK && comprLen < uncomprLen) {
		compr.resize(comprLen); // resize to actual data size
		setdataCompr(compr, uncomprLen);
		return true;
	}
	else {
		return false;
	}
}

bool JBTransaction::uncompress() throw()
{
	if (!m_sizecompr) return true;

	// Make uncompression
#ifndef BNP_UNCOMPRESS_ONCE
	std::string& compr = m_data;
	std::string uncompr;
	uLong comprLen = (uLong)m_data.size();
	uLong uncomprLen = 0;

	z_stream stream;
	int err;

	stream.zalloc = (alloc_func)Z_NULL;
	stream.zfree = (free_func)Z_NULL;
	stream.opaque = (voidpf)Z_NULL;

	stream.next_in = (Bytef*)compr.data();
	stream.avail_in = comprLen;
	stream.next_out = (Bytef*)uncompr.data();
	stream.avail_out = uncomprLen;

	err = inflateInit(&stream);
	if (err != Z_OK) return false;
	while (err != Z_STREAM_END && stream.total_in < comprLen) {
		if (err != Z_OK) break;
		if (!stream.avail_out) {
			uLong offset = uncomprLen - stream.avail_out;
			uncomprLen = (uncomprLen / BNP_COMPRESS_STEP + 1)*BNP_COMPRESS_STEP;
			uncompr.resize(uncomprLen);
			stream.next_out = (Bytef*)(uncompr.data() + offset);
			stream.avail_out = uncomprLen - offset;
		}
		else {
			// No way to here at normal decompression!!!
			err = inflate(&stream, Z_FINISH); // last try
			_ASSERT(err == Z_STREAM_END);
			break;
		}
		err = inflate(&stream, Z_SYNC_FLUSH);
	}
	if (err != Z_STREAM_END) { // uncompression was failed
		inflateEnd(&stream);
		return false;
	}

	uncomprLen = stream.total_out; // set to real size
	err = inflateEnd(&stream);
	_ASSERT(err == Z_OK);

#else
	std::string& compr = m_data;
	std::string uncompr;
	uLong comprLen = (uLong)m_data.size();
	uLong uncomprLen = m_sizeuncompr;

	uncompr.resize(uncomprLen); // resize to possible size
	int err = ::uncompress((Bytef*)uncompr.data(), &uncomprLen, (const Bytef*)compr.data(), comprLen);
	if (err != Z_OK) return false;
#endif // BNP_UNCOMPRESS_ONCE
	uncompr.resize(uncomprLen);
	setdataUncompr(uncompr);
	return true;
}

void JBTransaction::encrypt(ecrypt::JCipher* ctx, const u8* iv) throw()
{
	_ASSERT(ctx);
	std::string ciphertext(m_data.size(), 0);
	ctx->ivsetup(iv);
	ctx->encrypt_bytes((const u8*)m_data.data(), (u8*)ciphertext.data(), (u32)m_data.size());
	m_data = ciphertext;
}

void JBTransaction::decrypt(ecrypt::JCipher* ctx, const u8* iv) throw()
{
	_ASSERT(ctx);
	std::string plaintext(m_data.size(), 0);
	ctx->ivsetup(iv);
	ctx->decrypt_bytes((const u8*)m_data.data(), (u8*)plaintext.data(), (u32)m_data.size());
	m_data = plaintext;
}

void JBTransaction::restore(const std::string& content, bool enctypted)
{
	m_data = content;
}

crc16_t JBTransaction::realCRC() const throw()
{
	return CRC16CCITTtbl(m_data.data(), m_data.size(), Header::realCRC());
}

void JBTransaction::updateCRC() throw()
{
	m_crc = realCRC();
}

//-----------------------------------------------------------------------------

//
// class JBLink
//

JBLink::JBLink()
	: JLink()
{
	Clear();
}

JBLink::JBLink(SOCKET sock, const sockaddr_in& addr)
	: JLink(sock, addr)
{
	Clear();
}

const char* JBLink::getEncryptorName() const
{
	_ASSERT(ctxEncryptor);
	return ctxEncryptor->name();
}

void JBLink::setEncryptor(const char* ciphername)
{
	ctxEncryptor = JBNB::CipherFactory(ciphername);
}

const char* JBLink::getDecryptorName() const
{
	_ASSERT(ctxDecryptor);
	return ctxDecryptor->name();
}

void JBLink::setDecryptor(const char* ciphername)
{
	ctxDecryptor = JBNB::CipherFactory(ciphername);
}

void JBLink::keysetup(huge::number* key)
{
	memset(*m_key, 0, m_keysize);
	memcpy(*m_key, key->m_digit, min(m_keysize, key->m_size * sizeof(huge::digit)));

	// key size for encrypting is less then reserved for use
	_ASSERT(ctxDecryptor && ctxEncryptor);
	ctxDecryptor->keysetup(*m_key, ctxDecryptor->maxkeysize(), ctxDecryptor->maxivsize());
	ctxEncryptor->keysetup(*m_key, ctxEncryptor->maxkeysize(), ctxEncryptor->maxivsize());
	// inits "Skein" pseudorandom number generator
	m_uDecryptCount = m_uEncryptCount = 0;
}

void JBLink::Clear()
{
	m_keysize = 128;
	m_key = new JArrayRef<u08b_t>(m_keysize);
	memset(*m_key, 0, m_keysize);
	ctxDecryptor = ctxEncryptor = 0;
	m_uDecryptCount = m_uEncryptCount = 0;
	memset(m_nonceDecrypt.p32.a, 0, sizeof(m_nonceDecrypt));
	memset(m_nonceEncrypt.p32.a, 0, sizeof(m_nonceEncrypt));
}

//-----------------------------------------------------------------------------

//
// class JBNB
//

CHAR JBNB::szSignature[16] = BNP_SIGNATURE;
int JBNB::s_nCompression = Z_DEFAULT_COMPRESSION;

JBNB::JBNB()
	: JEngine()
{
}

void JBNB::Init()
{
	__super::Init();

	// Load previous state
	LoadState();
	InitLogs();
}

void JBNB::Done()
{
	// Save current state
	SaveState();

	__super::Done();
}

//-----------------------------------------------------------------------------

// --- Sending ---

bool JBNB::MakeAndPushTrn(SOCKET sock, WORD message, WORD trnid, const std::string& str, size_t ssi) throw()
{
	return PushTrn(sock, MakeTrn(message, trnid, str), ssi);
}

JPtr<JBTransaction> JBNB::MakeTrn(WORD message, WORD trnid, const std::string& str) const throw()
{
	JPtr<JBTransaction> jpTrn = jdynamic_cast<JBTransaction, JTransaction>(createTrn());
	_ASSERT(jpTrn != 0);

	jpTrn->init(message, trnid);
	jpTrn->setdataUncompr(str);
	jpTrn->prepare();
	return jpTrn;
}

JPtr<JBTransaction> JBNB::MakeTrn(WORD message, WORD trnid, LONG_PTR val) const throw()
{
	std::ostringstream os;
	io::pack(os, val);
	return MakeTrn(message, trnid, os.str());
}

JPtr<JBTransaction> JBNB::MakeTrn(WORD message, WORD trnid, const void* ptr, int size) const throw()
{
	std::ostringstream os;
	os.write((const char*)ptr, size);
	return MakeTrn(message, trnid, os.str());
}

JPtr<JLink> JBNB::createLink() const
{
	DoCS cs(&m_csLinks);
	return new JBLink();
}

JPtr<JLink> JBNB::createLink(SOCKET sock, const sockaddr_in& addr) const
{
	DoCS cs(&m_csLinks);
	return new JBLink(sock, addr);
}

//-----------------------------------------------------------------------------

// --- Transactions work ---

bool JBNB::DispatchTrn(SOCKET sock, JTransaction* jpTrn)
{
	JPtr<JLink> link = JLink::get(sock);
	JBTransaction* jpBTrn = dynamic_cast<JBTransaction*>(jpTrn);

	if (!jpBTrn) {
	}
	else if (jpBTrn->realCRC() != jpBTrn->crc) {
		EvTrnBadCRC(sock);
	}
	else if (!(link->bAccessAllowed || jpTrn->isPrimary())) {
		EvTrnIgnore(sock, jpBTrn->message, jpBTrn->trnid);
		JEngine::Stat.dlRecvTrnIgnored++; // update statistics
	}
	else if (!jpBTrn->uncompress() && jpBTrn->sizecompr > 0) {
		EvTrnIgnore(sock, jpBTrn->message, jpBTrn->trnid);
		JEngine::Stat.dlRecvTrnIgnored++; // update statistics
	}
	else {
		// Process expected transactions
		ParseTransaction(sock, jpBTrn->message, jpBTrn->trnid, io::mem(jpBTrn->data.data(), jpBTrn->data.size()));
		JEngine::Stat.dlRecvTrnProcessed++; // update statistics
		return true;
	}
	return false;
}

void JBNB::ParseTransaction(SOCKET sock, WORD message, WORD trnid, io::mem is)
{
	switch (NATIVEACTION(message))
	{
	case BNPM_COMMAND:
	{
		MapTrnCommand::const_iterator iter = m_mTrnCommand.find(NATIVEMESSAGE(message));
		if (iter != m_mTrnCommand.end()) {
			iter->second(sock, is);
		}
		break;
	}

	case BNPM_QUEST:
	{
		MapTrnQuest::const_iterator iter = m_mTrnQuest.find(NATIVEMESSAGE(message));
		if (iter != m_mTrnQuest.end()) {
			std::ostringstream os;
			iter->second(sock, trnid, is, os);
			PushTrn(sock, MakeTrn(REPLY(NATIVEMESSAGE(message)), trnid, os.str()));
			if (message == QUEST(BNPM_IDENTIFY) && *(const LRESULT*)os.str().data() == 0) {
				EvLinkStart(sock);
			}
		}
		break;
	}

	case BNPM_REPLY:
	{
		MapTrnReply::const_iterator iter = m_mTrnReply.find(NATIVEMESSAGE(message));
		if (iter != m_mTrnReply.end()) {
			iter->second(sock, trnid, is);
		}
		break;
	}

	case BNPM_NOTIFY:
	{
		MapTrnNotify::const_iterator iter = m_mTrnNotify.find(NATIVEMESSAGE(message));
		if (iter != m_mTrnNotify.end()) {
			iter->second(sock, is);
		}
		break;
	}
	}
}

//-----------------------------------------------------------------------------

// --- Beowolf Network Protocol Messages reciving ---

void JBNB::Recv_Cmd_NULL(SOCKET sock, io::mem& is)
{
	// Report about message
	EvLog("Null-message", elogItrn);
}

void JBNB::Recv_Quest_NULL(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
	// Report about message
	EvLog("Null-message", elogItrn);
}

void JBNB::Recv_Reply_NULL(SOCKET sock, WORD trnid, io::mem& is)
{
	// Report about message
	EvLog("Null-message", elogItrn);
}

void JBNB::Recv_Notify_NULL(SOCKET sock, io::mem& is)
{
	// Report about message
	EvLog("Null-message", elogItrn);
}

void JBNB::Recv_Quest_Ping(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
	// Report about message
	EvLog("Ping", elogItrn);
}

void JBNB::Recv_Reply_Ping(SOCKET sock, WORD trnid, io::mem& is)
{
	// Report about message
	EvLog("Pong", elogItrn);
}

void JBNB::Recv_Quest_Identify(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
	using namespace huge;
	JPtr<JBLink> blink = JBLink::get(sock);
	_ASSERT(blink);

	LRESULT result;
	char signature[16];
	digsize bsz;
	char* ciphername;
	try
	{
		io::unpack(is, signature);
		io::unpack(is, blink->m_dwProtocolVers);
		io::unpack(is, bsz);
		numptr B(bsz);
		io::unpackmem(is, B->m_digit, bsz * sizeof(digit));
		io::unpackstr(is, ciphername);

		if (lstrcmpA(szSignature, signature))
		{
			// Bad signature
			result = 4;
		}
		else if (blink->m_dwProtocolVers < getMinVersion())
		{
			// Version not supported
			result = 5;
		}
		else if (blink->m_dwProtocolVers > getCurVersion())
		{
			// Version not implimented
			result = 6;
		}
		else if (!isSupportedCipher(ciphername))
		{
			result = 7;
		}
		else
		{
			// Success access
			numptr a(256 / 8 / sizeof(digit));
			Skein_256_Ctxt_t ctx;
			Skein_256_Init(&ctx, 256);
			Skein_256_Update(&ctx, (const u08b_t*)m_passwordNet.data(), sizeof(TCHAR)*m_passwordNet.length());
			Skein_256_Final(&ctx, (u08b_t*)a->m_digit);
			numptr K(number::powmod(B, a, mod));

			blink->setDecryptor(ciphername);
			blink->keysetup(K);
			EvLinkAccess(sock, true);
			result = 0;
		}
	}
	catch (io::exception e)
	{
		result = e.count + 1;
	}

	// Report about message
	int i;
	struct
	{
		LRESULT code;
		const char* msg;
	} reply[] =
	{
		0, "password accepted",
		1, "Failed reading signature",
		2, "Failed reading version",
		3, "Failed reading password",
		4, "Bad signature",
		5, "Version not supported",
		6, "Version not implimented",
		7, "Cipher algorithm is not supported",
		-1, "Unknown reason"
	};
	for (i = 0; i < _countof(reply) - 1 && result != reply[i].code; i++);
	EvLog(reply[i].msg, elogWarn);

	// Reply
	Form_Reply_Identify(os, result, blink->getEncryptorName());
}

void JBNB::Recv_Reply_Identify(SOCKET sock, WORD trnid, io::mem& is)
{
	using namespace huge;

	LRESULT result;
	digsize bsz;
	char* ciphername;
	try
	{
		io::unpack(is, result);
		io::unpack(is, bsz);
		numptr B(bsz);
		io::unpackmem(is, B->m_digit, bsz * sizeof(digit));
		io::unpackstr(is, ciphername);

		if (!result)
		{
			numptr a(256 / 8 / sizeof(digit));
			Skein_256_Ctxt_t ctx;
			Skein_256_Init(&ctx, 256);
			Skein_256_Update(&ctx, (const u08b_t*)m_passwordNet.data(), sizeof(TCHAR)*m_passwordNet.length());
			Skein_256_Final(&ctx, (u08b_t*)a->m_digit);
			numptr K(number::powmod(B, a, mod));

			JPtr<JBLink> blink = JBLink::get(sock);
			_ASSERT(blink);
			blink->setDecryptor(ciphername);
			blink->keysetup(K);
			EvLinkIdentify(sock); // user can check access outside of engine
		}
	}
	catch (io::exception e)
	{
		result = -1;
	}

	// Report about message
	int i;
	struct
	{
		LRESULT code;
		const char* msg;
	} reply[] =
	{
		0, "password accepted",
		1, "Failed reading signature",
		2, "Failed reading version",
		3, "Failed reading password",
		4, "Bad signature",
		5, "Version not supported",
		6, "Version not implimented",
		7, "Cipher algorithm is not supported",
		-1, "Unknown reason"
	};
	for (i = 0; i < _countof(reply) - 1 && result != reply[i].code; i++);
	EvLog(reply[i].msg, elogWarn);

	if (!result) EvLinkStart(sock);
}

void JBNB::Recv_Cmd_Disconnect(SOCKET sock, io::mem& is)
{
	EvLinkClose(sock, 0);
	// Report about message
	EvLog("Disconnected by command", elogItrn);
}

//-----------------------------------------------------------------------------

// --- Beowolf Network Protocol Messages sending ---

JPtr<JBTransaction> JBNB::Make_Quest_Ping() const
{
	std::ostringstream os;
	return MakeTrn(QUEST(BNPM_PING), 0, os.str());
}

JPtr<JBTransaction> JBNB::Make_Quest_Identify(const char* ciphername) const
{
	using namespace huge;

	numptr a(256 / 8 / sizeof(digit));
	Skein_256_Ctxt_t ctx;
	Skein_256_Init(&ctx, 256);
	Skein_256_Update(&ctx, (const u08b_t*)m_passwordNet.data(), sizeof(TCHAR)*m_passwordNet.length());
	Skein_256_Final(&ctx, (u08b_t*)a->m_digit);
	numptr A(number::powmod(gen, a, mod));

	std::ostringstream os;
	os.write(szSignature, sizeof(szSignature));
	io::pack(os, getCurVersion());
	io::pack(os, A->m_size);
	os.write((const char*)A->m_digit, A->m_size * sizeof(digit));
	io::pack(os, ciphername);
	return MakeTrn(QUEST(BNPM_IDENTIFY), 0, os.str());
}

void JBNB::Form_Reply_Identify(std::ostream& os, LRESULT result, const char* ciphername) const
{
	using namespace huge;

	numptr a(256 / 8 / sizeof(digit));
	Skein_256_Ctxt_t ctx;
	Skein_256_Init(&ctx, 256);
	Skein_256_Update(&ctx, (const u08b_t*)m_passwordNet.data(), sizeof(TCHAR)*m_passwordNet.length());
	Skein_256_Final(&ctx, (u08b_t*)a->m_digit);
	numptr A(number::powmod(gen, a, mod));

	io::pack(os, result);
	io::pack(os, A->m_size);
	os.write((const char*)A->m_digit, A->m_size * sizeof(digit));
	io::pack(os, ciphername);
}

JPtr<JBTransaction> JBNB::Make_Cmd_Disconnect() const
{
	std::ostringstream os;
	return MakeTrn(COMMAND(BNPM_DISCONNECT), 0, os.str());
}

//-----------------------------------------------------------------------------

// --- Events response ---

void JBNB::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JBNB, node, src);
	if (node) {
		node->EvTrnProcess += MakeDelegate(this, &JBNB::OnTrnProcess);
		node->EvTrnIgnore += MakeDelegate(this, &JBNB::OnTrnIgnore);
		node->EvTrnBadCRC += MakeDelegate(this, &JBNB::OnTrnBadCRC);
	}
}

void JBNB::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JBNB, node, src);
	if (node) {
		node->EvTrnProcess -= MakeDelegate(this, &JBNB::OnTrnProcess);
		node->EvTrnIgnore -= MakeDelegate(this, &JBNB::OnTrnIgnore);
		node->EvTrnBadCRC -= MakeDelegate(this, &JBNB::OnTrnBadCRC);
	}

	__super::OnUnhook(src);
}

//-----------------------------------------------------------------------------

// --- Register/unregister transactions parsers ---

void JBNB::RegHandlers(JNode* src)
{
	__super::RegHandlers(src);

	JNODE(JBNB, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnCommand[BNPM_NULL] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Cmd_NULL);
		node->m_mTrnQuest[BNPM_NULL] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Quest_NULL);
		node->m_mTrnReply[BNPM_NULL] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Reply_NULL);
		node->m_mTrnNotify[BNPM_NULL] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Notify_NULL);
		node->m_mTrnQuest[BNPM_PING] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Quest_Ping);
		node->m_mTrnReply[BNPM_PING] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Reply_Ping);
		node->m_mTrnQuest[BNPM_IDENTIFY] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Quest_Identify);
		node->m_mTrnReply[BNPM_IDENTIFY] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Reply_Identify);
		node->m_mTrnCommand[BNPM_DISCONNECT] += fastdelegate::MakeDelegate(this, &JBNB::Recv_Cmd_Disconnect);
	}
}

void JBNB::UnregHandlers(JNode* src)
{
	JNODE(JBNB, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnCommand[BNPM_NULL] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Cmd_NULL);
		node->m_mTrnQuest[BNPM_NULL] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Quest_NULL);
		node->m_mTrnReply[BNPM_NULL] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Reply_NULL);
		node->m_mTrnNotify[BNPM_NULL] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Notify_NULL);
		node->m_mTrnQuest[BNPM_PING] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Quest_Ping);
		node->m_mTrnReply[BNPM_PING] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Reply_Ping);
		node->m_mTrnQuest[BNPM_IDENTIFY] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Quest_Identify);
		node->m_mTrnReply[BNPM_IDENTIFY] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Reply_Identify);
		node->m_mTrnCommand[BNPM_DISCONNECT] -= fastdelegate::MakeDelegate(this, &JBNB::Recv_Cmd_Disconnect);
	}

	__super::UnregHandlers(src);
}

void JBNB::OnLinkEstablished(SOCKET sock)
{
	JPtr<JBLink> blink = JBLink::get(sock);
	_ASSERT(blink);
	blink->setEncryptor(getEncryptorName());
}

void JBNB::OnTrnIgnore(SOCKET sock, WORD message, WORD trnid)
{
	EvLog(format("message %i ignored", NATIVEMESSAGE(message)), elogIgnor);
}

void JBNB::OnTrnBadCRC(SOCKET sock)
{
	_ASSERT(sock);
	EvLinkClose(sock, WSABADCRC);
}

//-----------------------------------------------------------------------------

// The End.