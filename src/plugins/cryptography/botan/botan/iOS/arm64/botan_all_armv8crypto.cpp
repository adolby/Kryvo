/*
* Botan 2.9.0 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("armv8crypto")
#endif
/*
* AES using ARMv8
* Contributed by Jeffrey Walton
*
* Further changes
* (C) 2017,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <arm_neon.h>

namespace Botan {

#define AES_ENC_4_ROUNDS(K)                \
   do                                      \
      {                                    \
      B0 = vaesmcq_u8(vaeseq_u8(B0, K));   \
      B1 = vaesmcq_u8(vaeseq_u8(B1, K));   \
      B2 = vaesmcq_u8(vaeseq_u8(B2, K));   \
      B3 = vaesmcq_u8(vaeseq_u8(B3, K));   \
      } while(0)

#define AES_ENC_4_LAST_ROUNDS(K, K2)       \
   do                                      \
      {                                    \
      B0 = veorq_u8(vaeseq_u8(B0, K), K2); \
      B1 = veorq_u8(vaeseq_u8(B1, K), K2); \
      B2 = veorq_u8(vaeseq_u8(B2, K), K2); \
      B3 = veorq_u8(vaeseq_u8(B3, K), K2); \
      } while(0)

#define AES_DEC_4_ROUNDS(K)                \
   do                                      \
      {                                    \
      B0 = vaesimcq_u8(vaesdq_u8(B0, K));  \
      B1 = vaesimcq_u8(vaesdq_u8(B1, K));  \
      B2 = vaesimcq_u8(vaesdq_u8(B2, K));  \
      B3 = vaesimcq_u8(vaesdq_u8(B3, K));  \
      } while(0)

#define AES_DEC_4_LAST_ROUNDS(K, K2)       \
   do                                      \
      {                                    \
      B0 = veorq_u8(vaesdq_u8(B0, K), K2); \
      B1 = veorq_u8(vaesdq_u8(B1, K), K2); \
      B2 = veorq_u8(vaesdq_u8(B2, K), K2); \
      B3 = veorq_u8(vaesdq_u8(B3, K), K2); \
      } while(0)

/*
* AES-128 Encryption
*/
BOTAN_FUNC_ISA("+crypto")
void AES_128::armv8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   BOTAN_ASSERT(m_EK.empty() == false, "Key was set");

   const uint8_t *skey = reinterpret_cast<const uint8_t*>(m_EK.data());
   const uint8_t *mkey = reinterpret_cast<const uint8_t*>(m_ME.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0);
   const uint8x16_t K1 = vld1q_u8(skey + 16);
   const uint8x16_t K2 = vld1q_u8(skey + 32);
   const uint8x16_t K3 = vld1q_u8(skey + 48);
   const uint8x16_t K4 = vld1q_u8(skey + 64);
   const uint8x16_t K5 = vld1q_u8(skey + 80);
   const uint8x16_t K6 = vld1q_u8(skey + 96);
   const uint8x16_t K7 = vld1q_u8(skey + 112);
   const uint8x16_t K8 = vld1q_u8(skey + 128);
   const uint8x16_t K9 = vld1q_u8(skey + 144);
   const uint8x16_t K10 = vld1q_u8(mkey);

   while(blocks >= 4)
      {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in+16);
      uint8x16_t B2 = vld1q_u8(in+32);
      uint8x16_t B3 = vld1q_u8(in+48);

      AES_ENC_4_ROUNDS(K0);
      AES_ENC_4_ROUNDS(K1);
      AES_ENC_4_ROUNDS(K2);
      AES_ENC_4_ROUNDS(K3);
      AES_ENC_4_ROUNDS(K4);
      AES_ENC_4_ROUNDS(K5);
      AES_ENC_4_ROUNDS(K6);
      AES_ENC_4_ROUNDS(K7);
      AES_ENC_4_ROUNDS(K8);
      AES_ENC_4_LAST_ROUNDS(K9, K10);

      vst1q_u8(out, B0);
      vst1q_u8(out+16, B1);
      vst1q_u8(out+32, B2);
      vst1q_u8(out+48, B3);

      in += 16*4;
      out += 16*4;
      blocks -= 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      uint8x16_t B = vld1q_u8(in+16*i);
      B = vaesmcq_u8(vaeseq_u8(B, K0));
      B = vaesmcq_u8(vaeseq_u8(B, K1));
      B = vaesmcq_u8(vaeseq_u8(B, K2));
      B = vaesmcq_u8(vaeseq_u8(B, K3));
      B = vaesmcq_u8(vaeseq_u8(B, K4));
      B = vaesmcq_u8(vaeseq_u8(B, K5));
      B = vaesmcq_u8(vaeseq_u8(B, K6));
      B = vaesmcq_u8(vaeseq_u8(B, K7));
      B = vaesmcq_u8(vaeseq_u8(B, K8));
      B = veorq_u8(vaeseq_u8(B, K9), K10);
      vst1q_u8(out+16*i, B);
      }
   }

