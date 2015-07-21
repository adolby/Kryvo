
#ifndef BOTAN_AMALGAMATION_INTERNAL_H__
#define BOTAN_AMALGAMATION_INTERNAL_H__

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>


namespace Botan {

template<typename T>
class Algo_Registry
   {
   public:
      typedef typename T::Spec Spec;

      typedef std::function<T* (const Spec&)> maker_fn;

      static Algo_Registry<T>& global_registry()
         {
         static Algo_Registry<T> g_registry;
         return g_registry;
         }

      void add(const std::string& name, const std::string& provider, maker_fn fn, byte pref)
         {
         std::unique_lock<std::mutex> lock(m_mutex);
         m_algo_info[name].add_provider(provider, fn, pref);
         }

      std::vector<std::string> providers_of(const Spec& spec)
         {
         std::unique_lock<std::mutex> lock(m_mutex);
         auto i = m_algo_info.find(spec.algo_name());
         if(i != m_algo_info.end())
            return i->second.providers();
         return std::vector<std::string>();
         }

      void set_provider_preference(const Spec& spec, const std::string& provider, byte pref)
         {
         std::unique_lock<std::mutex> lock(m_mutex);
         auto i = m_algo_info.find(spec.algo_name());
         if(i != m_algo_info.end())
            i->second.set_pref(provider, pref);
         }

      T* make(const Spec& spec, const std::string& provider = "")
         {
         const std::vector<maker_fn> makers = get_makers(spec, provider);

         try
            {
            for(auto&& maker : makers)
               {
               if(T* t = maker(spec))
                  return t;
               }
            }
         catch(std::exception& e)
            {
            throw std::runtime_error("Creating '" + spec.as_string() + "' failed: " + e.what());
            }

         return nullptr;
         }

      class Add
         {
         public:
            Add(const std::string& basename, maker_fn fn, const std::string& provider, byte pref)
               {
               Algo_Registry<T>::global_registry().add(basename, provider, fn, pref);
               }

            Add(bool cond, const std::string& basename, maker_fn fn, const std::string& provider, byte pref)
               {
               if(cond)
                  Algo_Registry<T>::global_registry().add(basename, provider, fn, pref);
               }
         };

   private:
      Algo_Registry() {}

      std::vector<maker_fn> get_makers(const Spec& spec, const std::string& provider)
         {
         std::unique_lock<std::mutex> lock(m_mutex);
         return m_algo_info[spec.algo_name()].get_makers(provider);
         }

      struct Algo_Info
         {
         public:
            void add_provider(const std::string& provider, maker_fn fn, byte pref)
               {
               if(m_maker_fns.count(provider) > 0)
                  throw std::runtime_error("Duplicated registration of '" + provider + "'");

               m_maker_fns[provider] = fn;
               m_prefs.insert(std::make_pair(pref, provider));
               }

            std::vector<std::string> providers() const
               {
               std::vector<std::string> v;
               for(auto&& k : m_prefs)
                  v.push_back(k.second);
               return v;
               }

            void set_pref(const std::string& provider, byte pref)
               {
               auto i = m_prefs.begin();
               while(i != m_prefs.end())
                  {
                  if(i->second == provider)
                     i = m_prefs.erase(i);
                  else
                     ++i;
                  }
               m_prefs.insert(std::make_pair(pref, provider));
               }

            std::vector<maker_fn> get_makers(const std::string& req_provider)
               {
               std::vector<maker_fn> r;

               if(req_provider != "")
                  {
                  // find one explicit provider requested by user or fail
                  auto i = m_maker_fns.find(req_provider);
                  if(i != m_maker_fns.end())
                     r.push_back(i->second);
                  }
               else
                  {
                  for(auto&& pref : m_prefs)
                     r.push_back(m_maker_fns[pref.second]);
                  }

               return r;
               }
         private:
            std::multimap<byte, std::string> m_prefs;
            std::unordered_map<std::string, maker_fn> m_maker_fns;
         };

