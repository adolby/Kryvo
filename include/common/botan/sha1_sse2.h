/*
* SHA-160
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SHA_160_SSE2_H__
#define BOTAN_SHA_160_SSE2_H__

#include <botan/sha160.h>

namespace Botan {

/**
* SHA-160 using SSE2 for the message expansion
*/
class BOTAN_DLL SHA_160_SSE2 : public SHA_160
   {
   public:
      HashFunction* clone() const { return new SHA_160_SSE2; }
      SHA_160_SSE2() : SHA_160(0) {} // no W needed
   private:
      void compress_n(const byte[], size_t blocks);
   };

}

#endif