/*
* AES-128 Decryption
*/
BOTAN_FUNC_ISA("+crypto")
void AES_128::armv8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   BOTAN_ASSERT(m_DK.empty() == false, "Key was set");

   const uint8_t *skey = reinterpret_cast<const uint8_t*>(m_DK.data());
   const uint8_t *mkey = reinterpret_cast<const uint8_t*>(m_MD.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0);
   const uint8x16_t K1 = vld1q_u8(skey + 16);
   const uint8x16_t K2 = vld1q_u8(skey + 32);
   const uint8x16_t K3 = vld1q_u8(skey + 48);
   const uint8x16_t K4 = vld1q_u8(skey + 64);
   const uint8x16_t K5 = vld1q_u8(skey + 80);
   const uint8x16_t K6 = vld1q_u8(skey + 96);
   const uint8x16_t K7 = vld1q_u8(skey + 112);
   const uint8x16_t K8 = vld1q_u8(skey + 128);
   const uint8x16_t K9 = vld1q_u8(skey + 144);
   const uint8x16_t K10 = vld1q_u8(mkey);

   while(blocks >= 4)
      {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in+16);
      uint8x16_t B2 = vld1q_u8(in+32);
      uint8x16_t B3 = vld1q_u8(in+48);

      AES_DEC_4_ROUNDS(K0);
      AES_DEC_4_ROUNDS(K1);
      AES_DEC_4_ROUNDS(K2);
      AES_DEC_4_ROUNDS(K3);
      AES_DEC_4_ROUNDS(K4);
      AES_DEC_4_ROUNDS(K5);
      AES_DEC_4_ROUNDS(K6);
      AES_DEC_4_ROUNDS(K7);
      AES_DEC_4_ROUNDS(K8);
      AES_DEC_4_LAST_ROUNDS(K9, K10);

      vst1q_u8(out, B0);
      vst1q_u8(out+16, B1);
      vst1q_u8(out+32, B2);
      vst1q_u8(out+48, B3);

      in += 16*4;
      out += 16*4;
      blocks -= 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      uint8x16_t B = vld1q_u8(in+16*i);
      B = vaesimcq_u8(vaesdq_u8(B, K0));
      B = vaesimcq_u8(vaesdq_u8(B, K1));
      B = vaesimcq_u8(vaesdq_u8(B, K2));
      B = vaesimcq_u8(vaesdq_u8(B, K3));
      B = vaesimcq_u8(vaesdq_u8(B, K4));
      B = vaesimcq_u8(vaesdq_u8(B, K5));
      B = vaesimcq_u8(vaesdq_u8(B, K6));
      B = vaesimcq_u8(vaesdq_u8(B, K7));
      B = vaesimcq_u8(vaesdq_u8(B, K8));
      B = veorq_u8(vaesdq_u8(B, K9), K10);
      vst1q_u8(out+16*i, B);
      }
   }

/*
* AES-192 Encryption
*/
BOTAN_FUNC_ISA("+crypto")
void AES_192::armv8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   BOTAN_ASSERT(m_EK.empty() == false, "Key was set");

   const uint8_t *skey = reinterpret_cast<const uint8_t*>(m_EK.data());
   const uint8_t *mkey = reinterpret_cast<const uint8_t*>(m_ME.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0);
   const uint8x16_t K1 = vld1q_u8(skey + 16);
   const uint8x16_t K2 = vld1q_u8(skey + 32);
   const uint8x16_t K3 = vld1q_u8(skey + 48);
   const uint8x16_t K4 = vld1q_u8(skey + 64);
   const uint8x16_t K5 = vld1q_u8(skey + 80);
   const uint8x16_t K6 = vld1q_u8(skey + 96);
   const uint8x16_t K7 = vld1q_u8(skey + 112);
   const uint8x16_t K8 = vld1q_u8(skey + 128);
   const uint8x16_t K9 = vld1q_u8(skey + 144);
   const uint8x16_t K10 = vld1q_u8(skey + 160);
   const uint8x16_t K11 = vld1q_u8(skey + 176);
   const uint8x16_t K12 = vld1q_u8(mkey);

   while(blocks >= 4)
      {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in+16);
      uint8x16_t B2 = vld1q_u8(in+32);
      uint8x16_t B3 = vld1q_u8(in+48);

      AES_ENC_4_ROUNDS(K0);
      AES_ENC_4_ROUNDS(K1);
      AES_ENC_4_ROUNDS(K2);
      AES_ENC_4_ROUNDS(K3);
      AES_ENC_4_ROUNDS(K4);
      AES_ENC_4_ROUNDS(K5);
      AES_ENC_4_ROUNDS(K6);
      AES_ENC_4_ROUNDS(K7);
      AES_ENC_4_ROUNDS(K8);
      AES_ENC_4_ROUNDS(K9);
      AES_ENC_4_ROUNDS(K10);
      AES_ENC_4_LAST_ROUNDS(K11, K12);

      vst1q_u8(out, B0);
      vst1q_u8(out+16, B1);
      vst1q_u8(out+32, B2);
      vst1q_u8(out+48, B3);

      in += 16*4;
      out += 16*4;
      blocks -= 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      uint8x16_t B = vld1q_u8(in+16*i);
      B = vaesmcq_u8(vaeseq_u8(B, K0));
      B = vaesmcq_u8(vaeseq_u8(B, K1));
      B = vaesmcq_u8(vaeseq_u8(B, K2));
      B = vaesmcq_u8(vaeseq_u8(B, K3));
      B = vaesmcq_u8(vaeseq_u8(B, K4));
      B = vaesmcq_u8(vaeseq_u8(B, K5));
      B = vaesmcq_u8(vaeseq_u8(B, K6));
      B = vaesmcq_u8(vaeseq_u8(B, K7));
      B = vaesmcq_u8(vaeseq_u8(B, K8));
      B = vaesmcq_u8(vaeseq_u8(B, K9));
      B = vaesmcq_u8(vaeseq_u8(B, K10));
      B = veorq_u8(vaeseq_u8(B, K11), K12);
      vst1q_u8(out+16*i, B);
      }
   }