      std::mutex m_mutex;
      std::unordered_map<std::string, Algo_Info> m_algo_info;
   };

template<typename T> T*
make_a(const typename T::Spec& spec, const std::string provider = "")
   {
   return Algo_Registry<T>::global_registry().make(spec, provider);
   }

template<typename T> std::vector<std::string> providers_of(const typename T::Spec& spec)
   {
   return Algo_Registry<T>::global_registry().providers_of(spec);
   }

template<typename T> T*
make_new_T(const typename Algo_Registry<T>::Spec& spec)
   {
   if(spec.arg_count() == 0)
      return new T;
   return nullptr;
   }

template<typename T, size_t DEF_VAL> T*
make_new_T_1len(const typename Algo_Registry<T>::Spec& spec)
   {
   return new T(spec.arg_as_integer(0, DEF_VAL));
   }

template<typename T, size_t DEF1, size_t DEF2> T*
make_new_T_2len(const typename Algo_Registry<T>::Spec& spec)
   {
   return new T(spec.arg_as_integer(0, DEF1), spec.arg_as_integer(1, DEF2));
   }

template<typename T> T*
make_new_T_1str(const typename Algo_Registry<T>::Spec& spec, const std::string& def)
   {
   return new T(spec.arg(0, def));
   }

template<typename T> T*
make_new_T_1str_req(const typename Algo_Registry<T>::Spec& spec)
   {
   return new T(spec.arg(0));
   }

template<typename T, typename X> T*
make_new_T_1X(const typename Algo_Registry<T>::Spec& spec)
   {
   std::unique_ptr<X> x(Algo_Registry<X>::global_registry().make(spec.arg(0)));
   if(!x)
      throw std::runtime_error(spec.arg(0));
   return new T(x.release());
   }

#define BOTAN_REGISTER_TYPE(T, type, name, maker, provider, pref)        \
   namespace { Algo_Registry<T>::Add g_ ## type ## _reg(name, maker, provider, pref); }

#define BOTAN_REGISTER_TYPE_COND(cond, T, type, name, maker, provider, pref) \
   namespace { Algo_Registry<T>::Add g_ ## type ## _reg(cond, name, maker, provider, pref); }

#define BOTAN_REGISTER_NAMED_T(T, name, type, maker)                 \
   BOTAN_REGISTER_TYPE(T, type, name, maker, "base", 128)

#define BOTAN_REGISTER_T(T, type, maker)                                \
   BOTAN_REGISTER_TYPE(T, type, #type, maker, "base", 128)

#define BOTAN_REGISTER_T_NOARGS(T, type) \
   BOTAN_REGISTER_TYPE(T, type, #type, make_new_T<type>, "base", 128)
#define BOTAN_REGISTER_T_1LEN(T, type, def) \
   BOTAN_REGISTER_TYPE(T, type, #type, (make_new_T_1len<type,def>), "base", 128)

#define BOTAN_REGISTER_NAMED_T_NOARGS(T, type, name, provider) \
   BOTAN_REGISTER_TYPE(T, type, name, make_new_T<type>, provider, 128)
#define BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, T, type, name, provider, pref) \
   BOTAN_REGISTER_TYPE_COND(cond, T, type, name, make_new_T<type>, provider, pref)

#define BOTAN_REGISTER_NAMED_T_2LEN(T, type, name, provider, len1, len2) \
   BOTAN_REGISTER_TYPE(T, type, name, (make_new_T_2len<type,len1,len2>), provider, 128)

// TODO move elsewhere:
#define BOTAN_REGISTER_TRANSFORM(name, maker) BOTAN_REGISTER_T(Transform, name, maker)
#define BOTAN_REGISTER_TRANSFORM_NOARGS(name) BOTAN_REGISTER_T_NOARGS(Transform, name)

}


namespace Botan {

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template<typename T>
inline bool is_power_of_2(T arg)
   {
   return ((arg != 0 && arg != 1) && ((arg & (arg-1)) == 0));
   }

/**
* Return the index of the highest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the highest set bit in n
*/
template<typename T>
inline size_t high_bit(T n)
   {
   for(size_t i = 8*sizeof(T); i > 0; --i)
      if((n >> (i - 1)) & 0x01)
         return i;
   return 0;
   }

/**
* Return the index of the lowest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the lowest set bit in n
*/
template<typename T>
inline size_t low_bit(T n)
   {
   for(size_t i = 0; i != 8*sizeof(T); ++i)
      if((n >> i) & 0x01)
         return (i + 1);
   return 0;
   }

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template<typename T>
inline size_t significant_bytes(T n)
   {
   for(size_t i = 0; i != sizeof(T); ++i)
      if(get_byte(i, n))
         return sizeof(T)-i;
   return 0;
   }

/**
* Compute Hamming weights
* @param n an integer value
* @return number of bits in n set to 1
*/
template<typename T>
inline size_t hamming_weight(T n)
   {
   const byte NIBBLE_WEIGHTS[] = {
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

   size_t weight = 0;
   for(size_t i = 0; i != 2*sizeof(T); ++i)
      weight += NIBBLE_WEIGHTS[(n >> (4*i)) & 0x0F];
   return weight;
   }

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template<typename T>
inline size_t ctz(T n)
   {
   for(size_t i = 0; i != 8*sizeof(T); ++i)
      if((n >> i) & 0x01)
         return i;
   return 8*sizeof(T);
   }

template<typename T>
size_t ceil_log2(T x)
   {
   if(x >> (sizeof(T)*8-1))
      return sizeof(T)*8;

   size_t result = 0;
   T compare = 1;

   while(compare < x)
      {
      compare <<= 1;
      result++;
      }

   return result;
   }

}


namespace Botan {

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


namespace Botan {

#define BOTAN_REGISTER_BLOCK_CIPHER(name, maker) BOTAN_REGISTER_T(BlockCipher, name, maker)
#define BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(name) BOTAN_REGISTER_T_NOARGS(BlockCipher, name)

#define BOTAN_REGISTER_BLOCK_CIPHER_1LEN(name, def) BOTAN_REGISTER_T_1LEN(BlockCipher, name, def)

#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(type, name) BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, make_new_T<type>)
#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, (make_new_T_1len<type,def>))
#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1STR(type, name, def) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, std::bind(make_new_T_1str<type>, std::placeholders::_1, def));

#define BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(cond, type, name, provider, pref) \
   BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, BlockCipher, type, name, provider, pref)

}


namespace Botan {

class donna128
   {
   public:
      donna128(u64bit ll = 0, u64bit hh = 0) { l = ll; h = hh; }

      donna128(const donna128&) = default;
      donna128& operator=(const donna128&) = default;

      friend donna128 operator>>(const donna128& x, size_t shift)
         {
         donna128 z = x;
         const u64bit carry = z.h << (64 - shift);
         z.h = (z.h >> shift);
         z.l = (z.l >> shift) | carry;
         return z;
         }

      friend donna128 operator<<(const donna128& x, size_t shift)
         {
         donna128 z = x;
         const u64bit carry = z.l >> (64 - shift);
         z.l = (z.l << shift);
         z.h = (z.h << shift) | carry;
         return z;
         }

      friend u64bit operator&(const donna128& x, u64bit mask)
         {
         return x.l & mask;
         }

      u64bit operator&=(u64bit mask)
         {
         h = 0;
         l &= mask;
         return l;
         }

      donna128& operator+=(const donna128& x)
         {
         l += x.l;
         h += (l < x.l);
         h += x.h;
         return *this;
         }

      donna128& operator+=(u64bit x)
         {
         l += x;
         h += (l < x);
         return *this;
         }

      u64bit lo() const { return l; }
      u64bit hi() const { return h; }
   private:
      u64bit h = 0, l = 0;
   };

inline donna128 operator*(const donna128& x, u64bit y)
   {
   BOTAN_ASSERT(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   u64bit lo = 0, hi = 0;
   mul64x64_128(x.lo(), y, &lo, &hi);
   return donna128(lo, hi);
   }

inline donna128 operator+(const donna128& x, const donna128& y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator+(const donna128& x, u64bit y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator|(const donna128& x, const donna128& y)
   {
   return donna128(x.lo() | y.lo(), x.hi() | y.hi());
   }

inline u64bit carry_shift(const donna128& a, size_t shift)
   {
   return (a >> shift).lo();
   }

inline u64bit combine_lower(const donna128 a, size_t s1,
                            const donna128 b, size_t s2)
   {
   donna128 z = (a >> s1) | (b << s2);
   return z.lo();
   }

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
inline u64bit carry_shift(const uint128_t a, size_t shift)
   {
   return static_cast<u64bit>(a >> shift);
   }

inline u64bit combine_lower(const uint128_t a, size_t s1,
                            const uint128_t b, size_t s2)
   {
   return static_cast<u64bit>((a >> s1) | (b << s2));
   }
#endif

}


namespace Botan {

/**
* Win32 Entropy Source
*/
class Win32_EntropySource : public EntropySource
   {
   public:
      std::string name() const { return "Win32 Statistics"; }
      void poll(Entropy_Accumulator& accum);
   };

}


namespace Botan {

#define BOTAN_REGISTER_HASH(name, maker) BOTAN_REGISTER_T(HashFunction, name, maker)
#define BOTAN_REGISTER_HASH_NOARGS(name) BOTAN_REGISTER_T_NOARGS(HashFunction, name)

#define BOTAN_REGISTER_HASH_1LEN(name, def) BOTAN_REGISTER_T_1LEN(HashFunction, name, def)

#define BOTAN_REGISTER_HASH_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(HashFunction, name, type, make_new_T<type>)
#define BOTAN_REGISTER_HASH_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(HashFunction, name, type, (make_new_T_1len<type,def>))

#define BOTAN_REGISTER_HASH_NOARGS_IF(cond, type, name, provider, pref)      \
   BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, HashFunction, type, name, provider, pref)

}


namespace Botan {

#define BOTAN_REGISTER_KDF_NOARGS(type, name)                    \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T<type>))
#define BOTAN_REGISTER_KDF_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T_1X<type, HashFunction>))

#define BOTAN_REGISTER_KDF_NAMED_1STR(type, name) \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T_1str_req<type>))

}


namespace Botan {

#define BOTAN_REGISTER_MAC(name, maker) BOTAN_REGISTER_T(MessageAuthenticationCode, name, maker)
#define BOTAN_REGISTER_MAC_NOARGS(name) BOTAN_REGISTER_T_NOARGS(MessageAuthenticationCode, name)

#define BOTAN_REGISTER_MAC_1LEN(name, def) BOTAN_REGISTER_T_1LEN(MessageAuthenticationCode, name, def)

#define BOTAN_REGISTER_MAC_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, name, type, make_new_T<type>)

#define BOTAN_REGISTER_MAC_NAMED_1LEN(type, name, def)                  \
   BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, name, type, (make_new_T_1len<type,def>))
#define BOTAN_REGISTER_MAC_NAMED_1STR(type, name, def) \
   BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, name, type, \
                          std::bind(make_new_T_1str<type>, std::placeholders::_1, def));

}


namespace Botan {

/**
* Round up
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
template<typename T>
inline T round_up(T n, T align_to)
   {
   if(align_to == 0)
      return n;

   if(n % align_to || n == 0)
      n += align_to - (n % align_to);
   return n;
   }

/**
* Round down
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded down to a multiple of align_to
*/
template<typename T>
inline T round_down(T n, T align_to)
   {
   if(align_to == 0)
      return n;

   return (n - (n % align_to));
   }

/**
* Clamp
*/
inline size_t clamp(size_t n, size_t lower_bound, size_t upper_bound)
   {
   if(n < lower_bound)
      return lower_bound;
   if(n > upper_bound)
      return upper_bound;
   return n;
   }

}


namespace Botan {

template<typename T>
T* make_block_cipher_mode(const Transform::Spec& spec)
   {
   if(BlockCipher* bc = get_block_cipher(spec.arg(0)))
      return new T(bc);
   return nullptr;
   }

template<typename T, size_t LEN1>
T* make_block_cipher_mode_len(const Transform::Spec& spec)
   {
   if(BlockCipher* bc = get_block_cipher(spec.arg(0)))
      {
      const size_t len1 = spec.arg_as_integer(1, LEN1);
      return new T(bc, len1);
      }

   return nullptr;
   }

template<typename T, size_t LEN1, size_t LEN2>
T* make_block_cipher_mode_len2(const Transform::Spec& spec)
   {
   if(BlockCipher* bc = get_block_cipher(spec.arg(0)))
      {
      const size_t len1 = spec.arg_as_integer(1, LEN1);
      const size_t len2 = spec.arg_as_integer(2, LEN2);
      return new T(bc, len1, len2);
      }

   return nullptr;
   }

#define BOTAN_REGISTER_BLOCK_CIPHER_MODE(E, D)                          \
   BOTAN_REGISTER_NAMED_T(Transform, #E, E, make_block_cipher_mode<E>); \
   BOTAN_REGISTER_NAMED_T(Transform, #D, D, make_block_cipher_mode<D>);

#define BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN(E, D, LEN)                          \
   BOTAN_REGISTER_NAMED_T(Transform, #E, E, (make_block_cipher_mode_len<E, LEN>)); \
   BOTAN_REGISTER_NAMED_T(Transform, #D, D, (make_block_cipher_mode_len<D, LEN>));

#define BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN2(E, D, LEN1, LEN2)                          \
   BOTAN_REGISTER_NAMED_T(Transform, #E, E, (make_block_cipher_mode_len2<E, LEN1, LEN2>)); \
   BOTAN_REGISTER_NAMED_T(Transform, #D, D, (make_block_cipher_mode_len2<D, LEN1, LEN2>));

}


namespace Botan {

#define BOTAN_REGISTER_PBKDF_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(PBKDF, name, type, (make_new_T_1X<type, HashFunction>))
#define BOTAN_REGISTER_PBKDF_1MAC(type, name)                    \
   BOTAN_REGISTER_NAMED_T(PBKDF, name, type, (make_new_T_1X<type, MessageAuthenticationCode>))

}


namespace Botan {

template<typename T>
inline void prefetch_readonly(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 0);
#endif
   }

template<typename T>
inline void prefetch_readwrite(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 1);
#endif
   }

}


namespace Botan {

class Semaphore
   {
   public:
      Semaphore(int value = 0) : m_value(value), m_wakeups(0) {}

      void acquire();

      void release(size_t n = 1);

   private:
      int m_value;
      int m_wakeups;
      std::mutex m_mutex;
      std::condition_variable m_cond;
   };

}

#define SBoxE1(B0, B1, B2, B3)                    \
   do {                                           \
      B3 ^= B0;                                   \
      auto B4 = B1;                               \
      B1 &= B3;                                   \
      B4 ^= B2;                                   \
      B1 ^= B0;                                   \
      B0 |= B3;                                   \
      B0 ^= B4;                                   \
      B4 ^= B3;                                   \
      B3 ^= B2;                                   \
      B2 |= B1;                                   \
      B2 ^= B4;                                   \
      B4 = ~B4;                                   \
      B4 |= B1;                                   \
      B1 ^= B3;                                   \
      B1 ^= B4;                                   \
      B3 |= B0;                                   \
      B1 ^= B3;                                   \
      B4 ^= B3;                                   \
      B3 = B0;                                    \
      B0 = B1;                                    \
      B1 = B4;                                    \
   } while(0);

#define SBoxE2(B0, B1, B2, B3)                    \
   do {                                           \
      B0 = ~B0;                                   \
      B2 = ~B2;                                   \
      auto B4 = B0;                               \
      B0 &= B1;                                   \
      B2 ^= B0;                                   \
      B0 |= B3;                                   \
      B3 ^= B2;                                   \
      B1 ^= B0;                                   \
      B0 ^= B4;                                   \
      B4 |= B1;                                   \
      B1 ^= B3;                                   \
      B2 |= B0;                                   \
      B2 &= B4;                                   \
      B0 ^= B1;                                   \
      B1 &= B2;                                   \
      B1 ^= B0;                                   \
      B0 &= B2;                                   \
      B4 ^= B0;                                   \
      B0 = B2;                                    \
      B2 = B3;                                    \
      B3 = B1;                                    \
      B1 = B4;                                    \
   } while(0);

#define SBoxE3(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B0;                               \
      B0 &= B2;                                   \
      B0 ^= B3;                                   \
      B2 ^= B1;                                   \
      B2 ^= B0;                                   \
      B3 |= B4;                                   \
      B3 ^= B1;                                   \
      B4 ^= B2;                                   \
      B1 = B3;                                    \
      B3 |= B4;                                   \
      B3 ^= B0;                                   \
      B0 &= B1;                                   \
      B4 ^= B0;                                   \
      B1 ^= B3;                                   \
      B1 ^= B4;                                   \
      B0 = B2;                                    \
      B2 = B1;                                    \
      B1 = B3;                                    \
      B3 = ~B4;                                   \
   } while(0);

#define SBoxE4(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B0;                               \
      B0 |= B3;                                   \
      B3 ^= B1;                                   \
      B1 &= B4;                                   \
      B4 ^= B2;                                   \
      B2 ^= B3;                                   \
      B3 &= B0;                                   \
      B4 |= B1;                                   \
      B3 ^= B4;                                   \
      B0 ^= B1;                                   \
      B4 &= B0;                                   \
      B1 ^= B3;                                   \
      B4 ^= B2;                                   \
      B1 |= B0;                                   \
      B1 ^= B2;                                   \
      B0 ^= B3;                                   \
      B2 = B1;                                    \
      B1 |= B3;                                   \
      B0 ^= B1;                                   \
      B1 = B2;                                    \
      B2 = B3;                                    \
      B3 = B4;                                    \
   } while(0);

#define SBoxE5(B0, B1, B2, B3)                    \
   do {                                           \
      B1 ^= B3;                                   \
      B3 = ~B3;                                   \
      B2 ^= B3;                                   \
      B3 ^= B0;                                   \
      auto B4 = B1;                               \
      B1 &= B3;                                   \
      B1 ^= B2;                                   \
      B4 ^= B3;                                   \
      B0 ^= B4;                                   \
      B2 &= B4;                                   \
      B2 ^= B0;                                   \
      B0 &= B1;                                   \
      B3 ^= B0;                                   \
      B4 |= B1;                                   \
      B4 ^= B0;                                   \
      B0 |= B3;                                   \
      B0 ^= B2;                                   \
      B2 &= B3;                                   \
      B0 = ~B0;                                   \
      B4 ^= B2;                                   \
      B2 = B0;                                    \
      B0 = B1;                                    \
      B1 = B4;                                    \
   } while(0);

#define SBoxE6(B0, B1, B2, B3)                    \
   do {                                           \
      B0 ^= B1;                                   \
      B1 ^= B3;                                   \
      B3 = ~B3;                                   \
      auto B4 = B1;                               \
      B1 &= B0;                                   \
      B2 ^= B3;                                   \
      B1 ^= B2;                                   \
      B2 |= B4;                                   \
      B4 ^= B3;                                   \
      B3 &= B1;                                   \
      B3 ^= B0;                                   \
      B4 ^= B1;                                   \
      B4 ^= B2;                                   \
      B2 ^= B0;                                   \
      B0 &= B3;                                   \
      B2 = ~B2;                                   \
      B0 ^= B4;                                   \
      B4 |= B3;                                   \
      B4 ^= B2;                                   \
      B2 = B0;                                    \
      B0 = B1;                                    \
      B1 = B3;                                    \
      B3 = B4;                                    \
   } while(0);

#define SBoxE7(B0, B1, B2, B3)                    \
   do {                                           \
      B2 = ~B2;                                   \
      auto B4 = B3;                               \
      B3 &= B0;                                   \
      B0 ^= B4;                                   \
      B3 ^= B2;                                   \
      B2 |= B4;                                   \
      B1 ^= B3;                                   \
      B2 ^= B0;                                   \
      B0 |= B1;                                   \
      B2 ^= B1;                                   \
      B4 ^= B0;                                   \
      B0 |= B3;                                   \
      B0 ^= B2;                                   \
      B4 ^= B3;                                   \
      B4 ^= B0;                                   \
      B3 = ~B3;                                   \
      B2 &= B4;                                   \
      B3 ^= B2;                                   \
      B2 = B4;                                    \
   } while(0);

#define SBoxE8(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B1;                               \
      B1 |= B2;                                   \
      B1 ^= B3;                                   \
      B4 ^= B2;                                   \
      B2 ^= B1;                                   \
      B3 |= B4;                                   \
      B3 &= B0;                                   \
      B4 ^= B2;                                   \
      B3 ^= B1;                                   \
      B1 |= B4;                                   \
      B1 ^= B0;                                   \
      B0 |= B4;                                   \
      B0 ^= B2;                                   \
      B1 ^= B4;                                   \
      B2 ^= B1;                                   \
      B1 &= B0;                                   \
      B1 ^= B4;                                   \
      B2 = ~B2;                                   \
      B2 |= B0;                                   \
      B4 ^= B2;                                   \
      B2 = B1;                                    \
      B1 = B3;                                    \
      B3 = B0;                                    \
      B0 = B4;                                    \
   } while(0);

#define SBoxD1(B0, B1, B2, B3)                    \
   do {                                           \
      B2 = ~B2;                                   \
      auto B4 = B1;                               \
      B1 |= B0;                                   \
      B4 = ~B4;                                   \
      B1 ^= B2;                                   \
      B2 |= B4;                                   \
      B1 ^= B3;                                   \
      B0 ^= B4;                                   \
      B2 ^= B0;                                   \
      B0 &= B3;                                   \
      B4 ^= B0;                                   \
      B0 |= B1;                                   \
      B0 ^= B2;                                   \
      B3 ^= B4;                                   \
      B2 ^= B1;                                   \
      B3 ^= B0;                                   \
      B3 ^= B1;                                   \
      B2 &= B3;                                   \
      B4 ^= B2;                                   \
      B2 = B1;                                    \
      B1 = B4;                                    \
      } while(0);

#define SBoxD2(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B1;                               \
      B1 ^= B3;                                   \
      B3 &= B1;                                   \
      B4 ^= B2;                                   \
      B3 ^= B0;                                   \
      B0 |= B1;                                   \
      B2 ^= B3;                                   \
      B0 ^= B4;                                   \
      B0 |= B2;                                   \
      B1 ^= B3;                                   \
      B0 ^= B1;                                   \
      B1 |= B3;                                   \
      B1 ^= B0;                                   \
      B4 = ~B4;                                   \
      B4 ^= B1;                                   \
      B1 |= B0;                                   \
      B1 ^= B0;                                   \
      B1 |= B4;                                   \
      B3 ^= B1;                                   \
      B1 = B0;                                    \
      B0 = B4;                                    \
      B4 = B2;                                    \
      B2 = B3;                                    \
      B3 = B4;                                    \
      } while(0);

#define SBoxD3(B0, B1, B2, B3)                    \
   do {                                           \
      B2 ^= B3;                                   \
      B3 ^= B0;                                   \
      auto B4 = B3;                               \
      B3 &= B2;                                   \
      B3 ^= B1;                                   \
      B1 |= B2;                                   \
      B1 ^= B4;                                   \
      B4 &= B3;                                   \
      B2 ^= B3;                                   \
      B4 &= B0;                                   \
      B4 ^= B2;                                   \
      B2 &= B1;                                   \
      B2 |= B0;                                   \
      B3 = ~B3;                                   \
      B2 ^= B3;                                   \
      B0 ^= B3;                                   \
      B0 &= B1;                                   \
      B3 ^= B4;                                   \
      B3 ^= B0;                                   \
      B0 = B1;                                    \
      B1 = B4;                                    \
      } while(0);

#define SBoxD4(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B2;                               \
      B2 ^= B1;                                   \
      B0 ^= B2;                                   \
      B4 &= B2;                                   \
      B4 ^= B0;                                   \
      B0 &= B1;                                   \
      B1 ^= B3;                                   \
      B3 |= B4;                                   \
      B2 ^= B3;                                   \
      B0 ^= B3;                                   \
      B1 ^= B4;                                   \
      B3 &= B2;                                   \
      B3 ^= B1;                                   \
      B1 ^= B0;                                   \
      B1 |= B2;                                   \
      B0 ^= B3;                                   \
      B1 ^= B4;                                   \
      B0 ^= B1;                                   \
      B4 = B0;                                    \
      B0 = B2;                                    \
      B2 = B3;                                    \
      B3 = B4;                                    \
      } while(0);

#define SBoxD5(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B2;                               \
      B2 &= B3;                                   \
      B2 ^= B1;                                   \
      B1 |= B3;                                   \
      B1 &= B0;                                   \
      B4 ^= B2;                                   \
      B4 ^= B1;                                   \
      B1 &= B2;                                   \
      B0 = ~B0;                                   \
      B3 ^= B4;                                   \
      B1 ^= B3;                                   \
      B3 &= B0;                                   \
      B3 ^= B2;                                   \
      B0 ^= B1;                                   \
      B2 &= B0;                                   \
      B3 ^= B0;                                   \
      B2 ^= B4;                                   \
      B2 |= B3;                                   \
      B3 ^= B0;                                   \
      B2 ^= B1;                                   \
      B1 = B3;                                    \
      B3 = B4;                                    \
      } while(0);

#define SBoxD6(B0, B1, B2, B3)                    \
   do {                                           \
      B1 = ~B1;                                   \
      auto B4 = B3;                               \
      B2 ^= B1;                                   \
      B3 |= B0;                                   \
      B3 ^= B2;                                   \
      B2 |= B1;                                   \
      B2 &= B0;                                   \
      B4 ^= B3;                                   \
      B2 ^= B4;                                   \
      B4 |= B0;                                   \
      B4 ^= B1;                                   \
      B1 &= B2;                                   \
      B1 ^= B3;                                   \
      B4 ^= B2;                                   \
      B3 &= B4;                                   \
      B4 ^= B1;                                   \
      B3 ^= B4;                                   \
      B4 = ~B4;                                   \
      B3 ^= B0;                                   \
      B0 = B1;                                    \
      B1 = B4;                                    \
      B4 = B3;                                    \
      B3 = B2;                                    \
      B2 = B4;                                    \
      } while(0);

#define SBoxD7(B0, B1, B2, B3)                    \
   do {                                           \
      B0 ^= B2;                                   \
      auto B4 = B2;                               \
      B2 &= B0;                                   \
      B4 ^= B3;                                   \
      B2 = ~B2;                                   \
      B3 ^= B1;                                   \
      B2 ^= B3;                                   \
      B4 |= B0;                                   \
      B0 ^= B2;                                   \
      B3 ^= B4;                                   \
      B4 ^= B1;                                   \
      B1 &= B3;                                   \
      B1 ^= B0;                                   \
      B0 ^= B3;                                   \
      B0 |= B2;                                   \
      B3 ^= B1;                                   \
      B4 ^= B0;                                   \
      B0 = B1;                                    \
      B1 = B2;                                    \
      B2 = B4;                                    \
      } while(0);

#define SBoxD8(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B2;                               \
      B2 ^= B0;                                   \
      B0 &= B3;                                   \
      B4 |= B3;                                   \
      B2 = ~B2;                                   \
      B3 ^= B1;                                   \
      B1 |= B0;                                   \
      B0 ^= B2;                                   \
      B2 &= B4;                                   \
      B3 &= B4;                                   \
      B1 ^= B2;                                   \
      B2 ^= B0;                                   \
      B0 |= B2;                                   \
      B4 ^= B1;                                   \
      B0 ^= B3;                                   \
      B3 ^= B4;                                   \
      B4 |= B0;                                   \
      B3 ^= B2;                                   \
      B4 ^= B2;                                   \
      B2 = B1;                                    \
      B1 = B0;                                    \
      B0 = B3;                                    \
      B3 = B4;                                    \
      } while(0);


#if defined(BOTAN_HAS_SIMD_SSE2)
#if defined(BOTAN_TARGET_SUPPORTS_SSE2)

#include <emmintrin.h>

namespace Botan {

class SIMD_SSE2
   {
   public:
      static bool enabled() { return CPUID::has_sse2(); }

