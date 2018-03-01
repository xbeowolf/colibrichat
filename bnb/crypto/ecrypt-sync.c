/* ecrypt-sync.c */

/* *** Please do not edit this file. *** */

#include "string.h"



#ifdef ECRYPT_USES_DEFAULT_ALL_IN_ONE

/*
 * Default implementation of all-in-one encryption/decryption of
 * (short) packets.
 */

#ifdef ECRYPT_HAS_SINGLE_BYTE_FUNCTION

void ECRYPTNC ECRYPT_encrypt_bytes(
  ECRYPT_ctx* ctx, 
  const u8* plaintext, 
  u8* ciphertext, 
  u32 msglen)
{
	ECRYPT_process_bytes(0, ctx, plaintext, ciphertext, msglen);
}

void ECRYPTNC ECRYPT_decrypt_bytes(
  ECRYPT_ctx* ctx, 
  const u8* ciphertext, 
  u8* plaintext, 
  u32 msglen)
{
	ECRYPT_process_bytes(1, ctx, ciphertext, plaintext, msglen);
}

#else

void ECRYPTNC ECRYPT_process_bytes(
  int action, 
  ECRYPT_ctx* ctx, 
  const u8* input, 
  u8* output, 
  u32 msglen)
{
  if (action == 0)
    ECRYPT_encrypt_bytes(ctx, input, output, msglen);
  else
    ECRYPT_decrypt_bytes(ctx, input, output, msglen);
}

#endif



#ifdef ECRYPT_GENERATES_KEYSTREAM

void ECRYPTNC ECRYPT_keystream_bytes(
  ECRYPT_ctx* ctx,
  u8* keystream,
  u32 length)
{
	memset(keystream, 0, length);
	ECRYPT_encrypt_bytes(ctx, keystream, keystream, length);
}

#endif



#ifdef ECRYPT_HAS_SINGLE_PACKET_FUNCTION

void ECRYPTNC ECRYPT_process_packet(
  int action,
  ECRYPT_ctx* ctx,
  const u8* iv,
  const u8* input,
  u8* output,
  u32 msglen)
{
  ECRYPT_ivsetup(ctx, iv);

#ifdef ECRYPT_HAS_SINGLE_BYTE_FUNCTION
  ECRYPT_process_bytes(action, ctx, input, output, msglen);
#else
  if (action == 0)
    ECRYPT_encrypt_bytes(ctx, input, output, msglen);
  else
    ECRYPT_decrypt_bytes(ctx, input, output, msglen);
#endif
}

void ECRYPTNC ECRYPT_encrypt_packet(
  ECRYPT_ctx* ctx, 
  const u8* iv,
  const u8* plaintext, 
  u8* ciphertext, 
  u32 msglen)
{
	ECRYPT_process_packet(0,
		ctx, iv, plaintext, ciphertext, msglen);
}

void ECRYPTNC ECRYPT_decrypt_packet(
  ECRYPT_ctx* ctx, 
  const u8* iv,
  const u8* ciphertext, 
  u8* plaintext, 
  u32 msglen)
{
	ECRYPT_process_packet(1,
		ctx, iv, ciphertext, plaintext, msglen);
}

#else

void ECRYPTNC ECRYPT_encrypt_packet(
  ECRYPT_ctx* ctx,
  const u8* iv,
  const u8* plaintext,
  u8* ciphertext,
  u32 msglen)
{
  ECRYPT_ivsetup(ctx, iv);
  ECRYPT_encrypt_bytes(ctx, plaintext, ciphertext, msglen);
}

void ECRYPTNC ECRYPT_decrypt_packet(
  ECRYPT_ctx* ctx,
  const u8* iv,
  const u8* ciphertext,
  u8* plaintext,
  u32 msglen)
{
  ECRYPT_ivsetup(ctx, iv);
  ECRYPT_decrypt_bytes(ctx, ciphertext, plaintext, msglen);
}

#endif




#ifdef ECRYPT_USES_DEFAULT_BLOCK_MACROS

void ECRYPTNC ECRYPT_process_blocks(
  int action,                 /* 0 = encrypt; 1 = decrypt; */
  ECRYPT_ctx* ctx, 
  const u8* input, 
  u8* output, 
  u32 blocks)
{
	ECRYPT_process_bytes(action, ctx, input, output,
		blocks * ECRYPT_BLOCKLENGTH);
}

void ECRYPTNC ECRYPT_encrypt_blocks(
  ECRYPT_ctx* ctx, 
  const u8* plaintext, 
  u8* ciphertext, 
  u32 blocks)
{
	ECRYPT_encrypt_bytes(ctx, plaintext, ciphertext,
		blocks * ECRYPT_BLOCKLENGTH);
}

void ECRYPTNC ECRYPT_decrypt_blocks(
  ECRYPT_ctx* ctx, 
  const u8* ciphertext, 
  u8* plaintext, 
  u32 blocks)
{
	ECRYPT_decrypt_bytes(ctx, ciphertext, plaintext,
		blocks * ECRYPT_BLOCKLENGTH);
}

#else

#ifdef ECRYPT_HAS_SINGLE_BLOCK_FUNCTION

void ECRYPTNC ECRYPT_encrypt_blocks(
  ECRYPT_ctx* ctx, 
  const u8* plaintext, 
  u8* ciphertext, 
  u32 blocks)
{
	ECRYPT_process_blocks(0, ctx, plaintext, ciphertext, blocks);
}

void ECRYPTNC ECRYPT_decrypt_blocks(
  ECRYPT_ctx* ctx, 
  const u8* ciphertext, 
  u8* plaintext, 
  u32 blocks)
{
	ECRYPT_process_blocks(1, ctx, ciphertext, plaintext, blocks);
}

#else

void ECRYPTNC ECRYPT_process_blocks(
  int action, 
  ECRYPT_ctx* ctx, 
  const u8* input, 
  u8* output, 
  u32 blocks)
{
  if (action == 0)
    ECRYPT_encrypt_blocks(ctx, input, output, blocks);
  else
    ECRYPT_decrypt_blocks(ctx, input, output, blocks);
}

#endif

#endif

#ifdef ECRYPT_GENERATES_BLOCK_KEYSTREAM

void ECRYPTNC ECRYPT_keystream_blocks(
  ECRYPT_ctx* ctx,
  u8* keystream,
  u32 blocks)
{
	ECRYPT_keystream_bytes(ctx, keystream,
		blocks * ECRYPT_BLOCKLENGTH);
}

#endif

#endif