/*
* AES-192 Decryption
*/
BOTAN_FUNC_ISA("+crypto")
void AES_192::armv8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   BOTAN_ASSERT(m_DK.empty() == false, "Key was set");
   const uint8_t *skey = reinterpret_cast<const uint8_t*>(m_DK.data());
   const uint8_t *mkey = reinterpret_cast<const uint8_t*>(m_MD.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0);
   const uint8x16_t K1 = vld1q_u8(skey + 16);
   const uint8x16_t K2 = vld1q_u8(skey + 32);
   const uint8x16_t K3 = vld1q_u8(skey + 48);
   const uint8x16_t K4 = vld1q_u8(skey + 64);
   const uint8x16_t K5 = vld1q_u8(skey + 80);
   const uint8x16_t K6 = vld1q_u8(skey + 96);
   const uint8x16_t K7 = vld1q_u8(skey + 112);
   const uint8x16_t K8 = vld1q_u8(skey + 128);
   const uint8x16_t K9 = vld1q_u8(skey + 144);
   const uint8x16_t K10 = vld1q_u8(skey + 160);
   const uint8x16_t K11 = vld1q_u8(skey + 176);
   const uint8x16_t K12 = vld1q_u8(mkey);

   while(blocks >= 4)
      {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in+16);
      uint8x16_t B2 = vld1q_u8(in+32);
      uint8x16_t B3 = vld1q_u8(in+48);

      AES_DEC_4_ROUNDS(K0);
      AES_DEC_4_ROUNDS(K1);
      AES_DEC_4_ROUNDS(K2);
      AES_DEC_4_ROUNDS(K3);
      AES_DEC_4_ROUNDS(K4);
      AES_DEC_4_ROUNDS(K5);
      AES_DEC_4_ROUNDS(K6);
      AES_DEC_4_ROUNDS(K7);
      AES_DEC_4_ROUNDS(K8);
      AES_DEC_4_ROUNDS(K9);
      AES_DEC_4_ROUNDS(K10);
      AES_DEC_4_LAST_ROUNDS(K11, K12);

      vst1q_u8(out, B0);
      vst1q_u8(out+16, B1);
      vst1q_u8(out+32, B2);
      vst1q_u8(out+48, B3);

      in += 16*4;
      out += 16*4;
      blocks -= 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      uint8x16_t B = vld1q_u8(in+16*i);
      B = vaesimcq_u8(vaesdq_u8(B, K0));
      B = vaesimcq_u8(vaesdq_u8(B, K1));
      B = vaesimcq_u8(vaesdq_u8(B, K2));
      B = vaesimcq_u8(vaesdq_u8(B, K3));
      B = vaesimcq_u8(vaesdq_u8(B, K4));
      B = vaesimcq_u8(vaesdq_u8(B, K5));
      B = vaesimcq_u8(vaesdq_u8(B, K6));
      B = vaesimcq_u8(vaesdq_u8(B, K7));
      B = vaesimcq_u8(vaesdq_u8(B, K8));
      B = vaesimcq_u8(vaesdq_u8(B, K9));
      B = vaesimcq_u8(vaesdq_u8(B, K10));
      B = veorq_u8(vaesdq_u8(B, K11), K12);
      vst1q_u8(out+16*i, B);
      }
   }

