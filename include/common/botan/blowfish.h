/*
* Blowfish
* (C) 1999-2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_BLOWFISH_H__
#define BOTAN_BLOWFISH_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Blowfish
*/
class BOTAN_DLL Blowfish : public Block_Cipher_Fixed_Params<8, 1, 56>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      /**
      * Modified EKSBlowfish key schedule, used for bcrypt password hashing
      */
      void eks_key_schedule(const byte key[], size_t key_length,
                            const byte salt[16], size_t workfactor);

      void clear();
      std::string name() const { return "Blowfish"; }
      BlockCipher* clone() const { return new Blowfish; }
   private:
      void key_schedule(const byte key[], size_t length);

      void key_expansion(const byte key[],
                         size_t key_length,
                         const byte salt[16]);

      void generate_sbox(secure_vector<u32bit>& box,
                         u32bit& L, u32bit& R,
                         const byte salt[16],
                         size_t salt_off) const;

      static const u32bit P_INIT[18];
      static const u32bit S_INIT[1024];

      secure_vector<u32bit> S, P;
   };

}

#endif