      SIMD_SSE2(const u32bit B[4])
         {
         reg = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B));
         }

      SIMD_SSE2(u32bit B0, u32bit B1, u32bit B2, u32bit B3)
         {
         reg = _mm_set_epi32(B0, B1, B2, B3);
         }

      SIMD_SSE2(u32bit B)
         {
         reg = _mm_set1_epi32(B);
         }

      static SIMD_SSE2 load_le(const void* in)
         {
         return _mm_loadu_si128(reinterpret_cast<const __m128i*>(in));
         }

      static SIMD_SSE2 load_be(const void* in)
         {
         return load_le(in).bswap();
         }

      void store_le(byte out[]) const
         {
         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), reg);
         }

      void store_be(byte out[]) const
         {
         bswap().store_le(out);
         }

      void rotate_left(size_t rot)
         {
         reg = _mm_or_si128(_mm_slli_epi32(reg, static_cast<int>(rot)),
                            _mm_srli_epi32(reg, static_cast<int>(32-rot)));
         }

      void rotate_right(size_t rot)
         {
         rotate_left(32 - rot);
         }

      void operator+=(const SIMD_SSE2& other)
         {
         reg = _mm_add_epi32(reg, other.reg);
         }

      SIMD_SSE2 operator+(const SIMD_SSE2& other) const
         {
         return _mm_add_epi32(reg, other.reg);
         }