/*
* AES-256 Encryption
*/
BOTAN_FUNC_ISA("+crypto")
void AES_256::armv8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   BOTAN_ASSERT(m_EK.empty() == false, "Key was set");

   const uint8_t *skey = reinterpret_cast<const uint8_t*>(m_EK.data());
   const uint8_t *mkey = reinterpret_cast<const uint8_t*>(m_ME.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0);
   const uint8x16_t K1 = vld1q_u8(skey + 16);
   const uint8x16_t K2 = vld1q_u8(skey + 32);
   const uint8x16_t K3 = vld1q_u8(skey + 48);
   const uint8x16_t K4 = vld1q_u8(skey + 64);
   const uint8x16_t K5 = vld1q_u8(skey + 80);
   const uint8x16_t K6 = vld1q_u8(skey + 96);
   const uint8x16_t K7 = vld1q_u8(skey + 112);
   const uint8x16_t K8 = vld1q_u8(skey + 128);
   const uint8x16_t K9 = vld1q_u8(skey + 144);
   const uint8x16_t K10 = vld1q_u8(skey + 160);
   const uint8x16_t K11 = vld1q_u8(skey + 176);
   const uint8x16_t K12 = vld1q_u8(skey + 192);
   const uint8x16_t K13 = vld1q_u8(skey + 208);
   const uint8x16_t K14 = vld1q_u8(mkey);

   while(blocks >= 4)
      {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in+16);
      uint8x16_t B2 = vld1q_u8(in+32);
      uint8x16_t B3 = vld1q_u8(in+48);

      AES_ENC_4_ROUNDS(K0);
      AES_ENC_4_ROUNDS(K1);
      AES_ENC_4_ROUNDS(K2);
      AES_ENC_4_ROUNDS(K3);
      AES_ENC_4_ROUNDS(K4);
      AES_ENC_4_ROUNDS(K5);
      AES_ENC_4_ROUNDS(K6);
      AES_ENC_4_ROUNDS(K7);
      AES_ENC_4_ROUNDS(K8);
      AES_ENC_4_ROUNDS(K9);
      AES_ENC_4_ROUNDS(K10);
      AES_ENC_4_ROUNDS(K11);
      AES_ENC_4_ROUNDS(K12);
      AES_ENC_4_LAST_ROUNDS(K13, K14);

      vst1q_u8(out, B0);
      vst1q_u8(out+16, B1);
      vst1q_u8(out+32, B2);
      vst1q_u8(out+48, B3);

      in += 16*4;
      out += 16*4;
      blocks -= 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      uint8x16_t B = vld1q_u8(in+16*i);
      B = vaesmcq_u8(vaeseq_u8(B, K0));
      B = vaesmcq_u8(vaeseq_u8(B, K1));
      B = vaesmcq_u8(vaeseq_u8(B, K2));
      B = vaesmcq_u8(vaeseq_u8(B, K3));
      B = vaesmcq_u8(vaeseq_u8(B, K4));
      B = vaesmcq_u8(vaeseq_u8(B, K5));
      B = vaesmcq_u8(vaeseq_u8(B, K6));
      B = vaesmcq_u8(vaeseq_u8(B, K7));
      B = vaesmcq_u8(vaeseq_u8(B, K8));
      B = vaesmcq_u8(vaeseq_u8(B, K9));
      B = vaesmcq_u8(vaeseq_u8(B, K10));
      B = vaesmcq_u8(vaeseq_u8(B, K11));
      B = vaesmcq_u8(vaeseq_u8(B, K12));
      B = veorq_u8(vaeseq_u8(B, K13), K14);
      vst1q_u8(out+16*i, B);
      }
   }

/*
* AES-256 Decryption
*/
BOTAN_FUNC_ISA("+crypto")
void AES_256::armv8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   BOTAN_ASSERT(m_DK.empty() == false, "Key was set");

   const uint8_t *skey = reinterpret_cast<const uint8_t*>(m_DK.data());
   const uint8_t *mkey = reinterpret_cast<const uint8_t*>(m_MD.data());

   const uint8x16_t K0 = vld1q_u8(skey + 0);
   const uint8x16_t K1 = vld1q_u8(skey + 16);
   const uint8x16_t K2 = vld1q_u8(skey + 32);
   const uint8x16_t K3 = vld1q_u8(skey + 48);
   const uint8x16_t K4 = vld1q_u8(skey + 64);
   const uint8x16_t K5 = vld1q_u8(skey + 80);
   const uint8x16_t K6 = vld1q_u8(skey + 96);
   const uint8x16_t K7 = vld1q_u8(skey + 112);
   const uint8x16_t K8 = vld1q_u8(skey + 128);
   const uint8x16_t K9 = vld1q_u8(skey + 144);
   const uint8x16_t K10 = vld1q_u8(skey + 160);
   const uint8x16_t K11 = vld1q_u8(skey + 176);
   const uint8x16_t K12 = vld1q_u8(skey + 192);
   const uint8x16_t K13 = vld1q_u8(skey + 208);
   const uint8x16_t K14 = vld1q_u8(mkey);

   while(blocks >= 4)
      {
      uint8x16_t B0 = vld1q_u8(in);
      uint8x16_t B1 = vld1q_u8(in+16);
      uint8x16_t B2 = vld1q_u8(in+32);
      uint8x16_t B3 = vld1q_u8(in+48);

      AES_DEC_4_ROUNDS(K0);
      AES_DEC_4_ROUNDS(K1);
      AES_DEC_4_ROUNDS(K2);
      AES_DEC_4_ROUNDS(K3);
      AES_DEC_4_ROUNDS(K4);
      AES_DEC_4_ROUNDS(K5);
      AES_DEC_4_ROUNDS(K6);
      AES_DEC_4_ROUNDS(K7);
      AES_DEC_4_ROUNDS(K8);
      AES_DEC_4_ROUNDS(K9);
      AES_DEC_4_ROUNDS(K10);
      AES_DEC_4_ROUNDS(K11);
      AES_DEC_4_ROUNDS(K12);
      AES_DEC_4_LAST_ROUNDS(K13, K14);

      vst1q_u8(out, B0);
      vst1q_u8(out+16, B1);
      vst1q_u8(out+32, B2);
      vst1q_u8(out+48, B3);

      in += 16*4;
      out += 16*4;
      blocks -= 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      uint8x16_t B = vld1q_u8(in+16*i);
      B = vaesimcq_u8(vaesdq_u8(B, K0));
      B = vaesimcq_u8(vaesdq_u8(B, K1));
      B = vaesimcq_u8(vaesdq_u8(B, K2));
      B = vaesimcq_u8(vaesdq_u8(B, K3));
      B = vaesimcq_u8(vaesdq_u8(B, K4));
      B = vaesimcq_u8(vaesdq_u8(B, K5));
      B = vaesimcq_u8(vaesdq_u8(B, K6));
      B = vaesimcq_u8(vaesdq_u8(B, K7));
      B = vaesimcq_u8(vaesdq_u8(B, K8));
      B = vaesimcq_u8(vaesdq_u8(B, K9));
      B = vaesimcq_u8(vaesdq_u8(B, K10));
      B = vaesimcq_u8(vaesdq_u8(B, K11));
      B = vaesimcq_u8(vaesdq_u8(B, K12));
      B = veorq_u8(vaesdq_u8(B, K13), K14);
      vst1q_u8(out+16*i, B);
      }
   }

