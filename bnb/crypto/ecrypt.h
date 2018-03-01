/******************************************************************************
*                                                                             *
* ecrypt.h -- cryptographic algorithms and cipher classes                     *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2010. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _ECRYPT_
#define _ECRYPT_

// Common
#include "JObjects.h"
#include "ecrypt-portable.h"

namespace ecrypt {
	union Nonce128 {
		struct {
			unsigned __int64 lo, hi;
		} two;
		struct {
			unsigned __int64 a[2];
		} p2;
		struct {
			unsigned __int32 a[4];
		} p4;
		struct {
			unsigned __int16 a[8];
		} p8;
		struct {
			unsigned __int8  a[16];
		} p16;
	};

	union Nonce256 {
		struct {
			Nonce128 lo, hi;
		} two;
		struct {
			unsigned __int64 a, b, c, d;
		} four;
		struct {
			unsigned __int64 a[4];
		} p4;
		struct {
			unsigned __int32 a[8];
		} p8;
		struct {
			unsigned __int16 a[16];
		} p16;
		struct {
			unsigned __int8  a[32];
		} p32;
	};

	class JCipher : public JClass {
	public:
		virtual const char* name() const = 0;
		virtual u32  maxkeysize() const = 0;
		virtual u32  maxivsize() const = 0;
		virtual u32  block_length() const = 0;

		virtual void keysetup(const u8* key, u32 keysize, u32 ivsize) = 0;
		virtual void ivsetup(const u8* iv) = 0;
		virtual void process_bytes(int action, const u8* input, u8* output, u32 msglen) = 0;
		virtual void encrypt_bytes(const u8* plaintext, u8* ciphertext, u32 msglen) = 0;
		virtual void decrypt_bytes(const u8* ciphertext, u8* plaintext, u32 msglen) = 0;
		virtual void keystream_bytes(u8* keystream, u32 length) = 0;
		virtual void process_packet(int action, const u8* iv, const u8* input, u8* output, u32 msglen) = 0;
		virtual void encrypt_packet(const u8* iv, const u8* plaintext, u8* ciphertext, u32 msglen) = 0;
		virtual void decrypt_packet(const u8* iv, const u8* ciphertext, u8* plaintext, u32 msglen) = 0;
		virtual void process_blocks(int action, const u8* input, u8* output, u32 msglen) = 0;
		virtual void encrypt_blocks(const u8* plaintext, u8* ciphertext, u32 msglen) = 0;
		virtual void decrypt_blocks(const u8* ciphertext, u8* plaintext, u32 msglen) = 0;
		virtual void keystream_blocks(u8* keystream, u32 blocks) = 0;
	}; // JCipher

#define ECRYPTCLASS(ns, JC) \
	class JC : public JCipher { \
	public: \
		const char* name() const {return ns::ECRYPT_NAME;} \
		u32  maxkeysize() const {return ns::ECRYPT_MAXKEYSIZE;} \
		u32  maxivsize() const {return ns::ECRYPT_MAXIVSIZE;} \
		u32  block_length() const {return ns::ECRYPT_BLOCKLENGTH;} \
		void keysetup(const u8* key, u32 keysize, u32 ivsize) { \
			ns::ECRYPT_keysetup(&ctx, key, keysize, ivsize);} \
		void ivsetup(const u8* iv) { \
			ns::ECRYPT_ivsetup(&ctx, iv);} \
		void process_bytes(int action, const u8* input, u8* output, u32 msglen) { \
			ns::ECRYPT_process_bytes(action, &ctx, input, output, msglen);} \
		void encrypt_bytes(const u8* plaintext, u8* ciphertext, u32 msglen) { \
			ns::ECRYPT_encrypt_bytes(&ctx, plaintext, ciphertext, msglen);} \
		void decrypt_bytes(const u8* ciphertext, u8* plaintext, u32 msglen) { \
			ns::ECRYPT_decrypt_bytes(&ctx, ciphertext, plaintext, msglen);} \
		void keystream_bytes(u8* keystream, u32 length) { \
			ns::ECRYPT_keystream_bytes(&ctx, keystream, length);} \
		void process_packet(int action, const u8* iv, const u8* input, u8* output, u32 msglen) { \
			ns::ECRYPT_process_packet(action, &ctx, iv, input, output, msglen);} \
		void encrypt_packet(const u8* iv, const u8* plaintext, u8* ciphertext, u32 msglen) { \
			ns::ECRYPT_encrypt_packet(&ctx, iv, plaintext, ciphertext, msglen);} \
		void decrypt_packet(const u8* iv, const u8* ciphertext, u8* plaintext, u32 msglen) { \
			ns::ECRYPT_decrypt_packet(&ctx, iv, ciphertext, plaintext, msglen);} \
		void process_blocks(int action, const u8* input, u8* output, u32 blocks) { \
			ns::ECRYPT_process_blocks(action, &ctx, input, output, blocks);} \
		void encrypt_blocks(const u8* plaintext, u8* ciphertext, u32 blocks) { \
			ns::ECRYPT_encrypt_blocks(&ctx, plaintext, ciphertext, blocks);} \
		void decrypt_blocks(const u8* ciphertext, u8* plaintext, u32 blocks) { \
			ns::ECRYPT_decrypt_blocks(&ctx, ciphertext, plaintext, blocks);} \
		void keystream_blocks(u8* keystream, u32 blocks) { \
			ns::ECRYPT_keystream_blocks(&ctx, keystream, blocks);} \
	private: \
		ns::ECRYPT_ctx ctx; \
	};
}; // ecrypt

#endif // _ECRYPT_