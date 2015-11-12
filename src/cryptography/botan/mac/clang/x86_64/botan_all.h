/*
* Botan 1.11.24 Amalgamation
* (C) 1999-2013,2014,2015 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AMALGAMATION_H__
#define BOTAN_AMALGAMATION_H__

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <istream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

/*
* This file was automatically generated Wed Nov 11 21:38:50 2015 UTC by
* andrew@Andrews-MBP running 'configure.py --cc=clang --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,dev_random,eax,filters,gcm,kdf2,keccak,locking_allocator,pbkdf2,serpent,serpent_simd'
*
* Target
*  - Compiler: clang++  -m64 -pthread -std=c++11 -D_REENTRANT -fstack-protector -O3
*  - Arch: x86_64/x86_64
*  - OS: darwin
*/

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 11
#define BOTAN_VERSION_PATCH 24
#define BOTAN_VERSION_DATESTAMP 20151104

#define BOTAN_VERSION_RELEASE_TYPE "released"

#define BOTAN_VERSION_VC_REVISION "git:cb4ab0662dfbe462dbe578ffa7d6f44effa51d82"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

#define BOTAN_INSTALL_PREFIX "/usr/local"
#define BOTAN_INSTALL_HEADER_DIR "include/botan-1.11"
#define BOTAN_INSTALL_LIB_DIR "lib"
#define BOTAN_LIB_LINK ""

#ifndef BOTAN_DLL
  #define BOTAN_DLL 
#endif

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Minimum and maximum sizes to allocate out of the mlock pool (bytes)
   Default min is 16 as smaller values are easily bruteforceable and thus
   likely not cryptographic keys.
*/
#define BOTAN_MLOCK_ALLOCATOR_MIN_ALLOCATION 16
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 128

/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 64

/*
* If enabled uses memset via volatile function pointer to zero memory,
* otherwise does a byte at a time write via a volatile pointer.
*/
#define BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO 1

/*
* If enabled the ECC implementation will use Montgomery ladder
* instead of a fixed window implementation.
*/
#define BOTAN_POINTGFP_BLINDED_MULTIPLY_USE_MONTGOMERY_LADDER 0

/*
* Set number of bits used to generate mask for blinding the scalar of
* a point multiplication. Set to zero to disable this side-channel
* countermeasure.
*/
#define BOTAN_POINTGFP_SCALAR_BLINDING_BITS 20

/*
* Set number of bits used to generate mask for blinding the
* representation of an ECC point. Set to zero to diable this
* side-channel countermeasure.
*/
#define BOTAN_POINTGFP_RANDOMIZE_BLINDING_BITS 80

/*
* Normally blinding is performed by choosing a random starting point (plus
* its inverse, of a form appropriate to the algorithm being blinded), and
* then choosing new blinding operands by successive squaring of both
* values. This is much faster than computing a new starting point but
* introduces some possible coorelation
*
* To avoid possible leakage problems in long-running processes, the blinder
* periodically reinitializes the sequence. This value specifies how often
* a new sequence should be started.
*/
#define BOTAN_BLINDING_REINIT_INTERVAL 32

/* PK key consistency checking toggles */
#define BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD 1
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD 0
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE 1

/*
* Define BOTAN_USE_CTGRIND to enable checking constant time
* annotations using ctgrind https://github.com/agl/ctgrind
*/
//#define BOTAN_USE_CTGRIND

/*
* RNGs will automatically poll the system for additional seed material
* after producing this many bytes of output.
*/
#define BOTAN_RNG_MAX_OUTPUT_BEFORE_RESEED 512
#define BOTAN_RNG_RESEED_POLL_BITS 128
#define BOTAN_RNG_AUTO_RESEED_TIMEOUT std::chrono::milliseconds(10)
#define BOTAN_RNG_RESEED_DEFAULT_TIMEOUT std::chrono::milliseconds(100)

/* Should we use GCC-style inline assembler? */
#if !defined(BOTAN_USE_GCC_INLINE_ASM) && defined(__GNUG__)
  #define BOTAN_USE_GCC_INLINE_ASM 1
#endif

#ifdef __GNUC__
  #define BOTAN_GCC_VERSION \
     (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#else
  #define BOTAN_GCC_VERSION 0
#endif

/* Target identification and feature test macros */
#define BOTAN_TARGET_OS_IS_DARWIN
#define BOTAN_TARGET_OS_HAS_DLOPEN
#define BOTAN_TARGET_OS_HAS_GETSID
#define BOTAN_TARGET_OS_HAS_GETTIMEOFDAY
#define BOTAN_TARGET_OS_HAS_GMTIME_R
#define BOTAN_TARGET_OS_HAS_MEMSET_S
#define BOTAN_TARGET_OS_HAS_READDIR
#define BOTAN_TARGET_OS_HAS_TIMEGM

#define BOTAN_TARGET_ARCH_IS_X86_64
#define BOTAN_TARGET_SUPPORTS_AESNI
#define BOTAN_TARGET_SUPPORTS_AVX2
#define BOTAN_TARGET_SUPPORTS_BMI2
#define BOTAN_TARGET_SUPPORTS_CLMUL
#define BOTAN_TARGET_SUPPORTS_RDRAND
#define BOTAN_TARGET_SUPPORTS_SHA
#define BOTAN_TARGET_SUPPORTS_SSE2
#define BOTAN_TARGET_SUPPORTS_SSE41
#define BOTAN_TARGET_SUPPORTS_SSE42
#define BOTAN_TARGET_SUPPORTS_SSSE3
#define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
#define BOTAN_TARGET_CPU_IS_X86_FAMILY
#define BOTAN_TARGET_CPU_NATIVE_WORD_SIZE 64
#define BOTAN_TARGET_CPU_HAS_NATIVE_64BIT
#define BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK 1

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN) || \
    defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
  #define BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS
#endif

#define BOTAN_BUILD_COMPILER_IS_CLANG

#if defined(_MSC_VER)
  // 4250: inherits via dominance (diamond inheritence issue)
  // 4251: needs DLL interface (STL DLL exports)
  #pragma warning(disable: 4250 4251)
#endif

/*
* Compile-time deprecatation warnings
*/
#if !defined(BOTAN_NO_DEPRECATED_WARNINGS)

  #if defined(__clang__)
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))

  #elif defined(_MSC_VER)
    #define BOTAN_DEPRECATED(msg) __declspec(deprecated(msg))

  #elif defined(__GNUG__)

    #if BOTAN_GCC_VERSION >= 450
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
    #else
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))
    #endif

  #endif

#endif

#if defined(_MSC_VER)
  #define BOTAN_CURRENT_FUNCTION __FUNCTION__
#else
  #define BOTAN_CURRENT_FUNCTION __func__
#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

#if defined(_MSC_VER)
  // noexcept is not supported in VS 2013
  #include <yvals.h>
  #define BOTAN_NOEXCEPT _NOEXCEPT
#else
  #define BOTAN_NOEXCEPT noexcept
#endif

/*
* Module availability definitions
*/
#define BOTAN_HAS_AEAD_EAX 20131128
#define BOTAN_HAS_AEAD_GCM 20131128
#define BOTAN_HAS_AEAD_MODES 20131128
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_AUTO_SEEDING_RNG 20131128
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_CMAC 20131128
#define BOTAN_HAS_COMPRESSION 20141117
#define BOTAN_HAS_CTR_BE 20131128
#define BOTAN_HAS_ENTROPY_SOURCE 20150201
#define BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM 20131128
#define BOTAN_HAS_FILTERS 20131128
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_RNG 20131128
#define BOTAN_HAS_KDF2 20131128
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KECCAK 20131128
#define BOTAN_HAS_LOCKING_ALLOCATOR 20131128
#define BOTAN_HAS_MAC 20150626
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MODES 20150626
#define BOTAN_HAS_PBKDF 20150626
#define BOTAN_HAS_PBKDF2 20131128
#define BOTAN_HAS_SERPENT 20131128
#define BOTAN_HAS_SERPENT_SIMD 20131128
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SIMD_32 20131128
#define BOTAN_HAS_SIMD_SSE2 20131128
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_TRANSFORM 20131209
#define BOTAN_HAS_UTIL_FUNCTIONS 20150919

/*
* Local configuration options (if any) follow
*/