      void operator-=(const SIMD_SSE2& other)
         {
         reg = _mm_sub_epi32(reg, other.reg);
         }

      SIMD_SSE2 operator-(const SIMD_SSE2& other) const
         {
         return _mm_sub_epi32(reg, other.reg);
         }

      void operator^=(const SIMD_SSE2& other)
         {
         reg = _mm_xor_si128(reg, other.reg);
         }

      SIMD_SSE2 operator^(const SIMD_SSE2& other) const
         {
         return _mm_xor_si128(reg, other.reg);
         }

      void operator|=(const SIMD_SSE2& other)
         {
         reg = _mm_or_si128(reg, other.reg);
         }

      SIMD_SSE2 operator&(const SIMD_SSE2& other)
         {
         return _mm_and_si128(reg, other.reg);
         }

      void operator&=(const SIMD_SSE2& other)
         {
         reg = _mm_and_si128(reg, other.reg);
         }

      SIMD_SSE2 operator<<(size_t shift) const
         {
         return _mm_slli_epi32(reg, static_cast<int>(shift));
         }

      SIMD_SSE2 operator>>(size_t shift) const
         {
         return _mm_srli_epi32(reg, static_cast<int>(shift));
         }

      SIMD_SSE2 operator~() const
         {
         return _mm_xor_si128(reg, _mm_set1_epi32(0xFFFFFFFF));
         }