#undef AES_ENC_4_ROUNDS
#undef AES_ENC_4_LAST_ROUNDS
#undef AES_DEC_4_ROUNDS
#undef AES_DEC_4_LAST_ROUNDS

}
/*
* Contributed by Jeffrey Walton
*
* Further changes
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
This follows the same pattern as the clmul implementation.

See also https://conradoplg.cryptoland.net/files/2010/12/gcm14.pdf
*/

namespace {

BOTAN_FUNC_ISA("+simd")
inline uint64x2_t gcm_reduce(uint32x4_t B0, uint32x4_t B1)
   {
   const uint32x4_t zero = vdupq_n_u32(0);

   uint32x4_t T0, T1, T2, T3, T4, T5;

   T4 = vshrq_n_u32(B0, 31);
   T0 = vshlq_n_u32(B0, 1);
   T5 = vshrq_n_u32(B1, 31);
   T3 = vshlq_n_u32(B1, 1);

   T2 = vextq_u32(T4, zero, 3);
   T5 = vextq_u32(zero, T5, 3);
   T4 = vextq_u32(zero, T4, 3);
   T0 = vorrq_u32(T0, T4);
   T3 = vorrq_u32(T3, T5);
   T3 = vorrq_u32(T3, T2);

   T4 = vshlq_n_u32(T0, 31);
   T5 = vshlq_n_u32(T0, 30);
   T2 = vshlq_n_u32(T0, 25);

   T4 = veorq_u32(T4, T5);
   T4 = veorq_u32(T4, T2);
   T5 = vextq_u32(T4, zero, 1);
   T3 = veorq_u32(T3, T5);
   T4 = vextq_u32(zero, T4, 1);
   T0 = veorq_u32(T0, T4);
   T3 = veorq_u32(T3, T0);

   T4 = vshrq_n_u32(T0, 1);
   T1 = vshrq_n_u32(T0, 2);
   T2 = vshrq_n_u32(T0, 7);
   T3 = veorq_u32(T3, T1);
   T3 = veorq_u32(T3, T2);
   T3 = veorq_u32(T3, T4);

   return vreinterpretq_u64_u32(T3);
   }

BOTAN_FUNC_ISA("+crypto")
inline uint32x4_t vmull(uint64_t x, uint64_t y)
   {
   return reinterpret_cast<uint32x4_t>(vmull_p64(x, y));
   }

BOTAN_FUNC_ISA("+crypto")
inline uint64x2_t gcm_multiply(uint64x2_t H, uint64x2_t x)
   {
   const uint32x4_t zero = vdupq_n_u32(0);

   const uint64_t x_hi = vgetq_lane_u64(x, 0);
   const uint64_t x_lo = vgetq_lane_u64(x, 1);
   const uint64_t H_hi = vgetq_lane_u64(H, 0);
   const uint64_t H_lo = vgetq_lane_u64(H, 1);

   uint32x4_t T0 = vmull(x_hi, H_hi);
   uint32x4_t T1 = vmull(x_lo, H_hi);
   uint32x4_t T2 = vmull(x_hi, H_lo);
   uint32x4_t T3 = vmull(x_lo, H_lo);

   T1 = veorq_u32(T1, T2);
   T0 = veorq_u32(T0, vextq_u32(zero, T1, 2));
   T3 = veorq_u32(T3, vextq_u32(T1, zero, 2));

   return gcm_reduce(T0, T3);
   }

BOTAN_FUNC_ISA("+crypto")
inline uint64x2_t gcm_multiply_x4(uint64x2_t H1, uint64x2_t H2, uint64x2_t H3, uint64x2_t H4,
                                  uint64x2_t X1, uint64x2_t X2, uint64x2_t X3, uint64x2_t X4)
   {
   const uint64_t H1_hi = vgetq_lane_u64(H1, 0);
   const uint64_t H1_lo = vgetq_lane_u64(H1, 1);
   const uint64_t H2_hi = vgetq_lane_u64(H2, 0);
   const uint64_t H2_lo = vgetq_lane_u64(H2, 1);
   const uint64_t H3_hi = vgetq_lane_u64(H3, 0);
   const uint64_t H3_lo = vgetq_lane_u64(H3, 1);
   const uint64_t H4_hi = vgetq_lane_u64(H4, 0);
   const uint64_t H4_lo = vgetq_lane_u64(H4, 1);

   const uint64_t X1_hi = vgetq_lane_u64(X1, 0);
   const uint64_t X1_lo = vgetq_lane_u64(X1, 1);
   const uint64_t X2_hi = vgetq_lane_u64(X2, 0);
   const uint64_t X2_lo = vgetq_lane_u64(X2, 1);
   const uint64_t X3_hi = vgetq_lane_u64(X3, 0);
   const uint64_t X3_lo = vgetq_lane_u64(X3, 1);
   const uint64_t X4_hi = vgetq_lane_u64(X4, 0);
   const uint64_t X4_lo = vgetq_lane_u64(X4, 1);

   const uint32x4_t H1_X1_lo = vmull(X1_lo, H1_lo);
   const uint32x4_t H2_X2_lo = vmull(X2_lo, H2_lo);
   const uint32x4_t H3_X3_lo = vmull(X3_lo, H3_lo);
   const uint32x4_t H4_X4_lo = vmull(X4_lo, H4_lo);

   const uint32x4_t lo = veorq_u32(
      veorq_u32(H1_X1_lo, H2_X2_lo),
      veorq_u32(H3_X3_lo, H4_X4_lo));

   const uint32x4_t H1_X1_hi = vmull(X1_hi, H1_hi);
   const uint32x4_t H2_X2_hi = vmull(X2_hi, H2_hi);
   const uint32x4_t H3_X3_hi = vmull(X3_hi, H3_hi);
   const uint32x4_t H4_X4_hi = vmull(X4_hi, H4_hi);

   const uint32x4_t hi = veorq_u32(
      veorq_u32(H1_X1_hi, H2_X2_hi),
      veorq_u32(H3_X3_hi, H4_X4_hi));

   uint32x4_t T0 = veorq_u32(lo, hi);

   T0 = veorq_u32(T0, vmull(X1_hi ^ X1_lo, H1_hi ^ H1_lo));
   T0 = veorq_u32(T0, vmull(X2_hi ^ X2_lo, H2_hi ^ H2_lo));
   T0 = veorq_u32(T0, vmull(X3_hi ^ X3_lo, H3_hi ^ H3_lo));
   T0 = veorq_u32(T0, vmull(X4_hi ^ X4_lo, H4_hi ^ H4_lo));

   const uint32x4_t zero = vdupq_n_u32(0);
   uint32x4_t B0 = veorq_u32(vextq_u32(zero, T0, 2), hi);
   uint32x4_t B1 = veorq_u32(vextq_u32(T0, zero, 2), lo);
   return gcm_reduce(B0, B1);
   }

BOTAN_FUNC_ISA("+simd")
inline uint8x16_t bswap_vec(uint8x16_t v)
   {
   const uint8_t maskb[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
   const uint8x16_t mask = vld1q_u8(maskb);
   return vqtbl1q_u8(v, mask);
   }

}

BOTAN_FUNC_ISA("+simd")
void gcm_pmull_precompute(const uint8_t H_bytes[16], uint64_t H_pow[4*2])
   {
   const uint64x2_t H = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(H_bytes)));
   const uint64x2_t H2 = gcm_multiply(H, H);
   const uint64x2_t H3 = gcm_multiply(H, H2);
   const uint64x2_t H4 = gcm_multiply(H, H3);

   vst1q_u64(H_pow  , H);
   vst1q_u64(H_pow+2, H2);
   vst1q_u64(H_pow+4, H3);
   vst1q_u64(H_pow+6, H4);
   }