namespace Botan {

/**
* Called when an assertion fails
*/
void BOTAN_DLL assertion_failure(const char* expr_str,
                                 const char* assertion_made,
                                 const char* func,
                                 const char* file,
                                 int line);

/**
* Make an assertion
*/
#define BOTAN_ASSERT(expr, assertion_made)                \
   do {                                                   \
      if(!(expr))                                         \
         Botan::assertion_failure(#expr,                  \
                                  assertion_made,         \
                                  BOTAN_CURRENT_FUNCTION, \
                                  __FILE__,               \
                                  __LINE__);              \
   } while(0)

/**
* Assert that value1 == value2
*/
#define BOTAN_ASSERT_EQUAL(expr1, expr2, assertion_made)   \
   do {                                                    \
     if((expr1) != (expr2))                                \
       Botan::assertion_failure(#expr1 " == " #expr2,      \
                                  assertion_made,          \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that expr1 (if true) implies expr2 is also true
*/
#define BOTAN_ASSERT_IMPLICATION(expr1, expr2, msg)        \
   do {                                                    \
     if((expr1) && !(expr2))                               \
       Botan::assertion_failure(#expr1 " implies " #expr2, \
                                msg,                       \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that a pointer is not null
*/
#define BOTAN_ASSERT_NONNULL(ptr)                          \
   do {                                                    \
     if((ptr) == nullptr)                                  \
         Botan::assertion_failure(#ptr " is not null",     \
                                  "",                      \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Mark variable as unused
*/
#define BOTAN_UNUSED(v) static_cast<void>(v)

}


/**
* The primary namespace for the botan library
*/
namespace Botan {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::int32_t;
using std::int64_t;
using std::size_t;

using byte   = std::uint8_t;
using u16bit = std::uint16_t;
using u32bit = std::uint32_t;
using u64bit = std::uint64_t;
using s32bit = std::int32_t;

/**
* A default buffer size; typically a memory page
*/
static const size_t DEFAULT_BUFFERSIZE = BOTAN_DEFAULT_BUFFER_SIZE;

}


namespace Botan {

/**
* Zeroize memory
* @param ptr a pointer to memory to zero out
* @param n the number of bytes pointed to by ptr
*/
BOTAN_DLL void zero_mem(void* ptr, size_t n);

/**
* Zeroize memory
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
*/
template<typename T> inline void clear_mem(T* ptr, size_t n)
   {
   std::memset(ptr, 0, sizeof(T)*n);
   }

/**
* Copy memory
* @param out the destination array
* @param in the source array
* @param n the number of elements of in/out
*/
template<typename T> inline void copy_mem(T* out, const T* in, size_t n)
   {
   std::memmove(out, in, sizeof(T)*n);
   }

/**
* Set memory to a fixed value
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
* @param val the value to set each byte to
*/
template<typename T>
inline void set_mem(T* ptr, size_t n, byte val)
   {
   std::memset(ptr, val, sizeof(T)*n);
   }

/**
* Memory comparison, input insensitive
* @param p1 a pointer to an array
* @param p2 a pointer to another array
* @param n the number of Ts in p1 and p2
* @return true iff p1[i] == p2[i] forall i in [0...n)
*/
template<typename T> inline bool same_mem(const T* p1, const T* p2, size_t n)
   {
   volatile T difference = 0;

   for(size_t i = 0; i != n; ++i)
      difference |= (p1[i] ^ p2[i]);

   return difference == 0;
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ out[i] forall i = 0...length
* @param out the input/output buffer
* @param in the read-only input buffer
* @param length the length of the buffers
*/
template<typename T>
void xor_buf(T out[], const T in[], size_t length)
   {
   while(length >= 8)
      {
      out[0] ^= in[0]; out[1] ^= in[1];
      out[2] ^= in[2]; out[3] ^= in[3];
      out[4] ^= in[4]; out[5] ^= in[5];
      out[6] ^= in[6]; out[7] ^= in[7];

      out += 8; in += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] ^= in[i];
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ in2[i] forall i = 0...length
* @param out the output buffer
* @param in the first input buffer
* @param in2 the second output buffer
* @param length the length of the three buffers
*/
template<typename T> void xor_buf(T out[],
                                  const T in[],
                                  const T in2[],
                                  size_t length)
   {
   while(length >= 8)
      {
      out[0] = in[0] ^ in2[0];
      out[1] = in[1] ^ in2[1];
      out[2] = in[2] ^ in2[2];
      out[3] = in[3] ^ in2[3];
      out[4] = in[4] ^ in2[4];
      out[5] = in[5] ^ in2[5];
      out[6] = in[6] ^ in2[6];
      out[7] = in[7] ^ in2[7];

      in += 8; in2 += 8; out += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] = in[i] ^ in2[i];
   }

#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK

template<>
inline void xor_buf<byte>(byte out[], const byte in[], size_t length)
   {
   while(length >= 8)
      {
      *reinterpret_cast<u64bit*>(out) ^= *reinterpret_cast<const u64bit*>(in);
      out += 8; in += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] ^= in[i];
   }

template<>
inline void xor_buf<byte>(byte out[],
                          const byte in[],
                          const byte in2[],
                          size_t length)
   {
   while(length >= 8)
      {
      *reinterpret_cast<u64bit*>(out) =
         *reinterpret_cast<const u64bit*>(in) ^
         *reinterpret_cast<const u64bit*>(in2);

      in += 8; in2 += 8; out += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] = in[i] ^ in2[i];
   }

#endif

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const std::vector<byte, Alloc2>& in,
             size_t n)
   {
   xor_buf(out.data(), in.data(), n);
   }

template<typename Alloc>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             size_t n)
   {
   xor_buf(out.data(), in, n);
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             const std::vector<byte, Alloc2>& in2,
             size_t n)
   {
   xor_buf(out.data(), in, in2.data(), n);
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator^=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   if(out.size() < in.size())
      out.resize(in.size());

   xor_buf(out.data(), in.data(), in.size());
   return out;
   }

}


#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)

namespace Botan {

class BOTAN_DLL mlock_allocator
   {
   public:
      static mlock_allocator& instance();

      void* allocate(size_t num_elems, size_t elem_size);

      bool deallocate(void* p, size_t num_elems, size_t elem_size);

      mlock_allocator(const mlock_allocator&) = delete;

      mlock_allocator& operator=(const mlock_allocator&) = delete;

   private:
      mlock_allocator();

      ~mlock_allocator();

      const size_t m_poolsize;

      std::mutex m_mutex;
      std::vector<std::pair<size_t, size_t>> m_freelist;
      byte* m_pool;
   };

}

#endif

namespace Botan {

template<typename T>
class secure_allocator
   {
   public:
      typedef T          value_type;

      typedef T*         pointer;
      typedef const T*   const_pointer;

      typedef T&         reference;
      typedef const T&   const_reference;

      typedef std::size_t     size_type;
      typedef std::ptrdiff_t  difference_type;

      secure_allocator() BOTAN_NOEXCEPT {}

      template<typename U>
      secure_allocator(const secure_allocator<U>&) BOTAN_NOEXCEPT {}

      ~secure_allocator() BOTAN_NOEXCEPT {}

      pointer address(reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      const_pointer address(const_reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      pointer allocate(size_type n, const void* = 0)
         {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(pointer p = static_cast<pointer>(mlock_allocator::instance().allocate(n, sizeof(T))))
            return p;
#endif

         pointer p = new T[n];
         clear_mem(p, n);
         return p;
         }

      void deallocate(pointer p, size_type n)
         {
         zero_mem(p, n);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(mlock_allocator::instance().deallocate(p, n, sizeof(T)))
            return;
#endif

         delete [] p;
         }

      size_type max_size() const BOTAN_NOEXCEPT
         {
         return static_cast<size_type>(-1) / sizeof(T);
         }

      template<typename U, typename... Args>
      void construct(U* p, Args&&... args)
         {
         ::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
         }

      template<typename U> void destroy(U* p) { p->~U(); }
   };

template<typename T, typename U> inline bool
operator==(const secure_allocator<T>&, const secure_allocator<U>&)
   { return true; }

template<typename T, typename U> inline bool
operator!=(const secure_allocator<T>&, const secure_allocator<U>&)
   { return false; }

template<typename T> using secure_vector = std::vector<T, secure_allocator<T>>;
template<typename T> using secure_deque = std::deque<T, secure_allocator<T>>;

template<typename T>
std::vector<T> unlock(const secure_vector<T>& in)
   {
   std::vector<T> out(in.size());
   copy_mem(out.data(), in.data(), in.size());
   return out;
   }

template<typename T, typename Alloc>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const T input[],
                     size_t input_length)
   {
   const size_t to_copy = std::min(input_length, buf.size() - buf_offset);
   if (to_copy > 0)
      {
      copy_mem(&buf[buf_offset], input, to_copy);
      }
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const std::vector<T, Alloc2>& input)
   {
   const size_t to_copy = std::min(input.size(), buf.size() - buf_offset);
   if (to_copy > 0)
      {
      copy_mem(&buf[buf_offset], input.data(), to_copy);
      }
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator+=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.size());
   if (in.size() > 0)
      {
      copy_mem(&out[copy_offset], in.data(), in.size());
      }
   return out;
   }

template<typename T, typename Alloc>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out, T in)
   {
   out.push_back(in);
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<const T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   if (in.second > 0)
      {
      copy_mem(&out[copy_offset], in.first, in.second);
      }
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   if (in.second > 0)
      {
      copy_mem(&out[copy_offset], in.first, in.second);
      }
   return out;
   }

/**
* Zeroise the values; length remains unchanged
* @param vec the vector to zeroise
*/
template<typename T, typename Alloc>
void zeroise(std::vector<T, Alloc>& vec)
   {
   clear_mem(vec.data(), vec.size());
   }

/**
* Zeroise the values then free the memory
* @param vec the vector to zeroise and free
*/
template<typename T, typename Alloc>
void zap(std::vector<T, Alloc>& vec)
   {
   zeroise(vec);
   vec.clear();
   vec.shrink_to_fit();
   }

}


namespace Botan {

/**
* Represents the length requirements on an algorithm key
*/
class BOTAN_DLL Key_Length_Specification
   {
   public:
      /**
      * Constructor for fixed length keys
      * @param keylen the supported key length
      */
      Key_Length_Specification(size_t keylen) :
         min_keylen(keylen),
         max_keylen(keylen),
         keylen_mod(1)
         {
         }

      /**
      * Constructor for variable length keys
      * @param min_k the smallest supported key length
      * @param max_k the largest supported key length
      * @param k_mod the number of bytes the key must be a multiple of
      */
      Key_Length_Specification(size_t min_k,
                               size_t max_k,
                               size_t k_mod = 1) :
         min_keylen(min_k),
         max_keylen(max_k ? max_k : min_k),
         keylen_mod(k_mod)
         {
         }

      /**
      * @param length is a key length in bytes
      * @return true iff this length is a valid length for this algo
      */
      bool valid_keylength(size_t length) const
         {
         return ((length >= min_keylen) &&
                 (length <= max_keylen) &&
                 (length % keylen_mod == 0));
         }

      /**
      * @return minimum key length in bytes
      */
      size_t minimum_keylength() const
         {
         return min_keylen;
         }

      /**
      * @return maximum key length in bytes
      */
      size_t maximum_keylength() const
         {
         return max_keylen;
         }

      /**
      * @return key length multiple in bytes
      */
      size_t keylength_multiple() const
         {
         return keylen_mod;
         }

      Key_Length_Specification multiple(size_t n) const
         {
         return Key_Length_Specification(n * min_keylen,
                                         n * max_keylen,
                                         n * keylen_mod);
         }

   private:
      size_t min_keylen, max_keylen, keylen_mod;
   };

}



namespace Botan {

/**
* Parse a SCAN-style algorithm name
* @param scan_name the name
* @return the name components
*/
BOTAN_DLL std::vector<std::string>
parse_algorithm_name(const std::string& scan_name);

/**
* Split a string
* @param str the input string
* @param delim the delimitor
* @return string split by delim
*/
BOTAN_DLL std::vector<std::string> split_on(
   const std::string& str, char delim);

/**
* Split a string on a character predicate
* @param str the input string
*/
BOTAN_DLL std::vector<std::string>
split_on_pred(const std::string& str,
              std::function<bool (char)> pred);

/**
* Erase characters from a string
*/
BOTAN_DLL std::string erase_chars(const std::string& str, const std::set<char>& chars);

/**
* Replace a character in a string
* @param str the input string
* @param from_char the character to replace
* @param to_char the character to replace it with
* @return str with all instances of from_char replaced by to_char
*/
BOTAN_DLL std::string replace_char(const std::string& str,
                                   char from_char,
                                   char to_char);

/**
* Replace a character in a string
* @param str the input string
* @param from_chars the characters to replace
* @param to_char the character to replace it with
* @return str with all instances of from_chars replaced by to_char
*/
BOTAN_DLL std::string replace_chars(const std::string& str,
                                    const std::set<char>& from_chars,
                                    char to_char);

/**
* Join a string
* @param strs strings to join
* @param delim the delimitor
* @return string joined by delim
*/
BOTAN_DLL std::string string_join(const std::vector<std::string>& strs,
                                  char delim);

/**
* Parse an ASN.1 OID
* @param oid the OID in string form
* @return OID components
*/
BOTAN_DLL std::vector<u32bit> parse_asn1_oid(const std::string& oid);

/**
* Compare two names using the X.509 comparison algorithm
* @param name1 the first name
* @param name2 the second name
* @return true if name1 is the same as name2 by the X.509 comparison rules
*/
BOTAN_DLL bool x500_name_cmp(const std::string& name1,
                             const std::string& name2);

/**
* Convert a string to a number
* @param str the string to convert
* @return number value of the string
*/
BOTAN_DLL u32bit to_u32bit(const std::string& str);

/**
* Convert a time specification to a number
* @param timespec the time specification
* @return number of seconds represented by timespec
*/
BOTAN_DLL u32bit timespec_to_u32bit(const std::string& timespec);

/**
* Convert a string representation of an IPv4 address to a number
* @param ip_str the string representation
* @return integer IPv4 address
*/
BOTAN_DLL u32bit string_to_ipv4(const std::string& ip_str);

/**
* Convert an IPv4 address to a string
* @param ip_addr the IPv4 address to convert
* @return string representation of the IPv4 address
*/
BOTAN_DLL std::string ipv4_to_string(u32bit ip_addr);

std::map<std::string, std::string> BOTAN_DLL read_cfg(std::istream& is);

std::string BOTAN_DLL clean_ws(const std::string& s);

bool BOTAN_DLL host_wildcard_match(const std::string& wildcard, const std::string& host);


}


namespace Botan {

typedef std::runtime_error Exception;
typedef std::invalid_argument Invalid_Argument;

/**
* Unsupported_Argument Exception
*
* An argument that is invalid because it is not supported by Botan.
* It might or might not be valid in another context like a standard.
*/
struct BOTAN_DLL Unsupported_Argument : public Invalid_Argument
   {
   Unsupported_Argument(const std::string& msg) : Invalid_Argument(msg) {}
   };

/**
* Invalid_State Exception
*/
struct BOTAN_DLL Invalid_State : public Exception
   {
   Invalid_State(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Lookup_Error Exception
*/
struct BOTAN_DLL Lookup_Error : public Exception
   {
   Lookup_Error(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Internal_Error Exception
*/
struct BOTAN_DLL Internal_Error : public Exception
   {
   Internal_Error(const std::string& err) :
      Exception("Internal error: " + err)
      {}
   };

/**
* Invalid_Key_Length Exception
*/
struct BOTAN_DLL Invalid_Key_Length : public Invalid_Argument
   {
   Invalid_Key_Length(const std::string& name, size_t length) :
      Invalid_Argument(name + " cannot accept a key of length " +
                       std::to_string(length))
      {}
   };

/**
* Invalid_IV_Length Exception
*/
struct BOTAN_DLL Invalid_IV_Length : public Invalid_Argument
   {
   Invalid_IV_Length(const std::string& mode, size_t bad_len) :
      Invalid_Argument("IV length " + std::to_string(bad_len) +
                       " is invalid for " + mode)
      {}
   };

/**
* PRNG_Unseeded Exception
*/
struct BOTAN_DLL PRNG_Unseeded : public Invalid_State
   {
   PRNG_Unseeded(const std::string& algo) :
      Invalid_State("PRNG not seeded: " + algo)
      {}
   };

/**
* Policy_Violation Exception
*/
struct BOTAN_DLL Policy_Violation : public Invalid_State
   {
   Policy_Violation(const std::string& err) :
      Invalid_State("Policy violation: " + err)
      {}
   };

/**
* Algorithm_Not_Found Exception
*/
struct BOTAN_DLL Algorithm_Not_Found : public Lookup_Error
   {
   Algorithm_Not_Found(const std::string& name) :
      Lookup_Error("Could not find any algorithm named \"" + name + "\"")
      {}
   };

/**
* No_Provider_Found Exception
*/
struct BOTAN_DLL No_Provider_Found : public Exception
   {
   No_Provider_Found(const std::string& name) :
      Exception("Could not find any provider for algorithm named \"" + name + "\"")
      {}
   };

/**
* Invalid_Algorithm_Name Exception
*/
struct BOTAN_DLL Invalid_Algorithm_Name : public Invalid_Argument
   {
   Invalid_Algorithm_Name(const std::string& name):
      Invalid_Argument("Invalid algorithm name: " + name)
      {}
   };

/**
* Encoding_Error Exception
*/
struct BOTAN_DLL Encoding_Error : public Invalid_Argument
   {
   Encoding_Error(const std::string& name) :
      Invalid_Argument("Encoding error: " + name) {}
   };

/**
* Decoding_Error Exception
*/
struct BOTAN_DLL Decoding_Error : public Invalid_Argument
   {
   Decoding_Error(const std::string& name) :
      Invalid_Argument("Decoding error: " + name) {}
   };

/**
* Integrity_Failure Exception
*/
struct BOTAN_DLL Integrity_Failure : public Exception
   {
   Integrity_Failure(const std::string& msg) :
      Exception("Integrity failure: " + msg) {}
   };

/**
* Invalid_OID Exception
*/
struct BOTAN_DLL Invalid_OID : public Decoding_Error
   {
   Invalid_OID(const std::string& oid) :
      Decoding_Error("Invalid ASN.1 OID: " + oid) {}
   };

/**
* Stream_IO_Error Exception
*/
struct BOTAN_DLL Stream_IO_Error : public Exception
   {
   Stream_IO_Error(const std::string& err) :
      Exception("I/O error: " + err)
      {}
   };

/**
* No_Filesystem_Access Exception
*/
struct BOTAN_DLL No_Filesystem_Access : public Exception
   {
   No_Filesystem_Access() : Exception("No filesystem access enabled.") {}
   };

/**
* Self Test Failure Exception
*/
struct BOTAN_DLL Self_Test_Failure : public Internal_Error
   {
   Self_Test_Failure(const std::string& err) :
      Internal_Error("Self test failed: " + err)
      {}
   };

/**
* Memory Allocation Exception
*/
struct BOTAN_DLL Memory_Exhaustion : public std::bad_alloc
   {
   const char* what() const BOTAN_NOEXCEPT override
      { return "Ran out of memory, allocation failed"; }
   };

}


namespace Botan {

/**
* Octet String
*/
class BOTAN_DLL OctetString
   {
   public:
      /**
      * @return size of this octet string in bytes
      */
      size_t length() const { return bits.size(); }
      size_t size() const { return bits.size(); }

      /**
      * @return this object as a secure_vector<byte>
      */
      secure_vector<byte> bits_of() const { return bits; }

      /**
      * @return start of this string
      */
      const byte* begin() const { return bits.data(); }

      /**
      * @return end of this string
      */
      const byte* end() const   { return begin() + bits.size(); }

      /**
      * @return this encoded as hex
      */
      std::string as_string() const;

      /**
      * XOR the contents of another octet string into this one
      * @param other octet string
      * @return reference to this
      */
      OctetString& operator^=(const OctetString& other);

      /**
      * Force to have odd parity
      */
      void set_odd_parity();

      /**
      * Create a new OctetString
      * @param str is a hex encoded string
      */
      OctetString(const std::string& str = "");

      /**
      * Create a new random OctetString
      * @param rng is a random number generator
      * @param len is the desired length in bytes
      */
      OctetString(class RandomNumberGenerator& rng, size_t len);

      /**
      * Create a new OctetString
      * @param in is an array
      * @param len is the length of in in bytes
      */
      OctetString(const byte in[], size_t len);

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const secure_vector<byte>& in) : bits(in) {}

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const std::vector<byte>& in) : bits(in.begin(), in.end()) {}
   private:
      secure_vector<byte> bits;
   };

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is equal to y
*/
BOTAN_DLL bool operator==(const OctetString& x,
                          const OctetString& y);

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is not equal to y
*/
BOTAN_DLL bool operator!=(const OctetString& x,
                          const OctetString& y);

/**
* Concatenate two strings
* @param x an octet string
* @param y an octet string
* @return x concatenated with y
*/
BOTAN_DLL OctetString operator+(const OctetString& x,
                                const OctetString& y);

/**
* XOR two strings
* @param x an octet string
* @param y an octet string
* @return x XORed with y
*/
BOTAN_DLL OctetString operator^(const OctetString& x,
                                const OctetString& y);


/**
* Alternate name for octet string showing intent to use as a key
*/
typedef OctetString SymmetricKey;

/**
* Alternate name for octet string showing intent to use as an IV
*/
typedef OctetString InitializationVector;

}


namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class BOTAN_DLL SCAN_Name
   {
   public:
      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(const char* algo_spec);

      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(std::string algo_spec);

      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(std::string algo_spec, const std::string& extra);

      /**
      * @return original input string
      */
      const std::string& as_string() const { return orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string& algo_name() const { return alg_name; }

      /**
      * @return algorithm name plus any arguments
      */
      std::string algo_name_and_args() const { return algo_name() + all_arguments(); }

      /**
      * @return all arguments
      */
      std::string all_arguments() const;

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const
         { return ((arg_count() >= lower) && (arg_count() <= upper)); }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, const std::string& def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const
         { return (mode_info.size() >= 1) ? mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const
         { return (mode_info.size() >= 2) ? mode_info[1] : ""; }

      static void add_alias(const std::string& alias, const std::string& basename);

      static std::string deref_alias(const std::string& alias);
   private:
      static std::mutex g_alias_map_mutex;
      static std::map<std::string, std::string> g_alias_map;

      std::string orig_algo_spec;
      std::string alg_name;
      std::vector<std::string> args;
      std::vector<std::string> mode_info;
   };

}


namespace Botan {

/**
* Interface for general transformations on data
*/
class BOTAN_DLL Transform
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      secure_vector<byte> start(const std::vector<byte, Alloc>& nonce)
         {
         return start(nonce.data(), nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      BOTAN_DEPRECATED("Use Transform::start")
      secure_vector<byte> start_vec(const std::vector<byte, Alloc>& nonce)
         {
         return start(nonce.data(), nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      * @param nonce_len length of nonce
      */
      secure_vector<byte> start(const byte nonce[], size_t nonce_len)
         {
         return start_raw(nonce, nonce_len);
         }

      /**
      * Begin processing a message.
      */
      secure_vector<byte> start()
         {
         return start_raw(nullptr, 0);
         }

      virtual secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) = 0;

      /**
      * Process some data. Input must be in size update_granularity() byte blocks.
      * @param blocks in/out parameter which will possibly be resized
      * @param offset an offset into blocks to begin processing
      */
      virtual void update(secure_vector<byte>& blocks, size_t offset = 0) = 0;

      /**
      * Complete processing of a message.
      *
      * @param final_block in/out parameter which must be at least
      *        minimum_final_size() bytes, and will be set to any final output
      * @param offset an offset into final_block to begin processing
      */
      virtual void finish(secure_vector<byte>& final_block, size_t offset = 0) = 0;

      /**
      * Returns the size of the output if this transform is used to process a
      * message with input_length bytes. Will throw if unable to give a precise
      * answer.
      */
      virtual size_t output_length(size_t input_length) const = 0;

      /**
      * @return size of required blocks to update
      */
      virtual size_t update_granularity() const = 0;

      /**
      * @return required minimium size to finalize() - may be any
      *         length larger than this.
      */
      virtual size_t minimum_final_size() const = 0;

      /**
      * Return the default size for a nonce
      */
      virtual size_t default_nonce_length() const = 0;

      /**
      * Return true iff nonce_len is a valid length for the nonce
      */
      virtual bool valid_nonce_length(size_t nonce_len) const = 0;

      /**
      * Return some short name describing the provider of this tranformation.
      * Useful in cases where multiple implementations are available (eg,
      * different implementations of AES). Default "core" is used for the
      * 'standard' implementation included in the library.
      */
      virtual std::string provider() const { return "core"; }

      virtual std::string name() const = 0;

      virtual void clear() = 0;

      virtual ~Transform() {}
   };

class BOTAN_DLL Keyed_Transform : public Transform
   {
   public:
      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      template<typename Alloc>
      void set_key(const std::vector<byte, Alloc>& key)
         {
         set_key(key.data(), key.size());
         }

      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      /**
      * Set the symmetric key of this transform
      * @param key contains the key material
      * @param length in bytes of key param
      */
      void set_key(const byte key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

   private:
      virtual void key_schedule(const byte key[], size_t length) = 0;
   };

typedef Transform Transformation;

BOTAN_DLL Transform* get_transform(const std::string& specstr,
                                   const std::string& provider = "",
                                   const std::string& dirstr = "");

}


namespace Botan {

/**
* This class represents a symmetric algorithm object.
*/
class BOTAN_DLL SymmetricAlgorithm
   {
   public:
      virtual ~SymmetricAlgorithm() {}

      virtual void clear() = 0;

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * @return minimum allowed key length
      */
      size_t maximum_keylength() const
         {
         return key_spec().maximum_keylength();
         }

      /**
      * @return maxmium allowed key length
      */
      size_t minimum_keylength() const
         {
         return key_spec().minimum_keylength();
         }

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * Set the symmetric key of this object.
      * @param key the SymmetricKey to be set.
      */
      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      template<typename Alloc>
      void set_key(const std::vector<byte, Alloc>& key)
         {
         set_key(key.data(), key.size());
         }

      /**
      * Set the symmetric key of this object.
      * @param key the to be set as a byte array.
      * @param length in bytes of key param
      */
      void set_key(const byte key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

      virtual std::string name() const = 0;

   private:
      /**
      * Run the key schedule
      * @param key the key
      * @param length of key
      */
      virtual void key_schedule(const byte key[], size_t length) = 0;
   };

}


namespace Botan {

/**
* Base class for all stream ciphers
*/
class BOTAN_DLL StreamCipher : public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<StreamCipher> create(const std::string& algo_spec,
                                                  const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * Encrypt or decrypt a message
      * @param in the plaintext
      * @param out the byte array to hold the output, i.e. the ciphertext
      * @param len the length of both in and out in bytes
      */
      virtual void cipher(const byte in[], byte out[], size_t len) = 0;

      /**
      * Encrypt or decrypt a message
      * @param buf the plaintext / ciphertext
      * @param len the length of buf in bytes
      */
      void cipher1(byte buf[], size_t len)
         { cipher(buf, buf, len); }

      template<typename Alloc>
         void encipher(std::vector<byte, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      template<typename Alloc>
         void encrypt(std::vector<byte, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      template<typename Alloc>
         void decrypt(std::vector<byte, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      /**
      * Resync the cipher using the IV
      * @param iv the initialization vector
      * @param iv_len the length of the IV in bytes
      */
      virtual void set_iv(const byte[], size_t iv_len);

      /**
      * @param iv_len the length of the IV in bytes
      * @return if the length is valid for this algorithm
      */
      virtual bool valid_iv_length(size_t iv_len) const { return (iv_len == 0); }

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual StreamCipher* clone() const = 0;

      StreamCipher();
      virtual ~StreamCipher();
   };

}


namespace Botan {

/**
* Interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode : public Keyed_Transform
   {
   public:
      /**
      * Returns true iff this mode provides authentication as well as
      * confidentiality.
      */
      virtual bool authenticated() const { return false; }

      /**
      * Return the size of the authentication tag used (in bytes)
      */
      virtual size_t tag_size() const { return 0; }
   };

/**
* The two possible directions for cipher filters, determining whether they
* actually perform encryption or decryption.
*/
enum Cipher_Dir { ENCRYPTION, DECRYPTION };

BOTAN_DLL Cipher_Mode* get_cipher_mode(const std::string& algo_spec, Cipher_Dir direction);

}


namespace Botan {

/**
* Interface for AEAD (Authenticated Encryption with Associated Data)
* modes. These modes provide both encryption and message
* authentication, and can authenticate additional per-message data
* which is not included in the ciphertext (for instance a sequence
* number).
*/
class BOTAN_DLL AEAD_Mode : public Cipher_Mode
   {
   public:
      bool authenticated() const override { return true; }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key and
      * before start.
      *
      * Unless reset by another call, the associated data is kept
      * between messages. Thus, if the AD does not change, calling
      * once (after set_key) is the optimum.
      *
      * @param ad the associated data
      * @param ad_len length of add in bytes
      */
      virtual void set_associated_data(const byte ad[], size_t ad_len) = 0;

      template<typename Alloc>
      void set_associated_data_vec(const std::vector<byte, Alloc>& ad)
         {
         set_associated_data(ad.data(), ad.size());
         }

      template<typename Alloc>
      void set_ad(const std::vector<byte, Alloc>& ad)
         {
         set_associated_data(ad.data(), ad.size());
         }

      /**
      * Default AEAD nonce size (a commonly supported value among AEAD
      * modes, and large enough that random collisions are unlikely).
      */
      size_t default_nonce_length() const override { return 12; }

      virtual ~AEAD_Mode();
   };

/**
* Get an AEAD mode by name (eg "AES-128/GCM" or "Serpent/EAX")
*/
BOTAN_DLL AEAD_Mode* get_aead(const std::string& name, Cipher_Dir direction);

}


namespace Botan {

/**
* This class represents a block cipher object.
*/
class BOTAN_DLL BlockCipher : public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<BlockCipher> create(const std::string& algo_spec,
                                                 const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return block size of this algorithm
      */
      virtual size_t block_size() const = 0;

      /**
      * @return native parallelism of this cipher in blocks
      */
      virtual size_t parallelism() const { return 1; }

      /**
      * @return prefererred parallelism of this cipher in bytes
      */
      size_t parallel_bytes() const
         {
         return parallelism() * block_size() * BOTAN_BLOCK_CIPHER_PAR_MULT;
         }

      /**
      * Encrypt a block.
      * @param in The plaintext block to be encrypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the encrypted block.
      * Must be of length block_size().
      */
      void encrypt(const byte in[], byte out[]) const
         { encrypt_n(in, out, 1); }

      /**
      * Decrypt a block.
      * @param in The ciphertext block to be decypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the decrypted block.
      * Must be of length block_size().
      */
      void decrypt(const byte in[], byte out[]) const
         { decrypt_n(in, out, 1); }

      /**
      * Encrypt a block.
      * @param block the plaintext block to be encrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void encrypt(byte block[]) const { encrypt_n(block, block, 1); }

      /**
      * Decrypt a block.
      * @param block the ciphertext block to be decrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void decrypt(byte block[]) const { decrypt_n(block, block, 1); }

      /**
      * Encrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void encrypt(std::vector<byte, Alloc>& block) const
         {
         return encrypt_n(block.data(), block.data(), block.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void decrypt(std::vector<byte, Alloc>& block) const
         {
         return decrypt_n(block.data(), block.data(), block.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void encrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return encrypt_n(in.data(), out.data(), in.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void decrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return decrypt_n(in.data(), out.data(), in.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void encrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void decrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual BlockCipher* clone() const = 0;

      virtual ~BlockCipher();
   };

/**
* Represents a block cipher with a single fixed block size
*/
template<size_t BS, size_t KMIN, size_t KMAX = 0, size_t KMOD = 1>
class Block_Cipher_Fixed_Params : public BlockCipher
   {
   public:
      enum { BLOCK_SIZE = BS };
      size_t block_size() const override { return BS; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(KMIN, KMAX, KMOD);
         }
   };

}


namespace Botan {

/**
* AES-128
*/
class BOTAN_DLL AES_128 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-128"; }
      BlockCipher* clone() const override { return new AES_128; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

/**
* AES-192
*/
class BOTAN_DLL AES_192 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-192"; }
      BlockCipher* clone() const override { return new AES_192; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

/**
* AES-256
*/
class BOTAN_DLL AES_256 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-256"; }
      BlockCipher* clone() const override { return new AES_256; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

}


namespace Botan {

/**
* Class used to accumulate the poll results of EntropySources
*/
class BOTAN_DLL Entropy_Accumulator
   {
   public:
      /**
      * Initialize an Entropy_Accumulator
      *
      * @param accum will be called with poll results, first params the data and
      * length, the second a best estimate of min-entropy for the entire buffer;
      * out of an abundance of caution this will be zero for many sources.
      * accum should return true if it wants the polling to stop, though it may
      * still be called again a few more times, and should be careful to return
      * true then as well.
      */
      Entropy_Accumulator(std::function<bool (const byte[], size_t, double)> accum) :
         m_accum_fn(accum) {}

      virtual ~Entropy_Accumulator() {}

      /**
      * @return if our polling goal has been achieved
      */
      bool polling_goal_achieved() const { return m_done; }

      bool polling_finished() const { return m_done; }

      /**
      * Add entropy to the accumulator
      * @param bytes the input bytes
      * @param length specifies how many bytes the input is
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      void add(const void* bytes, size_t length, double entropy_bits_per_byte)
         {
         m_done = m_accum_fn(reinterpret_cast<const byte*>(bytes),
                             length, entropy_bits_per_byte * length) || m_done;
         }

      /**
      * Add entropy to the accumulator
      * @param v is some value
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      template<typename T>
      void add(const T& v, double entropy_bits_per_byte)
         {
         add(&v, sizeof(T), entropy_bits_per_byte);
         }
   private:
      std::function<bool (const byte[], size_t, double)> m_accum_fn;
      bool m_done = false;
   };

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL EntropySource
   {
   public:
      static void poll_available_sources(class Entropy_Accumulator& accum);

      /**
      * @return name identifying this entropy source
      */
      virtual std::string name() const = 0;

      /**
      * Perform an entropy gathering poll
      * @param accum is an accumulator object that will be given entropy
      */
      virtual void poll(Entropy_Accumulator& accum) = 0;

      virtual ~EntropySource() {}
   };

}


namespace Botan {

/**
* This class represents a random number (RNG) generator object.
*/
class BOTAN_DLL RandomNumberGenerator
   {
   public:
      /**
      * Create a seeded and active RNG object for general application use
      * Added in 1.8.0
      */
      static RandomNumberGenerator* make_rng();

      /**
      * Randomize a byte array.
      * @param output the byte array to hold the random output.
      * @param length the length of the byte array output.
      */
      virtual void randomize(byte output[], size_t length) = 0;

      /**
      * Return a random vector
      * @param bytes number of bytes in the result
      * @return randomized vector of length bytes
      */
      virtual secure_vector<byte> random_vec(size_t bytes)
         {
         secure_vector<byte> output(bytes);
         randomize(output.data(), output.size());
         return output;
         }

      /**
      * Only usable with POD types, only useful with integers
      * get_random<u64bit>()
      */
      template<typename T> T get_random()
         {
         T r;
         this->randomize(reinterpret_cast<byte*>(&r), sizeof(r));
         return r;
         }

      /**
      * Return a value in range [0,2^bits)
      */
      u64bit gen_mask(size_t bits)
         {
         if(bits == 0 || bits > 64)
            throw std::invalid_argument("RandomNumberGenerator::gen_mask invalid argument");

         const u64bit mask = ((1 << bits) - 1);
         return this->get_random<u64bit>() & mask;
         }

      /**
      * Return a random byte
      * @return random byte
      */
      byte next_byte() { return get_random<byte>(); }

      byte next_nonzero_byte()
         {
         byte b = next_byte();
         while(b == 0)
            b = next_byte();
         return b;
         }

      /**
      * Check whether this RNG is seeded.
      * @return true if this RNG was already seeded, false otherwise.
      */
      virtual bool is_seeded() const = 0;

      /**
      * Clear all internally held values of this RNG.
      */
      virtual void clear() = 0;

      /**
      * Return the name of this object
      */
      virtual std::string name() const = 0;

      /**
      * Seed this RNG using the entropy sources it contains.
      * @param bits_to_collect is the number of bits of entropy to
               attempt to gather from the entropy sources
      */
      virtual void reseed(size_t bits_to_collect) = 0;

      /**
      * Add entropy to this RNG.
      * @param in a byte array containg the entropy to be added
      * @param length the length of the byte array in
      */
      virtual void add_entropy(const byte in[], size_t length) = 0;

      /*
      * Never copy a RNG, create a new one
      */
      RandomNumberGenerator(const RandomNumberGenerator& rng) = delete;
      RandomNumberGenerator& operator=(const RandomNumberGenerator& rng) = delete;

      RandomNumberGenerator() {}
      virtual ~RandomNumberGenerator() {}
   };

/**
* Null/stub RNG - fails if you try to use it for anything
*/
class BOTAN_DLL Null_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte[], size_t) override { throw PRNG_Unseeded("Null_RNG"); }

      void clear() override {}

      std::string name() const override { return "Null_RNG"; }

      void reseed(size_t) override {}
      bool is_seeded() const override { return false; }
      void add_entropy(const byte[], size_t) override {}
   };

/**
* Wraps access to a RNG in a mutex
*/
class BOTAN_DLL Serialized_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->randomize(out, len);
         }

      bool is_seeded() const override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->is_seeded();
         }

      void clear() override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->clear();
         }

      std::string name() const override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->name();
         }

      void reseed(size_t poll_bits) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->reseed(poll_bits);
         }

      void add_entropy(const byte in[], size_t len) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->add_entropy(in, len);
         }

      Serialized_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      mutable std::mutex m_mutex;
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}


namespace Botan {

class BOTAN_DLL AutoSeeded_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len) override
         { m_rng->randomize(out, len); }

      bool is_seeded() const override { return m_rng->is_seeded(); }

      void clear() override { m_rng->clear(); }

      std::string name() const override { return m_rng->name(); }

      void reseed(size_t poll_bits = 256) override { m_rng->reseed(poll_bits); }

      void add_entropy(const byte in[], size_t len) override
         { m_rng->add_entropy(in, len); }

      AutoSeeded_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}


namespace Botan {

/**
* Perform base64 encoding
* @param output an array of at least input_length*4/3 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_encode(char output[],
                               const byte input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs);

/**
* Perform base64 encoding
* @param input some input
* @param input_length length of input in bytes
* @return base64adecimal representation of input
*/
std::string BOTAN_DLL base64_encode(const byte input[],
                                    size_t input_length);

/**
* Perform base64 encoding
* @param input some input
* @return base64adecimal representation of input
*/
template<typename Alloc>
std::string base64_encode(const std::vector<byte, Alloc>& input)
   {
   return base64_encode(input.data(), input.size());
   }

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const char input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const char input[],
                               size_t input_length,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length/3*4 bytes
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const std::string& input,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<byte> BOTAN_DLL base64_decode(const char input[],
                                           size_t input_length,
                                           bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<byte> BOTAN_DLL base64_decode(const std::string& input,
                                           bool ignore_ws = true);

}


namespace Botan {

/**
* This class represents general abstract filter objects.
*/
class BOTAN_DLL Filter
   {
   public:
      /**
      * @return descriptive name for this filter
      */
      virtual std::string name() const = 0;

      /**
      * Write a portion of a message to this filter.
      * @param input the input as a byte array
      * @param length the length of the byte array input
      */
      virtual void write(const byte input[], size_t length) = 0;

      /**
      * Start a new message. Must be closed by end_msg() before another
      * message can be started.
      */
      virtual void start_msg() {}

      /**
      * Notify that the current message is finished; flush buffers and
      * do end-of-message processing (if any).
      */
      virtual void end_msg() {}

      /**
      * Check whether this filter is an attachable filter.
      * @return true if this filter is attachable, false otherwise
      */
      virtual bool attachable() { return true; }

      virtual ~Filter() {}
   protected:
      /**
      * @param in some input for the filter
      * @param length the length of in
      */
      virtual void send(const byte in[], size_t length);

      /**
      * @param in some input for the filter
      */
      void send(byte in) { send(&in, 1); }

      /**
      * @param in some input for the filter
      */
      void send(const secure_vector<byte>& in) { send(in.data(), in.size()); }

      /**
      * @param in some input for the filter
      */
      void send(const std::vector<byte>& in) { send(in.data(), in.size()); }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const secure_vector<byte>& in, size_t length)
         {
         send(in.data(), length);
         }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const std::vector<byte>& in, size_t length)
         {
         send(in.data(), length);
         }

      Filter();

      Filter(const Filter&) = delete;

      Filter& operator=(const Filter&) = delete;

   private:
      /**
      * Start a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void new_msg();

      /**
      * End a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void finish_msg();

      friend class Pipe;
      friend class Fanout_Filter;

      size_t total_ports() const;
      size_t current_port() const { return port_num; }

      /**
      * Set the active port
      * @param new_port the new value
      */
      void set_port(size_t new_port);

      size_t owns() const { return filter_owns; }

      /**
      * Attach another filter to this one
      * @param f filter to attach
      */
      void attach(Filter* f);

      /**
      * @param filters the filters to set
      * @param count number of items in filters
      */
      void set_next(Filter* filters[], size_t count);
      Filter* get_next() const;

      secure_vector<byte> write_queue;
      std::vector<Filter*> next;
      size_t port_num, filter_owns;

      // true if filter belongs to a pipe --> prohibit filter sharing!
      bool owned;
   };

/**
* This is the abstract Fanout_Filter base class.
**/
class BOTAN_DLL Fanout_Filter : public Filter
   {
   protected:
      /**
      * Increment the number of filters past us that we own
      */
      void incr_owns() { ++filter_owns; }

      void set_port(size_t n) { Filter::set_port(n); }

      void set_next(Filter* f[], size_t n) { Filter::set_next(f, n); }

      void attach(Filter* f) { Filter::attach(f); }

   private:
      friend class Threaded_Fork;
      using Filter::write_queue;
      using Filter::total_ports;
      using Filter::next;
   };

/**
* The type of checking to be performed by decoders:
* NONE - no checks, IGNORE_WS - perform checks, but ignore
* whitespaces, FULL_CHECK - perform checks, also complain
* about white spaces.
*/
enum Decoder_Checking { NONE, IGNORE_WS, FULL_CHECK };

}


namespace Botan {

/**
* BitBucket is a filter which simply discards all inputs
*/
struct BOTAN_DLL BitBucket : public Filter
   {
   void write(const byte[], size_t) override {}

   std::string name() const override { return "BitBucket"; }
   };

/**
* This class represents Filter chains. A Filter chain is an ordered
* concatenation of Filters, the input to a Chain sequentially passes
* through all the Filters contained in the Chain.
*/

class BOTAN_DLL Chain : public Fanout_Filter
   {
   public:
      void write(const byte input[], size_t length) override { send(input, length); }

      std::string name() const override;

      /**
      * Construct a chain of up to four filters. The filters are set
      * up in the same order as the arguments.
      */
      Chain(Filter* = nullptr, Filter* = nullptr,
            Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a chain from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Chain(Filter* filter_arr[], size_t length);
   };

/**
* This class represents a fork filter, whose purpose is to fork the
* flow of data. It causes an input message to result in n messages at
* the end of the filter, where n is the number of forks.
*/
class BOTAN_DLL Fork : public Fanout_Filter
   {
   public:
      void write(const byte input[], size_t length) override { send(input, length); }
      void set_port(size_t n) { Fanout_Filter::set_port(n); }

      std::string name() const override;

      /**
      * Construct a Fork filter with up to four forks.
      */
      Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Fork(Filter* filter_arr[], size_t length);
   };

/**
* This class is a threaded version of the Fork filter. While this uses
* threads, the class itself is NOT thread-safe. This is meant as a drop-
* in replacement for Fork where performance gains are possible.
*/
class BOTAN_DLL Threaded_Fork : public Fork
   {
   public:
      std::string name() const override;

      /**
      * Construct a Threaded_Fork filter with up to four forks.
      */
      Threaded_Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Threaded_Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Threaded_Fork(Filter* filter_arr[], size_t length);

      ~Threaded_Fork();

   protected:
      void set_next(Filter* f[], size_t n);
      void send(const byte in[], size_t length) override;

   private:
      void thread_delegate_work(const byte input[], size_t length);
      void thread_entry(Filter* filter);

      std::vector<std::shared_ptr<std::thread>> m_threads;
      std::unique_ptr<struct Threaded_Fork_Data> m_thread_data;
   };

}


namespace Botan {

/**
* Bit rotation left
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated left by rot bits
*/
template<typename T> inline T rotate_left(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input << rot) | (input >> (8*sizeof(T)-rot)));;
   }

/**
* Bit rotation right
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated right by rot bits
*/
template<typename T> inline T rotate_right(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input >> rot) | (input << (8*sizeof(T)-rot)));
   }

}


#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)
  #include <emmintrin.h>
#endif

namespace Botan {

/**
* Swap a 16 bit integer
*/
inline u16bit reverse_bytes(u16bit val)
   {
   return rotate_left(val, 8);
   }

/**
* Swap a 32 bit integer
*/
inline u32bit reverse_bytes(u32bit val)
   {
#if BOTAN_GCC_VERSION >= 430 && !defined(BOTAN_TARGET_ARCH_IS_ARM32)
   /*
   GCC intrinsic added in 4.3, works for a number of CPUs

   However avoid under ARM, as it branches to a function in libgcc
   instead of generating inline asm, so slower even than the generic
   rotate version below.
   */
   return __builtin_bswap32(val);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   // GCC-style inline assembly for x86 or x86-64
   asm("bswapl %0" : "=r" (val) : "0" (val));
   return val;

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_ARM32)

   asm ("eor r3, %1, %1, ror #16\n\t"
        "bic r3, r3, #0x00FF0000\n\t"
        "mov %0, %1, ror #8\n\t"
        "eor %0, %0, r3, lsr #8"
        : "=r" (val)
        : "0" (val)
        : "r3", "cc");

   return val;

#elif defined(_MSC_VER) && defined(BOTAN_TARGET_ARCH_IS_X86_32)

   // Visual C++ inline asm for 32-bit x86, by Yves Jerschow
   __asm mov eax, val;
   __asm bswap eax;

#else

   // Generic implementation
   return (rotate_right(val, 8) & 0xFF00FF00) |
          (rotate_left (val, 8) & 0x00FF00FF);

#endif
   }

/**
* Swap a 64 bit integer
*/
inline u64bit reverse_bytes(u64bit val)
   {
#if BOTAN_GCC_VERSION >= 430

   // GCC intrinsic added in 4.3, works for a number of CPUs
   return __builtin_bswap64(val);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_X86_64)
   // GCC-style inline assembly for x86-64
   asm("bswapq %0" : "=r" (val) : "0" (val));
   return val;

#else
   /* Generic implementation. Defined in terms of 32-bit bswap so any
    * optimizations in that version can help here (particularly
    * useful for 32-bit x86).
    */

   u32bit hi = static_cast<u32bit>(val >> 32);
   u32bit lo = static_cast<u32bit>(val);

   hi = reverse_bytes(hi);
   lo = reverse_bytes(lo);

   return (static_cast<u64bit>(lo) << 32) | hi;
#endif
   }

/**
* Swap 4 Ts in an array
*/
template<typename T>
inline void bswap_4(T x[4])
   {
   x[0] = reverse_bytes(x[0]);
   x[1] = reverse_bytes(x[1]);
   x[2] = reverse_bytes(x[2]);
   x[3] = reverse_bytes(x[3]);
   }

#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)

/**
* Swap 4 u32bits in an array using SSE2 shuffle instructions
*/
template<>
inline void bswap_4(u32bit x[4])
   {
   __m128i T = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));

   T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
   T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

   T =  _mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8));

