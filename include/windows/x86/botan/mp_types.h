/*
* Low Level MPI Types
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_MPI_TYPES_H__
#define BOTAN_MPI_TYPES_H__

#include <botan/types.h>
#include <botan/mul128.h>

namespace Botan {

#if (BOTAN_MP_WORD_BITS == 8)
  typedef byte word;
  typedef u16bit dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 16)
  typedef u16bit word;
  typedef u32bit dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 32)
  typedef u32bit word;
  typedef u64bit dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 64)
  typedef u64bit word;

  #if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
    typedef uint128_t dword;
    #define BOTAN_HAS_MP_DWORD
  #endif

#else
  #error BOTAN_MP_WORD_BITS must be 8, 16, 32, or 64
#endif

const word MP_WORD_MASK = ~static_cast<word>(0);
const word MP_WORD_TOP_BIT = static_cast<word>(1) << (8*sizeof(word) - 1);
const word MP_WORD_MAX = MP_WORD_MASK;

}

#endif