      // (~reg) & other
      SIMD_SSE2 andc(const SIMD_SSE2& other)
         {
         return _mm_andnot_si128(reg, other.reg);
         }

      SIMD_SSE2 bswap() const
         {
         __m128i T = reg;

         T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

         return _mm_or_si128(_mm_srli_epi16(T, 8),
                             _mm_slli_epi16(T, 8));
         }

      static void transpose(SIMD_SSE2& B0, SIMD_SSE2& B1,
                            SIMD_SSE2& B2, SIMD_SSE2& B3)
         {
         __m128i T0 = _mm_unpacklo_epi32(B0.reg, B1.reg);
         __m128i T1 = _mm_unpacklo_epi32(B2.reg, B3.reg);
         __m128i T2 = _mm_unpackhi_epi32(B0.reg, B1.reg);
         __m128i T3 = _mm_unpackhi_epi32(B2.reg, B3.reg);
         B0.reg = _mm_unpacklo_epi64(T0, T1);
         B1.reg = _mm_unpackhi_epi64(T0, T1);
         B2.reg = _mm_unpacklo_epi64(T2, T3);
         B3.reg = _mm_unpackhi_epi64(T2, T3);
         }

   private:
      SIMD_SSE2(__m128i in) { reg = in; }

      __m128i reg;
   };

}