   _mm_storeu_si128(reinterpret_cast<__m128i*>(x), T);
   }

#endif

}


#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)

#define BOTAN_ENDIAN_N2B(x) (x)
#define BOTAN_ENDIAN_B2N(x) (x)

#define BOTAN_ENDIAN_N2L(x) reverse_bytes(x)
#define BOTAN_ENDIAN_L2N(x) reverse_bytes(x)

#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)

#define BOTAN_ENDIAN_N2L(x) (x)
#define BOTAN_ENDIAN_L2N(x) (x)

#define BOTAN_ENDIAN_N2B(x) reverse_bytes(x)
#define BOTAN_ENDIAN_B2N(x) reverse_bytes(x)

#endif

#endif

namespace Botan {

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template<typename T> inline byte get_byte(size_t byte_num, T input)
   {
   return static_cast<byte>(
      input >> ((sizeof(T)-1-(byte_num&(sizeof(T)-1))) << 3)
      );
   }

/**
* Make a u16bit from two bytes
* @param i0 the first byte
* @param i1 the second byte
* @return i0 || i1
*/
inline u16bit make_u16bit(byte i0, byte i1)
   {
   return ((static_cast<u16bit>(i0) << 8) | i1);
   }

/**
* Make a u32bit from four bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @return i0 || i1 || i2 || i3
*/
inline u32bit make_u32bit(byte i0, byte i1, byte i2, byte i3)
   {
   return ((static_cast<u32bit>(i0) << 24) |
           (static_cast<u32bit>(i1) << 16) |
           (static_cast<u32bit>(i2) <<  8) |
           (static_cast<u32bit>(i3)));
   }

/**
* Make a u32bit from eight bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @param i4 the fifth byte
* @param i5 the sixth byte
* @param i6 the seventh byte
* @param i7 the eighth byte
* @return i0 || i1 || i2 || i3 || i4 || i5 || i6 || i7
*/
inline u64bit make_u64bit(byte i0, byte i1, byte i2, byte i3,
                          byte i4, byte i5, byte i6, byte i7)
    {
   return ((static_cast<u64bit>(i0) << 56) |
           (static_cast<u64bit>(i1) << 48) |
           (static_cast<u64bit>(i2) << 40) |
           (static_cast<u64bit>(i3) << 32) |
           (static_cast<u64bit>(i4) << 24) |
           (static_cast<u64bit>(i5) << 16) |
           (static_cast<u64bit>(i6) <<  8) |
           (static_cast<u64bit>(i7)));
    }

/**
* Load a big-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a big-endian value
*/
template<typename T>
inline T load_be(const byte in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[i];
   return out;
   }

/**
* Load a little-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a litte-endian value
*/
template<typename T>
inline T load_le(const byte in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[sizeof(T)-1-i];
   return out;
   }

/**
* Load a big-endian u16bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u16bit of in, as a big-endian value
*/
template<>
inline u16bit load_be<u16bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2B(*(reinterpret_cast<const u16bit*>(in) + off));
#else
   in += off * sizeof(u16bit);
   return make_u16bit(in[0], in[1]);
#endif
   }