BOTAN_FUNC_ISA("+simd")
void gcm_multiply_pmull(uint8_t x[16],
                        const uint64_t H64[8],
                        const uint8_t input[], size_t blocks)
   {
   const uint64x2_t H = vld1q_u64(H64);
   uint64x2_t a = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(x)));

   if(blocks >= 4)
      {
      const uint64x2_t H2 = vld1q_u64(H64 + 2);
      const uint64x2_t H3 = vld1q_u64(H64 + 4);
      const uint64x2_t H4 = vld1q_u64(H64 + 6);

      while(blocks >= 4)
         {
         const uint64x2_t m0 = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(input)));
         const uint64x2_t m1 = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(input + 16)));
         const uint64x2_t m2 = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(input + 32)));
         const uint64x2_t m3 = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(input + 48)));

         a = veorq_u64(a, m0);
         a = gcm_multiply_x4(H, H2, H3, H4, m3, m2, m1, a);

         input += 64;
         blocks -= 4;
         }
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      const uint64x2_t m = vreinterpretq_u64_u8(bswap_vec(vld1q_u8(input + 16*i)));
      a = veorq_u64(a, m);
      a = gcm_multiply(H, a);
      }

   vst1q_u8(x, bswap_vec(vreinterpretq_u8_u64(a)));
   }

}
/*
* SHA-1 using CPU instructions in ARMv8
*
* Contributed by Jeffrey Walton. Based on public domain code by
* Johannes Schneiders, Skip Hovsmith and Barry O'Rourke.
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* SHA-1 using CPU instructions in ARMv8
*/
//static
#if defined(BOTAN_HAS_SHA1_ARMV8)
BOTAN_FUNC_ISA("+crypto")
void SHA_160::sha1_armv8_compress_n(secure_vector<uint32_t>& digest, const uint8_t input8[], size_t blocks)
   {
   uint32x4_t ABCD;
   uint32_t E0;

   // Load magic constants
   const uint32x4_t C0 = vdupq_n_u32(0x5A827999);
   const uint32x4_t C1 = vdupq_n_u32(0x6ED9EBA1);
   const uint32x4_t C2 = vdupq_n_u32(0x8F1BBCDC);
   const uint32x4_t C3 = vdupq_n_u32(0xCA62C1D6);

   ABCD = vld1q_u32(&digest[0]);
   E0 = digest[4];

   // Intermediate void* cast due to https://llvm.org/bugs/show_bug.cgi?id=20670
   const uint32_t* input32 = reinterpret_cast<const uint32_t*>(reinterpret_cast<const void*>(input8));

   while (blocks)
      {
      // Save current hash
      const uint32x4_t ABCD_SAVED = ABCD;
      const uint32_t E0_SAVED = E0;

      uint32x4_t MSG0, MSG1, MSG2, MSG3;
      uint32x4_t TMP0, TMP1;
      uint32_t E1;

      MSG0 = vld1q_u32(input32 + 0);
      MSG1 = vld1q_u32(input32 + 4);
      MSG2 = vld1q_u32(input32 + 8);
      MSG3 = vld1q_u32(input32 + 12);

      MSG0 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG0)));
      MSG1 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG1)));
      MSG2 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG2)));
      MSG3 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG3)));

      TMP0 = vaddq_u32(MSG0, C0);
      TMP1 = vaddq_u32(MSG1, C0);

      // Rounds 0-3
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1cq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG2, C0);
      MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

      // Rounds 4-7
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1cq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG3, C0);
      MSG0 = vsha1su1q_u32(MSG0, MSG3);
      MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

      // Rounds 8-11
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1cq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG0, C0);
      MSG1 = vsha1su1q_u32(MSG1, MSG0);
      MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

      // Rounds 12-15
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1cq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG1, C1);
      MSG2 = vsha1su1q_u32(MSG2, MSG1);
      MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

      // Rounds 16-19
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1cq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG2, C1);
      MSG3 = vsha1su1q_u32(MSG3, MSG2);
      MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

      // Rounds 20-23
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG3, C1);
      MSG0 = vsha1su1q_u32(MSG0, MSG3);
      MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

      // Rounds 24-27
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG0, C1);
      MSG1 = vsha1su1q_u32(MSG1, MSG0);
      MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

      // Rounds 28-31
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG1, C1);
      MSG2 = vsha1su1q_u32(MSG2, MSG1);
      MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

      // Rounds 32-35
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG2, C2);
      MSG3 = vsha1su1q_u32(MSG3, MSG2);
      MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

      // Rounds 36-39
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG3, C2);
      MSG0 = vsha1su1q_u32(MSG0, MSG3);
      MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

      // Rounds 40-43
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1mq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG0, C2);
      MSG1 = vsha1su1q_u32(MSG1, MSG0);
      MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

      // Rounds 44-47
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1mq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG1, C2);
      MSG2 = vsha1su1q_u32(MSG2, MSG1);
      MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

      // Rounds 48-51
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1mq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG2, C2);
      MSG3 = vsha1su1q_u32(MSG3, MSG2);
      MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

      // Rounds 52-55
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1mq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG3, C3);
      MSG0 = vsha1su1q_u32(MSG0, MSG3);
      MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

      // Rounds 56-59
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1mq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG0, C3);
      MSG1 = vsha1su1q_u32(MSG1, MSG0);
      MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

      // Rounds 60-63
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG1, C3);
      MSG2 = vsha1su1q_u32(MSG2, MSG1);
      MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

      // Rounds 64-67
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E0, TMP0);
      TMP0 = vaddq_u32(MSG2, C3);
      MSG3 = vsha1su1q_u32(MSG3, MSG2);
      MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

      // Rounds 68-71
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E1, TMP1);
      TMP1 = vaddq_u32(MSG3, C3);
      MSG0 = vsha1su1q_u32(MSG0, MSG3);

      // Rounds 72-75
      E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E0, TMP0);

      // Rounds 76-79
      E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
      ABCD = vsha1pq_u32(ABCD, E1, TMP1);

      // Add state back
      E0 += E0_SAVED;
      ABCD = vaddq_u32(ABCD_SAVED, ABCD);

      input32 += 64/4;
      blocks--;
      }

   // Save digest
   vst1q_u32(&digest[0], ABCD);
   digest[4] = E0;
   }