#endif

  namespace Botan { typedef SIMD_SSE2 SIMD_32; }

#elif defined(BOTAN_HAS_SIMD_ALTIVEC)
  namespace Botan { typedef SIMD_Altivec SIMD_32; }

#elif defined(BOTAN_HAS_SIMD_SCALAR)
  namespace Botan { typedef SIMD_Scalar<u32bit,4> SIMD_32; }

#else
  #error "No SIMD module defined"

#endif


namespace Botan {

inline std::vector<byte> to_byte_vector(const std::string& s)
   {
   return std::vector<byte>(s.cbegin(), s.cend());
   }

/*
* Searching through a std::map
* @param mapping the map to search
* @param key is what to look for
* @param null_result is the value to return if key is not in mapping
* @return mapping[key] or null_result
*/
template<typename K, typename V>
inline V search_map(const std::map<K, V>& mapping,
                    const K& key,
                    const V& null_result = V())
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return i->second;
   }

template<typename K, typename V, typename R>
inline R search_map(const std::map<K, V>& mapping, const K& key,
                    const R& null_result, const R& found_result)
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return found_result;
   }

/*
* Insert a key/value pair into a multimap
*/
template<typename K, typename V>
void multimap_insert(std::multimap<K, V>& multimap,
                     const K& key, const V& value)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_SUN_STUDIO)
   // Work around a strange bug in Sun Studio
   multimap.insert(std::make_pair<const K, V>(key, value));