/**
* Load a little-endian u16bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u16bit of in, as a little-endian value
*/
template<>
inline u16bit load_le<u16bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2L(*(reinterpret_cast<const u16bit*>(in) + off));
#else
   in += off * sizeof(u16bit);
   return make_u16bit(in[1], in[0]);
#endif
   }

/**
* Load a big-endian u32bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u32bit of in, as a big-endian value
*/
template<>
inline u32bit load_be<u32bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2B(*(reinterpret_cast<const u32bit*>(in) + off));
#else
   in += off * sizeof(u32bit);
   return make_u32bit(in[0], in[1], in[2], in[3]);
#endif
   }

/**
* Load a little-endian u32bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u32bit of in, as a little-endian value
*/
template<>
inline u32bit load_le<u32bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2L(*(reinterpret_cast<const u32bit*>(in) + off));
#else
   in += off * sizeof(u32bit);
   return make_u32bit(in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load a big-endian u64bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u64bit of in, as a big-endian value
*/
template<>
inline u64bit load_be<u64bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2B(*(reinterpret_cast<const u64bit*>(in) + off));
#else
   in += off * sizeof(u64bit);
   return make_u64bit(in[0], in[1], in[2], in[3],
                      in[4], in[5], in[6], in[7]);
#endif
   }

/**
* Load a little-endian u64bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u64bit of in, as a little-endian value
*/
template<>
inline u64bit load_le<u64bit>(const byte in[], size_t off)
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   return BOTAN_ENDIAN_N2L(*(reinterpret_cast<const u64bit*>(in) + off));
#else
   in += off * sizeof(u64bit);
   return make_u64bit(in[7], in[6], in[5], in[4],
                      in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load two little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_le(const byte in[], T& x0, T& x1)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   }