#endif

}
/*
* SHA-256 using CPU instructions in ARMv8
*
* Contributed by Jeffrey Walton. Based on public domain code by
* Johannes Schneiders, Skip Hovsmith and Barry O'Rourke.
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* SHA-256 using CPU instructions in ARMv8
*/
//static
#if defined(BOTAN_HAS_SHA2_32_ARMV8)
BOTAN_FUNC_ISA("+crypto")
void SHA_256::compress_digest_armv8(secure_vector<uint32_t>& digest, const uint8_t input8[], size_t blocks)
   {
   static const uint32_t K[] = {
      0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
      0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
      0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
      0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
      0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
      0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
      0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
      0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
      0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
      0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
      0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
      0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
      0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
      0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
      0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
      0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
   };

   uint32x4_t STATE0, STATE1, ABEF_SAVE, CDGH_SAVE;
   uint32x4_t MSG0, MSG1, MSG2, MSG3;
   uint32x4_t TMP0, TMP1, TMP2;

   // Load initial values
   STATE0 = vld1q_u32(&digest[0]);
   STATE1 = vld1q_u32(&digest[4]);

   // Intermediate void* cast due to https://llvm.org/bugs/show_bug.cgi?id=20670
   const uint32_t* input32 = reinterpret_cast<const uint32_t*>(reinterpret_cast<const void*>(input8));

   while (blocks)
      {
      // Save current state
      ABEF_SAVE = STATE0;
      CDGH_SAVE = STATE1;

      MSG0 = vld1q_u32(input32 + 0);
      MSG1 = vld1q_u32(input32 + 4);
      MSG2 = vld1q_u32(input32 + 8);
      MSG3 = vld1q_u32(input32 + 12);

      MSG0 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG0)));
      MSG1 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG1)));
      MSG2 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG2)));
      MSG3 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG3)));

      TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[0x00]));

      // Rounds 0-3
      MSG0 = vsha256su0q_u32(MSG0, MSG1);
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[0x04]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
      MSG0 = vsha256su1q_u32(MSG0, MSG2, MSG3);

      // Rounds 4-7
      MSG1 = vsha256su0q_u32(MSG1, MSG2);
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG2, vld1q_u32(&K[0x08]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
      MSG1 = vsha256su1q_u32(MSG1, MSG3, MSG0);

      // Rounds 8-11
      MSG2 = vsha256su0q_u32(MSG2, MSG3);
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG3, vld1q_u32(&K[0x0c]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
      MSG2 = vsha256su1q_u32(MSG2, MSG0, MSG1);

      // Rounds 12-15
      MSG3 = vsha256su0q_u32(MSG3, MSG0);
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[0x10]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
      MSG3 = vsha256su1q_u32(MSG3, MSG1, MSG2);

      // Rounds 16-19
      MSG0 = vsha256su0q_u32(MSG0, MSG1);
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[0x14]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
      MSG0 = vsha256su1q_u32(MSG0, MSG2, MSG3);

      // Rounds 20-23
      MSG1 = vsha256su0q_u32(MSG1, MSG2);
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG2, vld1q_u32(&K[0x18]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
      MSG1 = vsha256su1q_u32(MSG1, MSG3, MSG0);

      // Rounds 24-27
      MSG2 = vsha256su0q_u32(MSG2, MSG3);
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG3, vld1q_u32(&K[0x1c]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
      MSG2 = vsha256su1q_u32(MSG2, MSG0, MSG1);

      // Rounds 28-31
      MSG3 = vsha256su0q_u32(MSG3, MSG0);
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[0x20]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
      MSG3 = vsha256su1q_u32(MSG3, MSG1, MSG2);

      // Rounds 32-35
      MSG0 = vsha256su0q_u32(MSG0, MSG1);
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[0x24]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
      MSG0 = vsha256su1q_u32(MSG0, MSG2, MSG3);

      // Rounds 36-39
      MSG1 = vsha256su0q_u32(MSG1, MSG2);
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG2, vld1q_u32(&K[0x28]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
      MSG1 = vsha256su1q_u32(MSG1, MSG3, MSG0);

      // Rounds 40-43
      MSG2 = vsha256su0q_u32(MSG2, MSG3);
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG3, vld1q_u32(&K[0x2c]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
      MSG2 = vsha256su1q_u32(MSG2, MSG0, MSG1);

      // Rounds 44-47
      MSG3 = vsha256su0q_u32(MSG3, MSG0);
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[0x30]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
      MSG3 = vsha256su1q_u32(MSG3, MSG1, MSG2);

      // Rounds 48-51
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[0x34]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);

      // Rounds 52-55
      TMP2 = STATE0;
      TMP0 = vaddq_u32(MSG2, vld1q_u32(&K[0x38]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);

      // Rounds 56-59
      TMP2 = STATE0;
      TMP1 = vaddq_u32(MSG3, vld1q_u32(&K[0x3c]));
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);

      // Rounds 60-63
      TMP2 = STATE0;
      STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
      STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);

      // Add back to state
      STATE0 = vaddq_u32(STATE0, ABEF_SAVE);
      STATE1 = vaddq_u32(STATE1, CDGH_SAVE);

      input32 += 64/4;
      blocks--;
      }

   // Save state
   vst1q_u32(&digest[0], STATE0);
   vst1q_u32(&digest[4], STATE1);
   }
#endif

}