#else
   multimap.insert(std::make_pair(key, value));
#endif
   }

/**
* Existence check for values
*/
template<typename T>
bool value_exists(const std::vector<T>& vec,
                  const T& val)
   {
   for(size_t i = 0; i != vec.size(); ++i)
      if(vec[i] == val)
         return true;
   return false;
   }

template<typename T, typename Pred>
void map_remove_if(Pred pred, T& assoc)
   {
   auto i = assoc.begin();
   while(i != assoc.end())
      {
      if(pred(i->first))
         assoc.erase(i++);
      else
         i++;
      }
   }

}


namespace Botan {

#define BOTAN_REGISTER_STREAM_CIPHER(name, maker) BOTAN_REGISTER_T(StreamCipher, name, maker)
#define BOTAN_REGISTER_STREAM_CIPHER_NOARGS(name) BOTAN_REGISTER_T_NOARGS(StreamCipher, name)

#define BOTAN_REGISTER_STREAM_CIPHER_1LEN(name, def) BOTAN_REGISTER_T_1LEN(StreamCipher, name, def)

#define BOTAN_REGISTER_STREAM_CIPHER_NAMED_NOARGS(type, name) BOTAN_REGISTER_NAMED_T(StreamCipher, name, type, make_new_T<type>)
#define BOTAN_REGISTER_STREAM_CIPHER_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(StreamCipher, name, type, (make_new_T_1len<type,def>))

}


namespace Botan {

namespace TA_CM {

/**
* Function used in timing attack countermeasures
* See Wagner, Molnar, et al "The Program Counter Security Model"
*
* @param in an integer
* @return 0 if in == 0 else 0xFFFFFFFF
*/
u32bit expand_mask_u32bit(u32bit in);


/**
 * Expand an input to a bit mask depending on it being being zero or
 * non-zero
 * @ param in the input
 * @return the mask 0xFFFF if tst is non-zero and 0 otherwise
 */
u16bit expand_mask_u16bit(u16bit in);

/**
* Branch-free maximum
* Note: assumes twos-complement signed representation
* @param a an integer
* @param b an integer
* @return max(a,b)
*/
u32bit max_32(u32bit a, u32bit b);

/**
* Branch-free minimum
* Note: assumes twos-complement signed representation
* @param a an integer
* @param b an integer
* @return min(a,b)
*/
u32bit min_32(u32bit a, u32bit b);

}

}


#endif