/**
* Load four little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_le(const byte in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   }

/**
* Load eight little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_le(const byte in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   x4 = load_le<T>(in, 4);
   x5 = load_le<T>(in, 5);
   x6 = load_le<T>(in, 6);
   x7 = load_le<T>(in, 7);
   }

/**
* Load a variable number of little-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_le(T out[],
                    const byte in[],
                    size_t count)
   {
#if defined(BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS)
   std::memcpy(out, in, sizeof(T)*count);

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
   const size_t blocks = count - (count % 4);
   const size_t left = count - blocks;

   for(size_t i = 0; i != blocks; i += 4)
      bswap_4(out + i);

   for(size_t i = 0; i != left; ++i)
      out[blocks+i] = reverse_bytes(out[blocks+i]);
#endif

#else
   for(size_t i = 0; i != count; ++i)
      out[i] = load_le<T>(in, i);
#endif
   }

/**
* Load two big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_be(const byte in[], T& x0, T& x1)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   }

/**
* Load four big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_be(const byte in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   }

/**
* Load eight big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_be(const byte in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   x4 = load_be<T>(in, 4);
   x5 = load_be<T>(in, 5);
   x6 = load_be<T>(in, 6);
   x7 = load_be<T>(in, 7);
   }

/**
* Load a variable number of big-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_be(T out[],
                    const byte in[],
                    size_t count)
   {
#if defined(BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS)
   std::memcpy(out, in, sizeof(T)*count);

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
   const size_t blocks = count - (count % 4);
   const size_t left = count - blocks;

   for(size_t i = 0; i != blocks; i += 4)
      bswap_4(out + i);

   for(size_t i = 0; i != left; ++i)
      out[blocks+i] = reverse_bytes(out[blocks+i]);
#endif

#else
   for(size_t i = 0; i != count; ++i)
      out[i] = load_be<T>(in, i);
#endif
   }

/**
* Store a big-endian u16bit
* @param in the input u16bit
* @param out the byte array to write to
*/
inline void store_be(u16bit in, byte out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u16bit*>(out) = BOTAN_ENDIAN_B2N(in);
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
#endif
   }

