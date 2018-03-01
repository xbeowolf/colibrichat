/* copy.c */

/* 
 * COPY stream cipher (does nothing, but in an API compliant way)
 *
 * Author: Christophe De Canni\`ere, K.U.Leuven.
 */

/* ------------------------------------------------------------------------- */

#include "copy.h"
#include <string.h>


#ifdef __cplusplus
#define ECRYPTNC ecrypt::copy::
using namespace ecrypt::copy;
#else
#define ECRYPTNC
#endif

const char* ECRYPTNC ECRYPT_NAME = "COPY";    /* [edit] */ 
const char* ECRYPTNC ECRYPT_PROFILE = "bench";

/* ------------------------------------------------------------------------- */

void ECRYPTNC ECRYPT_init(void)
{ }

/* ------------------------------------------------------------------------- */

void ECRYPTNC ECRYPT_keysetup(
  ECRYPT_ctx* ctx, 
  const u8* key, 
  u32 keysize,
  u32 ivsize)
{ }

/* ------------------------------------------------------------------------- */

void ECRYPTNC ECRYPT_ivsetup(
  ECRYPT_ctx* ctx, 
  const u8* iv)
{ }

/* ------------------------------------------------------------------------- */

void ECRYPTNC ECRYPT_process_bytes(
  int action,
  ECRYPT_ctx* ctx, 
  const u8* input, 
  u8* output, 
  u32 msglen)
{ 
  memmove(output, input, msglen);
}

/* ------------------------------------------------------------------------- */

#define ECRYPT_USES_DEFAULT_ALL_IN_ONE        /* [edit] */
#define ECRYPT_HAS_SINGLE_BYTE_FUNCTION       /* [edit] */
#define ECRYPT_GENERATES_KEYSTREAM            /* [edit] */
#define ECRYPT_HAS_SINGLE_PACKET_FUNCTION     /* [edit] */
#define ECRYPT_USES_DEFAULT_BLOCK_MACROS      /* [edit] */
#define ECRYPT_GENERATES_BLOCK_KEYSTREAM      /* [edit] */
#include "ecrypt-sync.c"

/* ------------------------------------------------------------------------- */