/**
* Store a little-endian u16bit
* @param in the input u16bit
* @param out the byte array to write to
*/
inline void store_le(u16bit in, byte out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u16bit*>(out) = BOTAN_ENDIAN_L2N(in);
#else
   out[0] = get_byte(1, in);
   out[1] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian u32bit
* @param in the input u32bit
* @param out the byte array to write to
*/
inline void store_be(u32bit in, byte out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u32bit*>(out) = BOTAN_ENDIAN_B2N(in);
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
#endif
   }

/**
* Store a little-endian u32bit
* @param in the input u32bit
* @param out the byte array to write to
*/
inline void store_le(u32bit in, byte out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u32bit*>(out) = BOTAN_ENDIAN_L2N(in);
#else
   out[0] = get_byte(3, in);
   out[1] = get_byte(2, in);
   out[2] = get_byte(1, in);
   out[3] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian u64bit
* @param in the input u64bit
* @param out the byte array to write to
*/
inline void store_be(u64bit in, byte out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u64bit*>(out) = BOTAN_ENDIAN_B2N(in);
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
   out[4] = get_byte(4, in);
   out[5] = get_byte(5, in);
   out[6] = get_byte(6, in);
   out[7] = get_byte(7, in);
#endif
   }

/**
* Store a little-endian u64bit
* @param in the input u64bit
* @param out the byte array to write to
*/
inline void store_le(u64bit in, byte out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   *reinterpret_cast<u64bit*>(out) = BOTAN_ENDIAN_L2N(in);
#else
   out[0] = get_byte(7, in);
   out[1] = get_byte(6, in);
   out[2] = get_byte(5, in);
   out[3] = get_byte(4, in);
   out[4] = get_byte(3, in);
   out[5] = get_byte(2, in);
   out[6] = get_byte(1, in);
   out[7] = get_byte(0, in);
#endif
   }

/**
* Store two little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   }

/**
* Store two big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   }

/**
* Store four little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1, T x2, T x3)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   }

/**
* Store four big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1, T x2, T x3)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   }

/**
* Store eight little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   store_le(x4, out + (4 * sizeof(T)));
   store_le(x5, out + (5 * sizeof(T)));
   store_le(x6, out + (6 * sizeof(T)));
   store_le(x7, out + (7 * sizeof(T)));
   }

/**
* Store eight big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   store_be(x4, out + (4 * sizeof(T)));
   store_be(x5, out + (5 * sizeof(T)));
   store_be(x6, out + (6 * sizeof(T)));
   store_be(x7, out + (7 * sizeof(T)));
   }

template<typename T>
void copy_out_be(byte out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_be(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(i%8, in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_be(byte out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_be(out, out_bytes, in.data());
   }

template<typename T>
void copy_out_le(byte out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_le(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(sizeof(T) - 1 - (i % 8), in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_le(byte out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_le(out, out_bytes, in.data());
   }

}


namespace Botan {

/**
* This class represents any kind of computation which uses an internal
* state, such as hash functions or MACs
*/
class BOTAN_DLL Buffered_Computation
   {
   public:
      /**
      * @return length of the output of this function in bytes
      */
      virtual size_t output_length() const = 0;

      /**
      * Add new input to process.
      * @param in the input to process as a byte array
      * @param length of param in in bytes
      */
      void update(const byte in[], size_t length) { add_data(in, length); }

      /**
      * Add new input to process.
      * @param in the input to process as a secure_vector
      */
      void update(const secure_vector<byte>& in)
         {
         add_data(in.data(), in.size());
         }

      /**
      * Add new input to process.
      * @param in the input to process as a std::vector
      */
      void update(const std::vector<byte>& in)
         {
         add_data(in.data(), in.size());
         }

      /**
      * Add an integer in big-endian order
      * @param in the value
      */
      template<typename T> void update_be(const T in)
         {
         for(size_t i = 0; i != sizeof(T); ++i)
            {
            byte b = get_byte(i, in);
            add_data(&b, 1);
            }
         }

      /**
      * Add new input to process.
      * @param str the input to process as a std::string. Will be interpreted
      * as a byte array based on
      * the strings encoding.
      */
      void update(const std::string& str)
         {
         add_data(reinterpret_cast<const byte*>(str.data()), str.size());
         }

      /**
      * Process a single byte.
      * @param in the byte to process
      */
      void update(byte in) { add_data(&in, 1); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @param out The byte array to be filled with the result.
      * Must be of length output_length()
      */
      void final(byte out[]) { final_result(out); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @return secure_vector holding the result
      */
      secure_vector<byte> final()
         {
         secure_vector<byte> output(output_length());
         final_result(output.data());
         return output;
         }

      template<typename Alloc>
         void final(std::vector<byte, Alloc>& out)
         {
         out.resize(output_length());
         final_result(out.data());
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a byte array
      * @param length the length of the byte array
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const byte in[], size_t length)
         {
         add_data(in, length);
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const secure_vector<byte>& in)
         {
         add_data(in.data(), in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const std::vector<byte>& in)
         {
         add_data(in.data(), in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a string
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const std::string& in)
         {
         update(in);
         return final();
         }

      virtual ~Buffered_Computation() {}
   private:
      /**
      * Add more data to the computation
      * @param input is an input buffer
      * @param length is the length of input in bytes
      */
      virtual void add_data(const byte input[], size_t length) = 0;

      /**
      * Write the final output to out
      * @param out is an output buffer of output_length()
      */
      virtual void final_result(byte out[]) = 0;
   };

}


namespace Botan {

/**
* This class represents hash function (message digest) objects
*/
class BOTAN_DLL HashFunction : public Buffered_Computation
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<HashFunction> create(const std::string& algo_spec,
                                                  const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual HashFunction* clone() const = 0;

      HashFunction();

      virtual ~HashFunction();

      virtual void clear() = 0;

      virtual std::string name() const = 0;

      /**
      * @return hash block size as defined for this algorithm
      */
      virtual size_t hash_block_size() const { return 0; }
   };

}


namespace Botan {

/**
* This class represents Message Authentication Code (MAC) objects.
*/
class BOTAN_DLL MessageAuthenticationCode : public Buffered_Computation,
                                            public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<MessageAuthenticationCode> create(const std::string& algo_spec,
                                                               const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      virtual ~MessageAuthenticationCode();

      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @param length the length of param in
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const byte in[], size_t length);

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual MessageAuthenticationCode* clone() const = 0;
   };

}


namespace Botan {

/*
* Get an algorithm object
*  NOTE: these functions create and return new objects, letting the
* caller assume ownership of them
*/

/**
* Block cipher factory method.
*
* @param algo_spec the name of the desired block cipher
* @return pointer to the block cipher object
*/
inline BlockCipher* get_block_cipher(const std::string& algo_spec,
                                     const std::string& provider = "")
   {
   return BlockCipher::create(algo_spec, provider).release();
   }

inline std::unique_ptr<BlockCipher> make_block_cipher(const std::string& algo_spec,
                                                      const std::string& provider = "")
   {
   std::unique_ptr<BlockCipher> p(BlockCipher::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline std::vector<std::string> get_block_cipher_providers(const std::string& algo_spec)
   {
   return BlockCipher::providers(algo_spec);
   }

/**
* Stream cipher factory method.
*
* @param algo_spec the name of the desired stream cipher
* @return pointer to the stream cipher object
*/
inline StreamCipher* get_stream_cipher(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return StreamCipher::create(algo_spec, provider).release();
   }

inline std::unique_ptr<StreamCipher> make_stream_cipher(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   std::unique_ptr<StreamCipher> p(StreamCipher::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline std::vector<std::string> get_stream_cipher_providers(const std::string& algo_spec)
   {
   return StreamCipher::providers(algo_spec);
   }

/**
* Hash function factory method.
*
* @param algo_spec the name of the desired hash function
* @return pointer to the hash function object
*/
inline HashFunction* get_hash_function(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return HashFunction::create(algo_spec, provider).release();
   }

inline std::unique_ptr<HashFunction> make_hash_function(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   std::unique_ptr<HashFunction> p(HashFunction::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline HashFunction* get_hash(const std::string& algo_spec,
                              const std::string& provider = "")
   {
   return get_hash_function(algo_spec, provider);
   }

inline std::vector<std::string> get_hash_function_providers(const std::string& algo_spec)
   {
   return HashFunction::providers(algo_spec);
   }

/**
* MAC factory method.
*
* @param algo_spec the name of the desired MAC
* @return pointer to the MAC object
*/
inline MessageAuthenticationCode* get_mac(const std::string& algo_spec,
                                             const std::string& provider = "")
   {
   return MessageAuthenticationCode::create(algo_spec, provider).release();
   }

inline std::unique_ptr<MessageAuthenticationCode> make_message_auth(const std::string& algo_spec,
                                                                       const std::string& provider = "")
   {
   std::unique_ptr<MessageAuthenticationCode> p(MessageAuthenticationCode::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline std::vector<std::string> get_mac_providers(const std::string& algo_spec)
   {
   return MessageAuthenticationCode::providers(algo_spec);
   }

}


namespace Botan {

/*
* Get information describing the version
*/

/**
* Get a human-readable string identifying the version of Botan.
* No particular format should be assumed.
* @return version string
*/
BOTAN_DLL std::string version_string();

BOTAN_DLL const char* version_cstr();

/**
* Return the date this version of botan was released, in an integer of
* the form YYYYMMDD. For instance a version released on May 21, 2013
* would return the integer 20130521. If the currently running version
* is not an official release, this function will return 0 instead.
*
* @return release date, or zero if unreleased
*/
BOTAN_DLL u32bit version_datestamp();

/**
* Get the major version number.
* @return major version number
*/
BOTAN_DLL u32bit version_major();

/**
* Get the minor version number.
* @return minor version number
*/
BOTAN_DLL u32bit version_minor();

/**
* Get the patch number.
* @return patch number
*/
BOTAN_DLL u32bit version_patch();

/*
* Macros for compile-time version checks
*/
#define BOTAN_VERSION_CODE_FOR(a,b,c) ((a << 16) | (b << 8) | (c))

/**
* Compare using BOTAN_VERSION_CODE_FOR, as in
*  # if BOTAN_VERSION_CODE < BOTAN_VERSION_CODE_FOR(1,8,0)
*  #    error "Botan version too old"
*  # endif
*/
#define BOTAN_VERSION_CODE BOTAN_VERSION_CODE_FOR(BOTAN_VERSION_MAJOR, \
                                                  BOTAN_VERSION_MINOR, \
                                                  BOTAN_VERSION_PATCH)

}



#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#endif


namespace Botan {

/**
* Filter mixin that breaks input into blocks, useful for
* cipher modes
*/
class BOTAN_DLL Buffered_Filter
   {
   public:
      /**
      * Write bytes into the buffered filter, which will them emit them
      * in calls to buffered_block in the subclass
      * @param in the input bytes
      * @param length of in in bytes
      */
      void write(const byte in[], size_t length);

      template<typename Alloc>
         void write(const std::vector<byte, Alloc>& in, size_t length)
         {
         write(in.data(), length);
         }

      /**
      * Finish a message, emitting to buffered_block and buffered_final
      * Will throw an exception if less than final_minimum bytes were
      * written into the filter.
      */
      void end_msg();

      /**
      * Initialize a Buffered_Filter
      * @param block_size the function buffered_block will be called
      *        with inputs which are a multiple of this size
      * @param final_minimum the function buffered_final will be called
      *        with at least this many bytes.
      */
      Buffered_Filter(size_t block_size, size_t final_minimum);

      virtual ~Buffered_Filter() {}
   protected:
      /**
      * The block processor, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be a multiple
      *        of block_size
      */
      virtual void buffered_block(const byte input[], size_t length) = 0;

      /**
      * The final block, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be at least
      *        final_minimum bytes
      */
      virtual void buffered_final(const byte input[], size_t length) = 0;

      /**
      * @return block size of inputs
      */
      size_t buffered_block_size() const { return main_block_mod; }

      /**
      * @return current position in the buffer
      */
      size_t current_position() const { return buffer_pos; }

      /**
      * Reset the buffer position
      */
      void buffer_reset() { buffer_pos = 0; }
   private:
      size_t main_block_mod, final_minimum;

      secure_vector<byte> buffer;
      size_t buffer_pos;
   };

}


namespace Botan {

/**
* Struct representing a particular date and time
*/
struct BOTAN_DLL calendar_point
   {
   /** The year */
   u32bit year;

   /** The month, 1 through 12 for Jan to Dec */
   u32bit month;

   /** The day of the month, 1 through 31 (or 28 or 30 based on month */
   u32bit day;

   /** Hour in 24-hour form, 0 to 23 */
   u32bit hour;

   /** Minutes in the hour, 0 to 60 */
   u32bit minutes;

   /** Seconds in the minute, 0 to 60, but might be slightly
       larger to deal with leap seconds on some systems
   */
   u32bit seconds;

   /**
   * Initialize a calendar_point
   * @param y the year
   * @param mon the month
   * @param d the day
   * @param h the hour
   * @param min the minute
   * @param sec the second
   */
   calendar_point(u32bit y, u32bit mon, u32bit d, u32bit h, u32bit min, u32bit sec) :
      year(y), month(mon), day(d), hour(h), minutes(min), seconds(sec) {}

   /**
   * Returns an STL timepoint object
   */
   std::chrono::system_clock::time_point to_std_timepoint();

   /**
   * Returns a human readable string of the struct's components.
   * Formatting might change over time. Currently it is RFC339 'iso-date-time'.
   */
   std::string to_string() const;
   };

/**
* Convert a time_point to a calendar_point
* @param time_point a time point from the system clock
* @return calendar_point object representing this time point
*/
BOTAN_DLL calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point);

}


namespace Botan {

/**
* The different charsets (nominally) supported by Botan.
*/
enum Character_Set {
   LOCAL_CHARSET,
   UCS2_CHARSET,
   UTF8_CHARSET,
   LATIN1_CHARSET
};

namespace Charset {

/*
* Character Set Handling
*/
std::string BOTAN_DLL transcode(const std::string& str,
                                Character_Set to,
                                Character_Set from);

bool BOTAN_DLL is_digit(char c);
bool BOTAN_DLL is_space(char c);
bool BOTAN_DLL caseless_cmp(char x, char y);

byte BOTAN_DLL char2digit(char c);
char BOTAN_DLL digit2char(byte b);

}

}


namespace Botan {

/**
* CMAC, also known as OMAC1
*/
class BOTAN_DLL CMAC : public MessageAuthenticationCode
   {
   public:
      std::string name() const override;
      size_t output_length() const override { return m_cipher->block_size(); }
      MessageAuthenticationCode* clone() const override;

      void clear() override;

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      /**
      * CMAC's polynomial doubling operation
      * @param in the input
      * @param polynomial the byte value of the polynomial
      */
      static secure_vector<byte> poly_double(const secure_vector<byte>& in);

      /**
      * @param cipher the underlying block cipher to use
      */
      CMAC(BlockCipher* cipher);

      static CMAC* make(const Spec& spec);

      CMAC(const CMAC&) = delete;
      CMAC& operator=(const CMAC&) = delete;
   private:
      void add_data(const byte[], size_t) override;
      void final_result(byte[]) override;
      void key_schedule(const byte[], size_t) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_buffer, m_state, m_B, m_P;
      size_t m_position;
   };

}


namespace Botan {

class Transform;
class Compressor_Transform;

/**
* Filter interface for compression/decompression
*/
class BOTAN_DLL Compression_Decompression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const byte input[], size_t input_length) override;
      void end_msg() override;

      std::string name() const override;

   protected:
      Compression_Decompression_Filter(Transform* t, size_t bs);

      void flush();
   private:
      std::unique_ptr<Compressor_Transform> m_transform;
      std::size_t m_buffersize;
      secure_vector<byte> m_buffer;
   };

class BOTAN_DLL Compression_Filter : public Compression_Decompression_Filter
   {
   public:
      Compression_Filter(const std::string& type,
                         size_t compression_level,
                         size_t buffer_size = 4096);

      using Compression_Decompression_Filter::flush;
   };

class BOTAN_DLL Decompression_Filter : public Compression_Decompression_Filter
   {
   public:
      Decompression_Filter(const std::string& type,
                           size_t buffer_size = 4096);
   };

}


namespace Botan {

class BOTAN_DLL Compressor_Transform : public Transform
   {
   public:
      size_t update_granularity() const override final { return 1; }

      size_t minimum_final_size() const override final { return 0; }

      size_t default_nonce_length() const override final { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override final
         { return nonce_len == 0; }

      virtual void flush(secure_vector<byte>& buf, size_t offset = 0) { update(buf, offset); }

      size_t output_length(size_t) const override final
         {
         throw std::runtime_error(name() + " output length indeterminate");
         }
   };

BOTAN_DLL Compressor_Transform* make_compressor(const std::string& type, size_t level);
BOTAN_DLL Compressor_Transform* make_decompressor(const std::string& type);

class Compression_Stream
   {
   public:
      virtual ~Compression_Stream() {}

      virtual void next_in(byte* b, size_t len) = 0;

      virtual void next_out(byte* b, size_t len) = 0;

      virtual size_t avail_in() const = 0;

      virtual size_t avail_out() const = 0;

      virtual u32bit run_flag() const = 0;
      virtual u32bit flush_flag() const = 0;
      virtual u32bit finish_flag() const = 0;

      virtual bool run(u32bit flags) = 0;
   };

class BOTAN_DLL Stream_Compression : public Compressor_Transform
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset = 0) override;

      void flush(secure_vector<byte>& buf, size_t offset = 0) override;

      void finish(secure_vector<byte>& buf, size_t offset = 0) override;

      void clear() override;

   private:
      secure_vector<byte> start_raw(const byte[], size_t) override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

class BOTAN_DLL Stream_Decompression : public Compressor_Transform
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset = 0) override;

      void finish(secure_vector<byte>& buf, size_t offset = 0) override;

      void clear() override;

   private:
      secure_vector<byte> start_raw(const byte[], size_t) override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

}


namespace Botan {

/**
* A class handling runtime CPU feature detection
*/
class BOTAN_DLL CPUID
   {
   public:
      /**
      * Probe the CPU and see what extensions are supported
      */
      static void initialize();

      /**
      * Return a best guess of the cache line size
      */
      static size_t cache_line_size() { initialize(); return g_cache_line_size; }

      /**
      * Check if the processor supports AltiVec/VMX
      */
      static bool has_altivec() { initialize(); return g_altivec_capable; }

      /**
      * Check if the processor supports RDTSC
      */
      static bool has_rdtsc()
         { return x86_processor_flags_has(CPUID_RDTSC_BIT); }

      /**
      * Check if the processor supports SSE2
      */
      static bool has_sse2()
         { return x86_processor_flags_has(CPUID_SSE2_BIT); }

      /**
      * Check if the processor supports SSSE3
      */
      static bool has_ssse3()
         { return x86_processor_flags_has(CPUID_SSSE3_BIT); }

      /**
      * Check if the processor supports SSE4.1
      */
      static bool has_sse41()
         { return x86_processor_flags_has(CPUID_SSE41_BIT); }

      /**
      * Check if the processor supports SSE4.2
      */
      static bool has_sse42()
         { return x86_processor_flags_has(CPUID_SSE42_BIT); }

      /**
      * Check if the processor supports AVX2
      */
      static bool has_avx2()
         { return x86_processor_flags_has(CPUID_AVX2_BIT); }

      /**
      * Check if the processor supports AVX-512F
      */
      static bool has_avx512f()
         { return x86_processor_flags_has(CPUID_AVX512F_BIT); }

      /**
      * Check if the processor supports BMI2
      */
      static bool has_bmi2()
         { return x86_processor_flags_has(CPUID_BMI2_BIT); }

      /**
      * Check if the processor supports AES-NI
      */
      static bool has_aes_ni()
         { return x86_processor_flags_has(CPUID_AESNI_BIT); }

      /**
      * Check if the processor supports CLMUL
      */
      static bool has_clmul()
         { return x86_processor_flags_has(CPUID_CLMUL_BIT); }

      /**
      * Check if the processor supports Intel SHA extension
      */
      static bool has_intel_sha()
         { return x86_processor_flags_has(CPUID_SHA_BIT); }

      /**
      * Check if the processor supports ADX extension
      */
      static bool has_adx()
         { return x86_processor_flags_has(CPUID_ADX_BIT); }

      /**
      * Check if the processor supports RDRAND
      */
      static bool has_rdrand()
         { return x86_processor_flags_has(CPUID_RDRAND_BIT); }

      /**
      * Check if the processor supports RDSEED
      */
      static bool has_rdseed()
         { return x86_processor_flags_has(CPUID_RDSEED_BIT); }

      static bool has_simd_32();

      static void print(std::ostream& o);
   private:
      enum CPUID_bits {
         CPUID_RDTSC_BIT = 4,
         CPUID_SSE2_BIT = 26,
         CPUID_CLMUL_BIT = 33,
         CPUID_SSSE3_BIT = 41,
         CPUID_SSE41_BIT = 51,
         CPUID_SSE42_BIT = 52,
         CPUID_AESNI_BIT = 57,
         CPUID_RDRAND_BIT = 62,

         CPUID_AVX2_BIT = 64+5,
         CPUID_BMI2_BIT = 64+8,
         CPUID_AVX512F_BIT = 64+16,
         CPUID_RDSEED_BIT = 64+18,
         CPUID_ADX_BIT = 64+19,
         CPUID_SHA_BIT = 64+29,
      };

      static bool x86_processor_flags_has(u64bit bit)
         {
         if(!g_initialized)
            initialize();
         return ((g_x86_processor_flags[bit/64] >> (bit % 64)) & 1);
         }

      static bool g_initialized;
      static u64bit g_x86_processor_flags[2];
      static size_t g_cache_line_size;
      static bool g_altivec_capable;
   };

}


namespace Botan {

/**
* CTR-BE (Counter mode, big-endian)
*/
class BOTAN_DLL CTR_BE : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length) override;

      void set_iv(const byte iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      std::string name() const override;

      CTR_BE* clone() const override
         { return new CTR_BE(m_cipher->clone()); }

      void clear() override;

      static CTR_BE* make(const Spec& spec);

      /**
      * @param cipher the underlying block cipher to use
      */
      CTR_BE(BlockCipher* cipher);
   private:
      void key_schedule(const byte key[], size_t key_len) override;
      void increment_counter();

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_counter, m_pad;
      size_t m_pad_pos;
   };

}


namespace Botan {

/**
* This class represents abstract data sink objects.
*/
class BOTAN_DLL DataSink : public Filter
   {
   public:
      bool attachable() override { return false; }
      DataSink() {}
      virtual ~DataSink() {}

      DataSink& operator=(const DataSink&) = delete;
      DataSink(const DataSink&) = delete;
   };

/**
* This class represents a data sink which writes its output to a stream.
*/
class BOTAN_DLL DataSink_Stream : public DataSink
   {
   public:
      std::string name() const override { return identifier; }

      void write(const byte[], size_t) override;

      /**
      * Construct a DataSink_Stream from a stream.
      * @param stream the stream to write to
      * @param name identifier
      */
      DataSink_Stream(std::ostream& stream,
                      const std::string& name = "<std::ostream>");

      /**
      * Construct a DataSink_Stream from a stream.
      * @param pathname the name of the file to open a stream to
      * @param use_binary indicates whether to treat the file
      * as a binary file or not
      */
      DataSink_Stream(const std::string& pathname,
                      bool use_binary = false);

      ~DataSink_Stream();
   private:
      const std::string identifier;

      std::ostream* sink_p;
      std::ostream& sink;
   };

}


namespace Botan {

/**
* This class represents an abstract data source object.
*/
class BOTAN_DLL DataSource
   {
   public:
      /**
      * Read from the source. Moves the internal offset so that every
      * call to read will return a new portion of the source.
      *
      * @param out the byte array to write the result to
      * @param length the length of the byte array out
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t read(byte out[], size_t length) = 0;

      virtual bool check_available(size_t n) = 0;

      /**
      * Read from the source but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the source starting at the same position.
      *
      * @param out the byte array to write the output to
      * @param length the length of the byte array out
      * @param peek_offset the offset into the stream to read at
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t peek(byte out[], size_t length,
                          size_t peek_offset) const = 0;

      /**
      * Test whether the source still has data that can be read.
      * @return true if there is still data to read, false otherwise
      */
      virtual bool end_of_data() const = 0;
      /**
      * return the id of this data source
      * @return std::string representing the id of this data source
      */
      virtual std::string id() const { return ""; }

      /**
      * Read one byte.
      * @param out the byte to read to
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t read_byte(byte& out);

      /**
      * Peek at one byte.
      * @param out an output byte
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t peek_byte(byte& out) const;

      /**
      * Discard the next N bytes of the data
      * @param N the number of bytes to discard
      * @return number of bytes actually discarded
      */
      size_t discard_next(size_t N);

      /**
      * @return number of bytes read so far.
      */
      virtual size_t get_bytes_read() const = 0;

      DataSource() {}
      virtual ~DataSource() {}
      DataSource& operator=(const DataSource&) = delete;
      DataSource(const DataSource&) = delete;
   };

/**
* This class represents a Memory-Based DataSource
*/
class BOTAN_DLL DataSource_Memory : public DataSource
   {
   public:
      size_t read(byte[], size_t) override;
      size_t peek(byte[], size_t, size_t) const override;
      bool check_available(size_t n) override;
      bool end_of_data() const override;

      /**
      * Construct a memory source that reads from a string
      * @param in the string to read from
      */
      DataSource_Memory(const std::string& in);

      /**
      * Construct a memory source that reads from a byte array
      * @param in the byte array to read from
      * @param length the length of the byte array
      */
      DataSource_Memory(const byte in[], size_t length) :
         source(in, in + length), offset(0) {}

      /**
      * Construct a memory source that reads from a secure_vector
      * @param in the MemoryRegion to read from
      */
      DataSource_Memory(const secure_vector<byte>& in) :
         source(in), offset(0) {}

      /**
      * Construct a memory source that reads from a std::vector
      * @param in the MemoryRegion to read from
      */
      DataSource_Memory(const std::vector<byte>& in) :
         source(in.begin(), in.end()), offset(0) {}

      size_t get_bytes_read() const override { return offset; }
   private:
      secure_vector<byte> source;
      size_t offset;
   };

/**
* This class represents a Stream-Based DataSource.
*/
class BOTAN_DLL DataSource_Stream : public DataSource
   {
   public:
      size_t read(byte[], size_t) override;
      size_t peek(byte[], size_t, size_t) const override;
      bool check_available(size_t n) override;
      bool end_of_data() const override;
      std::string id() const override;

      DataSource_Stream(std::istream&,
                        const std::string& id = "<std::istream>");

      /**
      * Construct a Stream-Based DataSource from file
      * @param file the name of the file
      * @param use_binary whether to treat the file as binary or not
      */
      DataSource_Stream(const std::string& file, bool use_binary = false);

      DataSource_Stream(const DataSource_Stream&) = delete;

      DataSource_Stream& operator=(const DataSource_Stream&) = delete;

      ~DataSource_Stream();

      size_t get_bytes_read() const override { return total_read; }
   private:
      const std::string identifier;

      std::istream* source_p;
      std::istream& source;
      size_t total_read;
   };

}


namespace Botan {

class BOTAN_DLL SQL_Database
   {
   public:
      class BOTAN_DLL Statement
         {
         public:
            /* Bind statement parameters */
            virtual void bind(int column, const std::string& str) = 0;

            virtual void bind(int column, size_t i) = 0;

            virtual void bind(int column, std::chrono::system_clock::time_point time) = 0;

            virtual void bind(int column, const std::vector<byte>& blob) = 0;

            /* Get output */
            virtual std::pair<const byte*, size_t> get_blob(int column) = 0;

            virtual size_t get_size_t(int column) = 0;

            /* Run to completion */
            virtual void spin() = 0;

            /* Maybe update */
            virtual bool step() = 0;

            virtual ~Statement() {}
         };

      /*
      * Create a new statement for execution.
      * Use ?1, ?2, ?3, etc for parameters to set later with bind
      */
      virtual std::shared_ptr<Statement> new_statement(const std::string& base_sql) const = 0;

      virtual size_t row_count(const std::string& table_name) = 0;

      virtual void create_table(const std::string& table_schema) = 0;

      virtual ~SQL_Database() {}
};

}


namespace Botan {

/**
* EAX base class
*/
class BOTAN_DLL EAX_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // EAX supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;
   protected:
      /**
      * @param cipher the cipher to use
      * @param tag_size is how big the auth tag will be
      */
      EAX_Mode(BlockCipher* cipher, size_t tag_size);

      size_t block_size() const { return m_cipher->block_size(); }

      size_t m_tag_size;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<MessageAuthenticationCode> m_cmac;

      secure_vector<byte> m_ad_mac;

      secure_vector<byte> m_nonce_mac;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* EAX Encryption
*/
class BOTAN_DLL EAX_Encryption : public EAX_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be
      */
      EAX_Encryption(BlockCipher* cipher, size_t tag_size = 0) :
         EAX_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* EAX Decryption
*/
class BOTAN_DLL EAX_Decryption : public EAX_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be
      */
      EAX_Decryption(BlockCipher* cipher, size_t tag_size = 0) :
         EAX_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

}



namespace Botan {

/**
* This class represents pipe objects.
* A set of filters can be placed into a pipe, and information flows
* through the pipe until it reaches the end, where the output is
* collected for retrieval.  If you're familiar with the Unix shell
* environment, this design will sound quite familiar.
*/
class BOTAN_DLL Pipe : public DataSource
   {
   public:
      /**
      * An opaque type that identifies a message in this Pipe
      */
      typedef size_t message_id;

      /**
      * Exception if you use an invalid message as an argument to
      * read, remaining, etc
      */
      struct BOTAN_DLL Invalid_Message_Number : public Invalid_Argument
         {
         /**
         * @param where the error occurred
         * @param msg the invalid message id that was used
         */
         Invalid_Message_Number(const std::string& where, message_id msg) :
            Invalid_Argument("Pipe::" + where + ": Invalid message number " +
                             std::to_string(msg))
            {}
         };

      /**
      * A meta-id for whatever the last message is
      */
      static const message_id LAST_MESSAGE;

      /**
      * A meta-id for the default message (set with set_default_msg)
      */
      static const message_id DEFAULT_MESSAGE;

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the byte array to write
      * @param length the length of the byte array in
      */
      void write(const byte in[], size_t length);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the secure_vector containing the data to write
      */
      void write(const secure_vector<byte>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the std::vector containing the data to write
      */
      void write(const std::vector<byte>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the string containing the data to write
      */
      void write(const std::string& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the DataSource to read the data from
      */
      void write(DataSource& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in a single byte to be written
      */
      void write(byte in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the byte array containing the data to write
      * @param length the length of the byte array to write
      */
      void process_msg(const byte in[], size_t length);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const secure_vector<byte>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const std::vector<byte>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the string containing the data to write
      */
      void process_msg(const std::string& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the DataSource providing the data to write
      */
      void process_msg(DataSource& in);

      /**
      * Find out how many bytes are ready to read.
      * @param msg the number identifying the message
      * for which the information is desired
      * @return number of bytes that can still be read
      */
      size_t remaining(message_id msg = DEFAULT_MESSAGE) const;

      /**
      * Read the default message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      *
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @return number of bytes actually read into output
      */
      size_t read(byte output[], size_t length) override;

      /**
      * Read a specified message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @param msg the number identifying the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(byte output[], size_t length, message_id msg);

      /**
      * Read a single byte from the pipe. Moves the internal offset so
      * that every call to read will return a new portion of the
      * message.
      *
      * @param output the byte to write the result to
      * @param msg the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(byte& output, message_id msg = DEFAULT_MESSAGE);

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return secure_vector holding the contents of the pipe
      */
      secure_vector<byte> read_all(message_id msg = DEFAULT_MESSAGE);

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return string holding the contents of the pipe
      */
      std::string read_all_as_string(message_id = DEFAULT_MESSAGE);

      /** Read from the default message but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte output[], size_t length, size_t offset) const override;

      /** Read from the specified message but do not modify the
      * internal offset. Consecutive calls to peek() will return
      * portions of the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte output[], size_t length,
                  size_t offset, message_id msg) const;

      /** Read a single byte from the specified message but do not
      * modify the internal offset. Consecutive calls to peek() will
      * return portions of the message starting at the same position.
      * @param output the byte to write the peeked message byte to
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte& output, size_t offset,
                  message_id msg = DEFAULT_MESSAGE) const;

      /**
      * @return the number of bytes read from the default message.
      */
      size_t get_bytes_read() const override;

      /**
      * @return the number of bytes read from the specified message.
      */
      size_t get_bytes_read(message_id msg) const;

      bool check_available(size_t n) override;
      bool check_available_msg(size_t n, message_id msg);

      /**
      * @return currently set default message
      */
      size_t default_msg() const { return default_read; }

      /**
      * Set the default message
      * @param msg the number identifying the message which is going to
      * be the new default message
      */
      void set_default_msg(message_id msg);

      /**
      * Get the number of messages the are in this pipe.
      * @return number of messages the are in this pipe
      */
      message_id message_count() const;

      /**
      * Test whether this pipe has any data that can be read from.
      * @return true if there is more data to read, false otherwise
      */
      bool end_of_data() const override;

      /**
      * Start a new message in the pipe. A potential other message in this pipe
      * must be closed with end_msg() before this function may be called.
      */
      void start_msg();

      /**
      * End the current message.
      */
      void end_msg();

      /**
      * Insert a new filter at the front of the pipe
      * @param filt the new filter to insert
      */
      void prepend(Filter* filt);

      /**
      * Insert a new filter at the back of the pipe
      * @param filt the new filter to insert
      */
      void append(Filter* filt);

      /**
      * Remove the first filter at the front of the pipe.
      */
      void pop();

      /**
      * Reset this pipe to an empty pipe.
      */
      void reset();

      /**
      * Construct a Pipe of up to four filters. The filters are set up
      * in the same order as the arguments.
      */
      Pipe(Filter* = nullptr, Filter* = nullptr,
           Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Pipe from a list of filters
      * @param filters the set of filters to use
      */
      Pipe(std::initializer_list<Filter*> filters);

      Pipe(const Pipe&) = delete;
      Pipe& operator=(const Pipe&) = delete;

      ~Pipe();
   private:
      void init();
      void destruct(Filter*);
      void find_endpoints(Filter*);
      void clear_endpoints(Filter*);

      message_id get_message_no(const std::string&, message_id) const;

      Filter* pipe;
      class Output_Buffers* outputs;
      message_id default_read;
      bool inside_msg;
   };

/**
* Stream output operator; dumps the results from pipe's default
* message to the output stream.
* @param out an output stream
* @param pipe the pipe
*/
BOTAN_DLL std::ostream& operator<<(std::ostream& out, Pipe& pipe);

/**
* Stream input operator; dumps the remaining bytes of input
* to the (assumed open) pipe message.
* @param in the input stream
* @param pipe the pipe
*/
BOTAN_DLL std::istream& operator>>(std::istream& in, Pipe& pipe);

}

#if defined(BOTAN_HAS_PIPE_UNIXFD_IO)
#endif


namespace Botan {

/**
* This class represents keyed filters, i.e. filters that have to be
* fed with a key in order to function.
*/
class BOTAN_DLL Keyed_Filter : public Filter
   {
   public:
      /**
      * Set the key of this filter
      * @param key the key to use
      */
      virtual void set_key(const SymmetricKey& key) = 0;

      /**
      * Set the initialization vector of this filter. Note: you should
      * call set_iv() only after you have called set_key()
      * @param iv the initialization vector to use
      */
      virtual void set_iv(const InitializationVector& iv);

      /**
      * Check whether a key length is valid for this filter
      * @param length the key length to be checked for validity
      * @return true if the key length is valid, false otherwise
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether an IV length is valid for this filter
      * @param length the IV length to be checked for validity
      * @return true if the IV length is valid, false otherwise
      */
      virtual bool valid_iv_length(size_t length) const
         { return (length == 0); }
   };



/*
* Get a cipher object
*/

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param iv the initialization vector to be used
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to newly allocated encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   const InitializationVector& iv,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters. No key will be
* set in the filter.
*
* @param algo_spec the name of the desired cipher
* @param direction determines whether the filter will be an encrypting or
* decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   Cipher_Dir direction);

}



#if defined(BOTAN_HAS_CODEC_FILTERS)
#endif

namespace Botan {

/**
* Stream Cipher Filter
*/
class BOTAN_DLL StreamCipher_Filter : public Keyed_Filter
   {
   public:

      std::string name() const override { return m_cipher->name(); }

      /**
      * Write input data
      * @param input data
      * @param input_len length of input in bytes
      */
      void write(const byte input[], size_t input_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return m_cipher->valid_iv_length(iv_len); }

      /**
      * Set the initialization vector for this filter.
      * @param iv the initialization vector to set
      */
      void set_iv(const InitializationVector& iv) override
         {
         m_cipher->set_iv(iv.begin(), iv.length());
         }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_cipher->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      */
      StreamCipher_Filter(StreamCipher* cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(StreamCipher* cipher, const SymmetricKey& key);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      */
      StreamCipher_Filter(const std::string& cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(const std::string& cipher, const SymmetricKey& key);
   private:
      secure_vector<byte> m_buffer;
      std::unique_ptr<StreamCipher> m_cipher;
   };

/**
* Hash Filter.
*/
class BOTAN_DLL Hash_Filter : public Filter
   {
   public:
      void write(const byte input[], size_t len) override { m_hash->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_hash->name(); }

      /**
      * Construct a hash filter.
      * @param hash_fun the hash function to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(HashFunction* hash, size_t len = 0) :
         m_hash(hash), m_out_len(len) {}

      /**
      * Construct a hash filter.
      * @param request the name of the hash algorithm to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(const std::string& request, size_t len = 0);

   private:
      std::unique_ptr<HashFunction> m_hash;
      const size_t m_out_len;
   };

/**
* MessageAuthenticationCode Filter.
*/
class BOTAN_DLL MAC_Filter : public Keyed_Filter
   {
   public:
      void write(const byte input[], size_t len) override { m_mac->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_mac->name(); }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_mac->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_mac->key_spec(); }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the MAC to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         }

      /**
      * Construct a MAC filter.
      * @param mac the MAC to use
      * @param key the MAC key to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 const SymmetricKey& key,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         m_mac->set_key(key);
         }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the name of the MAC to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, size_t len = 0);

      /**
      * Construct a MAC filter.
      * @param mac the name of the MAC to use
      * @param key the MAC key to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, const SymmetricKey& key,
                 size_t len = 0);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_out_len;
   };

}


namespace Botan {

class GHASH;

/**
* GCM Mode
*/
class BOTAN_DLL GCM_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // GCM supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;
   protected:
      GCM_Mode(BlockCipher* cipher, size_t tag_size);

      const size_t BS = 16;

      const size_t m_tag_size;
      const std::string m_cipher_name;

      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<GHASH> m_ghash;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* GCM Encryption
*/
class BOTAN_DLL GCM_Encryption : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* GCM Decryption
*/
class BOTAN_DLL GCM_Decryption : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* GCM's GHASH
* Maybe a Transform?
*/
class BOTAN_DLL GHASH : public SymmetricAlgorithm
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len);

      secure_vector<byte> nonce_hash(const byte nonce[], size_t len);

      void start(const byte nonce[], size_t len);

      /*
      * Assumes input len is multiple of 16
      */
      void update(const byte in[], size_t len);

      secure_vector<byte> final();

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(16); }

      void clear() override;

      std::string name() const override { return "GHASH"; }
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      void gcm_multiply(secure_vector<byte>& x) const;

      void ghash_update(secure_vector<byte>& x,
                        const byte input[], size_t input_len);

      void add_final_block(secure_vector<byte>& x,
                           size_t ad_len, size_t pt_len);

      secure_vector<byte> m_H;
      secure_vector<byte> m_H_ad;
      secure_vector<byte> m_nonce;
      secure_vector<byte> m_ghash;
      size_t m_ad_len = 0, m_text_len = 0;
   };

}


namespace Botan {

/**
* Perform hex encoding
* @param output an array of at least input_length*2 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
*/
void BOTAN_DLL hex_encode(char output[],
                          const byte input[],
                          size_t input_length,
                          bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
std::string BOTAN_DLL hex_encode(const byte input[],
                                 size_t input_length,
                                 bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
template<typename Alloc>
std::string hex_encode(const std::vector<byte, Alloc>& input,
                       bool uppercase = true)
   {
   return hex_encode(input.data(), input.size(), uppercase);
   }

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const char input[],
                            size_t input_length,
                            size_t& input_consumed,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const char input[],
                            size_t input_length,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const std::string& input,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<byte> BOTAN_DLL
hex_decode(const char input[],
           size_t input_length,
           bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<byte> BOTAN_DLL
hex_decode(const std::string& input,
           bool ignore_ws = true);


/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<byte> BOTAN_DLL
hex_decode_locked(const char input[],
                  size_t input_length,
                  bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<byte> BOTAN_DLL
hex_decode_locked(const std::string& input,
                  bool ignore_ws = true);

}


namespace Botan {

/**
* HMAC
*/
class BOTAN_DLL HMAC : public MessageAuthenticationCode
   {
   public:
      void clear() override;
      std::string name() const override;
      MessageAuthenticationCode* clone() const override;

      size_t output_length() const override { return m_hash->output_length(); }

      Key_Length_Specification key_spec() const override
         {
         // Absurd max length here is to support PBKDF2
         return Key_Length_Specification(0, 512);
         }

      /**
      * @param hash the hash to use for HMACing
      */
      HMAC(HashFunction* hash);

      static HMAC* make(const Spec& spec);

      HMAC(const HMAC&) = delete;
      HMAC& operator=(const HMAC&) = delete;
   private:
      void add_data(const byte[], size_t) override;
      void final_result(byte[]) override;
      void key_schedule(const byte[], size_t) override;

      std::unique_ptr<HashFunction> m_hash;
      secure_vector<byte> m_ikey, m_okey;
   };

}


namespace Botan {

/**
* HMAC_RNG - based on the design described in "On Extract-then-Expand
* Key Derivation Functions and an HMAC-based KDF" by Hugo Krawczyk
* (henceforce, 'E-t-E')
*
* However it actually can be parameterized with any two MAC functions,
* not restricted to HMAC (this variation is also described in
* Krawczyk's paper), for instance one could use HMAC(SHA-512) as the
* extractor and CMAC(AES-256) as the PRF.
*/
class BOTAN_DLL HMAC_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte buf[], size_t len) override;
      bool is_seeded() const override;
      void clear() override;
      std::string name() const override;

      void reseed(size_t poll_bits) override;

      void reseed_with_timeout(size_t poll_bits, std::chrono::milliseconds ms);

      void add_entropy(const byte[], size_t) override;

      /**
      * @param extractor a MAC used for extracting the entropy
      * @param prf a MAC used as a PRF using HKDF construction
      */
      HMAC_RNG(MessageAuthenticationCode* extractor,
               MessageAuthenticationCode* prf);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_extractor;
      std::unique_ptr<MessageAuthenticationCode> m_prf;

      enum HMAC_PRF_Label {
         Running,
         Reseed,
         ExtractorSeed,
      };
      void new_K_value(byte label);

      size_t m_collected_entropy_estimate = 0;
      size_t m_output_since_reseed = 0;

      secure_vector<byte> m_K;
      u32bit m_counter = 0;
   };

}


namespace Botan {

/*
* Previously botan had state whose lifetime had to be explicitly
* managed by the application. As of 1.11.14 this is no longer the
* case, and this class is no longer needed and kept only for backwards
* compatibility.
*/
class BOTAN_DLL LibraryInitializer
   {
   public:
      LibraryInitializer(const std::string& s = "") { initialize(s); }
      ~LibraryInitializer() { deinitialize(); }

      static void initialize(const std::string& = "");
      static void deinitialize();
   };

}


namespace Botan {

/**
* Key Derivation Function
*/
class BOTAN_DLL KDF
   {
   public:
      virtual ~KDF();

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<KDF> create(const std::string& algo_spec,
                                         const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      virtual std::string name() const = 0;

      virtual size_t kdf(byte key[], size_t key_len,
                         const byte secret[], size_t secret_len,
                         const byte salt[], size_t salt_len) const = 0;


      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const byte salt[],
                                    size_t salt_len) const
         {
         secure_vector<byte> key(key_len);
         key.resize(kdf(key.data(), key.size(), secret, secret_len, salt, salt_len));
         return key;
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const std::string& salt = "") const
         {
         return derive_key(key_len, secret.data(), secret.size(),
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      */
      template<typename Alloc, typename Alloc2>
      secure_vector<byte> derive_key(size_t key_len,
                                     const std::vector<byte, Alloc>& secret,
                                     const std::vector<byte, Alloc2>& salt) const
         {
         return derive_key(key_len,
                           secret.data(), secret.size(),
                           salt.data(), salt.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const byte salt[],
                                    size_t salt_len) const
         {
         return derive_key(key_len,
                           secret.data(), secret.size(),
                           salt, salt_len);
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const std::string& salt = "") const
         {
         return derive_key(key_len, secret, secret_len,
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length());
         }

      virtual KDF* clone() const = 0;

      typedef SCAN_Name Spec;

   };

/**
* Factory method for KDF (key derivation function)
* @param algo_spec the name of the KDF to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL KDF*  get_kdf(const std::string& algo_spec);

}


namespace Botan {

/**
* KDF2, from IEEE 1363
*/
class BOTAN_DLL KDF2 : public KDF
   {
   public:
      std::string name() const override { return "KDF2(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF2(m_hash->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      KDF2(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* Keccak[1600], a SHA-3 candidate
*/
class BOTAN_DLL Keccak_1600 : public HashFunction
   {
   public:

      /**
      * @param output_bits the size of the hash output; must be one of
      *                    224, 256, 384, or 512
      */
      Keccak_1600(size_t output_bits = 512);

      size_t hash_block_size() const override { return bitrate / 8; }
      size_t output_length() const override { return output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;
   private:
      void add_data(const byte input[], size_t length) override;
      void final_result(byte out[]) override;

      size_t output_bits, bitrate;
      secure_vector<u64bit> S;
      size_t S_pos;
   };

}


namespace Botan {

/**
* MDx Hash Function Base Class
*/
class BOTAN_DLL MDx_HashFunction : public HashFunction
   {
   public:
      /**
      * @param block_length is the number of bytes per block
      * @param big_byte_endian specifies if the hash uses big-endian bytes
      * @param big_bit_endian specifies if the hash uses big-endian bits
      * @param counter_size specifies the size of the counter var in bytes
      */
      MDx_HashFunction(size_t block_length,
                       bool big_byte_endian,
                       bool big_bit_endian,
                       size_t counter_size = 8);

      size_t hash_block_size() const override { return buffer.size(); }
   protected:
      void add_data(const byte input[], size_t length) override;
      void final_result(byte output[]) override;

      /**
      * Run the hash's compression function over a set of blocks
      * @param blocks the input
      * @param block_n the number of blocks
      */
      virtual void compress_n(const byte blocks[], size_t block_n) = 0;

      void clear() override;

      /**
      * Copy the output to the buffer
      * @param buffer to put the output into
      */
      virtual void copy_out(byte buffer[]) = 0;

      /**
      * Write the count, if used, to this spot
      * @param out where to write the counter to
      */
      virtual void write_count(byte out[]);
   private:
      secure_vector<byte> buffer;
      u64bit count;
      size_t position;

      const bool BIG_BYTE_ENDIAN, BIG_BIT_ENDIAN;
      const size_t COUNT_SIZE;
   };

}


namespace Botan {

#if defined(__SIZEOF_INT128__)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128
   typedef unsigned __int128 uint128_t;

#elif (BOTAN_GCC_VERSION > 440) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128
   typedef unsigned int uint128_t __attribute__((mode(TI)));
#endif

}

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi)      \
   do {                                      \
      const uint128_t r = static_cast<uint128_t>(a) * b;   \
      *hi = (r >> 64) & 0xFFFFFFFFFFFFFFFF;  \
      *lo = (r      ) & 0xFFFFFFFFFFFFFFFF;  \
   } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)

#include <intrin.h>
#pragma intrinsic(_umul128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) \
   do { *lo = _umul128(a, b, hi); } while(0)

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                           \
   asm("mulq %3" : "=d" (*hi), "=a" (*lo) : "a" (a), "rm" (b) : "cc"); \
   } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {              \
   asm("umulh %1,%2,%0" : "=r" (*hi) : "r" (a), "r" (b)); \
   *lo = a * b;                                           \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                \
   asm("xmpy.hu %0=%1,%2" : "=f" (*hi) : "f" (a), "f" (b)); \
   *lo = a * b;                                             \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                      \
   asm("mulhdu %0,%1,%2" : "=r" (*hi) : "r" (a), "r" (b) : "cc"); \
   *lo = a * b;                                                   \
} while(0)

#endif

#endif

namespace Botan {

/**
* Perform a 64x64->128 bit multiplication
*/
inline void mul64x64_128(u64bit a, u64bit b, u64bit* lo, u64bit* hi)
   {
#if defined(BOTAN_FAST_64X64_MUL)
   BOTAN_FAST_64X64_MUL(a, b, lo, hi);
#else

   /*
   * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
   * some adds and shifts. Last resort for CPUs like UltraSPARC (with
   * 64-bit registers/ALU, but no 64x64->128 multiply) or 32-bit CPUs.
   */
   const size_t HWORD_BITS = 32;
   const u32bit HWORD_MASK = 0xFFFFFFFF;

   const u32bit a_hi = (a >> HWORD_BITS);
   const u32bit a_lo = (a  & HWORD_MASK);
   const u32bit b_hi = (b >> HWORD_BITS);
   const u32bit b_lo = (b  & HWORD_MASK);

   u64bit x0 = static_cast<u64bit>(a_hi) * b_hi;
   u64bit x1 = static_cast<u64bit>(a_lo) * b_hi;
   u64bit x2 = static_cast<u64bit>(a_hi) * b_lo;
   u64bit x3 = static_cast<u64bit>(a_lo) * b_lo;

   // this cannot overflow as (2^32-1)^2 + 2^32-1 < 2^64-1
   x2 += x3 >> HWORD_BITS;

   // this one can overflow
   x2 += x1;

   // propagate the carry if any
   x0 += static_cast<u64bit>(static_cast<bool>(x2 < x1)) << HWORD_BITS;

   *hi = x0 + (x2 >> HWORD_BITS);
   *lo  = ((x2 & HWORD_MASK) << HWORD_BITS) + (x3 & HWORD_MASK);
#endif
   }

}


namespace Botan {

/**
* Base class for PBKDF (password based key derivation function)
* implementations. Converts a password into a key using a salt
* and iterated hashing to make brute force attacks harder.
*/
class BOTAN_DLL PBKDF
   {
   public:
      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<PBKDF> create(const std::string& algo_spec,
                                           const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      typedef SCAN_Name Spec;

      /**
      * @return new instance of this same algorithm
      */
      virtual PBKDF* clone() const = 0;

      virtual std::string name() const = 0;

      virtual ~PBKDF();

      /**
      * Derive a key from a passphrase for a number of iterations
      * specified by either iterations or if iterations == 0 then
      * running until seconds time has elapsed.
      *
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @return the number of iterations performed
      */
      virtual size_t pbkdf(byte out[], size_t out_len,
                           const std::string& passphrase,
                           const byte salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const = 0;

      void pbkdf_iterations(byte out[], size_t out_len,
                            const std::string& passphrase,
                            const byte salt[], size_t salt_len,
                            size_t iterations) const;

      void pbkdf_timed(byte out[], size_t out_len,
                         const std::string& passphrase,
                         const byte salt[], size_t salt_len,
                         std::chrono::milliseconds msec,
                         size_t& iterations) const;

      secure_vector<byte> pbkdf_iterations(size_t out_len,
                                           const std::string& passphrase,
                                           const byte salt[], size_t salt_len,
                                           size_t iterations) const;

      secure_vector<byte> pbkdf_timed(size_t out_len,
                                      const std::string& passphrase,
                                      const byte salt[], size_t salt_len,
                                      std::chrono::milliseconds msec,
                                      size_t& iterations) const;

      // Following kept for compat with 1.10:

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt, salt_len, iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param iterations the number of iterations to use (use 10K or more)
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt.data(), salt.size(), iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt, salt_len, msec, iterations);
         }

      /**
      * Derive a key from a passphrase using a certain amount of time
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt.data(), salt.size(), msec, iterations);
         }
   };

/**
* Password based key derivation function factory method
* @param algo_spec the name of the desired PBKDF algorithm
* @return pointer to newly allocated object of that type
*/
inline PBKDF* get_pbkdf(const std::string& algo_spec,
                           const std::string& provider = "")
   {
   std::unique_ptr<PBKDF> p(PBKDF::create(algo_spec, provider));
   if(p)
      return p.release();
   throw Algorithm_Not_Found(algo_spec);
   }

}


namespace Botan {

BOTAN_DLL size_t pbkdf2(MessageAuthenticationCode& prf,
                        byte out[],
                        size_t out_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec);

/**
* PKCS #5 PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF2 : public PBKDF
   {
   public:
      std::string name() const override
         {
         return "PBKDF2(" + mac->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF2(mac->clone());
         }

      size_t pbkdf(byte output_buf[], size_t output_len,
                   const std::string& passphrase,
                   const byte salt[], size_t salt_len,
                   size_t iterations,
                   std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : mac(mac_fn) {}

      static PKCS5_PBKDF2* make(const Spec& spec);
   private:
      std::unique_ptr<MessageAuthenticationCode> mac;
   };

}


namespace Botan {

/**
* A queue that knows how to zeroize itself
*/
class BOTAN_DLL SecureQueue : public Fanout_Filter, public DataSource
   {
   public:
      std::string name() const override { return "Queue"; }

      void write(const byte[], size_t) override;

      size_t read(byte[], size_t) override;
      size_t peek(byte[], size_t, size_t = 0) const override;
      size_t get_bytes_read() const override;

      bool end_of_data() const override;

      bool empty() const;

      bool check_available(size_t n) override { return n <= size(); }

      /**
      * @return number of bytes available in the queue
      */
      size_t size() const;

      bool attachable() override { return false; }

      /**
      * SecureQueue assignment
      * @param other the queue to copy
      */
      SecureQueue& operator=(const SecureQueue& other);

      /**
      * SecureQueue default constructor (creates empty queue)
      */
      SecureQueue();

      /**
      * SecureQueue copy constructor
      * @param other the queue to copy
      */
      SecureQueue(const SecureQueue& other);

      ~SecureQueue() { destroy(); }

   private:
      void destroy();
      size_t m_bytes_read;
      class SecureQueueNode* m_head;
      class SecureQueueNode* m_tail;
   };

}


namespace Botan {

/**
* Serpent, an AES finalist
*/
class BOTAN_DLL Serpent : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Serpent"; }
      BlockCipher* clone() const override { return new Serpent; }
   protected:
      /**
      * For use by subclasses using SIMD, asm, etc
      * @return const reference to the key schedule
      */
      const secure_vector<u32bit>& get_round_keys() const
         { return round_key; }

      /**
      * For use by subclasses that implement the key schedule
      * @param ks is the new key schedule value to set
      */
      void set_round_keys(const u32bit ks[132])
         {
         round_key.assign(&ks[0], &ks[132]);
         }

   private:
      void key_schedule(const byte key[], size_t length) override;
      secure_vector<u32bit> round_key;
   };

}


namespace Botan {

/**
* Serpent implementation using SIMD
*/
class BOTAN_DLL Serpent_SIMD : public Serpent
   {
   public:
      size_t parallelism() const override { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      BlockCipher* clone() const override { return new Serpent_SIMD; }
   };

}


namespace Botan {

/**
* SHA-224
*/
class BOTAN_DLL SHA_224 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-224"; }
      size_t output_length() const override { return 28; }
      HashFunction* clone() const override { return new SHA_224; }

      void clear() override;

      SHA_224() : MDx_HashFunction(64, true, true), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u32bit> digest;
   };

/**
* SHA-256
*/
class BOTAN_DLL SHA_256 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-256"; }
      size_t output_length() const override { return 32; }
      HashFunction* clone() const override { return new SHA_256; }

      void clear() override;

      SHA_256() : MDx_HashFunction(64, true, true), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u32bit> digest;
   };

}


namespace Botan {

/**
* SHA-384
*/
class BOTAN_DLL SHA_384 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-384"; }
      size_t output_length() const override { return 48; }
      HashFunction* clone() const override { return new SHA_384; }

      void clear() override;

      SHA_384() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u64bit> m_digest;
   };

/**
* SHA-512
*/
class BOTAN_DLL SHA_512 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-512"; }
      size_t output_length() const override { return 64; }
      HashFunction* clone() const override { return new SHA_512; }

      void clear() override;

      SHA_512() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u64bit> m_digest;
   };

/**
* SHA-512/256
*/
class BOTAN_DLL SHA_512_256 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-512/256"; }
      size_t output_length() const override { return 32; }
      HashFunction* clone() const override { return new SHA_512_256; }

      void clear() override;

      SHA_512_256() : MDx_HashFunction(128, true, true, 16), m_digest(8) { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u64bit> m_digest;
   };

}


namespace Botan {

class BOTAN_DLL Stream_Cipher_Mode : public Cipher_Mode
   {
   public:
      Stream_Cipher_Mode(StreamCipher* cipher) : m_cipher(cipher) {}

      void update(secure_vector<byte>& buf, size_t offset) override
         {
         if(offset < buf.size())
            m_cipher->cipher1(&buf[offset], buf.size() - offset);
         }

      void finish(secure_vector<byte>& buf, size_t offset) override
         { return update(buf, offset); }

      size_t output_length(size_t input_length) const override { return input_length; }

      size_t update_granularity() const override { return 64; /* arbitrary */ }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return m_cipher->valid_iv_length(nonce_len); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      std::string name() const override { return m_cipher->name(); }

      void clear() override { return m_cipher->clear(); }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override
         {
         m_cipher->set_iv(nonce, nonce_len);
         return secure_vector<byte>();
         }

      void key_schedule(const byte key[], size_t length) override
         {
         m_cipher->set_key(key, length);
         }

      std::unique_ptr<StreamCipher> m_cipher;
   };

}


namespace Botan {

/**
* Filter interface for Transforms
*/
class BOTAN_DLL Transform_Filter : public Keyed_Filter,
                                   private Buffered_Filter
   {
   public:
      Transform_Filter(Transform* t);

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   protected:
      const Transform& get_transform() const { return *m_transform; }

      Transform& get_transform() { return *m_transform; }

   private:
      void write(const byte input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const byte input[], size_t input_length) override;
      void buffered_final(const byte input[], size_t input_length) override;

      class Nonce_State
         {
         public:
            Nonce_State(bool allow_null_nonce) : m_fresh_nonce(allow_null_nonce) {}

            void update(const InitializationVector& iv);
            std::vector<byte> get();
         private:
            bool m_fresh_nonce;
            std::vector<byte> m_nonce;
         };

      Nonce_State m_nonce;
      std::unique_ptr<Transform> m_transform;
      secure_vector<byte> m_buffer;
   };

typedef Transform_Filter Transformation_Filter;

}


#endif
