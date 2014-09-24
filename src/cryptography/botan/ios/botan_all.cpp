/*
* Botan 1.11.9 Amalgamation
* (C) 1999-2013,2014 Jack Lloyd and others
*
* Distributed under the terms of the Botan license
*/

#include "botan_all.h"
#include <map>
#include <string>
#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>


namespace Botan {

/**
* Core Engine
*/
class Core_Engine : public Engine
   {
   public:
      std::string provider_name() const override { return "core"; }

      PK_Ops::Key_Agreement*
         get_key_agreement_op(const Private_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Signature*
         get_signature_op(const Private_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Verification* get_verify_op(const Public_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Encryption* get_encryption_op(const Public_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Decryption* get_decryption_op(const Private_Key& key, RandomNumberGenerator& rng) const override;

      Modular_Exponentiator* mod_exp(const BigInt& n,
                                     Power_Mod::Usage_Hints) const override;

      Keyed_Filter* get_cipher(const std::string&, Cipher_Dir,
                               Algorithm_Factory&);

      BlockCipher* find_block_cipher(const SCAN_Name&,
                                     Algorithm_Factory&) const override;

      StreamCipher* find_stream_cipher(const SCAN_Name&,
                                       Algorithm_Factory&) const override;

      HashFunction* find_hash(const SCAN_Name& request,
                              Algorithm_Factory&) const override;

      MessageAuthenticationCode* find_mac(const SCAN_Name& request,
                                          Algorithm_Factory&) const override;

      PBKDF* find_pbkdf(const SCAN_Name& algo_spec,
                        Algorithm_Factory& af) const override;
   };

/**
* Create a cipher mode filter object
* @param block_cipher a block cipher object
* @param direction are we encrypting or decrypting?
* @param mode the name of the cipher mode to use
* @param padding the mode padding to use (only used for ECB, CBC)
*/
Keyed_Filter* get_cipher_mode(const BlockCipher* block_cipher,
                              Cipher_Dir direction,
                              const std::string& mode,
                              const std::string& padding);

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

inline void xor_buf(byte out[], const byte in[], size_t length)
   {
   while(length >= 8)
      {
      *reinterpret_cast<u64bit*>(out) ^= *reinterpret_cast<const u64bit*>(in);
      out += 8; in += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] ^= in[i];
   }

inline void xor_buf(byte out[],
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
   xor_buf(&out[0], &in[0], n);
   }

template<typename Alloc>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             size_t n)
   {
   xor_buf(&out[0], in, n);
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             const std::vector<byte, Alloc2>& in2,
             size_t n)
   {
   xor_buf(&out[0], &in[0], &in2[0], n);
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator^=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   if(out.size() < in.size())
      out.resize(in.size());

   xor_buf(&out[0], &in[0], in.size());
   return out;
   }

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

}


namespace Botan {

inline std::vector<byte> to_byte_vector(const std::string& s)
   {
   return std::vector<byte>(reinterpret_cast<const byte*>(&s[0]),
                            reinterpret_cast<const byte*>(&s[s.size()]));
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

/**
* @param prov_name a provider name
* @return weight for this provider
*/
size_t static_provider_weight(const std::string& prov_name);

/**
* Algorithm_Cache (used by Algorithm_Factory)
*/
template<typename T>
class Algorithm_Cache
   {
   public:
      /**
      * @param algo_spec names the requested algorithm
      * @param pref_provider suggests a preferred provider
      * @return prototype object, or NULL
      */
      const T* get(const std::string& algo_spec,
                   const std::string& pref_provider);

      /**
      * Add a new algorithm implementation to the cache
      * @param algo the algorithm prototype object
      * @param requested_name how this name will be requested
      * @param provider_name is the name of the provider of this prototype
      */
      void add(T* algo,
               const std::string& requested_name,
               const std::string& provider_name);

      /**
      * Set the preferred provider
      * @param algo_spec names the algorithm
      * @param provider names the preferred provider
      */
      void set_preferred_provider(const std::string& algo_spec,
                                  const std::string& provider);

      /**
      * Return the list of providers of this algorithm
      * @param algo_name names the algorithm
      * @return list of providers of this algorithm
      */
      std::vector<std::string> providers_of(const std::string& algo_name);

      /**
      * Clear the cache
      */
      void clear_cache();

      ~Algorithm_Cache() { clear_cache(); }
   private:
      typename std::map<std::string, std::map<std::string, T*> >::const_iterator
         find_algorithm(const std::string& algo_spec);

      std::mutex mutex;
      std::map<std::string, std::string> aliases;
      std::map<std::string, std::string> pref_providers;
      std::map<std::string, std::map<std::string, T*> > algorithms;
   };

/*
* Look for an algorithm implementation in the cache, also checking aliases
* Assumes object lock is held
*/
template<typename T>
typename std::map<std::string, std::map<std::string, T*> >::const_iterator
Algorithm_Cache<T>::find_algorithm(const std::string& algo_spec)
   {
   auto algo = algorithms.find(algo_spec);

   // Not found? Check if a known alias
   if(algo == algorithms.end())
      {
      auto alias = aliases.find(algo_spec);

      if(alias != aliases.end())
         algo = algorithms.find(alias->second);
      }

   return algo;
   }

/*
* Look for an algorithm implementation by a particular provider
*/
template<typename T>
const T* Algorithm_Cache<T>::get(const std::string& algo_spec,
                                 const std::string& requested_provider)
   {
   std::lock_guard<std::mutex> lock(mutex);

   auto algo = find_algorithm(algo_spec);
   if(algo == algorithms.end()) // algo not found at all (no providers)
      return nullptr;

   // If a provider is requested specifically, return it or fail entirely
   if(requested_provider != "")
      {
      auto prov = algo->second.find(requested_provider);
      if(prov != algo->second.end())
         return prov->second;
      return nullptr;
      }

   const T* prototype = nullptr;
   std::string prototype_provider;
   size_t prototype_prov_weight = 0;

   const std::string pref_provider = search_map(pref_providers, algo_spec);

   for(auto i = algo->second.begin(); i != algo->second.end(); ++i)
      {
      // preferred prov exists, return immediately
      if(i->first == pref_provider)
         return i->second;

      const size_t prov_weight = static_provider_weight(i->first);

      if(prototype == nullptr || prov_weight > prototype_prov_weight)
         {
         prototype = i->second;
         prototype_provider = i->first;
         prototype_prov_weight = prov_weight;
         }
      }

   return prototype;
   }

/*
* Add an implementation to the cache
*/
template<typename T>
void Algorithm_Cache<T>::add(T* algo,
                             const std::string& requested_name,
                             const std::string& provider)
   {
   if(!algo)
      return;

   std::lock_guard<std::mutex> lock(mutex);

   if(algo->name() != requested_name &&
      aliases.find(requested_name) == aliases.end())
      {
      aliases[requested_name] = algo->name();
      }

   if(!algorithms[algo->name()][provider])
      algorithms[algo->name()][provider] = algo;
   else
      delete algo;
   }

/*
* Find the providers of this algo (if any)
*/
template<typename T> std::vector<std::string>
Algorithm_Cache<T>::providers_of(const std::string& algo_name)
   {
   std::lock_guard<std::mutex> lock(mutex);

   std::vector<std::string> providers;

   auto algo = find_algorithm(algo_name);
   if(algo != algorithms.end())
      {
      auto provider = algo->second.begin();

      while(provider != algo->second.end())
         {
         providers.push_back(provider->first);
         ++provider;
         }
      }

   return providers;
   }

/*
* Set the preferred provider for an algorithm
*/
template<typename T>
void Algorithm_Cache<T>::set_preferred_provider(const std::string& algo_spec,
                                                const std::string& provider)
   {
   std::lock_guard<std::mutex> lock(mutex);

   pref_providers[algo_spec] = provider;
   }

/*
* Clear out the cache
*/
template<typename T>
void Algorithm_Cache<T>::clear_cache()
   {
   auto algo = algorithms.begin();

   while(algo != algorithms.end())
      {
      auto provider = algo->second.begin();

      while(provider != algo->second.end())
         {
         delete provider->second;
         ++provider;
         }

      ++algo;
      }

   algorithms.clear();
   }

}


namespace Botan {

/**
* Fake SIMD, using plain scalar operations
* Often still faster than iterative on superscalar machines
*/
template<typename T, size_t N>
class SIMD_Scalar
   {
   public:
      static bool enabled() { return true; }

      static size_t size() { return N; }

      SIMD_Scalar() { /* uninitialized */ }

      SIMD_Scalar(const T B[N])
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] = B[i];
         }

      SIMD_Scalar(T B)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] = B;
         }

      static SIMD_Scalar<T,N> load_le(const void* in)
         {
         SIMD_Scalar<T,N> out;
         const byte* in_b = static_cast<const byte*>(in);

         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] = Botan::load_le<T>(in_b, i);

         return out;
         }

      static SIMD_Scalar<T,N> load_be(const void* in)
         {
         SIMD_Scalar<T,N> out;
         const byte* in_b = static_cast<const byte*>(in);

         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] = Botan::load_be<T>(in_b, i);

         return out;
         }

      void store_le(byte out[]) const
         {
         for(size_t i = 0; i != size(); ++i)
            Botan::store_le(m_v[i], out + i*sizeof(T));
         }

      void store_be(byte out[]) const
         {
         for(size_t i = 0; i != size(); ++i)
            Botan::store_be(m_v[i], out + i*sizeof(T));
         }

      void rotate_left(size_t rot)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] = Botan::rotate_left(m_v[i], rot);
         }

      void rotate_right(size_t rot)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] = Botan::rotate_right(m_v[i], rot);
         }

      void operator+=(const SIMD_Scalar<T,N>& other)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] += other.m_v[i];
         }

      void operator-=(const SIMD_Scalar<T,N>& other)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] -= other.m_v[i];
         }

      SIMD_Scalar<T,N> operator+(const SIMD_Scalar<T,N>& other) const
         {
         SIMD_Scalar<T,N> out = *this;
         out += other;
         return out;
         }

      SIMD_Scalar<T,N> operator-(const SIMD_Scalar<T,N>& other) const
         {
         SIMD_Scalar<T,N> out = *this;
         out -= other;
         return out;
         }

      void operator^=(const SIMD_Scalar<T,N>& other)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] ^= other.m_v[i];
         }

      SIMD_Scalar<T,N> operator^(const SIMD_Scalar<T,N>& other) const
         {
         SIMD_Scalar<T,N> out = *this;
         out ^= other;
         return out;
         }

      void operator|=(const SIMD_Scalar<T,N>& other)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] |= other.m_v[i];
         }

      void operator&=(const SIMD_Scalar<T,N>& other)
         {
         for(size_t i = 0; i != size(); ++i)
            m_v[i] &= other.m_v[i];
         }

      SIMD_Scalar<T,N> operator&(const SIMD_Scalar<T,N>& other)
         {
         SIMD_Scalar<T,N> out = *this;
         out &= other;
         return out;
         }

      SIMD_Scalar<T,N> operator<<(size_t shift) const
         {
         SIMD_Scalar<T,N> out = *this;
         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] <<= shift;
         return out;
         }

      SIMD_Scalar<T,N> operator>>(size_t shift) const
         {
         SIMD_Scalar<T,N> out = *this;
         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] >>= shift;
         return out;
         }

      SIMD_Scalar<T,N> operator~() const
         {
         SIMD_Scalar<T,N> out = *this;
         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] = ~out.m_v[i];
         return out;
         }

      // (~reg) & other
      SIMD_Scalar<T,N> andc(const SIMD_Scalar<T,N>& other)
         {
         SIMD_Scalar<T,N> out;
         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] = (~m_v[i]) & other.m_v[i];
         return out;
         }

      SIMD_Scalar<T,N> bswap() const
         {
         SIMD_Scalar<T,N> out;
         for(size_t i = 0; i != size(); ++i)
            out.m_v[i] = reverse_bytes(m_v[i]);
         return out;
         }

      static void transpose(SIMD_Scalar<T,N>& B0, SIMD_Scalar<T,N>& B1,
                            SIMD_Scalar<T,N>& B2, SIMD_Scalar<T,N>& B3)
         {
         static_assert(N == 4, "4x4 transpose");
         SIMD_Scalar<T,N> T0({B0.m_v[0], B1.m_v[0], B2.m_v[0], B3.m_v[0]});
         SIMD_Scalar<T,N> T1({B0.m_v[1], B1.m_v[1], B2.m_v[1], B3.m_v[1]});
         SIMD_Scalar<T,N> T2({B0.m_v[2], B1.m_v[2], B2.m_v[2], B3.m_v[2]});
         SIMD_Scalar<T,N> T3({B0.m_v[3], B1.m_v[3], B2.m_v[3], B3.m_v[3]});

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
         }

   private:
      SIMD_Scalar(std::initializer_list<T> B)
         {
         size_t i = 0;
         for(auto v = B.begin(); v != B.end(); ++v)
            m_v[i++] = *v;
         }

      T m_v[N];
   };

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

/**
* Container of output buffers for Pipe
*/
class Output_Buffers
   {
   public:
      size_t read(byte[], size_t, Pipe::message_id);
      size_t peek(byte[], size_t, size_t, Pipe::message_id) const;
      size_t get_bytes_read(Pipe::message_id) const;
      size_t remaining(Pipe::message_id) const;

      void add(class SecureQueue*);
      void retire();

      Pipe::message_id message_count() const;

      Output_Buffers();
      ~Output_Buffers();
   private:
      class SecureQueue* get(Pipe::message_id) const;

      std::deque<SecureQueue*> buffers;
      Pipe::message_id offset;
   };

}


namespace Botan {

/**
* Engine for implementations that use some kind of SIMD
*/
class SIMD_Engine : public Engine
   {
   public:
      std::string provider_name() const { return "simd"; }

      BlockCipher* find_block_cipher(const SCAN_Name&,
                                     Algorithm_Factory&) const;

      HashFunction* find_hash(const SCAN_Name& request,
                              Algorithm_Factory&) const;
   };

}


namespace Botan {

/**
* Fixed Window Exponentiator
*/
class Fixed_Window_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&);
      void set_base(const BigInt&);
      BigInt execute() const;

      Modular_Exponentiator* copy() const
         { return new Fixed_Window_Exponentiator(*this); }

      Fixed_Window_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      Modular_Reducer reducer;
      BigInt exp;
      size_t window_bits;
      std::vector<BigInt> g;
      Power_Mod::Usage_Hints hints;
   };

/**
* Montgomery Exponentiator
*/
class Montgomery_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&);
      void set_base(const BigInt&);
      BigInt execute() const;

      Modular_Exponentiator* copy() const
         { return new Montgomery_Exponentiator(*this); }

      Montgomery_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      BigInt m_exp, m_modulus, m_R_mod, m_R2_mod;
      word m_mod_prime;
      size_t m_mod_words, m_exp_bits, m_window_bits;
      Power_Mod::Usage_Hints m_hints;
      std::vector<BigInt> m_g;
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
  namespace Botan { typedef SIMD_SSE2 SIMD_32; }

#elif defined(BOTAN_HAS_SIMD_ALTIVEC)
  namespace Botan { typedef SIMD_Altivec SIMD_32; }

#elif defined(BOTAN_HAS_SIMD_SCALAR)
  namespace Botan { typedef SIMD_Scalar<u32bit,4> SIMD_32; }

#else
  #error "No SIMD module defined"

#endif


namespace Botan {

/**
* Entropy source reading from kernel devices like /dev/random
*/
class Device_EntropySource : public EntropySource
   {
   public:
      std::string name() const { return "RNG Device Reader"; }

      void poll(Entropy_Accumulator& accum);

      Device_EntropySource(const std::vector<std::string>& fsnames);
      ~Device_EntropySource();
   private:
      typedef int fd_type;

      std::vector<fd_type> m_devices;
   };

}


namespace Botan {

Public_Key* make_public_key(const AlgorithmIdentifier& alg_id,
                            const secure_vector<byte>& key_bits);

Private_Key* make_private_key(const AlgorithmIdentifier& alg_id,
                              const secure_vector<byte>& key_bits,
                              RandomNumberGenerator& rng);

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


namespace Botan {

extern "C" {

/*
* Word Multiply/Add
*/
inline word word_madd2(word a, word b, word* c)
   {
#if defined(BOTAN_HAS_MP_DWORD)
   const dword s = static_cast<dword>(a) * b + *c;
   *c = static_cast<word>(s >> BOTAN_MP_WORD_BITS);
   return static_cast<word>(s);
#else
   static_assert(BOTAN_MP_WORD_BITS == 64, "Unexpected word size");

   word hi = 0, lo = 0;

   mul64x64_128(a, b, &lo, &hi);

   lo += *c;
   hi += (lo < *c); // carry?

   *c = hi;
   return lo;
#endif
   }

/*
* Word Multiply/Add
*/
inline word word_madd3(word a, word b, word c, word* d)
   {
#if defined(BOTAN_HAS_MP_DWORD)
   const dword s = static_cast<dword>(a) * b + c + *d;
   *d = static_cast<word>(s >> BOTAN_MP_WORD_BITS);
   return static_cast<word>(s);
#else
   static_assert(BOTAN_MP_WORD_BITS == 64, "Unexpected word size");

   word hi = 0, lo = 0;

   mul64x64_128(a, b, &lo, &hi);

   lo += c;
   hi += (lo < c); // carry?

   lo += *d;
   hi += (lo < *d); // carry?

   *d = hi;
   return lo;
#endif
   }

}

}


namespace Botan {

/*
* The size of the word type, in bits
*/
const size_t MP_WORD_BITS = BOTAN_MP_WORD_BITS;

extern "C" {

/**
* Two operand addition
* @param x the first operand (and output)
* @param x_size size of x
* @param y the second operand
* @param y_size size of y (must be >= x_size)
*/
void bigint_add2(word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Three operand addition
*/
void bigint_add3(word z[],
                 const word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Two operand addition with carry out
*/
word bigint_add2_nc(word x[], size_t x_size, const word y[], size_t y_size);

/**
* Three operand addition with carry out
*/
word bigint_add3_nc(word z[],
                    const word x[], size_t x_size,
                    const word y[], size_t y_size);

/**
* Two operand subtraction
*/
word bigint_sub2(word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Two operand subtraction, x = y - x; assumes y >= x
*/
void bigint_sub2_rev(word x[], const word y[], size_t y_size);

/**
* Three operand subtraction
*/
word bigint_sub3(word z[],
                 const word x[], size_t x_size,
                 const word y[], size_t y_size);

/*
* Shift Operations
*/
void bigint_shl1(word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shr1(word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shl2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shr2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

/*
* Simple O(N^2) Multiplication and Squaring
*/
void bigint_simple_mul(word z[],
                       const word x[], size_t x_size,
                       const word y[], size_t y_size);

void bigint_simple_sqr(word z[], const word x[], size_t x_size);

/*
* Linear Multiply
*/
void bigint_linmul2(word x[], size_t x_size, word y);
void bigint_linmul3(word z[], const word x[], size_t x_size, word y);

/**
* Montgomery Reduction
* @param z integer to reduce, of size exactly 2*(p_size+1).
           Output is in the first p_size+1 words, higher
           words are set to zero.
* @param p modulus
* @param p_size size of p
* @param p_dash Montgomery value
* @param workspace array of at least 2*(p_size+1) words
*/
void bigint_monty_redc(word z[],
                       const word p[], size_t p_size,
                       word p_dash,
                       word workspace[]);

/*
* Montgomery Multiplication
*/
void bigint_monty_mul(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word y[], size_t y_size, size_t y_sw,
                      const word p[], size_t p_size, word p_dash,
                      word workspace[]);

/*
* Montgomery Squaring
*/
void bigint_monty_sqr(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word p[], size_t p_size, word p_dash,
                      word workspace[]);

/**
* Compare x and y
*/
s32bit bigint_cmp(const word x[], size_t x_size,
                  const word y[], size_t y_size);

/**
* Compute ((n1<<bits) + n0) / d
*/
word bigint_divop(word n1, word n0, word d);

/**
* Compute ((n1<<bits) + n0) % d
*/
word bigint_modop(word n1, word n0, word d);

/*
* Comba Multiplication / Squaring
*/
void bigint_comba_mul4(word z[8], const word x[4], const word y[4]);
void bigint_comba_mul6(word z[12], const word x[6], const word y[6]);
void bigint_comba_mul8(word z[16], const word x[8], const word y[8]);
void bigint_comba_mul16(word z[32], const word x[16], const word y[16]);

void bigint_comba_sqr4(word out[8], const word in[4]);
void bigint_comba_sqr6(word out[12], const word in[6]);
void bigint_comba_sqr8(word out[16], const word in[8]);
void bigint_comba_sqr16(word out[32], const word in[16]);

}

/*
* High Level Multiplication/Squaring Interfaces
*/
void bigint_mul(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw,
                const word y[], size_t y_size, size_t y_sw);

void bigint_sqr(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw);

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

extern "C" {

/*
* Word Addition
*/
inline word word_add(word x, word y, word* carry)
   {
   word z = x + y;
   word c1 = (z < x);
   z += *carry;
   *carry = c1 | (z < *carry);
   return z;
   }

/*
* Eight Word Block Addition, Two Argument
*/
inline word word8_add2(word x[8], const word y[8], word carry)
   {
   x[0] = word_add(x[0], y[0], &carry);
   x[1] = word_add(x[1], y[1], &carry);
   x[2] = word_add(x[2], y[2], &carry);
   x[3] = word_add(x[3], y[3], &carry);
   x[4] = word_add(x[4], y[4], &carry);
   x[5] = word_add(x[5], y[5], &carry);
   x[6] = word_add(x[6], y[6], &carry);
   x[7] = word_add(x[7], y[7], &carry);
   return carry;
   }

/*
* Eight Word Block Addition, Three Argument
*/
inline word word8_add3(word z[8], const word x[8],
                       const word y[8], word carry)
   {
   z[0] = word_add(x[0], y[0], &carry);
   z[1] = word_add(x[1], y[1], &carry);
   z[2] = word_add(x[2], y[2], &carry);
   z[3] = word_add(x[3], y[3], &carry);
   z[4] = word_add(x[4], y[4], &carry);
   z[5] = word_add(x[5], y[5], &carry);
   z[6] = word_add(x[6], y[6], &carry);
   z[7] = word_add(x[7], y[7], &carry);
   return carry;
   }

/*
* Word Subtraction
*/
inline word word_sub(word x, word y, word* carry)
   {
   word t0 = x - y;
   word c1 = (t0 > x);
   word z = t0 - *carry;
   *carry = c1 | (z > t0);
   return z;
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2(word x[8], const word y[8], word carry)
   {
   x[0] = word_sub(x[0], y[0], &carry);
   x[1] = word_sub(x[1], y[1], &carry);
   x[2] = word_sub(x[2], y[2], &carry);
   x[3] = word_sub(x[3], y[3], &carry);
   x[4] = word_sub(x[4], y[4], &carry);
   x[5] = word_sub(x[5], y[5], &carry);
   x[6] = word_sub(x[6], y[6], &carry);
   x[7] = word_sub(x[7], y[7], &carry);
   return carry;
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2_rev(word x[8], const word y[8], word carry)
   {
   x[0] = word_sub(y[0], x[0], &carry);
   x[1] = word_sub(y[1], x[1], &carry);
   x[2] = word_sub(y[2], x[2], &carry);
   x[3] = word_sub(y[3], x[3], &carry);
   x[4] = word_sub(y[4], x[4], &carry);
   x[5] = word_sub(y[5], x[5], &carry);
   x[6] = word_sub(y[6], x[6], &carry);
   x[7] = word_sub(y[7], x[7], &carry);
   return carry;
   }

/*
* Eight Word Block Subtraction, Three Argument
*/
inline word word8_sub3(word z[8], const word x[8],
                       const word y[8], word carry)
   {
   z[0] = word_sub(x[0], y[0], &carry);
   z[1] = word_sub(x[1], y[1], &carry);
   z[2] = word_sub(x[2], y[2], &carry);
   z[3] = word_sub(x[3], y[3], &carry);
   z[4] = word_sub(x[4], y[4], &carry);
   z[5] = word_sub(x[5], y[5], &carry);
   z[6] = word_sub(x[6], y[6], &carry);
   z[7] = word_sub(x[7], y[7], &carry);
   return carry;
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul2(word x[8], word y, word carry)
   {
   x[0] = word_madd2(x[0], y, &carry);
   x[1] = word_madd2(x[1], y, &carry);
   x[2] = word_madd2(x[2], y, &carry);
   x[3] = word_madd2(x[3], y, &carry);
   x[4] = word_madd2(x[4], y, &carry);
   x[5] = word_madd2(x[5], y, &carry);
   x[6] = word_madd2(x[6], y, &carry);
   x[7] = word_madd2(x[7], y, &carry);
   return carry;
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul3(word z[8], const word x[8], word y, word carry)
   {
   z[0] = word_madd2(x[0], y, &carry);
   z[1] = word_madd2(x[1], y, &carry);
   z[2] = word_madd2(x[2], y, &carry);
   z[3] = word_madd2(x[3], y, &carry);
   z[4] = word_madd2(x[4], y, &carry);
   z[5] = word_madd2(x[5], y, &carry);
   z[6] = word_madd2(x[6], y, &carry);
   z[7] = word_madd2(x[7], y, &carry);
   return carry;
   }

/*
* Eight Word Block Multiply/Add
*/
inline word word8_madd3(word z[8], const word x[8], word y, word carry)
   {
   z[0] = word_madd3(x[0], y, z[0], &carry);
   z[1] = word_madd3(x[1], y, z[1], &carry);
   z[2] = word_madd3(x[2], y, z[2], &carry);
   z[3] = word_madd3(x[3], y, z[3], &carry);
   z[4] = word_madd3(x[4], y, z[4], &carry);
   z[5] = word_madd3(x[5], y, z[5], &carry);
   z[6] = word_madd3(x[6], y, z[6], &carry);
   z[7] = word_madd3(x[7], y, z[7], &carry);
   return carry;
   }

/*
* Multiply-Add Accumulator
*/
inline void word3_muladd(word* w2, word* w1, word* w0, word a, word b)
   {
   word carry = *w0;
   *w0 = word_madd2(a, b, &carry);
   *w1 += carry;
   *w2 += (*w1 < carry) ? 1 : 0;
   }

/*
* Multiply-Add Accumulator
*/
inline void word3_muladd_2(word* w2, word* w1, word* w0, word a, word b)
   {
   word carry = 0;
   a = word_madd2(a, b, &carry);
   b = carry;

   word top = (b >> (BOTAN_MP_WORD_BITS-1));
   b <<= 1;
   b |= (a >> (BOTAN_MP_WORD_BITS-1));
   a <<= 1;

   carry = 0;
   *w0 = word_add(*w0, a, &carry);
   *w1 = word_add(*w1, b, &carry);
   *w2 = word_add(*w2, top, &carry);
   }

}

}

/*
* SCAN Name Abstraction
* (C) 2008-2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

std::string make_arg(
   const std::vector<std::pair<size_t, std::string> >& name, size_t start)
   {
   std::string output = name[start].second;
   size_t level = name[start].first;

   size_t paren_depth = 0;

   for(size_t i = start + 1; i != name.size(); ++i)
      {
      if(name[i].first <= name[start].first)
         break;

      if(name[i].first > level)
         {
         output += '(' + name[i].second;
         ++paren_depth;
         }
      else if(name[i].first < level)
         {
         output += ")," + name[i].second;
         --paren_depth;
         }
      else
         {
         if(output[output.size() - 1] != '(')
            output += ",";
         output += name[i].second;
         }

      level = name[i].first;
      }

   for(size_t i = 0; i != paren_depth; ++i)
      output += ')';

   return output;
   }

std::pair<size_t, std::string>
deref_aliases(const std::pair<size_t, std::string>& in)
   {
   return std::make_pair(in.first,
                         SCAN_Name::deref_alias(in.second));
   }

}

std::mutex SCAN_Name::s_alias_map_mutex;
std::map<std::string, std::string> SCAN_Name::s_alias_map;

SCAN_Name::SCAN_Name(std::string algo_spec)
   {
   orig_algo_spec = algo_spec;

   std::vector<std::pair<size_t, std::string> > name;
   size_t level = 0;
   std::pair<size_t, std::string> accum = std::make_pair(level, "");

   std::string decoding_error = "Bad SCAN name '" + algo_spec + "': ";

   algo_spec = SCAN_Name::deref_alias(algo_spec);

   for(size_t i = 0; i != algo_spec.size(); ++i)
      {
      char c = algo_spec[i];

      if(c == '/' || c == ',' || c == '(' || c == ')')
         {
         if(c == '(')
            ++level;
         else if(c == ')')
            {
            if(level == 0)
               throw Decoding_Error(decoding_error + "Mismatched parens");
            --level;
            }

         if(c == '/' && level > 0)
            accum.second.push_back(c);
         else
            {
            if(accum.second != "")
               name.push_back(deref_aliases(accum));
            accum = std::make_pair(level, "");
            }
         }
      else
         accum.second.push_back(c);
      }

   if(accum.second != "")
      name.push_back(deref_aliases(accum));

   if(level != 0)
      throw Decoding_Error(decoding_error + "Missing close paren");

   if(name.size() == 0)
      throw Decoding_Error(decoding_error + "Empty name");

   alg_name = name[0].second;

   bool in_modes = false;

   for(size_t i = 1; i != name.size(); ++i)
      {
      if(name[i].first == 0)
         {
         mode_info.push_back(make_arg(name, i));
         in_modes = true;
         }
      else if(name[i].first == 1 && !in_modes)
         args.push_back(make_arg(name, i));
      }
   }

std::string SCAN_Name::algo_name_and_args() const
   {
   std::string out;

   out = algo_name();

   if(arg_count())
      {
      out += '(';
      for(size_t i = 0; i != arg_count(); ++i)
         {
         out += arg(i);
         if(i != arg_count() - 1)
            out += ',';
         }
      out += ')';

      }

   return out;
   }

std::string SCAN_Name::arg(size_t i) const
   {
   if(i >= arg_count())
      throw std::range_error("SCAN_Name::argument - i out of range");
   return args[i];
   }

std::string SCAN_Name::arg(size_t i, const std::string& def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return args[i];
   }

size_t SCAN_Name::arg_as_integer(size_t i, size_t def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return to_u32bit(args[i]);
   }

void SCAN_Name::add_alias(const std::string& alias, const std::string& basename)
   {
   std::lock_guard<std::mutex> lock(s_alias_map_mutex);

   if(s_alias_map.find(alias) == s_alias_map.end())
      s_alias_map[alias] = basename;
   }

std::string SCAN_Name::deref_alias(const std::string& alias)
   {
   std::lock_guard<std::mutex> lock(s_alias_map_mutex);

   std::string name = alias;

   for(auto i = s_alias_map.find(name); i != s_alias_map.end(); i = s_alias_map.find(name))
      name = i->second;

   return name;
   }

void SCAN_Name::set_default_aliases()
   {
   // common variations worth supporting
   SCAN_Name::add_alias("EME-PKCS1-v1_5",  "PKCS1v15");
   SCAN_Name::add_alias("3DES",     "TripleDES");
   SCAN_Name::add_alias("DES-EDE",  "TripleDES");
   SCAN_Name::add_alias("CAST5",    "CAST-128");
   SCAN_Name::add_alias("SHA1",     "SHA-160");
   SCAN_Name::add_alias("SHA-1",    "SHA-160");
   SCAN_Name::add_alias("MARK-4",   "RC4(256)");
   SCAN_Name::add_alias("ARC4",     "RC4");
   SCAN_Name::add_alias("OMAC",     "CMAC");

   SCAN_Name::add_alias("EMSA-PSS",        "PSSR");
   SCAN_Name::add_alias("PSS-MGF1",        "PSSR");
   SCAN_Name::add_alias("EME-OAEP",        "OAEP");

   SCAN_Name::add_alias("EMSA2",           "EMSA_X931");
   SCAN_Name::add_alias("EMSA3",           "EMSA_PKCS1");
   SCAN_Name::add_alias("EMSA-PKCS1-v1_5", "EMSA_PKCS1");

   // should be renamed in sources
   SCAN_Name::add_alias("X9.31",           "EMSA2");

   // kept for compatability with old library versions
   SCAN_Name::add_alias("EMSA4",           "PSSR");
   SCAN_Name::add_alias("EME1",            "OAEP");

   // probably can be removed
   SCAN_Name::add_alias("GOST",     "GOST-28147-89");
   SCAN_Name::add_alias("GOST-34.11", "GOST-R-34.11-94");
   }

}
/*
* OctetString
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Create an OctetString from RNG output
*/
OctetString::OctetString(RandomNumberGenerator& rng,
                         size_t length)
   {
   bits = rng.random_vec(length);
   }

/*
* Create an OctetString from a hex string
*/
OctetString::OctetString(const std::string& hex_string)
   {
   bits.resize(1 + hex_string.length() / 2);
   bits.resize(hex_decode(&bits[0], hex_string));
   }

/*
* Create an OctetString from a byte string
*/
OctetString::OctetString(const byte in[], size_t n)
   {
   bits.assign(in, in + n);
   }

/*
* Set the parity of each key byte to odd
*/
void OctetString::set_odd_parity()
   {
   const byte ODD_PARITY[256] = {
      0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x07, 0x07, 0x08, 0x08, 0x0B, 0x0B,
      0x0D, 0x0D, 0x0E, 0x0E, 0x10, 0x10, 0x13, 0x13, 0x15, 0x15, 0x16, 0x16,
      0x19, 0x19, 0x1A, 0x1A, 0x1C, 0x1C, 0x1F, 0x1F, 0x20, 0x20, 0x23, 0x23,
      0x25, 0x25, 0x26, 0x26, 0x29, 0x29, 0x2A, 0x2A, 0x2C, 0x2C, 0x2F, 0x2F,
      0x31, 0x31, 0x32, 0x32, 0x34, 0x34, 0x37, 0x37, 0x38, 0x38, 0x3B, 0x3B,
      0x3D, 0x3D, 0x3E, 0x3E, 0x40, 0x40, 0x43, 0x43, 0x45, 0x45, 0x46, 0x46,
      0x49, 0x49, 0x4A, 0x4A, 0x4C, 0x4C, 0x4F, 0x4F, 0x51, 0x51, 0x52, 0x52,
      0x54, 0x54, 0x57, 0x57, 0x58, 0x58, 0x5B, 0x5B, 0x5D, 0x5D, 0x5E, 0x5E,
      0x61, 0x61, 0x62, 0x62, 0x64, 0x64, 0x67, 0x67, 0x68, 0x68, 0x6B, 0x6B,
      0x6D, 0x6D, 0x6E, 0x6E, 0x70, 0x70, 0x73, 0x73, 0x75, 0x75, 0x76, 0x76,
      0x79, 0x79, 0x7A, 0x7A, 0x7C, 0x7C, 0x7F, 0x7F, 0x80, 0x80, 0x83, 0x83,
      0x85, 0x85, 0x86, 0x86, 0x89, 0x89, 0x8A, 0x8A, 0x8C, 0x8C, 0x8F, 0x8F,
      0x91, 0x91, 0x92, 0x92, 0x94, 0x94, 0x97, 0x97, 0x98, 0x98, 0x9B, 0x9B,
      0x9D, 0x9D, 0x9E, 0x9E, 0xA1, 0xA1, 0xA2, 0xA2, 0xA4, 0xA4, 0xA7, 0xA7,
      0xA8, 0xA8, 0xAB, 0xAB, 0xAD, 0xAD, 0xAE, 0xAE, 0xB0, 0xB0, 0xB3, 0xB3,
      0xB5, 0xB5, 0xB6, 0xB6, 0xB9, 0xB9, 0xBA, 0xBA, 0xBC, 0xBC, 0xBF, 0xBF,
      0xC1, 0xC1, 0xC2, 0xC2, 0xC4, 0xC4, 0xC7, 0xC7, 0xC8, 0xC8, 0xCB, 0xCB,
      0xCD, 0xCD, 0xCE, 0xCE, 0xD0, 0xD0, 0xD3, 0xD3, 0xD5, 0xD5, 0xD6, 0xD6,
      0xD9, 0xD9, 0xDA, 0xDA, 0xDC, 0xDC, 0xDF, 0xDF, 0xE0, 0xE0, 0xE3, 0xE3,
      0xE5, 0xE5, 0xE6, 0xE6, 0xE9, 0xE9, 0xEA, 0xEA, 0xEC, 0xEC, 0xEF, 0xEF,
      0xF1, 0xF1, 0xF2, 0xF2, 0xF4, 0xF4, 0xF7, 0xF7, 0xF8, 0xF8, 0xFB, 0xFB,
      0xFD, 0xFD, 0xFE, 0xFE };

   for(size_t j = 0; j != bits.size(); ++j)
      bits[j] = ODD_PARITY[bits[j]];
   }

/*
* Hex encode an OctetString
*/
std::string OctetString::as_string() const
   {
   return hex_encode(&bits[0], bits.size());
   }

/*
* XOR Operation for OctetStrings
*/
OctetString& OctetString::operator^=(const OctetString& k)
   {
   if(&k == this) { zeroise(bits); return (*this); }
   xor_buf(&bits[0], k.begin(), std::min(length(), k.length()));
   return (*this);
   }

/*
* Equality Operation for OctetStrings
*/
bool operator==(const OctetString& s1, const OctetString& s2)
   {
   return (s1.bits_of() == s2.bits_of());
   }

/*
* Unequality Operation for OctetStrings
*/
bool operator!=(const OctetString& s1, const OctetString& s2)
   {
   return !(s1 == s2);
   }

/*
* Append Operation for OctetStrings
*/
OctetString operator+(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<byte> out;
   out += k1.bits_of();
   out += k2.bits_of();
   return OctetString(out);
   }

/*
* XOR Operation for OctetStrings
*/
OctetString operator^(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<byte> ret(std::max(k1.length(), k2.length()));

   copy_mem(&ret[0], k1.begin(), k1.length());
   xor_buf(&ret[0], k2.begin(), k2.length());
   return OctetString(ret);
   }

}
/*
* Algorithm Factory
* (C) 2008-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/




namespace Botan {

namespace {

/*
* Template functions for the factory prototype/search algorithm
*/
template<typename T>
T* engine_get_algo(Engine*,
                   const SCAN_Name&,
                   Algorithm_Factory&)
   { return nullptr; }

template<>
BlockCipher* engine_get_algo(Engine* engine,
                             const SCAN_Name& request,
                             Algorithm_Factory& af)
   { return engine->find_block_cipher(request, af); }

template<>
StreamCipher* engine_get_algo(Engine* engine,
                              const SCAN_Name& request,
                              Algorithm_Factory& af)
   { return engine->find_stream_cipher(request, af); }

template<>
HashFunction* engine_get_algo(Engine* engine,
                              const SCAN_Name& request,
                              Algorithm_Factory& af)
   { return engine->find_hash(request, af); }

template<>
MessageAuthenticationCode* engine_get_algo(Engine* engine,
                                           const SCAN_Name& request,
                                           Algorithm_Factory& af)
   { return engine->find_mac(request, af); }

template<>
PBKDF* engine_get_algo(Engine* engine,
                       const SCAN_Name& request,
                       Algorithm_Factory& af)
   { return engine->find_pbkdf(request, af); }

template<typename T>
const T* factory_prototype(const std::string& algo_spec,
                           const std::string& provider,
                           const std::vector<Engine*>& engines,
                           Algorithm_Factory& af,
                           Algorithm_Cache<T>& cache)
   {
   if(const T* cache_hit = cache.get(algo_spec, provider))
      return cache_hit;

   SCAN_Name scan_name(algo_spec);

   if(scan_name.cipher_mode() != "")
      return nullptr;

   for(size_t i = 0; i != engines.size(); ++i)
      {
      if(provider == "" || engines[i]->provider_name() == provider)
         {
         if(T* impl = engine_get_algo<T>(engines[i], scan_name, af))
            cache.add(impl, algo_spec, engines[i]->provider_name());
         }
      }

   return cache.get(algo_spec, provider);
   }

}

/*
* Setup caches
*/
Algorithm_Factory::Algorithm_Factory()
   {
   block_cipher_cache.reset(new Algorithm_Cache<BlockCipher>());
   stream_cipher_cache.reset(new Algorithm_Cache<StreamCipher>());
   hash_cache.reset(new Algorithm_Cache<HashFunction>());
   mac_cache.reset(new Algorithm_Cache<MessageAuthenticationCode>());
   pbkdf_cache.reset(new Algorithm_Cache<PBKDF>());
   }

/*
* Delete all engines
*/
Algorithm_Factory::~Algorithm_Factory()
   {
   for(auto i = engines.begin(); i != engines.end(); ++i)
      delete *i;
   }

void Algorithm_Factory::clear_caches()
   {
   block_cipher_cache->clear_cache();
   stream_cipher_cache->clear_cache();
   hash_cache->clear_cache();
   mac_cache->clear_cache();
   pbkdf_cache->clear_cache();
   }

void Algorithm_Factory::add_engine(Engine* engine)
   {
   clear_caches();
   engines.push_back(engine);
   }

/*
* Set the preferred provider for an algorithm
*/
void Algorithm_Factory::set_preferred_provider(const std::string& algo_spec,
                                               const std::string& provider)
   {
   if(prototype_block_cipher(algo_spec))
      block_cipher_cache->set_preferred_provider(algo_spec, provider);
   else if(prototype_stream_cipher(algo_spec))
      stream_cipher_cache->set_preferred_provider(algo_spec, provider);
   else if(prototype_hash_function(algo_spec))
      hash_cache->set_preferred_provider(algo_spec, provider);
   else if(prototype_mac(algo_spec))
      mac_cache->set_preferred_provider(algo_spec, provider);
   else if(prototype_pbkdf(algo_spec))
      pbkdf_cache->set_preferred_provider(algo_spec, provider);
   }

/*
* Get an engine out of the list
*/
Engine* Algorithm_Factory::get_engine_n(size_t n) const
   {
   if(n >= engines.size())
      return nullptr;
   return engines[n];
   }

/*
* Return the possible providers of a request
* Note: assumes you don't have different types by the same name
*/
std::vector<std::string>
Algorithm_Factory::providers_of(const std::string& algo_spec)
   {
   /* The checks with if(prototype_X(algo_spec)) have the effect of
      forcing a full search, since otherwise there might not be any
      providers at all in the cache.
   */

   if(prototype_block_cipher(algo_spec))
      return block_cipher_cache->providers_of(algo_spec);
   else if(prototype_stream_cipher(algo_spec))
      return stream_cipher_cache->providers_of(algo_spec);
   else if(prototype_hash_function(algo_spec))
      return hash_cache->providers_of(algo_spec);
   else if(prototype_mac(algo_spec))
      return mac_cache->providers_of(algo_spec);
   else if(prototype_pbkdf(algo_spec))
      return pbkdf_cache->providers_of(algo_spec);
   else
      return std::vector<std::string>();
   }

/*
* Return the prototypical block cipher corresponding to this request
*/
const BlockCipher*
Algorithm_Factory::prototype_block_cipher(const std::string& algo_spec,
                                          const std::string& provider)
   {
   return factory_prototype<BlockCipher>(algo_spec, provider, engines,
                                          *this, *block_cipher_cache);
   }

/*
* Return the prototypical stream cipher corresponding to this request
*/
const StreamCipher*
Algorithm_Factory::prototype_stream_cipher(const std::string& algo_spec,
                                           const std::string& provider)
   {
   return factory_prototype<StreamCipher>(algo_spec, provider, engines,
                                          *this, *stream_cipher_cache);
   }

/*
* Return the prototypical object corresponding to this request (if found)
*/
const HashFunction*
Algorithm_Factory::prototype_hash_function(const std::string& algo_spec,
                                           const std::string& provider)
   {
   return factory_prototype<HashFunction>(algo_spec, provider, engines,
                                          *this, *hash_cache);
   }

/*
* Return the prototypical object corresponding to this request
*/
const MessageAuthenticationCode*
Algorithm_Factory::prototype_mac(const std::string& algo_spec,
                                 const std::string& provider)
   {
   return factory_prototype<MessageAuthenticationCode>(algo_spec, provider,
                                                       engines,
                                                       *this, *mac_cache);
   }

/*
* Return the prototypical object corresponding to this request
*/
const PBKDF*
Algorithm_Factory::prototype_pbkdf(const std::string& algo_spec,
                                   const std::string& provider)
   {
   return factory_prototype<PBKDF>(algo_spec, provider,
                                   engines,
                                   *this, *pbkdf_cache);
   }

/*
* Return a new block cipher corresponding to this request
*/
BlockCipher*
Algorithm_Factory::make_block_cipher(const std::string& algo_spec,
                                     const std::string& provider)
   {
   if(const BlockCipher* proto = prototype_block_cipher(algo_spec, provider))
      return proto->clone();
   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Return a new stream cipher corresponding to this request
*/
StreamCipher*
Algorithm_Factory::make_stream_cipher(const std::string& algo_spec,
                                      const std::string& provider)
   {
   if(const StreamCipher* proto = prototype_stream_cipher(algo_spec, provider))
      return proto->clone();
   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Return a new object corresponding to this request
*/
HashFunction*
Algorithm_Factory::make_hash_function(const std::string& algo_spec,
                                      const std::string& provider)
   {
   if(const HashFunction* proto = prototype_hash_function(algo_spec, provider))
      return proto->clone();
   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Return a new object corresponding to this request
*/
MessageAuthenticationCode*
Algorithm_Factory::make_mac(const std::string& algo_spec,
                            const std::string& provider)
   {
   if(const MessageAuthenticationCode* proto = prototype_mac(algo_spec, provider))
      return proto->clone();
   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Return a new object corresponding to this request
*/
PBKDF*
Algorithm_Factory::make_pbkdf(const std::string& algo_spec,
                              const std::string& provider)
   {
   if(const PBKDF* proto = prototype_pbkdf(algo_spec, provider))
      return proto->clone();
   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Add a new block cipher
*/
void Algorithm_Factory::add_block_cipher(BlockCipher* block_cipher,
                                         const std::string& provider)
   {
   block_cipher_cache->add(block_cipher, block_cipher->name(), provider);
   }

/*
* Add a new stream cipher
*/
void Algorithm_Factory::add_stream_cipher(StreamCipher* stream_cipher,
                                         const std::string& provider)
   {
   stream_cipher_cache->add(stream_cipher, stream_cipher->name(), provider);
   }

/*
* Add a new hash
*/
void Algorithm_Factory::add_hash_function(HashFunction* hash,
                                          const std::string& provider)
   {
   hash_cache->add(hash, hash->name(), provider);
   }

/*
* Add a new mac
*/
void Algorithm_Factory::add_mac(MessageAuthenticationCode* mac,
                                const std::string& provider)
   {
   mac_cache->add(mac, mac->name(), provider);
   }

/*
* Add a new PBKDF
*/
void Algorithm_Factory::add_pbkdf(PBKDF* pbkdf,
                                  const std::string& provider)
   {
   pbkdf_cache->add(pbkdf, pbkdf->name(), provider);
   }

}
/*
* Default provider weights for Algorithm_Cache
* (C) 2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/**
* Return a static provider weighing
*/
size_t static_provider_weight(const std::string& prov_name)
   {
   /*
   * Prefer asm over C++, but prefer anything over OpenSSL or GNU MP; to use
   * them, set the provider explicitly for the algorithms you want
   */

   if(prov_name == "aes_isa") return 9;
   if(prov_name == "simd") return 8;
   if(prov_name == "asm") return 7;

   if(prov_name == "core") return 5;

   if(prov_name == "openssl") return 2;
   if(prov_name == "gmp") return 1;

   return 0; // other/unknown
   }

}
/*
* Mlock Allocator
* (C) 2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#include <sys/mman.h>
#include <sys/resource.h>

namespace Botan {

namespace {

/**
* Requests for objects of sizeof(T) will be aligned at
* sizeof(T)*ALIGNMENT_MULTIPLE bytes.
*/
const size_t ALIGNMENT_MULTIPLE = 2;

size_t mlock_limit()
   {
   /*
   * Linux defaults to only 64 KiB of mlockable memory per process
   * (too small) but BSDs offer a small fraction of total RAM (more
   * than we need). Bound the total mlock size to 512 KiB which is
   * enough to run the entire test suite without spilling to non-mlock
   * memory (and thus presumably also enough for many useful
   * programs), but small enough that we should not cause problems
   * even if many processes are mlocking on the same machine.
   */
   const size_t MLOCK_UPPER_BOUND = 512*1024;

   struct rlimit limits;
   ::getrlimit(RLIMIT_MEMLOCK, &limits);

   if(limits.rlim_cur < limits.rlim_max)
      {
      limits.rlim_cur = limits.rlim_max;
      ::setrlimit(RLIMIT_MEMLOCK, &limits);
      ::getrlimit(RLIMIT_MEMLOCK, &limits);
      }

   return std::min<size_t>(limits.rlim_cur, MLOCK_UPPER_BOUND);
   }

bool ptr_in_pool(const void* pool_ptr, size_t poolsize,
                 const void* buf_ptr, size_t bufsize)
   {
   const uintptr_t pool = reinterpret_cast<uintptr_t>(pool_ptr);
   const uintptr_t buf = reinterpret_cast<uintptr_t>(buf_ptr);

   if(buf < pool || buf >= pool + poolsize)
      return false;

   BOTAN_ASSERT(buf + bufsize <= pool + poolsize,
                "Pointer does not partially overlap pool");

   return true;
   }

size_t padding_for_alignment(size_t offset, size_t desired_alignment)
   {
   size_t mod = offset % desired_alignment;
   if(mod == 0)
      return 0; // already right on
   return desired_alignment - mod;
   }

}

void* mlock_allocator::allocate(size_t num_elems, size_t elem_size)
   {
   if(!m_pool)
      return nullptr;

   const size_t n = num_elems * elem_size;
   const size_t alignment = ALIGNMENT_MULTIPLE * elem_size;

   if(n / elem_size != num_elems)
      return nullptr; // overflow!

   if(n > m_poolsize || n > BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION)
      return nullptr;

   std::lock_guard<std::mutex> lock(m_mutex);

   auto best_fit = m_freelist.end();

   for(auto i = m_freelist.begin(); i != m_freelist.end(); ++i)
      {
      // If we have a perfect fit, use it immediately
      if(i->second == n && (i->first % alignment) == 0)
         {
         const size_t offset = i->first;
         m_freelist.erase(i);
         clear_mem(m_pool + offset, n);

         BOTAN_ASSERT((reinterpret_cast<size_t>(m_pool) + offset) % alignment == 0,
                      "Returning correctly aligned pointer");

         return m_pool + offset;
         }

      if((i->second >= (n + padding_for_alignment(i->first, alignment)) &&
          ((best_fit == m_freelist.end()) || (best_fit->second > i->second))))
         {
         best_fit = i;
         }
      }

   if(best_fit != m_freelist.end())
      {
      const size_t offset = best_fit->first;

      const size_t alignment_padding = padding_for_alignment(offset, alignment);

      best_fit->first += n + alignment_padding;
      best_fit->second -= n + alignment_padding;

      // Need to realign, split the block
      if(alignment_padding)
         {
         /*
         If we used the entire block except for small piece used for
         alignment at the beginning, so just update the entry already
         in place (as it is in the correct location), rather than
         deleting the empty range and inserting the new one in the
         same location.
         */
         if(best_fit->second == 0)
            {
            best_fit->first = offset;
            best_fit->second = alignment_padding;
            }
         else
            m_freelist.insert(best_fit, std::make_pair(offset, alignment_padding));
         }

      clear_mem(m_pool + offset + alignment_padding, n);

      BOTAN_ASSERT((reinterpret_cast<size_t>(m_pool) + offset + alignment_padding) % alignment == 0,
                   "Returning correctly aligned pointer");

      return m_pool + offset + alignment_padding;
      }

   return nullptr;
   }

bool mlock_allocator::deallocate(void* p, size_t num_elems, size_t elem_size)
   {
   if(!m_pool)
      return false;

   size_t n = num_elems * elem_size;

   /*
   We return nullptr in allocate if there was an overflow, so we
   should never ever see an overflow in a deallocation.
   */
   BOTAN_ASSERT(n / elem_size == num_elems,
                "No overflow in deallocation");

   if(!ptr_in_pool(m_pool, m_poolsize, p, n))
      return false;

   std::lock_guard<std::mutex> lock(m_mutex);

   const size_t start = static_cast<byte*>(p) - m_pool;

   auto comp = [](std::pair<size_t, size_t> x, std::pair<size_t, size_t> y){ return x.first < y.first; };

   auto i = std::lower_bound(m_freelist.begin(), m_freelist.end(),
                             std::make_pair(start, 0), comp);

   // try to merge with later block
   if(i != m_freelist.end() && start + n == i->first)
      {
      i->first = start;
      i->second += n;
      n = 0;
      }

   // try to merge with previous block
   if(i != m_freelist.begin())
      {
      auto prev = std::prev(i);

      if(prev->first + prev->second == start)
         {
         if(n)
            {
            prev->second += n;
            n = 0;
            }
         else
            {
            // merge adjoining
            prev->second += i->second;
            m_freelist.erase(i);
            }
         }
      }

   if(n != 0) // no merge possible?
      m_freelist.insert(i, std::make_pair(start, n));

   return true;
   }

mlock_allocator::mlock_allocator() :
   m_poolsize(mlock_limit()),
   m_pool(nullptr)
   {
#if !defined(MAP_NOCORE)
   #define MAP_NOCORE 0
#endif

#if !defined(MAP_ANONYMOUS)
   #define MAP_ANONYMOUS MAP_ANON
#endif

   if(m_poolsize)
      {
      m_pool = static_cast<byte*>(
         ::mmap(
            nullptr, m_poolsize,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_SHARED | MAP_NOCORE,
            -1, 0));

      if(m_pool == static_cast<byte*>(MAP_FAILED))
         {
         m_pool = nullptr;
         throw std::runtime_error("Failed to mmap locking_allocator pool");
         }

      clear_mem(m_pool, m_poolsize);

      if(::mlock(m_pool, m_poolsize) != 0)
         {
         ::munmap(m_pool, m_poolsize);
         m_pool = nullptr;
         throw std::runtime_error("Could not mlock " + std::to_string(m_poolsize) + " bytes");
         }

      m_freelist.push_back(std::make_pair(0, m_poolsize));
      }
   }

mlock_allocator::~mlock_allocator()
   {
   if(m_pool)
      {
      clear_mem(m_pool, m_poolsize);
      ::munlock(m_pool, m_poolsize);
      ::munmap(m_pool, m_poolsize);
      m_pool = nullptr;
      }
   }

mlock_allocator& mlock_allocator::instance()
   {
   static mlock_allocator mlock;
   return mlock;
   }

}
/*
* Algorithm Identifier
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const OID& alg_id,
                                         const std::vector<byte>& param)
   {
   oid = alg_id;
   parameters = param;
   }

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const std::string& alg_id,
                                         const std::vector<byte>& param)
   {
   oid = OIDS::lookup(alg_id);
   parameters = param;
   }

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const OID& alg_id,
                                         Encoding_Option option)
   {
   const byte DER_NULL[] = { 0x05, 0x00 };

   oid = alg_id;

   if(option == USE_NULL_PARAM)
      parameters += std::pair<const byte*, size_t>(DER_NULL, sizeof(DER_NULL));
   }

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const std::string& alg_id,
                                         Encoding_Option option)
   {
   const byte DER_NULL[] = { 0x05, 0x00 };

   oid = OIDS::lookup(alg_id);

   if(option == USE_NULL_PARAM)
      parameters += std::pair<const byte*, size_t>(DER_NULL, sizeof(DER_NULL));
   }

/*
* Compare two AlgorithmIdentifiers
*/
bool operator==(const AlgorithmIdentifier& a1, const AlgorithmIdentifier& a2)
   {
   if(a1.oid != a2.oid)
      return false;
   if(a1.parameters != a2.parameters)
      return false;
   return true;
   }

/*
* Compare two AlgorithmIdentifiers
*/
bool operator!=(const AlgorithmIdentifier& a1, const AlgorithmIdentifier& a2)
   {
   return !(a1 == a2);
   }

/*
* DER encode an AlgorithmIdentifier
*/
void AlgorithmIdentifier::encode_into(DER_Encoder& codec) const
   {
   codec.start_cons(SEQUENCE)
      .encode(oid)
      .raw_bytes(parameters)
   .end_cons();
   }

/*
* Decode a BER encoded AlgorithmIdentifier
*/
void AlgorithmIdentifier::decode_from(BER_Decoder& codec)
   {
   codec.start_cons(SEQUENCE)
      .decode(oid)
      .raw_bytes(parameters)
   .end_cons();
   }

}
/*
* AlternativeName
* (C) 1999-2007 Jack Lloyd
*     2007 Yves Jerschow
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* Check if type is a known ASN.1 string type
*/
bool is_string_type(ASN1_Tag tag)
   {
   return (tag == NUMERIC_STRING ||
           tag == PRINTABLE_STRING ||
           tag == VISIBLE_STRING ||
           tag == T61_STRING ||
           tag == IA5_STRING ||
           tag == UTF8_STRING ||
           tag == BMP_STRING);
   }

}

/*
* Create an AlternativeName
*/
AlternativeName::AlternativeName(const std::string& email_addr,
                                 const std::string& uri,
                                 const std::string& dns,
                                 const std::string& ip)
   {
   add_attribute("RFC822", email_addr);
   add_attribute("DNS", dns);
   add_attribute("URI", uri);
   add_attribute("IP", ip);
   }

/*
* Add an attribute to an alternative name
*/
void AlternativeName::add_attribute(const std::string& type,
                                    const std::string& str)
   {
   if(type == "" || str == "")
      return;

   auto range = alt_info.equal_range(type);
   for(auto j = range.first; j != range.second; ++j)
      if(j->second == str)
         return;

   multimap_insert(alt_info, type, str);
   }

/*
* Add an OtherName field
*/
void AlternativeName::add_othername(const OID& oid, const std::string& value,
                                    ASN1_Tag type)
   {
   if(value == "")
      return;
   multimap_insert(othernames, oid, ASN1_String(value, type));
   }

/*
* Get the attributes of this alternative name
*/
std::multimap<std::string, std::string> AlternativeName::get_attributes() const
   {
   return alt_info;
   }

/*
* Get the otherNames
*/
std::multimap<OID, ASN1_String> AlternativeName::get_othernames() const
   {
   return othernames;
   }

/*
* Return all of the alternative names
*/
std::multimap<std::string, std::string> AlternativeName::contents() const
   {
   std::multimap<std::string, std::string> names;

   for(auto i = alt_info.begin(); i != alt_info.end(); ++i)
      multimap_insert(names, i->first, i->second);

   for(auto i = othernames.begin(); i != othernames.end(); ++i)
      multimap_insert(names, OIDS::lookup(i->first), i->second.value());

   return names;
   }

/*
* Return if this object has anything useful
*/
bool AlternativeName::has_items() const
   {
   return (alt_info.size() > 0 || othernames.size() > 0);
   }

namespace {

/*
* DER encode an AlternativeName entry
*/
void encode_entries(DER_Encoder& encoder,
                    const std::multimap<std::string, std::string>& attr,
                    const std::string& type, ASN1_Tag tagging)
   {
   auto range = attr.equal_range(type);

   for(auto i = range.first; i != range.second; ++i)
      {
      if(type == "RFC822" || type == "DNS" || type == "URI")
         {
         ASN1_String asn1_string(i->second, IA5_STRING);
         encoder.add_object(tagging, CONTEXT_SPECIFIC, asn1_string.iso_8859());
         }
      else if(type == "IP")
         {
         const u32bit ip = string_to_ipv4(i->second);
         byte ip_buf[4] = { 0 };
         store_be(ip, ip_buf);
         encoder.add_object(tagging, CONTEXT_SPECIFIC, ip_buf, 4);
         }
      }
   }

}

/*
* DER encode an AlternativeName extension
*/
void AlternativeName::encode_into(DER_Encoder& der) const
   {
   der.start_cons(SEQUENCE);

   encode_entries(der, alt_info, "RFC822", ASN1_Tag(1));
   encode_entries(der, alt_info, "DNS", ASN1_Tag(2));
   encode_entries(der, alt_info, "URI", ASN1_Tag(6));
   encode_entries(der, alt_info, "IP", ASN1_Tag(7));

   for(auto i = othernames.begin(); i != othernames.end(); ++i)
      {
      der.start_explicit(0)
         .encode(i->first)
         .start_explicit(0)
            .encode(i->second)
         .end_explicit()
      .end_explicit();
      }

   der.end_cons();
   }

/*
* Decode a BER encoded AlternativeName
*/
void AlternativeName::decode_from(BER_Decoder& source)
   {
   BER_Decoder names = source.start_cons(SEQUENCE);

   while(names.more_items())
      {
      BER_Object obj = names.get_next_object();
      if((obj.class_tag != CONTEXT_SPECIFIC) &&
         (obj.class_tag != (CONTEXT_SPECIFIC | CONSTRUCTED)))
         continue;

      const ASN1_Tag tag = obj.type_tag;

      if(tag == 0)
         {
         BER_Decoder othername(obj.value);

         OID oid;
         othername.decode(oid);
         if(othername.more_items())
            {
            BER_Object othername_value_outer = othername.get_next_object();
            othername.verify_end();

            if(othername_value_outer.type_tag != ASN1_Tag(0) ||
               othername_value_outer.class_tag !=
                   (CONTEXT_SPECIFIC | CONSTRUCTED)
               )
               throw Decoding_Error("Invalid tags on otherName value");

            BER_Decoder othername_value_inner(othername_value_outer.value);

            BER_Object value = othername_value_inner.get_next_object();
            othername_value_inner.verify_end();

            const ASN1_Tag value_type = value.type_tag;

            if(is_string_type(value_type) && value.class_tag == UNIVERSAL)
               add_othername(oid, ASN1::to_string(value), value_type);
            }
         }
      else if(tag == 1 || tag == 2 || tag == 6)
         {
         const std::string value = Charset::transcode(ASN1::to_string(obj),
                                                      LATIN1_CHARSET,
                                                      LOCAL_CHARSET);

         if(tag == 1) add_attribute("RFC822", value);
         if(tag == 2) add_attribute("DNS", value);
         if(tag == 6) add_attribute("URI", value);
         }
      else if(tag == 7)
         {
         if(obj.value.size() == 4)
            {
            const u32bit ip = load_be<u32bit>(&obj.value[0], 0);
            add_attribute("IP", ipv4_to_string(ip));
            }
         }

      }
   }

}
/*
* Attribute
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Create an Attribute
*/
Attribute::Attribute(const OID& attr_oid, const std::vector<byte>& attr_value)
   {
   oid = attr_oid;
   parameters = attr_value;
   }

/*
* Create an Attribute
*/
Attribute::Attribute(const std::string& attr_oid,
                     const std::vector<byte>& attr_value)
   {
   oid = OIDS::lookup(attr_oid);
   parameters = attr_value;
   }

/*
* DER encode a Attribute
*/
void Attribute::encode_into(DER_Encoder& codec) const
   {
   codec.start_cons(SEQUENCE)
      .encode(oid)
      .start_cons(SET)
         .raw_bytes(parameters)
      .end_cons()
   .end_cons();
   }

/*
* Decode a BER encoded Attribute
*/
void Attribute::decode_from(BER_Decoder& codec)
   {
   codec.start_cons(SEQUENCE)
      .decode(oid)
      .start_cons(SET)
         .raw_bytes(parameters)
      .end_cons()
   .end_cons();
   }

}
/*
* ASN.1 Internals
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* BER Decoding Exceptions
*/
BER_Decoding_Error::BER_Decoding_Error(const std::string& str) :
   Decoding_Error("BER: " + str) {}

BER_Bad_Tag::BER_Bad_Tag(const std::string& str, ASN1_Tag tag) :
      BER_Decoding_Error(str + ": " + std::to_string(tag)) {}

BER_Bad_Tag::BER_Bad_Tag(const std::string& str,
                         ASN1_Tag tag1, ASN1_Tag tag2) :
   BER_Decoding_Error(str + ": " + std::to_string(tag1) + "/" + std::to_string(tag2)) {}

namespace ASN1 {

/*
* Put some arbitrary bytes into a SEQUENCE
*/
std::vector<byte> put_in_sequence(const std::vector<byte>& contents)
   {
   return DER_Encoder()
      .start_cons(SEQUENCE)
         .raw_bytes(contents)
      .end_cons()
   .get_contents_unlocked();
   }

/*
* Convert a BER object into a string object
*/
std::string to_string(const BER_Object& obj)
   {
   return std::string(reinterpret_cast<const char*>(&obj.value[0]),
                      obj.value.size());
   }

/*
* Do heuristic tests for BER data
*/
bool maybe_BER(DataSource& source)
   {
   byte first_byte;
   if(!source.peek_byte(first_byte))
      throw Stream_IO_Error("ASN1::maybe_BER: Source was empty");

   if(first_byte == (SEQUENCE | CONSTRUCTED))
      return true;
   return false;
   }

}

}
/*
* ASN.1 OID
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* ASN.1 OID Constructor
*/
OID::OID(const std::string& oid_str)
   {
   if(oid_str != "")
      {
      try
         {
         id = parse_asn1_oid(oid_str);
         }
      catch(...)
         {
         throw Invalid_OID(oid_str);
         }

      if(id.size() < 2 || id[0] > 2)
         throw Invalid_OID(oid_str);
      if((id[0] == 0 || id[0] == 1) && id[1] > 39)
         throw Invalid_OID(oid_str);
      }
   }

/*
* Clear the current OID
*/
void OID::clear()
   {
   id.clear();
   }

/*
* Return this OID as a string
*/
std::string OID::as_string() const
   {
   std::string oid_str;
   for(size_t i = 0; i != id.size(); ++i)
      {
      oid_str += std::to_string(id[i]);
      if(i != id.size() - 1)
         oid_str += '.';
      }
   return oid_str;
   }

/*
* OID equality comparison
*/
bool OID::operator==(const OID& oid) const
   {
   if(id.size() != oid.id.size())
      return false;
   for(size_t i = 0; i != id.size(); ++i)
      if(id[i] != oid.id[i])
         return false;
   return true;
   }

/*
* Append another component to the OID
*/
OID& OID::operator+=(u32bit component)
   {
   id.push_back(component);
   return (*this);
   }

/*
* Append another component to the OID
*/
OID operator+(const OID& oid, u32bit component)
   {
   OID new_oid(oid);
   new_oid += component;
   return new_oid;
   }

/*
* OID inequality comparison
*/
bool operator!=(const OID& a, const OID& b)
   {
   return !(a == b);
   }

/*
* Compare two OIDs
*/
bool operator<(const OID& a, const OID& b)
   {
   const std::vector<u32bit>& oid1 = a.get_id();
   const std::vector<u32bit>& oid2 = b.get_id();

   if(oid1.size() < oid2.size())
      return true;
   if(oid1.size() > oid2.size())
      return false;
   for(size_t i = 0; i != oid1.size(); ++i)
      {
      if(oid1[i] < oid2[i])
         return true;
      if(oid1[i] > oid2[i])
         return false;
      }
   return false;
   }

/*
* DER encode an OBJECT IDENTIFIER
*/
void OID::encode_into(DER_Encoder& der) const
   {
   if(id.size() < 2)
      throw Invalid_Argument("OID::encode_into: OID is invalid");

   std::vector<byte> encoding;
   encoding.push_back(40 * id[0] + id[1]);

   for(size_t i = 2; i != id.size(); ++i)
      {
      if(id[i] == 0)
         encoding.push_back(0);
      else
         {
         size_t blocks = high_bit(id[i]) + 6;
         blocks = (blocks - (blocks % 7)) / 7;

         for(size_t j = 0; j != blocks - 1; ++j)
            encoding.push_back(0x80 | ((id[i] >> 7*(blocks-j-1)) & 0x7F));
         encoding.push_back(id[i] & 0x7F);
         }
      }
   der.add_object(OBJECT_ID, UNIVERSAL, encoding);
   }

/*
* Decode a BER encoded OBJECT IDENTIFIER
*/
void OID::decode_from(BER_Decoder& decoder)
   {
   BER_Object obj = decoder.get_next_object();
   if(obj.type_tag != OBJECT_ID || obj.class_tag != UNIVERSAL)
      throw BER_Bad_Tag("Error decoding OID, unknown tag",
                        obj.type_tag, obj.class_tag);
   if(obj.value.size() < 2)
      throw BER_Decoding_Error("OID encoding is too short");


   clear();
   id.push_back(obj.value[0] / 40);
   id.push_back(obj.value[0] % 40);

   size_t i = 0;
   while(i != obj.value.size() - 1)
      {
      u32bit component = 0;
      while(i != obj.value.size() - 1)
         {
         ++i;

         if(component >> (32-7))
            throw Decoding_Error("OID component overflow");

         component = (component << 7) + (obj.value[i] & 0x7F);

         if(!(obj.value[i] & 0x80))
            break;
         }
      id.push_back(component);
      }
   }

}
/*
* Simple ASN.1 String Types
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* Choose an encoding for the string
*/
ASN1_Tag choose_encoding(const std::string& str,
                         const std::string& type)
   {
   static const byte IS_PRINTABLE[256] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
      0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00 };

   for(size_t i = 0; i != str.size(); ++i)
      {
      if(!IS_PRINTABLE[static_cast<byte>(str[i])])
         {
         if(type == "utf8")   return UTF8_STRING;
         if(type == "latin1") return T61_STRING;
         throw Invalid_Argument("choose_encoding: Bad string type " + type);
         }
      }
   return PRINTABLE_STRING;
   }

}

/*
* Create an ASN1_String
*/
ASN1_String::ASN1_String(const std::string& str, ASN1_Tag t) : tag(t)
   {
   iso_8859_str = Charset::transcode(str, LOCAL_CHARSET, LATIN1_CHARSET);

   if(tag == DIRECTORY_STRING)
      tag = choose_encoding(iso_8859_str, "latin1");

   if(tag != NUMERIC_STRING &&
      tag != PRINTABLE_STRING &&
      tag != VISIBLE_STRING &&
      tag != T61_STRING &&
      tag != IA5_STRING &&
      tag != UTF8_STRING &&
      tag != BMP_STRING)
      throw Invalid_Argument("ASN1_String: Unknown string type " +
                             std::to_string(tag));
   }

/*
* Create an ASN1_String
*/
ASN1_String::ASN1_String(const std::string& str)
   {
   iso_8859_str = Charset::transcode(str, LOCAL_CHARSET, LATIN1_CHARSET);
   tag = choose_encoding(iso_8859_str, "latin1");
   }

/*
* Return this string in ISO 8859-1 encoding
*/
std::string ASN1_String::iso_8859() const
   {
   return iso_8859_str;
   }

/*
* Return this string in local encoding
*/
std::string ASN1_String::value() const
   {
   return Charset::transcode(iso_8859_str, LATIN1_CHARSET, LOCAL_CHARSET);
   }

/*
* Return the type of this string object
*/
ASN1_Tag ASN1_String::tagging() const
   {
   return tag;
   }

/*
* DER encode an ASN1_String
*/
void ASN1_String::encode_into(DER_Encoder& encoder) const
   {
   std::string value = iso_8859();
   if(tagging() == UTF8_STRING)
      value = Charset::transcode(value, LATIN1_CHARSET, UTF8_CHARSET);
   encoder.add_object(tagging(), UNIVERSAL, value);
   }

/*
* Decode a BER encoded ASN1_String
*/
void ASN1_String::decode_from(BER_Decoder& source)
   {
   BER_Object obj = source.get_next_object();

   Character_Set charset_is;

   if(obj.type_tag == BMP_STRING)
      charset_is = UCS2_CHARSET;
   else if(obj.type_tag == UTF8_STRING)
      charset_is = UTF8_CHARSET;
   else
      charset_is = LATIN1_CHARSET;

   *this = ASN1_String(
      Charset::transcode(ASN1::to_string(obj), charset_is, LOCAL_CHARSET),
      obj.type_tag);
   }

}
/*
* X.509 Time Types
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Create an X509_Time
*/
X509_Time::X509_Time(const std::string& time_str)
   {
   set_to(time_str);
   }

/*
* Create a X509_Time from a time point
*/
X509_Time::X509_Time(const std::chrono::system_clock::time_point& time)
   {
   calendar_point cal = calendar_value(time);

   year   = cal.year;
   month  = cal.month;
   day    = cal.day;
   hour   = cal.hour;
   minute = cal.minutes;
   second = cal.seconds;

   tag = (year >= 2050) ? GENERALIZED_TIME : UTC_TIME;
   }

/*
* Create an X509_Time
*/
X509_Time::X509_Time(const std::string& t_spec, ASN1_Tag t) : tag(t)
   {
   set_to(t_spec, tag);
   }

/*
* Set the time with a human readable string
*/
void X509_Time::set_to(const std::string& time_str)
   {
   if(time_str == "")
      {
      year = month = day = hour = minute = second = 0;
      tag = NO_OBJECT;
      return;
      }

   std::vector<std::string> params;
   std::string current;

   for(size_t j = 0; j != time_str.size(); ++j)
      {
      if(Charset::is_digit(time_str[j]))
         current += time_str[j];
      else
         {
         if(current != "")
            params.push_back(current);
         current.clear();
         }
      }
   if(current != "")
      params.push_back(current);

   if(params.size() < 3 || params.size() > 6)
      throw Invalid_Argument("Invalid time specification " + time_str);

   year   = to_u32bit(params[0]);
   month  = to_u32bit(params[1]);
   day    = to_u32bit(params[2]);
   hour   = (params.size() >= 4) ? to_u32bit(params[3]) : 0;
   minute = (params.size() >= 5) ? to_u32bit(params[4]) : 0;
   second = (params.size() == 6) ? to_u32bit(params[5]) : 0;

   tag = (year >= 2050) ? GENERALIZED_TIME : UTC_TIME;

   if(!passes_sanity_check())
      throw Invalid_Argument("Invalid time specification " + time_str);
   }

/*
* Set the time with an ISO time format string
*/
void X509_Time::set_to(const std::string& t_spec, ASN1_Tag spec_tag)
   {
   if(spec_tag == GENERALIZED_TIME)
      {
      if(t_spec.size() != 13 && t_spec.size() != 15)
         throw Invalid_Argument("Invalid GeneralizedTime: " + t_spec);
      }
   else if(spec_tag == UTC_TIME)
      {
      if(t_spec.size() != 11 && t_spec.size() != 13)
         throw Invalid_Argument("Invalid UTCTime: " + t_spec);
      }
   else
      {
      throw Invalid_Argument("Invalid time tag " + std::to_string(spec_tag) + " val " + t_spec);
      }

   if(t_spec[t_spec.size()-1] != 'Z')
      throw Invalid_Argument("Invalid time encoding: " + t_spec);

   const size_t YEAR_SIZE = (spec_tag == UTC_TIME) ? 2 : 4;

   std::vector<std::string> params;
   std::string current;

   for(size_t j = 0; j != YEAR_SIZE; ++j)
      current += t_spec[j];
   params.push_back(current);
   current.clear();

   for(size_t j = YEAR_SIZE; j != t_spec.size() - 1; ++j)
      {
      current += t_spec[j];
      if(current.size() == 2)
         {
         params.push_back(current);
         current.clear();
         }
      }

   year   = to_u32bit(params[0]);
   month  = to_u32bit(params[1]);
   day    = to_u32bit(params[2]);
   hour   = to_u32bit(params[3]);
   minute = to_u32bit(params[4]);
   second = (params.size() == 6) ? to_u32bit(params[5]) : 0;
   tag    = spec_tag;

   if(spec_tag == UTC_TIME)
      {
      if(year >= 50) year += 1900;
      else           year += 2000;
      }

   if(!passes_sanity_check())
      throw Invalid_Argument("Invalid time specification " + t_spec);
   }

/*
* DER encode a X509_Time
*/
void X509_Time::encode_into(DER_Encoder& der) const
   {
   if(tag != GENERALIZED_TIME && tag != UTC_TIME)
      throw Invalid_Argument("X509_Time: Bad encoding tag");

   der.add_object(tag, UNIVERSAL,
                  Charset::transcode(as_string(),
                                     LOCAL_CHARSET,
                                     LATIN1_CHARSET));
   }

/*
* Decode a BER encoded X509_Time
*/
void X509_Time::decode_from(BER_Decoder& source)
   {
   BER_Object ber_time = source.get_next_object();

   set_to(Charset::transcode(ASN1::to_string(ber_time),
                             LATIN1_CHARSET,
                             LOCAL_CHARSET),
          ber_time.type_tag);
   }

/*
* Return a string representation of the time
*/
std::string X509_Time::as_string() const
   {
   if(time_is_set() == false)
      throw Invalid_State("X509_Time::as_string: No time set");

   u32bit full_year = year;

   if(tag == UTC_TIME)
      {
      if(year < 1950 || year >= 2050)
         throw Encoding_Error("X509_Time: The time " + readable_string() +
                              " cannot be encoded as a UTCTime");

      full_year = (year >= 2000) ? (year - 2000) : (year - 1900);
      }

   std::string repr = std::to_string(full_year*10000000000 +
                                     month*100000000 +
                                     day*1000000 +
                                     hour*10000 +
                                     minute*100 +
                                     second) + "Z";

   u32bit desired_size = (tag == UTC_TIME) ? 13 : 15;

   while(repr.size() < desired_size)
      repr = "0" + repr;

   return repr;
   }

/*
* Return if the time has been set somehow
*/
bool X509_Time::time_is_set() const
   {
   return (year != 0);
   }

/*
* Return a human readable string representation
*/
std::string X509_Time::readable_string() const
   {
   if(time_is_set() == false)
      throw Invalid_State("X509_Time::readable_string: No time set");

   std::string output(24, 0);

   std::sprintf(&output[0], "%04d/%02d/%02d %02d:%02d:%02d UTC",
                year, month, day, hour, minute, second);

   output.resize(23); // remove trailing null

   return output;
   }

/*
* Do a general sanity check on the time
*/
bool X509_Time::passes_sanity_check() const
   {
   if(year < 1950 || year > 2100)
      return false;
   if(month == 0 || month > 12)
      return false;
   if(day == 0 || day > 31)
      return false;
   if(hour >= 24 || minute > 60 || second > 60)
      return false;
   return true;
   }

/*
* Compare this time against another
*/
s32bit X509_Time::cmp(const X509_Time& other) const
   {
   if(time_is_set() == false)
      throw Invalid_State("X509_Time::cmp: No time set");

   const s32bit EARLIER = -1, LATER = 1, SAME_TIME = 0;

   if(year < other.year)     return EARLIER;
   if(year > other.year)     return LATER;
   if(month < other.month)   return EARLIER;
   if(month > other.month)   return LATER;
   if(day < other.day)       return EARLIER;
   if(day > other.day)       return LATER;
   if(hour < other.hour)     return EARLIER;
   if(hour > other.hour)     return LATER;
   if(minute < other.minute) return EARLIER;
   if(minute > other.minute) return LATER;
   if(second < other.second) return EARLIER;
   if(second > other.second) return LATER;

   return SAME_TIME;
   }

/*
* Compare two X509_Times for in various ways
*/
bool operator==(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) == 0); }
bool operator!=(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) != 0); }

bool operator<=(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) <= 0); }
bool operator>=(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) >= 0); }

bool operator<(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) < 0); }
bool operator>(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) > 0); }

}
/*
* BER Decoder
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* BER decode an ASN.1 type tag
*/
size_t decode_tag(DataSource* ber, ASN1_Tag& type_tag, ASN1_Tag& class_tag)
   {
   byte b;
   if(!ber->read_byte(b))
      {
      class_tag = type_tag = NO_OBJECT;
      return 0;
      }

   if((b & 0x1F) != 0x1F)
      {
      type_tag = ASN1_Tag(b & 0x1F);
      class_tag = ASN1_Tag(b & 0xE0);
      return 1;
      }

   size_t tag_bytes = 1;
   class_tag = ASN1_Tag(b & 0xE0);

   size_t tag_buf = 0;
   while(true)
      {
      if(!ber->read_byte(b))
         throw BER_Decoding_Error("Long-form tag truncated");
      if(tag_buf & 0xFF000000)
         throw BER_Decoding_Error("Long-form tag overflowed 32 bits");
      ++tag_bytes;
      tag_buf = (tag_buf << 7) | (b & 0x7F);
      if((b & 0x80) == 0) break;
      }
   type_tag = ASN1_Tag(tag_buf);
   return tag_bytes;
   }

/*
* Find the EOC marker
*/
size_t find_eoc(DataSource*);

/*
* BER decode an ASN.1 length field
*/
size_t decode_length(DataSource* ber, size_t& field_size)
   {
   byte b;
   if(!ber->read_byte(b))
      throw BER_Decoding_Error("Length field not found");
   field_size = 1;
   if((b & 0x80) == 0)
      return b;

   field_size += (b & 0x7F);
   if(field_size == 1) return find_eoc(ber);
   if(field_size > 5)
      throw BER_Decoding_Error("Length field is too large");

   size_t length = 0;

   for(size_t i = 0; i != field_size - 1; ++i)
      {
      if(get_byte(0, length) != 0)
         throw BER_Decoding_Error("Field length overflow");
      if(!ber->read_byte(b))
         throw BER_Decoding_Error("Corrupted length field");
      length = (length << 8) | b;
      }
   return length;
   }

/*
* BER decode an ASN.1 length field
*/
size_t decode_length(DataSource* ber)
   {
   size_t dummy;
   return decode_length(ber, dummy);
   }

/*
* Find the EOC marker
*/
size_t find_eoc(DataSource* ber)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE), data;

   while(true)
      {
      const size_t got = ber->peek(&buffer[0], buffer.size(), data.size());
      if(got == 0)
         break;

      data += std::make_pair(&buffer[0], got);
      }

   DataSource_Memory source(data);
   data.clear();

   size_t length = 0;
   while(true)
      {
      ASN1_Tag type_tag, class_tag;
      size_t tag_size = decode_tag(&source, type_tag, class_tag);
      if(type_tag == NO_OBJECT)
         break;

      size_t length_size = 0;
      size_t item_size = decode_length(&source, length_size);
      source.discard_next(item_size);

      length += item_size + length_size + tag_size;

      if(type_tag == EOC)
         break;
      }
   return length;
   }

}

/*
* Check a type invariant on BER data
*/
void BER_Object::assert_is_a(ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(this->type_tag != type_tag || this->class_tag != class_tag)
      throw BER_Decoding_Error("Tag mismatch when decoding got " +
                               std::to_string(this->type_tag) + "/" +
                               std::to_string(this->class_tag) + " expected " +
                               std::to_string(type_tag) + "/" +
                               std::to_string(class_tag));
   }

/*
* Check if more objects are there
*/
bool BER_Decoder::more_items() const
   {
   if(source->end_of_data() && (pushed.type_tag == NO_OBJECT))
      return false;
   return true;
   }

/*
* Verify that no bytes remain in the source
*/
BER_Decoder& BER_Decoder::verify_end()
   {
   if(!source->end_of_data() || (pushed.type_tag != NO_OBJECT))
      throw Invalid_State("BER_Decoder::verify_end called, but data remains");
   return (*this);
   }

/*
* Save all the bytes remaining in the source
*/
BER_Decoder& BER_Decoder::raw_bytes(secure_vector<byte>& out)
   {
   out.clear();
   byte buf;
   while(source->read_byte(buf))
      out.push_back(buf);
   return (*this);
   }

BER_Decoder& BER_Decoder::raw_bytes(std::vector<byte>& out)
   {
   out.clear();
   byte buf;
   while(source->read_byte(buf))
      out.push_back(buf);
   return (*this);
   }

/*
* Discard all the bytes remaining in the source
*/
BER_Decoder& BER_Decoder::discard_remaining()
   {
   byte buf;
   while(source->read_byte(buf))
      ;
   return (*this);
   }

/*
* Return the BER encoding of the next object
*/
BER_Object BER_Decoder::get_next_object()
   {
   BER_Object next;

   if(pushed.type_tag != NO_OBJECT)
      {
      next = pushed;
      pushed.class_tag = pushed.type_tag = NO_OBJECT;
      return next;
      }

   decode_tag(source, next.type_tag, next.class_tag);
   if(next.type_tag == NO_OBJECT)
      return next;

   size_t length = decode_length(source);
   next.value.resize(length);
   if(source->read(&next.value[0], length) != length)
      throw BER_Decoding_Error("Value truncated");

   if(next.type_tag == EOC && next.class_tag == UNIVERSAL)
      return get_next_object();

   return next;
   }

BER_Decoder& BER_Decoder::get_next(BER_Object& ber)
   {
   ber = get_next_object();
   return (*this);
   }

/*
* Push a object back into the stream
*/
void BER_Decoder::push_back(const BER_Object& obj)
   {
   if(pushed.type_tag != NO_OBJECT)
      throw Invalid_State("BER_Decoder: Only one push back is allowed");
   pushed = obj;
   }

/*
* Begin decoding a CONSTRUCTED type
*/
BER_Decoder BER_Decoder::start_cons(ASN1_Tag type_tag,
                                    ASN1_Tag class_tag)
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, ASN1_Tag(class_tag | CONSTRUCTED));

   BER_Decoder result(&obj.value[0], obj.value.size());
   result.parent = this;
   return result;
   }

/*
* Finish decoding a CONSTRUCTED type
*/
BER_Decoder& BER_Decoder::end_cons()
   {
   if(!parent)
      throw Invalid_State("BER_Decoder::end_cons called with NULL parent");
   if(!source->end_of_data())
      throw Decoding_Error("BER_Decoder::end_cons called with data left");
   return (*parent);
   }

/*
* BER_Decoder Constructor
*/
BER_Decoder::BER_Decoder(DataSource& src)
   {
   source = &src;
   owns = false;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Constructor
 */
BER_Decoder::BER_Decoder(const byte data[], size_t length)
   {
   source = new DataSource_Memory(data, length);
   owns = true;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Constructor
*/
BER_Decoder::BER_Decoder(const secure_vector<byte>& data)
   {
   source = new DataSource_Memory(data);
   owns = true;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Constructor
*/
BER_Decoder::BER_Decoder(const std::vector<byte>& data)
   {
   source = new DataSource_Memory(&data[0], data.size());
   owns = true;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Copy Constructor
*/
BER_Decoder::BER_Decoder(const BER_Decoder& other)
   {
   source = other.source;
   owns = false;
   if(other.owns)
      {
      other.owns = false;
      owns = true;
      }
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = other.parent;
   }

/*
* BER_Decoder Destructor
*/
BER_Decoder::~BER_Decoder()
   {
   if(owns)
      delete source;
   source = nullptr;
   }

/*
* Request for an object to decode itself
*/
BER_Decoder& BER_Decoder::decode(ASN1_Object& obj,
                                 ASN1_Tag, ASN1_Tag)
   {
   obj.decode_from(*this);
   return (*this);
   }

/*
* Decode a BER encoded NULL
*/
BER_Decoder& BER_Decoder::decode_null()
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(NULL_TAG, UNIVERSAL);
   if(obj.value.size())
      throw BER_Decoding_Error("NULL object had nonzero size");
   return (*this);
   }

/*
* Decode a BER encoded BOOLEAN
*/
BER_Decoder& BER_Decoder::decode(bool& out)
   {
   return decode(out, BOOLEAN, UNIVERSAL);
   }

/*
* Decode a small BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(size_t& out)
   {
   return decode(out, INTEGER, UNIVERSAL);
   }

/*
* Decode a BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(BigInt& out)
   {
   return decode(out, INTEGER, UNIVERSAL);
   }

BER_Decoder& BER_Decoder::decode_octet_string_bigint(BigInt& out)
   {
   secure_vector<byte> out_vec;
   decode(out_vec, OCTET_STRING);
   out = BigInt::decode(&out_vec[0], out_vec.size());
   return (*this);
   }

std::vector<byte> BER_Decoder::get_next_octet_string()
   {
   std::vector<byte> out_vec;
   decode(out_vec, OCTET_STRING);
   return out_vec;
   }

/*
* Decode a BER encoded BOOLEAN
*/
BER_Decoder& BER_Decoder::decode(bool& out,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(obj.value.size() != 1)
      throw BER_Decoding_Error("BER boolean value had invalid size");

   out = (obj.value[0]) ? true : false;
   return (*this);
   }

/*
* Decode a small BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(size_t& out,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   BigInt integer;
   decode(integer, type_tag, class_tag);

   if(integer.bits() > 32)
      throw BER_Decoding_Error("Decoded integer value larger than expected");

   out = 0;
   for(size_t i = 0; i != 4; ++i)
      out = (out << 8) | integer.byte_at(3-i);

   return (*this);
   }

/*
* Decode a small BER encoded INTEGER
*/
u64bit BER_Decoder::decode_constrained_integer(ASN1_Tag type_tag,
                                               ASN1_Tag class_tag,
                                               size_t T_bytes)
   {
   if(T_bytes > 8)
      throw BER_Decoding_Error("Can't decode small integer over 8 bytes");

   BigInt integer;
   decode(integer, type_tag, class_tag);

   if(integer.bits() > 8*T_bytes)
      throw BER_Decoding_Error("Decoded integer value larger than expected");

   u64bit out = 0;
   for(size_t i = 0; i != 8; ++i)
      out = (out << 8) | integer.byte_at(7-i);

   return out;
   }

/*
* Decode a BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(BigInt& out,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(obj.value.empty())
      out = 0;
   else
      {
      const bool negative = (obj.value[0] & 0x80) ? true : false;

      if(negative)
         {
         for(size_t i = obj.value.size(); i > 0; --i)
            if(obj.value[i-1]--)
               break;
         for(size_t i = 0; i != obj.value.size(); ++i)
            obj.value[i] = ~obj.value[i];
         }

      out = BigInt(&obj.value[0], obj.value.size());

      if(negative)
         out.flip_sign();
      }

   return (*this);
   }

/*
* BER decode a BIT STRING or OCTET STRING
*/
BER_Decoder& BER_Decoder::decode(secure_vector<byte>& out, ASN1_Tag real_type)
   {
   return decode(out, real_type, real_type, UNIVERSAL);
   }

/*
* BER decode a BIT STRING or OCTET STRING
*/
BER_Decoder& BER_Decoder::decode(std::vector<byte>& out, ASN1_Tag real_type)
   {
   return decode(out, real_type, real_type, UNIVERSAL);
   }

/*
* BER decode a BIT STRING or OCTET STRING
*/
BER_Decoder& BER_Decoder::decode(secure_vector<byte>& buffer,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(real_type != OCTET_STRING && real_type != BIT_STRING)
      throw BER_Bad_Tag("Bad tag for {BIT,OCTET} STRING", real_type);

   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(real_type == OCTET_STRING)
      buffer = obj.value;
   else
      {
      if(obj.value[0] >= 8)
         throw BER_Decoding_Error("Bad number of unused bits in BIT STRING");

      buffer.resize(obj.value.size() - 1);
      copy_mem(&buffer[0], &obj.value[1], obj.value.size() - 1);
      }
   return (*this);
   }

BER_Decoder& BER_Decoder::decode(std::vector<byte>& buffer,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(real_type != OCTET_STRING && real_type != BIT_STRING)
      throw BER_Bad_Tag("Bad tag for {BIT,OCTET} STRING", real_type);

   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(real_type == OCTET_STRING)
      buffer = unlock(obj.value);
   else
      {
      if(obj.value[0] >= 8)
         throw BER_Decoding_Error("Bad number of unused bits in BIT STRING");

      buffer.resize(obj.value.size() - 1);
      copy_mem(&buffer[0], &obj.value[1], obj.value.size() - 1);
      }
   return (*this);
   }

}
/*
* DER Encoder
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* DER encode an ASN.1 type tag
*/
secure_vector<byte> encode_tag(ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if((class_tag | 0xE0) != 0xE0)
      throw Encoding_Error("DER_Encoder: Invalid class tag " +
                           std::to_string(class_tag));

   secure_vector<byte> encoded_tag;
   if(type_tag <= 30)
      encoded_tag.push_back(static_cast<byte>(type_tag | class_tag));
   else
      {
      size_t blocks = high_bit(type_tag) + 6;
      blocks = (blocks - (blocks % 7)) / 7;

      encoded_tag.push_back(class_tag | 0x1F);
      for(size_t i = 0; i != blocks - 1; ++i)
         encoded_tag.push_back(0x80 | ((type_tag >> 7*(blocks-i-1)) & 0x7F));
      encoded_tag.push_back(type_tag & 0x7F);
      }

   return encoded_tag;
   }

/*
* DER encode an ASN.1 length field
*/
secure_vector<byte> encode_length(size_t length)
   {
   secure_vector<byte> encoded_length;
   if(length <= 127)
      encoded_length.push_back(static_cast<byte>(length));
   else
      {
      const size_t top_byte = significant_bytes(length);

      encoded_length.push_back(static_cast<byte>(0x80 | top_byte));

      for(size_t i = sizeof(length) - top_byte; i != sizeof(length); ++i)
         encoded_length.push_back(get_byte(i, length));
      }
   return encoded_length;
   }

}

/*
* Return the encoded SEQUENCE/SET
*/
secure_vector<byte> DER_Encoder::DER_Sequence::get_contents()
   {
   const ASN1_Tag real_class_tag = ASN1_Tag(class_tag | CONSTRUCTED);

   if(type_tag == SET)
      {
      std::sort(set_contents.begin(), set_contents.end());
      for(size_t i = 0; i != set_contents.size(); ++i)
         contents += set_contents[i];
      set_contents.clear();
      }

   secure_vector<byte> result;
   result += encode_tag(type_tag, real_class_tag);
   result += encode_length(contents.size());
   result += contents;
   contents.clear();

   return result;
   }

/*
* Add an encoded value to the SEQUENCE/SET
*/
void DER_Encoder::DER_Sequence::add_bytes(const byte data[], size_t length)
   {
   if(type_tag == SET)
      set_contents.push_back(secure_vector<byte>(data, data + length));
   else
      contents += std::make_pair(data, length);
   }

/*
* Return the type and class taggings
*/
ASN1_Tag DER_Encoder::DER_Sequence::tag_of() const
   {
   return ASN1_Tag(type_tag | class_tag);
   }

/*
* DER_Sequence Constructor
*/
DER_Encoder::DER_Sequence::DER_Sequence(ASN1_Tag t1, ASN1_Tag t2) :
   type_tag(t1), class_tag(t2)
   {
   }

/*
* Return the encoded contents
*/
secure_vector<byte> DER_Encoder::get_contents()
   {
   if(subsequences.size() != 0)
      throw Invalid_State("DER_Encoder: Sequence hasn't been marked done");

   secure_vector<byte> output;
   std::swap(output, contents);
   return output;
   }

/*
* Start a new ASN.1 SEQUENCE/SET/EXPLICIT
*/
DER_Encoder& DER_Encoder::start_cons(ASN1_Tag type_tag,
                                     ASN1_Tag class_tag)
   {
   subsequences.push_back(DER_Sequence(type_tag, class_tag));
   return (*this);
   }

/*
* Finish the current ASN.1 SEQUENCE/SET/EXPLICIT
*/
DER_Encoder& DER_Encoder::end_cons()
   {
   if(subsequences.empty())
      throw Invalid_State("DER_Encoder::end_cons: No such sequence");

   secure_vector<byte> seq = subsequences[subsequences.size()-1].get_contents();
   subsequences.pop_back();
   raw_bytes(seq);
   return (*this);
   }

/*
* Start a new ASN.1 EXPLICIT encoding
*/
DER_Encoder& DER_Encoder::start_explicit(u16bit type_no)
   {
   ASN1_Tag type_tag = static_cast<ASN1_Tag>(type_no);

   if(type_tag == SET)
      throw Internal_Error("DER_Encoder.start_explicit(SET); cannot perform");

   return start_cons(type_tag, CONTEXT_SPECIFIC);
   }

/*
* Finish the current ASN.1 EXPLICIT encoding
*/
DER_Encoder& DER_Encoder::end_explicit()
   {
   return end_cons();
   }

/*
* Write raw bytes into the stream
*/
DER_Encoder& DER_Encoder::raw_bytes(const secure_vector<byte>& val)
   {
   return raw_bytes(&val[0], val.size());
   }

DER_Encoder& DER_Encoder::raw_bytes(const std::vector<byte>& val)
   {
   return raw_bytes(&val[0], val.size());
   }

/*
* Write raw bytes into the stream
*/
DER_Encoder& DER_Encoder::raw_bytes(const byte bytes[], size_t length)
   {
   if(subsequences.size())
      subsequences[subsequences.size()-1].add_bytes(bytes, length);
   else
      contents += std::make_pair(bytes, length);

   return (*this);
   }

/*
* Encode a NULL object
*/
DER_Encoder& DER_Encoder::encode_null()
   {
   return add_object(NULL_TAG, UNIVERSAL, nullptr, 0);
   }

/*
* DER encode a BOOLEAN
*/
DER_Encoder& DER_Encoder::encode(bool is_true)
   {
   return encode(is_true, BOOLEAN, UNIVERSAL);
   }

/*
* DER encode a small INTEGER
*/
DER_Encoder& DER_Encoder::encode(size_t n)
   {
   return encode(BigInt(n), INTEGER, UNIVERSAL);
   }

/*
* DER encode a small INTEGER
*/
DER_Encoder& DER_Encoder::encode(const BigInt& n)
   {
   return encode(n, INTEGER, UNIVERSAL);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const secure_vector<byte>& bytes,
                                 ASN1_Tag real_type)
   {
   return encode(&bytes[0], bytes.size(),
                 real_type, real_type, UNIVERSAL);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const std::vector<byte>& bytes,
                                 ASN1_Tag real_type)
   {
   return encode(&bytes[0], bytes.size(),
                 real_type, real_type, UNIVERSAL);
   }

/*
* Encode this object
*/
DER_Encoder& DER_Encoder::encode(const byte bytes[], size_t length,
                                 ASN1_Tag real_type)
   {
   return encode(bytes, length, real_type, real_type, UNIVERSAL);
   }

/*
* DER encode a BOOLEAN
*/
DER_Encoder& DER_Encoder::encode(bool is_true,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   byte val = is_true ? 0xFF : 0x00;
   return add_object(type_tag, class_tag, &val, 1);
   }

/*
* DER encode a small INTEGER
*/
DER_Encoder& DER_Encoder::encode(size_t n,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   return encode(BigInt(n), type_tag, class_tag);
   }

/*
* DER encode an INTEGER
*/
DER_Encoder& DER_Encoder::encode(const BigInt& n,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(n == 0)
      return add_object(type_tag, class_tag, 0);

   bool extra_zero = (n.bits() % 8 == 0);
   secure_vector<byte> contents(extra_zero + n.bytes());
   BigInt::encode(&contents[extra_zero], n);
   if(n < 0)
      {
      for(size_t i = 0; i != contents.size(); ++i)
         contents[i] = ~contents[i];
      for(size_t i = contents.size(); i > 0; --i)
         if(++contents[i-1])
            break;
      }

   return add_object(type_tag, class_tag, contents);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const secure_vector<byte>& bytes,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   return encode(&bytes[0], bytes.size(),
                 real_type, type_tag, class_tag);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const std::vector<byte>& bytes,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   return encode(&bytes[0], bytes.size(),
                 real_type, type_tag, class_tag);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const byte bytes[], size_t length,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(real_type != OCTET_STRING && real_type != BIT_STRING)
      throw Invalid_Argument("DER_Encoder: Invalid tag for byte/bit string");

   if(real_type == BIT_STRING)
      {
      secure_vector<byte> encoded;
      encoded.push_back(0);
      encoded += std::make_pair(bytes, length);
      return add_object(type_tag, class_tag, encoded);
      }
   else
      return add_object(type_tag, class_tag, bytes, length);
   }

/*
* Conditionally write some values to the stream
*/
DER_Encoder& DER_Encoder::encode_if(bool cond, DER_Encoder& codec)
   {
   if(cond)
      return raw_bytes(codec.get_contents());
   return (*this);
   }

DER_Encoder& DER_Encoder::encode_if(bool cond, const ASN1_Object& obj)
   {
   if(cond)
      encode(obj);
   return (*this);
   }

/*
* Request for an object to encode itself
*/
DER_Encoder& DER_Encoder::encode(const ASN1_Object& obj)
   {
   obj.encode_into(*this);
   return (*this);
   }

/*
* Write the encoding of the byte(s)
*/
DER_Encoder& DER_Encoder::add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                                     const byte rep[], size_t length)
   {
   secure_vector<byte> buffer;
   buffer += encode_tag(type_tag, class_tag);
   buffer += encode_length(length);
   buffer += std::make_pair(rep, length);

   return raw_bytes(buffer);
   }

/*
* Write the encoding of the byte(s)
*/
DER_Encoder& DER_Encoder::add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                                     const std::string& rep_str)
   {
   const byte* rep = reinterpret_cast<const byte*>(rep_str.data());
   const size_t rep_len = rep_str.size();
   return add_object(type_tag, class_tag, rep, rep_len);
   }

/*
* Write the encoding of the byte
*/
DER_Encoder& DER_Encoder::add_object(ASN1_Tag type_tag,
                                     ASN1_Tag class_tag, byte rep)
   {
   return add_object(type_tag, class_tag, &rep, 1);
   }

}
/*
* OID Registry
* (C) 1999-2010,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace OIDS {

/*
* Load all of the default OIDs
*/
void set_defaults()
   {
   /* Public key types */
   OIDS::add_oidstr("1.2.840.113549.1.1.1", "RSA");
   OIDS::add_oidstr("2.5.8.1.1", "RSA"); // RSA alternate
   OIDS::add_oidstr("1.2.840.10040.4.1", "DSA");
   OIDS::add_oidstr("1.2.840.10046.2.1", "DH");
   OIDS::add_oidstr("1.3.6.1.4.1.3029.1.2.1", "ElGamal");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.1.1", "RW");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.1.2", "NR");

   // X9.62 ecPublicKey, valid for ECDSA and ECDH (RFC 3279 sec 2.3.5)
   OIDS::add_oidstr("1.2.840.10045.2.1", "ECDSA");

   /*
   * This is an OID defined for ECDH keys though rarely used for such.
   * In this configuration it is accepted on decoding, but not used for
   * encoding. You can enable it for encoding by calling
   *    OIDS::add_str2oid("ECDH", "1.3.132.1.12")
   * from your application code.
   */
   OIDS::add_oid2str(OID("1.3.132.1.12"), "ECDH");

   OIDS::add_oidstr("1.2.643.2.2.19", "GOST-34.10"); // RFC 4491

   /* Ciphers */
   OIDS::add_oidstr("1.3.14.3.2.7", "DES/CBC");
   OIDS::add_oidstr("1.2.840.113549.3.7", "TripleDES/CBC");
   OIDS::add_oidstr("1.2.840.113549.3.2", "RC2/CBC");
   OIDS::add_oidstr("1.2.840.113533.7.66.10", "CAST-128/CBC");
   OIDS::add_oidstr("2.16.840.1.101.3.4.1.2", "AES-128/CBC");
   OIDS::add_oidstr("2.16.840.1.101.3.4.1.22", "AES-192/CBC");
   OIDS::add_oidstr("2.16.840.1.101.3.4.1.42", "AES-256/CBC");
   OIDS::add_oidstr("1.2.410.200004.1.4", "SEED/CBC"); // RFC 4010
   OIDS::add_oidstr("1.3.6.1.4.1.25258.3.1", "Serpent/CBC");

   /* Hash Functions */
   OIDS::add_oidstr("1.2.840.113549.2.5", "MD5");
   OIDS::add_oidstr("1.3.6.1.4.1.11591.12.2", "Tiger(24,3)");

   OIDS::add_oidstr("1.3.14.3.2.26", "SHA-160");
   OIDS::add_oidstr("2.16.840.1.101.3.4.2.4", "SHA-224");
   OIDS::add_oidstr("2.16.840.1.101.3.4.2.1", "SHA-256");
   OIDS::add_oidstr("2.16.840.1.101.3.4.2.2", "SHA-384");
   OIDS::add_oidstr("2.16.840.1.101.3.4.2.3", "SHA-512");

   /* MACs */
   OIDS::add_oidstr("1.2.840.113549.2.7", "HMAC(SHA-160)");
   OIDS::add_oidstr("1.2.840.113549.2.8", "HMAC(SHA-224)");
   OIDS::add_oidstr("1.2.840.113549.2.9", "HMAC(SHA-256)");
   OIDS::add_oidstr("1.2.840.113549.2.10", "HMAC(SHA-384)");
   OIDS::add_oidstr("1.2.840.113549.2.11", "HMAC(SHA-512)");

   /* Key Wrap */
   OIDS::add_oidstr("1.2.840.113549.1.9.16.3.6", "KeyWrap.TripleDES");
   OIDS::add_oidstr("1.2.840.113549.1.9.16.3.7", "KeyWrap.RC2");
   OIDS::add_oidstr("1.2.840.113533.7.66.15", "KeyWrap.CAST-128");
   OIDS::add_oidstr("2.16.840.1.101.3.4.1.5", "KeyWrap.AES-128");
   OIDS::add_oidstr("2.16.840.1.101.3.4.1.25", "KeyWrap.AES-192");
   OIDS::add_oidstr("2.16.840.1.101.3.4.1.45", "KeyWrap.AES-256");

   /* Compression */
   OIDS::add_oidstr("1.2.840.113549.1.9.16.3.8", "Compression.Zlib");

   /* Public key signature schemes */
   OIDS::add_oidstr("1.2.840.113549.1.1.1", "RSA/EME-PKCS1-v1_5");
   OIDS::add_oidstr("1.2.840.113549.1.1.2", "RSA/EMSA3(MD2)");
   OIDS::add_oidstr("1.2.840.113549.1.1.4", "RSA/EMSA3(MD5)");
   OIDS::add_oidstr("1.2.840.113549.1.1.5", "RSA/EMSA3(SHA-160)");
   OIDS::add_oidstr("1.2.840.113549.1.1.11", "RSA/EMSA3(SHA-256)");
   OIDS::add_oidstr("1.2.840.113549.1.1.12", "RSA/EMSA3(SHA-384)");
   OIDS::add_oidstr("1.2.840.113549.1.1.13", "RSA/EMSA3(SHA-512)");
   OIDS::add_oidstr("1.3.36.3.3.1.2", "RSA/EMSA3(RIPEMD-160)");

   OIDS::add_oidstr("1.2.840.10040.4.3", "DSA/EMSA1(SHA-160)");
   OIDS::add_oidstr("2.16.840.1.101.3.4.3.1", "DSA/EMSA1(SHA-224)");
   OIDS::add_oidstr("2.16.840.1.101.3.4.3.2", "DSA/EMSA1(SHA-256)");

   OIDS::add_oidstr("0.4.0.127.0.7.1.1.4.1.1", "ECDSA/EMSA1_BSI(SHA-160)");
   OIDS::add_oidstr("0.4.0.127.0.7.1.1.4.1.2", "ECDSA/EMSA1_BSI(SHA-224)");
   OIDS::add_oidstr("0.4.0.127.0.7.1.1.4.1.3", "ECDSA/EMSA1_BSI(SHA-256)");
   OIDS::add_oidstr("0.4.0.127.0.7.1.1.4.1.4", "ECDSA/EMSA1_BSI(SHA-384)");
   OIDS::add_oidstr("0.4.0.127.0.7.1.1.4.1.5", "ECDSA/EMSA1_BSI(SHA-512)");
   OIDS::add_oidstr("0.4.0.127.0.7.1.1.4.1.6", "ECDSA/EMSA1_BSI(RIPEMD-160)");

   OIDS::add_oidstr("1.2.840.10045.4.1", "ECDSA/EMSA1(SHA-160)");
   OIDS::add_oidstr("1.2.840.10045.4.3.1", "ECDSA/EMSA1(SHA-224)");
   OIDS::add_oidstr("1.2.840.10045.4.3.2", "ECDSA/EMSA1(SHA-256)");
   OIDS::add_oidstr("1.2.840.10045.4.3.3", "ECDSA/EMSA1(SHA-384)");
   OIDS::add_oidstr("1.2.840.10045.4.3.4", "ECDSA/EMSA1(SHA-512)");

   OIDS::add_oidstr("1.2.643.2.2.3", "GOST-34.10/EMSA1(GOST-R-34.11-94)");

   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.1.1", "RW/EMSA2(RIPEMD-160)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.1.2", "RW/EMSA2(SHA-160)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.1.3", "RW/EMSA2(SHA-224)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.1.4", "RW/EMSA2(SHA-256)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.1.5", "RW/EMSA2(SHA-384)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.1.6", "RW/EMSA2(SHA-512)");

   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.2.1", "RW/EMSA4(RIPEMD-160)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.2.2", "RW/EMSA4(SHA-160)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.2.3", "RW/EMSA4(SHA-224)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.2.4", "RW/EMSA4(SHA-256)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.2.5", "RW/EMSA4(SHA-384)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.1.2.6", "RW/EMSA4(SHA-512)");

   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.2.1.1", "NR/EMSA2(RIPEMD-160)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.2.1.2", "NR/EMSA2(SHA-160)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.2.1.3", "NR/EMSA2(SHA-224)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.2.1.4", "NR/EMSA2(SHA-256)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.2.1.5", "NR/EMSA2(SHA-384)");
   OIDS::add_oidstr("1.3.6.1.4.1.25258.2.2.1.6", "NR/EMSA2(SHA-512)");

   OIDS::add_oidstr("2.5.4.3",  "X520.CommonName");
   OIDS::add_oidstr("2.5.4.4",  "X520.Surname");
   OIDS::add_oidstr("2.5.4.5",  "X520.SerialNumber");
   OIDS::add_oidstr("2.5.4.6",  "X520.Country");
   OIDS::add_oidstr("2.5.4.7",  "X520.Locality");
   OIDS::add_oidstr("2.5.4.8",  "X520.State");
   OIDS::add_oidstr("2.5.4.10", "X520.Organization");
   OIDS::add_oidstr("2.5.4.11", "X520.OrganizationalUnit");
   OIDS::add_oidstr("2.5.4.12", "X520.Title");
   OIDS::add_oidstr("2.5.4.42", "X520.GivenName");
   OIDS::add_oidstr("2.5.4.43", "X520.Initials");
   OIDS::add_oidstr("2.5.4.44", "X520.GenerationalQualifier");
   OIDS::add_oidstr("2.5.4.46", "X520.DNQualifier");
   OIDS::add_oidstr("2.5.4.65", "X520.Pseudonym");

   OIDS::add_oidstr("1.2.840.113549.1.5.12", "PKCS5.PBKDF2");
   OIDS::add_oidstr("1.2.840.113549.1.5.13", "PBE-PKCS5v20");

   OIDS::add_oidstr("1.2.840.113549.1.9.1", "PKCS9.EmailAddress");
   OIDS::add_oidstr("1.2.840.113549.1.9.2", "PKCS9.UnstructuredName");
   OIDS::add_oidstr("1.2.840.113549.1.9.3", "PKCS9.ContentType");
   OIDS::add_oidstr("1.2.840.113549.1.9.4", "PKCS9.MessageDigest");
   OIDS::add_oidstr("1.2.840.113549.1.9.7", "PKCS9.ChallengePassword");
   OIDS::add_oidstr("1.2.840.113549.1.9.14", "PKCS9.ExtensionRequest");

   OIDS::add_oidstr("1.2.840.113549.1.7.1",      "CMS.DataContent");
   OIDS::add_oidstr("1.2.840.113549.1.7.2",      "CMS.SignedData");
   OIDS::add_oidstr("1.2.840.113549.1.7.3",      "CMS.EnvelopedData");
   OIDS::add_oidstr("1.2.840.113549.1.7.5",      "CMS.DigestedData");
   OIDS::add_oidstr("1.2.840.113549.1.7.6",      "CMS.EncryptedData");
   OIDS::add_oidstr("1.2.840.113549.1.9.16.1.2", "CMS.AuthenticatedData");
   OIDS::add_oidstr("1.2.840.113549.1.9.16.1.9", "CMS.CompressedData");

   OIDS::add_oidstr("2.5.29.14", "X509v3.SubjectKeyIdentifier");
   OIDS::add_oidstr("2.5.29.15", "X509v3.KeyUsage");
   OIDS::add_oidstr("2.5.29.17", "X509v3.SubjectAlternativeName");
   OIDS::add_oidstr("2.5.29.18", "X509v3.IssuerAlternativeName");
   OIDS::add_oidstr("2.5.29.19", "X509v3.BasicConstraints");
   OIDS::add_oidstr("2.5.29.20", "X509v3.CRLNumber");
   OIDS::add_oidstr("2.5.29.21", "X509v3.ReasonCode");
   OIDS::add_oidstr("2.5.29.23", "X509v3.HoldInstructionCode");
   OIDS::add_oidstr("2.5.29.24", "X509v3.InvalidityDate");
   OIDS::add_oidstr("2.5.29.31", "X509v3.CRLDistributionPoints");
   OIDS::add_oidstr("2.5.29.32", "X509v3.CertificatePolicies");
   OIDS::add_oidstr("2.5.29.35", "X509v3.AuthorityKeyIdentifier");
   OIDS::add_oidstr("2.5.29.36", "X509v3.PolicyConstraints");
   OIDS::add_oidstr("2.5.29.37", "X509v3.ExtendedKeyUsage");
   OIDS::add_oidstr("1.3.6.1.5.5.7.1.1", "PKIX.AuthorityInformationAccess");

   OIDS::add_oidstr("2.5.29.32.0", "X509v3.AnyPolicy");

   OIDS::add_oidstr("1.3.6.1.5.5.7.3.1", "PKIX.ServerAuth");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.2", "PKIX.ClientAuth");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.3", "PKIX.CodeSigning");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.4", "PKIX.EmailProtection");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.5", "PKIX.IPsecEndSystem");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.6", "PKIX.IPsecTunnel");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.7", "PKIX.IPsecUser");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.8", "PKIX.TimeStamping");
   OIDS::add_oidstr("1.3.6.1.5.5.7.3.9", "PKIX.OCSPSigning");

   OIDS::add_oidstr("1.3.6.1.5.5.7.8.5", "PKIX.XMPPAddr");

   OIDS::add_oidstr("1.3.6.1.5.5.7.48.1", "PKIX.OCSP");
   OIDS::add_oidstr("1.3.6.1.5.5.7.48.1.1", "PKIX.OCSP.BasicResponse");

   /* ECC domain parameters */
   OIDS::add_oidstr("1.3.132.0.6",  "secp112r1");
   OIDS::add_oidstr("1.3.132.0.7",  "secp112r2");
   OIDS::add_oidstr("1.3.132.0.8",  "secp160r1");
   OIDS::add_oidstr("1.3.132.0.9",  "secp160k1");
   OIDS::add_oidstr("1.3.132.0.10", "secp256k1");
   OIDS::add_oidstr("1.3.132.0.28", "secp128r1");
   OIDS::add_oidstr("1.3.132.0.29", "secp128r2");
   OIDS::add_oidstr("1.3.132.0.30", "secp160r2");
   OIDS::add_oidstr("1.3.132.0.31", "secp192k1");
   OIDS::add_oidstr("1.3.132.0.32", "secp224k1");
   OIDS::add_oidstr("1.3.132.0.33", "secp224r1");
   OIDS::add_oidstr("1.3.132.0.34", "secp384r1");
   OIDS::add_oidstr("1.3.132.0.35", "secp521r1");

   OIDS::add_oidstr("1.2.840.10045.3.1.1", "secp192r1");
   OIDS::add_oidstr("1.2.840.10045.3.1.2", "x962_p192v2");
   OIDS::add_oidstr("1.2.840.10045.3.1.3", "x962_p192v3");
   OIDS::add_oidstr("1.2.840.10045.3.1.4", "x962_p239v1");
   OIDS::add_oidstr("1.2.840.10045.3.1.5", "x962_p239v2");
   OIDS::add_oidstr("1.2.840.10045.3.1.6", "x962_p239v3");
   OIDS::add_oidstr("1.2.840.10045.3.1.7", "secp256r1");

   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.1",  "brainpool160r1");
   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.3",  "brainpool192r1");
   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.5",  "brainpool224r1");
   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.7",  "brainpool256r1");
   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.9",  "brainpool320r1");
   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.11", "brainpool384r1");
   OIDS::add_oidstr("1.3.36.3.3.2.8.1.1.13", "brainpool512r1");

   OIDS::add_oidstr("1.2.643.2.2.35.1", "gost_256A");
   OIDS::add_oidstr("1.2.643.2.2.36.0", "gost_256A");

   /* CVC */
   OIDS::add_oidstr("0.4.0.127.0.7.3.1.2.1", "CertificateHolderAuthorizationTemplate");
   }

}

}
/*
* OID Registry
* (C) 1999-2008,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace OIDS {

namespace {

class OID_Map
   {
   public:
      void add_oid(const OID& oid, const std::string& str)
         {
         add_str2oid(oid, str);
         add_oid2str(oid, str);
         }

      void add_str2oid(const OID& oid, const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         auto i = m_str2oid.find(str);
         if(i == m_str2oid.end())
            m_str2oid.insert(std::make_pair(str, oid));
         }

      void add_oid2str(const OID& oid, const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         auto i = m_oid2str.find(oid);
         if(i == m_oid2str.end())
            m_oid2str.insert(std::make_pair(oid, str));
         }

      std::string lookup(const OID& oid)
         {
         std::lock_guard<std::mutex> lock(m_mutex);

         auto i = m_oid2str.find(oid);
         if(i != m_oid2str.end())
            return i->second;

         return "";
         }

      OID lookup(const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);

         auto i = m_str2oid.find(str);
         if(i != m_str2oid.end())
            return i->second;

         // Try to parse as plain OID
         try
            {
            return OID(str);
            }
         catch(...) {}

         throw Lookup_Error("No object identifier found for " + str);
         }

      bool have_oid(const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_str2oid.find(str) != m_str2oid.end();
         }

   private:
      std::mutex m_mutex;
      std::map<std::string, OID> m_str2oid;
      std::map<OID, std::string> m_oid2str;
   };

OID_Map& global_oid_map()
   {
   static OID_Map map;
   return map;
   }

}

void add_oid(const OID& oid, const std::string& name)
   {
   global_oid_map().add_oid(oid, name);
   }

void add_oidstr(const char* oidstr, const char* name)
   {
   add_oid(OID(oidstr), name);
   }

void add_oid2str(const OID& oid, const std::string& name)
   {
   global_oid_map().add_oid2str(oid, name);
   }

void add_str2oid(const OID& oid, const std::string& name)
   {
   global_oid_map().add_oid2str(oid, name);
   }

std::string lookup(const OID& oid)
   {
   return global_oid_map().lookup(oid);
   }

OID lookup(const std::string& name)
   {
   return global_oid_map().lookup(name);
   }

bool have_oid(const std::string& name)
   {
   return global_oid_map().have_oid(name);
   }

bool name_of(const OID& oid, const std::string& name)
   {
   return (oid == lookup(name));
   }

}

}
/*
* X509_DN
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <ostream>

namespace Botan {

/*
* Create an empty X509_DN
*/
X509_DN::X509_DN()
   {
   }

/*
* Create an X509_DN
*/
X509_DN::X509_DN(const std::multimap<OID, std::string>& args)
   {
   for(auto i = args.begin(); i != args.end(); ++i)
      add_attribute(i->first, i->second);
   }

/*
* Create an X509_DN
*/
X509_DN::X509_DN(const std::multimap<std::string, std::string>& args)
   {
   for(auto i = args.begin(); i != args.end(); ++i)
      add_attribute(OIDS::lookup(i->first), i->second);
   }

/*
* Add an attribute to a X509_DN
*/
void X509_DN::add_attribute(const std::string& type,
                            const std::string& str)
   {
   OID oid = OIDS::lookup(type);
   add_attribute(oid, str);
   }

/*
* Add an attribute to a X509_DN
*/
void X509_DN::add_attribute(const OID& oid, const std::string& str)
   {
   if(str == "")
      return;

   auto range = dn_info.equal_range(oid);
   for(auto i = range.first; i != range.second; ++i)
      if(i->second.value() == str)
         return;

   multimap_insert(dn_info, oid, ASN1_String(str));
   dn_bits.clear();
   }

/*
* Get the attributes of this X509_DN
*/
std::multimap<OID, std::string> X509_DN::get_attributes() const
   {
   std::multimap<OID, std::string> retval;
   for(auto i = dn_info.begin(); i != dn_info.end(); ++i)
      multimap_insert(retval, i->first, i->second.value());
   return retval;
   }

/*
* Get the contents of this X.500 Name
*/
std::multimap<std::string, std::string> X509_DN::contents() const
   {
   std::multimap<std::string, std::string> retval;
   for(auto i = dn_info.begin(); i != dn_info.end(); ++i)
      multimap_insert(retval, OIDS::lookup(i->first), i->second.value());
   return retval;
   }

/*
* Get a single attribute type
*/
std::vector<std::string> X509_DN::get_attribute(const std::string& attr) const
   {
   const OID oid = OIDS::lookup(deref_info_field(attr));

   auto range = dn_info.equal_range(oid);

   std::vector<std::string> values;
   for(auto i = range.first; i != range.second; ++i)
      values.push_back(i->second.value());
   return values;
   }

/*
* Return the BER encoded data, if any
*/
std::vector<byte> X509_DN::get_bits() const
   {
   return dn_bits;
   }

/*
* Deref aliases in a subject/issuer info request
*/
std::string X509_DN::deref_info_field(const std::string& info)
   {
   if(info == "Name" || info == "CommonName") return "X520.CommonName";
   if(info == "SerialNumber")                 return "X520.SerialNumber";
   if(info == "Country")                      return "X520.Country";
   if(info == "Organization")                 return "X520.Organization";
   if(info == "Organizational Unit" || info == "OrgUnit")
      return "X520.OrganizationalUnit";
   if(info == "Locality")                     return "X520.Locality";
   if(info == "State" || info == "Province")  return "X520.State";
   if(info == "Email")                        return "RFC822";
   return info;
   }

/*
* Compare two X509_DNs for equality
*/
bool operator==(const X509_DN& dn1, const X509_DN& dn2)
   {
   auto attr1 = dn1.get_attributes();
   auto attr2 = dn2.get_attributes();

   if(attr1.size() != attr2.size()) return false;

   auto p1 = attr1.begin();
   auto p2 = attr2.begin();

   while(true)
      {
      if(p1 == attr1.end() && p2 == attr2.end())
         break;
      if(p1 == attr1.end())      return false;
      if(p2 == attr2.end())      return false;
      if(p1->first != p2->first) return false;
      if(!x500_name_cmp(p1->second, p2->second))
         return false;
      ++p1;
      ++p2;
      }
   return true;
   }

/*
* Compare two X509_DNs for inequality
*/
bool operator!=(const X509_DN& dn1, const X509_DN& dn2)
   {
   return !(dn1 == dn2);
   }

/*
* Induce an arbitrary ordering on DNs
*/
bool operator<(const X509_DN& dn1, const X509_DN& dn2)
   {
   auto attr1 = dn1.get_attributes();
   auto attr2 = dn2.get_attributes();

   if(attr1.size() < attr2.size()) return true;
   if(attr1.size() > attr2.size()) return false;

   for(auto p1 = attr1.begin(); p1 != attr1.end(); ++p1)
      {
      auto p2 = attr2.find(p1->first);
      if(p2 == attr2.end())       return false;
      if(p1->second > p2->second) return false;
      if(p1->second < p2->second) return true;
      }
   return false;
   }

namespace {

/*
* DER encode a RelativeDistinguishedName
*/
void do_ava(DER_Encoder& encoder,
            const std::multimap<OID, std::string>& dn_info,
            ASN1_Tag string_type, const std::string& oid_str,
            bool must_exist = false)
   {
   const OID oid = OIDS::lookup(oid_str);
   const bool exists = (dn_info.find(oid) != dn_info.end());

   if(!exists && must_exist)
      throw Encoding_Error("X509_DN: No entry for " + oid_str);
   if(!exists) return;

   auto range = dn_info.equal_range(oid);

   for(auto i = range.first; i != range.second; ++i)
      {
      encoder.start_cons(SET)
         .start_cons(SEQUENCE)
            .encode(oid)
            .encode(ASN1_String(i->second, string_type))
         .end_cons()
      .end_cons();
      }
   }

}

/*
* DER encode a DistinguishedName
*/
void X509_DN::encode_into(DER_Encoder& der) const
   {
   auto dn_info = get_attributes();

   der.start_cons(SEQUENCE);

   if(!dn_bits.empty())
      der.raw_bytes(dn_bits);
   else
      {
      do_ava(der, dn_info, PRINTABLE_STRING, "X520.Country");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.State");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.Locality");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.Organization");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.OrganizationalUnit");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.CommonName");
      do_ava(der, dn_info, PRINTABLE_STRING, "X520.SerialNumber");
      }

   der.end_cons();
   }

/*
* Decode a BER encoded DistinguishedName
*/
void X509_DN::decode_from(BER_Decoder& source)
   {
   std::vector<byte> bits;

   source.start_cons(SEQUENCE)
      .raw_bytes(bits)
   .end_cons();

   BER_Decoder sequence(bits);

   while(sequence.more_items())
      {
      BER_Decoder rdn = sequence.start_cons(SET);

      while(rdn.more_items())
         {
         OID oid;
         ASN1_String str;

         rdn.start_cons(SEQUENCE)
            .decode(oid)
            .decode(str)
            .verify_end()
        .end_cons();

         add_attribute(oid, str.value());
         }
      }

   dn_bits = bits;
   }

namespace {

std::string to_short_form(const std::string& long_id)
   {
   if(long_id == "X520.CommonName")
      return "CN";

   if(long_id == "X520.Organization")
      return "O";

   if(long_id == "X520.OrganizationalUnit")
      return "OU";

   return long_id;
   }

}

std::ostream& operator<<(std::ostream& out, const X509_DN& dn)
   {
   std::multimap<std::string, std::string> contents = dn.contents();

   for(std::multimap<std::string, std::string>::const_iterator i = contents.begin();
       i != contents.end(); ++i)
      {
      out << to_short_form(i->first) << "=" << i->second << ' ';
      }
   return out;
   }

}
/*
* AES
* (C) 1999-2010 Jack Lloyd
*
* Based on the public domain reference implemenation
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

const byte SE[256] = {
   0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
   0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
   0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
   0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
   0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
   0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
   0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
   0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
   0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
   0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
   0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
   0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
   0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
   0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
   0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
   0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
   0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
   0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
   0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
   0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
   0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
   0xB0, 0x54, 0xBB, 0x16 };

const byte SD[256] = {
   0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E,
   0x81, 0xF3, 0xD7, 0xFB, 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,
   0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB, 0x54, 0x7B, 0x94, 0x32,
   0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
   0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49,
   0x6D, 0x8B, 0xD1, 0x25, 0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,
   0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92, 0x6C, 0x70, 0x48, 0x50,
   0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
   0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05,
   0xB8, 0xB3, 0x45, 0x06, 0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
   0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B, 0x3A, 0x91, 0x11, 0x41,
   0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
   0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8,
   0x1C, 0x75, 0xDF, 0x6E, 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
   0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B, 0xFC, 0x56, 0x3E, 0x4B,
   0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
   0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59,
   0x27, 0x80, 0xEC, 0x5F, 0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,
   0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF, 0xA0, 0xE0, 0x3B, 0x4D,
   0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
   0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63,
   0x55, 0x21, 0x0C, 0x7D };

const u32bit TE[1024] = {
   0xC66363A5, 0xF87C7C84, 0xEE777799, 0xF67B7B8D, 0xFFF2F20D, 0xD66B6BBD,
   0xDE6F6FB1, 0x91C5C554, 0x60303050, 0x02010103, 0xCE6767A9, 0x562B2B7D,
   0xE7FEFE19, 0xB5D7D762, 0x4DABABE6, 0xEC76769A, 0x8FCACA45, 0x1F82829D,
   0x89C9C940, 0xFA7D7D87, 0xEFFAFA15, 0xB25959EB, 0x8E4747C9, 0xFBF0F00B,
   0x41ADADEC, 0xB3D4D467, 0x5FA2A2FD, 0x45AFAFEA, 0x239C9CBF, 0x53A4A4F7,
   0xE4727296, 0x9BC0C05B, 0x75B7B7C2, 0xE1FDFD1C, 0x3D9393AE, 0x4C26266A,
   0x6C36365A, 0x7E3F3F41, 0xF5F7F702, 0x83CCCC4F, 0x6834345C, 0x51A5A5F4,
   0xD1E5E534, 0xF9F1F108, 0xE2717193, 0xABD8D873, 0x62313153, 0x2A15153F,
   0x0804040C, 0x95C7C752, 0x46232365, 0x9DC3C35E, 0x30181828, 0x379696A1,
   0x0A05050F, 0x2F9A9AB5, 0x0E070709, 0x24121236, 0x1B80809B, 0xDFE2E23D,
   0xCDEBEB26, 0x4E272769, 0x7FB2B2CD, 0xEA75759F, 0x1209091B, 0x1D83839E,
   0x582C2C74, 0x341A1A2E, 0x361B1B2D, 0xDC6E6EB2, 0xB45A5AEE, 0x5BA0A0FB,
   0xA45252F6, 0x763B3B4D, 0xB7D6D661, 0x7DB3B3CE, 0x5229297B, 0xDDE3E33E,
   0x5E2F2F71, 0x13848497, 0xA65353F5, 0xB9D1D168, 0x00000000, 0xC1EDED2C,
   0x40202060, 0xE3FCFC1F, 0x79B1B1C8, 0xB65B5BED, 0xD46A6ABE, 0x8DCBCB46,
   0x67BEBED9, 0x7239394B, 0x944A4ADE, 0x984C4CD4, 0xB05858E8, 0x85CFCF4A,
   0xBBD0D06B, 0xC5EFEF2A, 0x4FAAAAE5, 0xEDFBFB16, 0x864343C5, 0x9A4D4DD7,
   0x66333355, 0x11858594, 0x8A4545CF, 0xE9F9F910, 0x04020206, 0xFE7F7F81,
   0xA05050F0, 0x783C3C44, 0x259F9FBA, 0x4BA8A8E3, 0xA25151F3, 0x5DA3A3FE,
   0x804040C0, 0x058F8F8A, 0x3F9292AD, 0x219D9DBC, 0x70383848, 0xF1F5F504,
   0x63BCBCDF, 0x77B6B6C1, 0xAFDADA75, 0x42212163, 0x20101030, 0xE5FFFF1A,
   0xFDF3F30E, 0xBFD2D26D, 0x81CDCD4C, 0x180C0C14, 0x26131335, 0xC3ECEC2F,
   0xBE5F5FE1, 0x359797A2, 0x884444CC, 0x2E171739, 0x93C4C457, 0x55A7A7F2,
   0xFC7E7E82, 0x7A3D3D47, 0xC86464AC, 0xBA5D5DE7, 0x3219192B, 0xE6737395,
   0xC06060A0, 0x19818198, 0x9E4F4FD1, 0xA3DCDC7F, 0x44222266, 0x542A2A7E,
   0x3B9090AB, 0x0B888883, 0x8C4646CA, 0xC7EEEE29, 0x6BB8B8D3, 0x2814143C,
   0xA7DEDE79, 0xBC5E5EE2, 0x160B0B1D, 0xADDBDB76, 0xDBE0E03B, 0x64323256,
   0x743A3A4E, 0x140A0A1E, 0x924949DB, 0x0C06060A, 0x4824246C, 0xB85C5CE4,
   0x9FC2C25D, 0xBDD3D36E, 0x43ACACEF, 0xC46262A6, 0x399191A8, 0x319595A4,
   0xD3E4E437, 0xF279798B, 0xD5E7E732, 0x8BC8C843, 0x6E373759, 0xDA6D6DB7,
   0x018D8D8C, 0xB1D5D564, 0x9C4E4ED2, 0x49A9A9E0, 0xD86C6CB4, 0xAC5656FA,
   0xF3F4F407, 0xCFEAEA25, 0xCA6565AF, 0xF47A7A8E, 0x47AEAEE9, 0x10080818,
   0x6FBABAD5, 0xF0787888, 0x4A25256F, 0x5C2E2E72, 0x381C1C24, 0x57A6A6F1,
   0x73B4B4C7, 0x97C6C651, 0xCBE8E823, 0xA1DDDD7C, 0xE874749C, 0x3E1F1F21,
   0x964B4BDD, 0x61BDBDDC, 0x0D8B8B86, 0x0F8A8A85, 0xE0707090, 0x7C3E3E42,
   0x71B5B5C4, 0xCC6666AA, 0x904848D8, 0x06030305, 0xF7F6F601, 0x1C0E0E12,
   0xC26161A3, 0x6A35355F, 0xAE5757F9, 0x69B9B9D0, 0x17868691, 0x99C1C158,
   0x3A1D1D27, 0x279E9EB9, 0xD9E1E138, 0xEBF8F813, 0x2B9898B3, 0x22111133,
   0xD26969BB, 0xA9D9D970, 0x078E8E89, 0x339494A7, 0x2D9B9BB6, 0x3C1E1E22,
   0x15878792, 0xC9E9E920, 0x87CECE49, 0xAA5555FF, 0x50282878, 0xA5DFDF7A,
   0x038C8C8F, 0x59A1A1F8, 0x09898980, 0x1A0D0D17, 0x65BFBFDA, 0xD7E6E631,
   0x844242C6, 0xD06868B8, 0x824141C3, 0x299999B0, 0x5A2D2D77, 0x1E0F0F11,
   0x7BB0B0CB, 0xA85454FC, 0x6DBBBBD6, 0x2C16163A, 0xA5C66363, 0x84F87C7C,
   0x99EE7777, 0x8DF67B7B, 0x0DFFF2F2, 0xBDD66B6B, 0xB1DE6F6F, 0x5491C5C5,
   0x50603030, 0x03020101, 0xA9CE6767, 0x7D562B2B, 0x19E7FEFE, 0x62B5D7D7,
   0xE64DABAB, 0x9AEC7676, 0x458FCACA, 0x9D1F8282, 0x4089C9C9, 0x87FA7D7D,
   0x15EFFAFA, 0xEBB25959, 0xC98E4747, 0x0BFBF0F0, 0xEC41ADAD, 0x67B3D4D4,
   0xFD5FA2A2, 0xEA45AFAF, 0xBF239C9C, 0xF753A4A4, 0x96E47272, 0x5B9BC0C0,
   0xC275B7B7, 0x1CE1FDFD, 0xAE3D9393, 0x6A4C2626, 0x5A6C3636, 0x417E3F3F,
   0x02F5F7F7, 0x4F83CCCC, 0x5C683434, 0xF451A5A5, 0x34D1E5E5, 0x08F9F1F1,
   0x93E27171, 0x73ABD8D8, 0x53623131, 0x3F2A1515, 0x0C080404, 0x5295C7C7,
   0x65462323, 0x5E9DC3C3, 0x28301818, 0xA1379696, 0x0F0A0505, 0xB52F9A9A,
   0x090E0707, 0x36241212, 0x9B1B8080, 0x3DDFE2E2, 0x26CDEBEB, 0x694E2727,
   0xCD7FB2B2, 0x9FEA7575, 0x1B120909, 0x9E1D8383, 0x74582C2C, 0x2E341A1A,
   0x2D361B1B, 0xB2DC6E6E, 0xEEB45A5A, 0xFB5BA0A0, 0xF6A45252, 0x4D763B3B,
   0x61B7D6D6, 0xCE7DB3B3, 0x7B522929, 0x3EDDE3E3, 0x715E2F2F, 0x97138484,
   0xF5A65353, 0x68B9D1D1, 0x00000000, 0x2CC1EDED, 0x60402020, 0x1FE3FCFC,
   0xC879B1B1, 0xEDB65B5B, 0xBED46A6A, 0x468DCBCB, 0xD967BEBE, 0x4B723939,
   0xDE944A4A, 0xD4984C4C, 0xE8B05858, 0x4A85CFCF, 0x6BBBD0D0, 0x2AC5EFEF,
   0xE54FAAAA, 0x16EDFBFB, 0xC5864343, 0xD79A4D4D, 0x55663333, 0x94118585,
   0xCF8A4545, 0x10E9F9F9, 0x06040202, 0x81FE7F7F, 0xF0A05050, 0x44783C3C,
   0xBA259F9F, 0xE34BA8A8, 0xF3A25151, 0xFE5DA3A3, 0xC0804040, 0x8A058F8F,
   0xAD3F9292, 0xBC219D9D, 0x48703838, 0x04F1F5F5, 0xDF63BCBC, 0xC177B6B6,
   0x75AFDADA, 0x63422121, 0x30201010, 0x1AE5FFFF, 0x0EFDF3F3, 0x6DBFD2D2,
   0x4C81CDCD, 0x14180C0C, 0x35261313, 0x2FC3ECEC, 0xE1BE5F5F, 0xA2359797,
   0xCC884444, 0x392E1717, 0x5793C4C4, 0xF255A7A7, 0x82FC7E7E, 0x477A3D3D,
   0xACC86464, 0xE7BA5D5D, 0x2B321919, 0x95E67373, 0xA0C06060, 0x98198181,
   0xD19E4F4F, 0x7FA3DCDC, 0x66442222, 0x7E542A2A, 0xAB3B9090, 0x830B8888,
   0xCA8C4646, 0x29C7EEEE, 0xD36BB8B8, 0x3C281414, 0x79A7DEDE, 0xE2BC5E5E,
   0x1D160B0B, 0x76ADDBDB, 0x3BDBE0E0, 0x56643232, 0x4E743A3A, 0x1E140A0A,
   0xDB924949, 0x0A0C0606, 0x6C482424, 0xE4B85C5C, 0x5D9FC2C2, 0x6EBDD3D3,
   0xEF43ACAC, 0xA6C46262, 0xA8399191, 0xA4319595, 0x37D3E4E4, 0x8BF27979,
   0x32D5E7E7, 0x438BC8C8, 0x596E3737, 0xB7DA6D6D, 0x8C018D8D, 0x64B1D5D5,
   0xD29C4E4E, 0xE049A9A9, 0xB4D86C6C, 0xFAAC5656, 0x07F3F4F4, 0x25CFEAEA,
   0xAFCA6565, 0x8EF47A7A, 0xE947AEAE, 0x18100808, 0xD56FBABA, 0x88F07878,
   0x6F4A2525, 0x725C2E2E, 0x24381C1C, 0xF157A6A6, 0xC773B4B4, 0x5197C6C6,
   0x23CBE8E8, 0x7CA1DDDD, 0x9CE87474, 0x213E1F1F, 0xDD964B4B, 0xDC61BDBD,
   0x860D8B8B, 0x850F8A8A, 0x90E07070, 0x427C3E3E, 0xC471B5B5, 0xAACC6666,
   0xD8904848, 0x05060303, 0x01F7F6F6, 0x121C0E0E, 0xA3C26161, 0x5F6A3535,
   0xF9AE5757, 0xD069B9B9, 0x91178686, 0x5899C1C1, 0x273A1D1D, 0xB9279E9E,
   0x38D9E1E1, 0x13EBF8F8, 0xB32B9898, 0x33221111, 0xBBD26969, 0x70A9D9D9,
   0x89078E8E, 0xA7339494, 0xB62D9B9B, 0x223C1E1E, 0x92158787, 0x20C9E9E9,
   0x4987CECE, 0xFFAA5555, 0x78502828, 0x7AA5DFDF, 0x8F038C8C, 0xF859A1A1,
   0x80098989, 0x171A0D0D, 0xDA65BFBF, 0x31D7E6E6, 0xC6844242, 0xB8D06868,
   0xC3824141, 0xB0299999, 0x775A2D2D, 0x111E0F0F, 0xCB7BB0B0, 0xFCA85454,
   0xD66DBBBB, 0x3A2C1616, 0x63A5C663, 0x7C84F87C, 0x7799EE77, 0x7B8DF67B,
   0xF20DFFF2, 0x6BBDD66B, 0x6FB1DE6F, 0xC55491C5, 0x30506030, 0x01030201,
   0x67A9CE67, 0x2B7D562B, 0xFE19E7FE, 0xD762B5D7, 0xABE64DAB, 0x769AEC76,
   0xCA458FCA, 0x829D1F82, 0xC94089C9, 0x7D87FA7D, 0xFA15EFFA, 0x59EBB259,
   0x47C98E47, 0xF00BFBF0, 0xADEC41AD, 0xD467B3D4, 0xA2FD5FA2, 0xAFEA45AF,
   0x9CBF239C, 0xA4F753A4, 0x7296E472, 0xC05B9BC0, 0xB7C275B7, 0xFD1CE1FD,
   0x93AE3D93, 0x266A4C26, 0x365A6C36, 0x3F417E3F, 0xF702F5F7, 0xCC4F83CC,
   0x345C6834, 0xA5F451A5, 0xE534D1E5, 0xF108F9F1, 0x7193E271, 0xD873ABD8,
   0x31536231, 0x153F2A15, 0x040C0804, 0xC75295C7, 0x23654623, 0xC35E9DC3,
   0x18283018, 0x96A13796, 0x050F0A05, 0x9AB52F9A, 0x07090E07, 0x12362412,
   0x809B1B80, 0xE23DDFE2, 0xEB26CDEB, 0x27694E27, 0xB2CD7FB2, 0x759FEA75,
   0x091B1209, 0x839E1D83, 0x2C74582C, 0x1A2E341A, 0x1B2D361B, 0x6EB2DC6E,
   0x5AEEB45A, 0xA0FB5BA0, 0x52F6A452, 0x3B4D763B, 0xD661B7D6, 0xB3CE7DB3,
   0x297B5229, 0xE33EDDE3, 0x2F715E2F, 0x84971384, 0x53F5A653, 0xD168B9D1,
   0x00000000, 0xED2CC1ED, 0x20604020, 0xFC1FE3FC, 0xB1C879B1, 0x5BEDB65B,
   0x6ABED46A, 0xCB468DCB, 0xBED967BE, 0x394B7239, 0x4ADE944A, 0x4CD4984C,
   0x58E8B058, 0xCF4A85CF, 0xD06BBBD0, 0xEF2AC5EF, 0xAAE54FAA, 0xFB16EDFB,
   0x43C58643, 0x4DD79A4D, 0x33556633, 0x85941185, 0x45CF8A45, 0xF910E9F9,
   0x02060402, 0x7F81FE7F, 0x50F0A050, 0x3C44783C, 0x9FBA259F, 0xA8E34BA8,
   0x51F3A251, 0xA3FE5DA3, 0x40C08040, 0x8F8A058F, 0x92AD3F92, 0x9DBC219D,
   0x38487038, 0xF504F1F5, 0xBCDF63BC, 0xB6C177B6, 0xDA75AFDA, 0x21634221,
   0x10302010, 0xFF1AE5FF, 0xF30EFDF3, 0xD26DBFD2, 0xCD4C81CD, 0x0C14180C,
   0x13352613, 0xEC2FC3EC, 0x5FE1BE5F, 0x97A23597, 0x44CC8844, 0x17392E17,
   0xC45793C4, 0xA7F255A7, 0x7E82FC7E, 0x3D477A3D, 0x64ACC864, 0x5DE7BA5D,
   0x192B3219, 0x7395E673, 0x60A0C060, 0x81981981, 0x4FD19E4F, 0xDC7FA3DC,
   0x22664422, 0x2A7E542A, 0x90AB3B90, 0x88830B88, 0x46CA8C46, 0xEE29C7EE,
   0xB8D36BB8, 0x143C2814, 0xDE79A7DE, 0x5EE2BC5E, 0x0B1D160B, 0xDB76ADDB,
   0xE03BDBE0, 0x32566432, 0x3A4E743A, 0x0A1E140A, 0x49DB9249, 0x060A0C06,
   0x246C4824, 0x5CE4B85C, 0xC25D9FC2, 0xD36EBDD3, 0xACEF43AC, 0x62A6C462,
   0x91A83991, 0x95A43195, 0xE437D3E4, 0x798BF279, 0xE732D5E7, 0xC8438BC8,
   0x37596E37, 0x6DB7DA6D, 0x8D8C018D, 0xD564B1D5, 0x4ED29C4E, 0xA9E049A9,
   0x6CB4D86C, 0x56FAAC56, 0xF407F3F4, 0xEA25CFEA, 0x65AFCA65, 0x7A8EF47A,
   0xAEE947AE, 0x08181008, 0xBAD56FBA, 0x7888F078, 0x256F4A25, 0x2E725C2E,
   0x1C24381C, 0xA6F157A6, 0xB4C773B4, 0xC65197C6, 0xE823CBE8, 0xDD7CA1DD,
   0x749CE874, 0x1F213E1F, 0x4BDD964B, 0xBDDC61BD, 0x8B860D8B, 0x8A850F8A,
   0x7090E070, 0x3E427C3E, 0xB5C471B5, 0x66AACC66, 0x48D89048, 0x03050603,
   0xF601F7F6, 0x0E121C0E, 0x61A3C261, 0x355F6A35, 0x57F9AE57, 0xB9D069B9,
   0x86911786, 0xC15899C1, 0x1D273A1D, 0x9EB9279E, 0xE138D9E1, 0xF813EBF8,
   0x98B32B98, 0x11332211, 0x69BBD269, 0xD970A9D9, 0x8E89078E, 0x94A73394,
   0x9BB62D9B, 0x1E223C1E, 0x87921587, 0xE920C9E9, 0xCE4987CE, 0x55FFAA55,
   0x28785028, 0xDF7AA5DF, 0x8C8F038C, 0xA1F859A1, 0x89800989, 0x0D171A0D,
   0xBFDA65BF, 0xE631D7E6, 0x42C68442, 0x68B8D068, 0x41C38241, 0x99B02999,
   0x2D775A2D, 0x0F111E0F, 0xB0CB7BB0, 0x54FCA854, 0xBBD66DBB, 0x163A2C16,
   0x6363A5C6, 0x7C7C84F8, 0x777799EE, 0x7B7B8DF6, 0xF2F20DFF, 0x6B6BBDD6,
   0x6F6FB1DE, 0xC5C55491, 0x30305060, 0x01010302, 0x6767A9CE, 0x2B2B7D56,
   0xFEFE19E7, 0xD7D762B5, 0xABABE64D, 0x76769AEC, 0xCACA458F, 0x82829D1F,
   0xC9C94089, 0x7D7D87FA, 0xFAFA15EF, 0x5959EBB2, 0x4747C98E, 0xF0F00BFB,
   0xADADEC41, 0xD4D467B3, 0xA2A2FD5F, 0xAFAFEA45, 0x9C9CBF23, 0xA4A4F753,
   0x727296E4, 0xC0C05B9B, 0xB7B7C275, 0xFDFD1CE1, 0x9393AE3D, 0x26266A4C,
   0x36365A6C, 0x3F3F417E, 0xF7F702F5, 0xCCCC4F83, 0x34345C68, 0xA5A5F451,
   0xE5E534D1, 0xF1F108F9, 0x717193E2, 0xD8D873AB, 0x31315362, 0x15153F2A,
   0x04040C08, 0xC7C75295, 0x23236546, 0xC3C35E9D, 0x18182830, 0x9696A137,
   0x05050F0A, 0x9A9AB52F, 0x0707090E, 0x12123624, 0x80809B1B, 0xE2E23DDF,
   0xEBEB26CD, 0x2727694E, 0xB2B2CD7F, 0x75759FEA, 0x09091B12, 0x83839E1D,
   0x2C2C7458, 0x1A1A2E34, 0x1B1B2D36, 0x6E6EB2DC, 0x5A5AEEB4, 0xA0A0FB5B,
   0x5252F6A4, 0x3B3B4D76, 0xD6D661B7, 0xB3B3CE7D, 0x29297B52, 0xE3E33EDD,
   0x2F2F715E, 0x84849713, 0x5353F5A6, 0xD1D168B9, 0x00000000, 0xEDED2CC1,
   0x20206040, 0xFCFC1FE3, 0xB1B1C879, 0x5B5BEDB6, 0x6A6ABED4, 0xCBCB468D,
   0xBEBED967, 0x39394B72, 0x4A4ADE94, 0x4C4CD498, 0x5858E8B0, 0xCFCF4A85,
   0xD0D06BBB, 0xEFEF2AC5, 0xAAAAE54F, 0xFBFB16ED, 0x4343C586, 0x4D4DD79A,
   0x33335566, 0x85859411, 0x4545CF8A, 0xF9F910E9, 0x02020604, 0x7F7F81FE,
   0x5050F0A0, 0x3C3C4478, 0x9F9FBA25, 0xA8A8E34B, 0x5151F3A2, 0xA3A3FE5D,
   0x4040C080, 0x8F8F8A05, 0x9292AD3F, 0x9D9DBC21, 0x38384870, 0xF5F504F1,
   0xBCBCDF63, 0xB6B6C177, 0xDADA75AF, 0x21216342, 0x10103020, 0xFFFF1AE5,
   0xF3F30EFD, 0xD2D26DBF, 0xCDCD4C81, 0x0C0C1418, 0x13133526, 0xECEC2FC3,
   0x5F5FE1BE, 0x9797A235, 0x4444CC88, 0x1717392E, 0xC4C45793, 0xA7A7F255,
   0x7E7E82FC, 0x3D3D477A, 0x6464ACC8, 0x5D5DE7BA, 0x19192B32, 0x737395E6,
   0x6060A0C0, 0x81819819, 0x4F4FD19E, 0xDCDC7FA3, 0x22226644, 0x2A2A7E54,
   0x9090AB3B, 0x8888830B, 0x4646CA8C, 0xEEEE29C7, 0xB8B8D36B, 0x14143C28,
   0xDEDE79A7, 0x5E5EE2BC, 0x0B0B1D16, 0xDBDB76AD, 0xE0E03BDB, 0x32325664,
   0x3A3A4E74, 0x0A0A1E14, 0x4949DB92, 0x06060A0C, 0x24246C48, 0x5C5CE4B8,
   0xC2C25D9F, 0xD3D36EBD, 0xACACEF43, 0x6262A6C4, 0x9191A839, 0x9595A431,
   0xE4E437D3, 0x79798BF2, 0xE7E732D5, 0xC8C8438B, 0x3737596E, 0x6D6DB7DA,
   0x8D8D8C01, 0xD5D564B1, 0x4E4ED29C, 0xA9A9E049, 0x6C6CB4D8, 0x5656FAAC,
   0xF4F407F3, 0xEAEA25CF, 0x6565AFCA, 0x7A7A8EF4, 0xAEAEE947, 0x08081810,
   0xBABAD56F, 0x787888F0, 0x25256F4A, 0x2E2E725C, 0x1C1C2438, 0xA6A6F157,
   0xB4B4C773, 0xC6C65197, 0xE8E823CB, 0xDDDD7CA1, 0x74749CE8, 0x1F1F213E,
   0x4B4BDD96, 0xBDBDDC61, 0x8B8B860D, 0x8A8A850F, 0x707090E0, 0x3E3E427C,
   0xB5B5C471, 0x6666AACC, 0x4848D890, 0x03030506, 0xF6F601F7, 0x0E0E121C,
   0x6161A3C2, 0x35355F6A, 0x5757F9AE, 0xB9B9D069, 0x86869117, 0xC1C15899,
   0x1D1D273A, 0x9E9EB927, 0xE1E138D9, 0xF8F813EB, 0x9898B32B, 0x11113322,
   0x6969BBD2, 0xD9D970A9, 0x8E8E8907, 0x9494A733, 0x9B9BB62D, 0x1E1E223C,
   0x87879215, 0xE9E920C9, 0xCECE4987, 0x5555FFAA, 0x28287850, 0xDFDF7AA5,
   0x8C8C8F03, 0xA1A1F859, 0x89898009, 0x0D0D171A, 0xBFBFDA65, 0xE6E631D7,
   0x4242C684, 0x6868B8D0, 0x4141C382, 0x9999B029, 0x2D2D775A, 0x0F0F111E,
   0xB0B0CB7B, 0x5454FCA8, 0xBBBBD66D, 0x16163A2C };

const u32bit TD[1024] = {
   0x51F4A750, 0x7E416553, 0x1A17A4C3, 0x3A275E96, 0x3BAB6BCB, 0x1F9D45F1,
   0xACFA58AB, 0x4BE30393, 0x2030FA55, 0xAD766DF6, 0x88CC7691, 0xF5024C25,
   0x4FE5D7FC, 0xC52ACBD7, 0x26354480, 0xB562A38F, 0xDEB15A49, 0x25BA1B67,
   0x45EA0E98, 0x5DFEC0E1, 0xC32F7502, 0x814CF012, 0x8D4697A3, 0x6BD3F9C6,
   0x038F5FE7, 0x15929C95, 0xBF6D7AEB, 0x955259DA, 0xD4BE832D, 0x587421D3,
   0x49E06929, 0x8EC9C844, 0x75C2896A, 0xF48E7978, 0x99583E6B, 0x27B971DD,
   0xBEE14FB6, 0xF088AD17, 0xC920AC66, 0x7DCE3AB4, 0x63DF4A18, 0xE51A3182,
   0x97513360, 0x62537F45, 0xB16477E0, 0xBB6BAE84, 0xFE81A01C, 0xF9082B94,
   0x70486858, 0x8F45FD19, 0x94DE6C87, 0x527BF8B7, 0xAB73D323, 0x724B02E2,
   0xE31F8F57, 0x6655AB2A, 0xB2EB2807, 0x2FB5C203, 0x86C57B9A, 0xD33708A5,
   0x302887F2, 0x23BFA5B2, 0x02036ABA, 0xED16825C, 0x8ACF1C2B, 0xA779B492,
   0xF307F2F0, 0x4E69E2A1, 0x65DAF4CD, 0x0605BED5, 0xD134621F, 0xC4A6FE8A,
   0x342E539D, 0xA2F355A0, 0x058AE132, 0xA4F6EB75, 0x0B83EC39, 0x4060EFAA,
   0x5E719F06, 0xBD6E1051, 0x3E218AF9, 0x96DD063D, 0xDD3E05AE, 0x4DE6BD46,
   0x91548DB5, 0x71C45D05, 0x0406D46F, 0x605015FF, 0x1998FB24, 0xD6BDE997,
   0x894043CC, 0x67D99E77, 0xB0E842BD, 0x07898B88, 0xE7195B38, 0x79C8EEDB,
   0xA17C0A47, 0x7C420FE9, 0xF8841EC9, 0x00000000, 0x09808683, 0x322BED48,
   0x1E1170AC, 0x6C5A724E, 0xFD0EFFFB, 0x0F853856, 0x3DAED51E, 0x362D3927,
   0x0A0FD964, 0x685CA621, 0x9B5B54D1, 0x24362E3A, 0x0C0A67B1, 0x9357E70F,
   0xB4EE96D2, 0x1B9B919E, 0x80C0C54F, 0x61DC20A2, 0x5A774B69, 0x1C121A16,
   0xE293BA0A, 0xC0A02AE5, 0x3C22E043, 0x121B171D, 0x0E090D0B, 0xF28BC7AD,
   0x2DB6A8B9, 0x141EA9C8, 0x57F11985, 0xAF75074C, 0xEE99DDBB, 0xA37F60FD,
   0xF701269F, 0x5C72F5BC, 0x44663BC5, 0x5BFB7E34, 0x8B432976, 0xCB23C6DC,
   0xB6EDFC68, 0xB8E4F163, 0xD731DCCA, 0x42638510, 0x13972240, 0x84C61120,
   0x854A247D, 0xD2BB3DF8, 0xAEF93211, 0xC729A16D, 0x1D9E2F4B, 0xDCB230F3,
   0x0D8652EC, 0x77C1E3D0, 0x2BB3166C, 0xA970B999, 0x119448FA, 0x47E96422,
   0xA8FC8CC4, 0xA0F03F1A, 0x567D2CD8, 0x223390EF, 0x87494EC7, 0xD938D1C1,
   0x8CCAA2FE, 0x98D40B36, 0xA6F581CF, 0xA57ADE28, 0xDAB78E26, 0x3FADBFA4,
   0x2C3A9DE4, 0x5078920D, 0x6A5FCC9B, 0x547E4662, 0xF68D13C2, 0x90D8B8E8,
   0x2E39F75E, 0x82C3AFF5, 0x9F5D80BE, 0x69D0937C, 0x6FD52DA9, 0xCF2512B3,
   0xC8AC993B, 0x10187DA7, 0xE89C636E, 0xDB3BBB7B, 0xCD267809, 0x6E5918F4,
   0xEC9AB701, 0x834F9AA8, 0xE6956E65, 0xAAFFE67E, 0x21BCCF08, 0xEF15E8E6,
   0xBAE79BD9, 0x4A6F36CE, 0xEA9F09D4, 0x29B07CD6, 0x31A4B2AF, 0x2A3F2331,
   0xC6A59430, 0x35A266C0, 0x744EBC37, 0xFC82CAA6, 0xE090D0B0, 0x33A7D815,
   0xF104984A, 0x41ECDAF7, 0x7FCD500E, 0x1791F62F, 0x764DD68D, 0x43EFB04D,
   0xCCAA4D54, 0xE49604DF, 0x9ED1B5E3, 0x4C6A881B, 0xC12C1FB8, 0x4665517F,
   0x9D5EEA04, 0x018C355D, 0xFA877473, 0xFB0B412E, 0xB3671D5A, 0x92DBD252,
   0xE9105633, 0x6DD64713, 0x9AD7618C, 0x37A10C7A, 0x59F8148E, 0xEB133C89,
   0xCEA927EE, 0xB761C935, 0xE11CE5ED, 0x7A47B13C, 0x9CD2DF59, 0x55F2733F,
   0x1814CE79, 0x73C737BF, 0x53F7CDEA, 0x5FFDAA5B, 0xDF3D6F14, 0x7844DB86,
   0xCAAFF381, 0xB968C43E, 0x3824342C, 0xC2A3405F, 0x161DC372, 0xBCE2250C,
   0x283C498B, 0xFF0D9541, 0x39A80171, 0x080CB3DE, 0xD8B4E49C, 0x6456C190,
   0x7BCB8461, 0xD532B670, 0x486C5C74, 0xD0B85742, 0x5051F4A7, 0x537E4165,
   0xC31A17A4, 0x963A275E, 0xCB3BAB6B, 0xF11F9D45, 0xABACFA58, 0x934BE303,
   0x552030FA, 0xF6AD766D, 0x9188CC76, 0x25F5024C, 0xFC4FE5D7, 0xD7C52ACB,
   0x80263544, 0x8FB562A3, 0x49DEB15A, 0x6725BA1B, 0x9845EA0E, 0xE15DFEC0,
   0x02C32F75, 0x12814CF0, 0xA38D4697, 0xC66BD3F9, 0xE7038F5F, 0x9515929C,
   0xEBBF6D7A, 0xDA955259, 0x2DD4BE83, 0xD3587421, 0x2949E069, 0x448EC9C8,
   0x6A75C289, 0x78F48E79, 0x6B99583E, 0xDD27B971, 0xB6BEE14F, 0x17F088AD,
   0x66C920AC, 0xB47DCE3A, 0x1863DF4A, 0x82E51A31, 0x60975133, 0x4562537F,
   0xE0B16477, 0x84BB6BAE, 0x1CFE81A0, 0x94F9082B, 0x58704868, 0x198F45FD,
   0x8794DE6C, 0xB7527BF8, 0x23AB73D3, 0xE2724B02, 0x57E31F8F, 0x2A6655AB,
   0x07B2EB28, 0x032FB5C2, 0x9A86C57B, 0xA5D33708, 0xF2302887, 0xB223BFA5,
   0xBA02036A, 0x5CED1682, 0x2B8ACF1C, 0x92A779B4, 0xF0F307F2, 0xA14E69E2,
   0xCD65DAF4, 0xD50605BE, 0x1FD13462, 0x8AC4A6FE, 0x9D342E53, 0xA0A2F355,
   0x32058AE1, 0x75A4F6EB, 0x390B83EC, 0xAA4060EF, 0x065E719F, 0x51BD6E10,
   0xF93E218A, 0x3D96DD06, 0xAEDD3E05, 0x464DE6BD, 0xB591548D, 0x0571C45D,
   0x6F0406D4, 0xFF605015, 0x241998FB, 0x97D6BDE9, 0xCC894043, 0x7767D99E,
   0xBDB0E842, 0x8807898B, 0x38E7195B, 0xDB79C8EE, 0x47A17C0A, 0xE97C420F,
   0xC9F8841E, 0x00000000, 0x83098086, 0x48322BED, 0xAC1E1170, 0x4E6C5A72,
   0xFBFD0EFF, 0x560F8538, 0x1E3DAED5, 0x27362D39, 0x640A0FD9, 0x21685CA6,
   0xD19B5B54, 0x3A24362E, 0xB10C0A67, 0x0F9357E7, 0xD2B4EE96, 0x9E1B9B91,
   0x4F80C0C5, 0xA261DC20, 0x695A774B, 0x161C121A, 0x0AE293BA, 0xE5C0A02A,
   0x433C22E0, 0x1D121B17, 0x0B0E090D, 0xADF28BC7, 0xB92DB6A8, 0xC8141EA9,
   0x8557F119, 0x4CAF7507, 0xBBEE99DD, 0xFDA37F60, 0x9FF70126, 0xBC5C72F5,
   0xC544663B, 0x345BFB7E, 0x768B4329, 0xDCCB23C6, 0x68B6EDFC, 0x63B8E4F1,
   0xCAD731DC, 0x10426385, 0x40139722, 0x2084C611, 0x7D854A24, 0xF8D2BB3D,
   0x11AEF932, 0x6DC729A1, 0x4B1D9E2F, 0xF3DCB230, 0xEC0D8652, 0xD077C1E3,
   0x6C2BB316, 0x99A970B9, 0xFA119448, 0x2247E964, 0xC4A8FC8C, 0x1AA0F03F,
   0xD8567D2C, 0xEF223390, 0xC787494E, 0xC1D938D1, 0xFE8CCAA2, 0x3698D40B,
   0xCFA6F581, 0x28A57ADE, 0x26DAB78E, 0xA43FADBF, 0xE42C3A9D, 0x0D507892,
   0x9B6A5FCC, 0x62547E46, 0xC2F68D13, 0xE890D8B8, 0x5E2E39F7, 0xF582C3AF,
   0xBE9F5D80, 0x7C69D093, 0xA96FD52D, 0xB3CF2512, 0x3BC8AC99, 0xA710187D,
   0x6EE89C63, 0x7BDB3BBB, 0x09CD2678, 0xF46E5918, 0x01EC9AB7, 0xA8834F9A,
   0x65E6956E, 0x7EAAFFE6, 0x0821BCCF, 0xE6EF15E8, 0xD9BAE79B, 0xCE4A6F36,
   0xD4EA9F09, 0xD629B07C, 0xAF31A4B2, 0x312A3F23, 0x30C6A594, 0xC035A266,
   0x37744EBC, 0xA6FC82CA, 0xB0E090D0, 0x1533A7D8, 0x4AF10498, 0xF741ECDA,
   0x0E7FCD50, 0x2F1791F6, 0x8D764DD6, 0x4D43EFB0, 0x54CCAA4D, 0xDFE49604,
   0xE39ED1B5, 0x1B4C6A88, 0xB8C12C1F, 0x7F466551, 0x049D5EEA, 0x5D018C35,
   0x73FA8774, 0x2EFB0B41, 0x5AB3671D, 0x5292DBD2, 0x33E91056, 0x136DD647,
   0x8C9AD761, 0x7A37A10C, 0x8E59F814, 0x89EB133C, 0xEECEA927, 0x35B761C9,
   0xEDE11CE5, 0x3C7A47B1, 0x599CD2DF, 0x3F55F273, 0x791814CE, 0xBF73C737,
   0xEA53F7CD, 0x5B5FFDAA, 0x14DF3D6F, 0x867844DB, 0x81CAAFF3, 0x3EB968C4,
   0x2C382434, 0x5FC2A340, 0x72161DC3, 0x0CBCE225, 0x8B283C49, 0x41FF0D95,
   0x7139A801, 0xDE080CB3, 0x9CD8B4E4, 0x906456C1, 0x617BCB84, 0x70D532B6,
   0x74486C5C, 0x42D0B857, 0xA75051F4, 0x65537E41, 0xA4C31A17, 0x5E963A27,
   0x6BCB3BAB, 0x45F11F9D, 0x58ABACFA, 0x03934BE3, 0xFA552030, 0x6DF6AD76,
   0x769188CC, 0x4C25F502, 0xD7FC4FE5, 0xCBD7C52A, 0x44802635, 0xA38FB562,
   0x5A49DEB1, 0x1B6725BA, 0x0E9845EA, 0xC0E15DFE, 0x7502C32F, 0xF012814C,
   0x97A38D46, 0xF9C66BD3, 0x5FE7038F, 0x9C951592, 0x7AEBBF6D, 0x59DA9552,
   0x832DD4BE, 0x21D35874, 0x692949E0, 0xC8448EC9, 0x896A75C2, 0x7978F48E,
   0x3E6B9958, 0x71DD27B9, 0x4FB6BEE1, 0xAD17F088, 0xAC66C920, 0x3AB47DCE,
   0x4A1863DF, 0x3182E51A, 0x33609751, 0x7F456253, 0x77E0B164, 0xAE84BB6B,
   0xA01CFE81, 0x2B94F908, 0x68587048, 0xFD198F45, 0x6C8794DE, 0xF8B7527B,
   0xD323AB73, 0x02E2724B, 0x8F57E31F, 0xAB2A6655, 0x2807B2EB, 0xC2032FB5,
   0x7B9A86C5, 0x08A5D337, 0x87F23028, 0xA5B223BF, 0x6ABA0203, 0x825CED16,
   0x1C2B8ACF, 0xB492A779, 0xF2F0F307, 0xE2A14E69, 0xF4CD65DA, 0xBED50605,
   0x621FD134, 0xFE8AC4A6, 0x539D342E, 0x55A0A2F3, 0xE132058A, 0xEB75A4F6,
   0xEC390B83, 0xEFAA4060, 0x9F065E71, 0x1051BD6E, 0x8AF93E21, 0x063D96DD,
   0x05AEDD3E, 0xBD464DE6, 0x8DB59154, 0x5D0571C4, 0xD46F0406, 0x15FF6050,
   0xFB241998, 0xE997D6BD, 0x43CC8940, 0x9E7767D9, 0x42BDB0E8, 0x8B880789,
   0x5B38E719, 0xEEDB79C8, 0x0A47A17C, 0x0FE97C42, 0x1EC9F884, 0x00000000,
   0x86830980, 0xED48322B, 0x70AC1E11, 0x724E6C5A, 0xFFFBFD0E, 0x38560F85,
   0xD51E3DAE, 0x3927362D, 0xD9640A0F, 0xA621685C, 0x54D19B5B, 0x2E3A2436,
   0x67B10C0A, 0xE70F9357, 0x96D2B4EE, 0x919E1B9B, 0xC54F80C0, 0x20A261DC,
   0x4B695A77, 0x1A161C12, 0xBA0AE293, 0x2AE5C0A0, 0xE0433C22, 0x171D121B,
   0x0D0B0E09, 0xC7ADF28B, 0xA8B92DB6, 0xA9C8141E, 0x198557F1, 0x074CAF75,
   0xDDBBEE99, 0x60FDA37F, 0x269FF701, 0xF5BC5C72, 0x3BC54466, 0x7E345BFB,
   0x29768B43, 0xC6DCCB23, 0xFC68B6ED, 0xF163B8E4, 0xDCCAD731, 0x85104263,
   0x22401397, 0x112084C6, 0x247D854A, 0x3DF8D2BB, 0x3211AEF9, 0xA16DC729,
   0x2F4B1D9E, 0x30F3DCB2, 0x52EC0D86, 0xE3D077C1, 0x166C2BB3, 0xB999A970,
   0x48FA1194, 0x642247E9, 0x8CC4A8FC, 0x3F1AA0F0, 0x2CD8567D, 0x90EF2233,
   0x4EC78749, 0xD1C1D938, 0xA2FE8CCA, 0x0B3698D4, 0x81CFA6F5, 0xDE28A57A,
   0x8E26DAB7, 0xBFA43FAD, 0x9DE42C3A, 0x920D5078, 0xCC9B6A5F, 0x4662547E,
   0x13C2F68D, 0xB8E890D8, 0xF75E2E39, 0xAFF582C3, 0x80BE9F5D, 0x937C69D0,
   0x2DA96FD5, 0x12B3CF25, 0x993BC8AC, 0x7DA71018, 0x636EE89C, 0xBB7BDB3B,
   0x7809CD26, 0x18F46E59, 0xB701EC9A, 0x9AA8834F, 0x6E65E695, 0xE67EAAFF,
   0xCF0821BC, 0xE8E6EF15, 0x9BD9BAE7, 0x36CE4A6F, 0x09D4EA9F, 0x7CD629B0,
   0xB2AF31A4, 0x23312A3F, 0x9430C6A5, 0x66C035A2, 0xBC37744E, 0xCAA6FC82,
   0xD0B0E090, 0xD81533A7, 0x984AF104, 0xDAF741EC, 0x500E7FCD, 0xF62F1791,
   0xD68D764D, 0xB04D43EF, 0x4D54CCAA, 0x04DFE496, 0xB5E39ED1, 0x881B4C6A,
   0x1FB8C12C, 0x517F4665, 0xEA049D5E, 0x355D018C, 0x7473FA87, 0x412EFB0B,
   0x1D5AB367, 0xD25292DB, 0x5633E910, 0x47136DD6, 0x618C9AD7, 0x0C7A37A1,
   0x148E59F8, 0x3C89EB13, 0x27EECEA9, 0xC935B761, 0xE5EDE11C, 0xB13C7A47,
   0xDF599CD2, 0x733F55F2, 0xCE791814, 0x37BF73C7, 0xCDEA53F7, 0xAA5B5FFD,
   0x6F14DF3D, 0xDB867844, 0xF381CAAF, 0xC43EB968, 0x342C3824, 0x405FC2A3,
   0xC372161D, 0x250CBCE2, 0x498B283C, 0x9541FF0D, 0x017139A8, 0xB3DE080C,
   0xE49CD8B4, 0xC1906456, 0x84617BCB, 0xB670D532, 0x5C74486C, 0x5742D0B8,
   0xF4A75051, 0x4165537E, 0x17A4C31A, 0x275E963A, 0xAB6BCB3B, 0x9D45F11F,
   0xFA58ABAC, 0xE303934B, 0x30FA5520, 0x766DF6AD, 0xCC769188, 0x024C25F5,
   0xE5D7FC4F, 0x2ACBD7C5, 0x35448026, 0x62A38FB5, 0xB15A49DE, 0xBA1B6725,
   0xEA0E9845, 0xFEC0E15D, 0x2F7502C3, 0x4CF01281, 0x4697A38D, 0xD3F9C66B,
   0x8F5FE703, 0x929C9515, 0x6D7AEBBF, 0x5259DA95, 0xBE832DD4, 0x7421D358,
   0xE0692949, 0xC9C8448E, 0xC2896A75, 0x8E7978F4, 0x583E6B99, 0xB971DD27,
   0xE14FB6BE, 0x88AD17F0, 0x20AC66C9, 0xCE3AB47D, 0xDF4A1863, 0x1A3182E5,
   0x51336097, 0x537F4562, 0x6477E0B1, 0x6BAE84BB, 0x81A01CFE, 0x082B94F9,
   0x48685870, 0x45FD198F, 0xDE6C8794, 0x7BF8B752, 0x73D323AB, 0x4B02E272,
   0x1F8F57E3, 0x55AB2A66, 0xEB2807B2, 0xB5C2032F, 0xC57B9A86, 0x3708A5D3,
   0x2887F230, 0xBFA5B223, 0x036ABA02, 0x16825CED, 0xCF1C2B8A, 0x79B492A7,
   0x07F2F0F3, 0x69E2A14E, 0xDAF4CD65, 0x05BED506, 0x34621FD1, 0xA6FE8AC4,
   0x2E539D34, 0xF355A0A2, 0x8AE13205, 0xF6EB75A4, 0x83EC390B, 0x60EFAA40,
   0x719F065E, 0x6E1051BD, 0x218AF93E, 0xDD063D96, 0x3E05AEDD, 0xE6BD464D,
   0x548DB591, 0xC45D0571, 0x06D46F04, 0x5015FF60, 0x98FB2419, 0xBDE997D6,
   0x4043CC89, 0xD99E7767, 0xE842BDB0, 0x898B8807, 0x195B38E7, 0xC8EEDB79,
   0x7C0A47A1, 0x420FE97C, 0x841EC9F8, 0x00000000, 0x80868309, 0x2BED4832,
   0x1170AC1E, 0x5A724E6C, 0x0EFFFBFD, 0x8538560F, 0xAED51E3D, 0x2D392736,
   0x0FD9640A, 0x5CA62168, 0x5B54D19B, 0x362E3A24, 0x0A67B10C, 0x57E70F93,
   0xEE96D2B4, 0x9B919E1B, 0xC0C54F80, 0xDC20A261, 0x774B695A, 0x121A161C,
   0x93BA0AE2, 0xA02AE5C0, 0x22E0433C, 0x1B171D12, 0x090D0B0E, 0x8BC7ADF2,
   0xB6A8B92D, 0x1EA9C814, 0xF1198557, 0x75074CAF, 0x99DDBBEE, 0x7F60FDA3,
   0x01269FF7, 0x72F5BC5C, 0x663BC544, 0xFB7E345B, 0x4329768B, 0x23C6DCCB,
   0xEDFC68B6, 0xE4F163B8, 0x31DCCAD7, 0x63851042, 0x97224013, 0xC6112084,
   0x4A247D85, 0xBB3DF8D2, 0xF93211AE, 0x29A16DC7, 0x9E2F4B1D, 0xB230F3DC,
   0x8652EC0D, 0xC1E3D077, 0xB3166C2B, 0x70B999A9, 0x9448FA11, 0xE9642247,
   0xFC8CC4A8, 0xF03F1AA0, 0x7D2CD856, 0x3390EF22, 0x494EC787, 0x38D1C1D9,
   0xCAA2FE8C, 0xD40B3698, 0xF581CFA6, 0x7ADE28A5, 0xB78E26DA, 0xADBFA43F,
   0x3A9DE42C, 0x78920D50, 0x5FCC9B6A, 0x7E466254, 0x8D13C2F6, 0xD8B8E890,
   0x39F75E2E, 0xC3AFF582, 0x5D80BE9F, 0xD0937C69, 0xD52DA96F, 0x2512B3CF,
   0xAC993BC8, 0x187DA710, 0x9C636EE8, 0x3BBB7BDB, 0x267809CD, 0x5918F46E,
   0x9AB701EC, 0x4F9AA883, 0x956E65E6, 0xFFE67EAA, 0xBCCF0821, 0x15E8E6EF,
   0xE79BD9BA, 0x6F36CE4A, 0x9F09D4EA, 0xB07CD629, 0xA4B2AF31, 0x3F23312A,
   0xA59430C6, 0xA266C035, 0x4EBC3774, 0x82CAA6FC, 0x90D0B0E0, 0xA7D81533,
   0x04984AF1, 0xECDAF741, 0xCD500E7F, 0x91F62F17, 0x4DD68D76, 0xEFB04D43,
   0xAA4D54CC, 0x9604DFE4, 0xD1B5E39E, 0x6A881B4C, 0x2C1FB8C1, 0x65517F46,
   0x5EEA049D, 0x8C355D01, 0x877473FA, 0x0B412EFB, 0x671D5AB3, 0xDBD25292,
   0x105633E9, 0xD647136D, 0xD7618C9A, 0xA10C7A37, 0xF8148E59, 0x133C89EB,
   0xA927EECE, 0x61C935B7, 0x1CE5EDE1, 0x47B13C7A, 0xD2DF599C, 0xF2733F55,
   0x14CE7918, 0xC737BF73, 0xF7CDEA53, 0xFDAA5B5F, 0x3D6F14DF, 0x44DB8678,
   0xAFF381CA, 0x68C43EB9, 0x24342C38, 0xA3405FC2, 0x1DC37216, 0xE2250CBC,
   0x3C498B28, 0x0D9541FF, 0xA8017139, 0x0CB3DE08, 0xB4E49CD8, 0x56C19064,
   0xCB84617B, 0x32B670D5, 0x6C5C7448, 0xB85742D0 };

/*
* AES Encryption
*/
void aes_encrypt_n(const byte in[], byte out[],
                   size_t blocks,
                   const secure_vector<u32bit>& EK,
                   const secure_vector<byte>& ME)
   {
   BOTAN_ASSERT(EK.size() && ME.size() == 16, "Key was set");

   const size_t BLOCK_SIZE = 16;

   const u32bit* TE0 = TE;
   const u32bit* TE1 = TE + 256;
   const u32bit* TE2 = TE + 512;
   const u32bit* TE3 = TE + 768;

   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit T0 = load_be<u32bit>(in, 0) ^ EK[0];
      u32bit T1 = load_be<u32bit>(in, 1) ^ EK[1];
      u32bit T2 = load_be<u32bit>(in, 2) ^ EK[2];
      u32bit T3 = load_be<u32bit>(in, 3) ^ EK[3];

      /* Use only the first 256 entries of the TE table and do the
      * rotations directly in the code. This reduces the number of
      * cache lines potentially used in the first round from 64 to 16
      * (assuming a typical 64 byte cache line), which makes timing
      * attacks a little harder; the first round is particularly
      * vulnerable.
      */

      u32bit B0 = TE[get_byte(0, T0)] ^
                  rotate_right(TE[get_byte(1, T1)],  8) ^
                  rotate_right(TE[get_byte(2, T2)], 16) ^
                  rotate_right(TE[get_byte(3, T3)], 24) ^ EK[4];

      u32bit B1 = TE[get_byte(0, T1)] ^
                  rotate_right(TE[get_byte(1, T2)],  8) ^
                  rotate_right(TE[get_byte(2, T3)], 16) ^
                  rotate_right(TE[get_byte(3, T0)], 24) ^ EK[5];

      u32bit B2 = TE[get_byte(0, T2)] ^
                  rotate_right(TE[get_byte(1, T3)],  8) ^
                  rotate_right(TE[get_byte(2, T0)], 16) ^
                  rotate_right(TE[get_byte(3, T1)], 24) ^ EK[6];

      u32bit B3 = TE[get_byte(0, T3)] ^
                  rotate_right(TE[get_byte(1, T0)],  8) ^
                  rotate_right(TE[get_byte(2, T1)], 16) ^
                  rotate_right(TE[get_byte(3, T2)], 24) ^ EK[7];

      for(size_t r = 2*4; r < EK.size(); r += 2*4)
         {
         T0 = TE0[get_byte(0, B0)] ^ TE1[get_byte(1, B1)] ^
              TE2[get_byte(2, B2)] ^ TE3[get_byte(3, B3)] ^ EK[r];
         T1 = TE0[get_byte(0, B1)] ^ TE1[get_byte(1, B2)] ^
              TE2[get_byte(2, B3)] ^ TE3[get_byte(3, B0)] ^ EK[r+1];
         T2 = TE0[get_byte(0, B2)] ^ TE1[get_byte(1, B3)] ^
              TE2[get_byte(2, B0)] ^ TE3[get_byte(3, B1)] ^ EK[r+2];
         T3 = TE0[get_byte(0, B3)] ^ TE1[get_byte(1, B0)] ^
              TE2[get_byte(2, B1)] ^ TE3[get_byte(3, B2)] ^ EK[r+3];

         B0 = TE0[get_byte(0, T0)] ^ TE1[get_byte(1, T1)] ^
              TE2[get_byte(2, T2)] ^ TE3[get_byte(3, T3)] ^ EK[r+4];
         B1 = TE0[get_byte(0, T1)] ^ TE1[get_byte(1, T2)] ^
              TE2[get_byte(2, T3)] ^ TE3[get_byte(3, T0)] ^ EK[r+5];
         B2 = TE0[get_byte(0, T2)] ^ TE1[get_byte(1, T3)] ^
              TE2[get_byte(2, T0)] ^ TE3[get_byte(3, T1)] ^ EK[r+6];
         B3 = TE0[get_byte(0, T3)] ^ TE1[get_byte(1, T0)] ^
              TE2[get_byte(2, T1)] ^ TE3[get_byte(3, T2)] ^ EK[r+7];
         }

      /*
      Joseph Bonneau and Ilya Mironov's paper "Cache-Collision Timing
      Attacks Against AES" describes an attack that can recover AES
      keys with as few as 2**13 samples.

      http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.88.4753

      They recommend using a byte-wide table, which still allows an attack
      but increases the samples required from 2**13 to 2**25:

      """In addition to OpenSSL v. 0.9.8.(a), which was used in our
      experiments, the AES implementations of Crypto++ 5.2.1 and
      LibTomCrypt 1.09 use the original Rijndael C implementation with
      very few changes and are highly vulnerable. The AES implementations
      in libgcrypt v. 1.2.2 and Botan v. 1.4.2 are also vulnerable, but
      use a smaller byte-wide final table which lessens the effectiveness
      of the attacks."""
      */

      out[ 0] = SE[get_byte(0, B0)] ^ ME[0];
      out[ 1] = SE[get_byte(1, B1)] ^ ME[1];
      out[ 2] = SE[get_byte(2, B2)] ^ ME[2];
      out[ 3] = SE[get_byte(3, B3)] ^ ME[3];
      out[ 4] = SE[get_byte(0, B1)] ^ ME[4];
      out[ 5] = SE[get_byte(1, B2)] ^ ME[5];
      out[ 6] = SE[get_byte(2, B3)] ^ ME[6];
      out[ 7] = SE[get_byte(3, B0)] ^ ME[7];
      out[ 8] = SE[get_byte(0, B2)] ^ ME[8];
      out[ 9] = SE[get_byte(1, B3)] ^ ME[9];
      out[10] = SE[get_byte(2, B0)] ^ ME[10];
      out[11] = SE[get_byte(3, B1)] ^ ME[11];
      out[12] = SE[get_byte(0, B3)] ^ ME[12];
      out[13] = SE[get_byte(1, B0)] ^ ME[13];
      out[14] = SE[get_byte(2, B1)] ^ ME[14];
      out[15] = SE[get_byte(3, B2)] ^ ME[15];

      in += BLOCK_SIZE;
      out += BLOCK_SIZE;
      }
   }

/*
* AES Decryption
*/
void aes_decrypt_n(const byte in[], byte out[], size_t blocks,
                   const secure_vector<u32bit>& DK,
                   const secure_vector<byte>& MD)
   {
   BOTAN_ASSERT(DK.size() && MD.size() == 16, "Key was set");

   const size_t BLOCK_SIZE = 16;

   const u32bit* TD0 = TD;
   const u32bit* TD1 = TD + 256;
   const u32bit* TD2 = TD + 512;
   const u32bit* TD3 = TD + 768;

   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit T0 = load_be<u32bit>(in, 0) ^ DK[0];
      u32bit T1 = load_be<u32bit>(in, 1) ^ DK[1];
      u32bit T2 = load_be<u32bit>(in, 2) ^ DK[2];
      u32bit T3 = load_be<u32bit>(in, 3) ^ DK[3];

      u32bit B0 = TD[get_byte(0, T0)] ^
                  rotate_right(TD[get_byte(1, T3)],  8) ^
                  rotate_right(TD[get_byte(2, T2)], 16) ^
                  rotate_right(TD[get_byte(3, T1)], 24) ^ DK[4];

      u32bit B1 = TD[get_byte(0, T1)] ^
                  rotate_right(TD[get_byte(1, T0)],  8) ^
                  rotate_right(TD[get_byte(2, T3)], 16) ^
                  rotate_right(TD[get_byte(3, T2)], 24) ^ DK[5];

      u32bit B2 = TD[get_byte(0, T2)] ^
                  rotate_right(TD[get_byte(1, T1)],  8) ^
                  rotate_right(TD[get_byte(2, T0)], 16) ^
                  rotate_right(TD[get_byte(3, T3)], 24) ^ DK[6];

      u32bit B3 = TD[get_byte(0, T3)] ^
                  rotate_right(TD[get_byte(1, T2)],  8) ^
                  rotate_right(TD[get_byte(2, T1)], 16) ^
                  rotate_right(TD[get_byte(3, T0)], 24) ^ DK[7];

      for(size_t r = 2*4; r < DK.size(); r += 2*4)
         {
         T0 = TD0[get_byte(0, B0)] ^ TD1[get_byte(1, B3)] ^
              TD2[get_byte(2, B2)] ^ TD3[get_byte(3, B1)] ^ DK[r];
         T1 = TD0[get_byte(0, B1)] ^ TD1[get_byte(1, B0)] ^
              TD2[get_byte(2, B3)] ^ TD3[get_byte(3, B2)] ^ DK[r+1];
         T2 = TD0[get_byte(0, B2)] ^ TD1[get_byte(1, B1)] ^
              TD2[get_byte(2, B0)] ^ TD3[get_byte(3, B3)] ^ DK[r+2];
         T3 = TD0[get_byte(0, B3)] ^ TD1[get_byte(1, B2)] ^
              TD2[get_byte(2, B1)] ^ TD3[get_byte(3, B0)] ^ DK[r+3];

         B0 = TD0[get_byte(0, T0)] ^ TD1[get_byte(1, T3)] ^
              TD2[get_byte(2, T2)] ^ TD3[get_byte(3, T1)] ^ DK[r+4];
         B1 = TD0[get_byte(0, T1)] ^ TD1[get_byte(1, T0)] ^
              TD2[get_byte(2, T3)] ^ TD3[get_byte(3, T2)] ^ DK[r+5];
         B2 = TD0[get_byte(0, T2)] ^ TD1[get_byte(1, T1)] ^
              TD2[get_byte(2, T0)] ^ TD3[get_byte(3, T3)] ^ DK[r+6];
         B3 = TD0[get_byte(0, T3)] ^ TD1[get_byte(1, T2)] ^
              TD2[get_byte(2, T1)] ^ TD3[get_byte(3, T0)] ^ DK[r+7];
         }

      out[ 0] = SD[get_byte(0, B0)] ^ MD[0];
      out[ 1] = SD[get_byte(1, B3)] ^ MD[1];
      out[ 2] = SD[get_byte(2, B2)] ^ MD[2];
      out[ 3] = SD[get_byte(3, B1)] ^ MD[3];
      out[ 4] = SD[get_byte(0, B1)] ^ MD[4];
      out[ 5] = SD[get_byte(1, B0)] ^ MD[5];
      out[ 6] = SD[get_byte(2, B3)] ^ MD[6];
      out[ 7] = SD[get_byte(3, B2)] ^ MD[7];
      out[ 8] = SD[get_byte(0, B2)] ^ MD[8];
      out[ 9] = SD[get_byte(1, B1)] ^ MD[9];
      out[10] = SD[get_byte(2, B0)] ^ MD[10];
      out[11] = SD[get_byte(3, B3)] ^ MD[11];
      out[12] = SD[get_byte(0, B3)] ^ MD[12];
      out[13] = SD[get_byte(1, B2)] ^ MD[13];
      out[14] = SD[get_byte(2, B1)] ^ MD[14];
      out[15] = SD[get_byte(3, B0)] ^ MD[15];

      in += BLOCK_SIZE;
      out += BLOCK_SIZE;
      }
   }

void aes_key_schedule(const byte key[], size_t length,
                      secure_vector<u32bit>& EK,
                      secure_vector<u32bit>& DK,
                      secure_vector<byte>& ME,
                      secure_vector<byte>& MD)
   {
   static const u32bit RC[10] = {
      0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
      0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000 };

   const size_t rounds = (length / 4) + 6;

   secure_vector<u32bit> XEK(length + 32), XDK(length + 32);

   const size_t X = length / 4;
   for(size_t i = 0; i != X; ++i)
      XEK[i] = load_be<u32bit>(key, i);

   for(size_t i = X; i < 4*(rounds+1); i += X)
      {
      XEK[i] = XEK[i-X] ^ RC[(i-X)/X] ^
               make_u32bit(SE[get_byte(1, XEK[i-1])],
                           SE[get_byte(2, XEK[i-1])],
                           SE[get_byte(3, XEK[i-1])],
                           SE[get_byte(0, XEK[i-1])]);

      for(size_t j = 1; j != X; ++j)
         {
         XEK[i+j] = XEK[i+j-X];

         if(X == 8 && j == 4)
            XEK[i+j] ^= make_u32bit(SE[get_byte(0, XEK[i+j-1])],
                                    SE[get_byte(1, XEK[i+j-1])],
                                    SE[get_byte(2, XEK[i+j-1])],
                                    SE[get_byte(3, XEK[i+j-1])]);
         else
            XEK[i+j] ^= XEK[i+j-1];
         }
      }

   for(size_t i = 0; i != 4*(rounds+1); i += 4)
      {
      XDK[i  ] = XEK[4*rounds-i  ];
      XDK[i+1] = XEK[4*rounds-i+1];
      XDK[i+2] = XEK[4*rounds-i+2];
      XDK[i+3] = XEK[4*rounds-i+3];
      }

   for(size_t i = 4; i != length + 24; ++i)
      XDK[i] = TD[SE[get_byte(0, XDK[i])] +   0] ^
               TD[SE[get_byte(1, XDK[i])] + 256] ^
               TD[SE[get_byte(2, XDK[i])] + 512] ^
               TD[SE[get_byte(3, XDK[i])] + 768];

   ME.resize(16);
   MD.resize(16);

   for(size_t i = 0; i != 4; ++i)
      {
      store_be(XEK[i+4*rounds], &ME[4*i]);
      store_be(XEK[i], &MD[4*i]);
      }

   EK.resize(length + 24);
   DK.resize(length + 24);
   copy_mem(&EK[0], &XEK[0], EK.size());
   copy_mem(&DK[0], &XDK[0], DK.size());
   }

}

void AES_128::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_encrypt_n(in, out, blocks, EK, ME);
   }

void AES_128::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_decrypt_n(in, out, blocks, DK, MD);
   }

void AES_128::key_schedule(const byte key[], size_t length)
   {
   aes_key_schedule(key, length, EK, DK, ME, MD);
   }

void AES_128::clear()
   {
   zap(EK);
   zap(DK);
   zap(ME);
   zap(MD);
   }

void AES_192::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_encrypt_n(in, out, blocks, EK, ME);
   }

void AES_192::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_decrypt_n(in, out, blocks, DK, MD);
   }

void AES_192::key_schedule(const byte key[], size_t length)
   {
   aes_key_schedule(key, length, EK, DK, ME, MD);
   }

void AES_192::clear()
   {
   zap(EK);
   zap(DK);
   zap(ME);
   zap(MD);
   }

void AES_256::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_encrypt_n(in, out, blocks, EK, ME);
   }

void AES_256::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_decrypt_n(in, out, blocks, DK, MD);
   }

void AES_256::key_schedule(const byte key[], size_t length)
   {
   aes_key_schedule(key, length, EK, DK, ME, MD);
   }

void AES_256::clear()
   {
   zap(EK);
   zap(DK);
   zap(ME);
   zap(MD);
   }

}
/*
* Serpent
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* Serpent's Linear Transformation
*/
inline void transform(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
   {
   B0  = rotate_left(B0, 13);   B2  = rotate_left(B2, 3);
   B1 ^= B0 ^ B2;               B3 ^= B2 ^ (B0 << 3);
   B1  = rotate_left(B1, 1);    B3  = rotate_left(B3, 7);
   B0 ^= B1 ^ B3;               B2 ^= B3 ^ (B1 << 7);
   B0  = rotate_left(B0, 5);    B2  = rotate_left(B2, 22);
   }

/*
* Serpent's Inverse Linear Transformation
*/
inline void i_transform(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
   {
   B2  = rotate_right(B2, 22);  B0  = rotate_right(B0, 5);
   B2 ^= B3 ^ (B1 << 7);        B0 ^= B1 ^ B3;
   B3  = rotate_right(B3, 7);   B1  = rotate_right(B1, 1);
   B3 ^= B2 ^ (B0 << 3);        B1 ^= B0 ^ B2;
   B2  = rotate_right(B2, 3);   B0  = rotate_right(B0, 13);
   }

}

/*
* XOR a key block with a data block
*/
#define key_xor(round, B0, B1, B2, B3) \
   B0 ^= round_key[4*round  ]; \
   B1 ^= round_key[4*round+1]; \
   B2 ^= round_key[4*round+2]; \
   B3 ^= round_key[4*round+3];

/*
* Serpent Encryption
*/
void Serpent::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit B0 = load_le<u32bit>(in, 0);
      u32bit B1 = load_le<u32bit>(in, 1);
      u32bit B2 = load_le<u32bit>(in, 2);
      u32bit B3 = load_le<u32bit>(in, 3);

      key_xor( 0,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 1,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 2,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 3,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 4,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 5,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 6,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 7,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 8,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor( 9,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(10,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(11,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(12,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(13,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(14,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(15,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(16,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(17,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(18,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(19,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(20,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(21,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(22,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(23,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(24,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(25,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(26,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(27,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(28,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(29,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(30,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
      key_xor(31,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); key_xor(32,B0,B1,B2,B3);

      store_le(out, B0, B1, B2, B3);

      in += BLOCK_SIZE;
      out += BLOCK_SIZE;
      }
   }

/*
* Serpent Decryption
*/
void Serpent::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit B0 = load_le<u32bit>(in, 0);
      u32bit B1 = load_le<u32bit>(in, 1);
      u32bit B2 = load_le<u32bit>(in, 2);
      u32bit B3 = load_le<u32bit>(in, 3);

      key_xor(32,B0,B1,B2,B3);  SBoxD8(B0,B1,B2,B3); key_xor(31,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(30,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(29,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(28,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(27,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(26,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(25,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(24,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(23,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(22,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(21,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(20,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(19,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(18,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(17,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(16,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(15,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(14,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(13,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(12,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(11,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(10,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 9,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 8,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor( 7,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor( 6,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor( 5,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor( 4,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor( 3,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor( 2,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 1,B0,B1,B2,B3);
      i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 0,B0,B1,B2,B3);

      store_le(out, B0, B1, B2, B3);

      in += BLOCK_SIZE;
      out += BLOCK_SIZE;
      }
   }

#undef key_xor
#undef transform
#undef i_transform

/*
* Serpent Key Schedule
*/
void Serpent::key_schedule(const byte key[], size_t length)
   {
   const u32bit PHI = 0x9E3779B9;

   secure_vector<u32bit> W(140);
   for(size_t i = 0; i != length / 4; ++i)
      W[i] = load_le<u32bit>(key, i);

   W[length / 4] |= u32bit(1) << ((length%4)*8);

   for(size_t i = 8; i != 140; ++i)
      {
      u32bit wi = W[i-8] ^ W[i-5] ^ W[i-3] ^ W[i-1] ^ PHI ^ u32bit(i-8);
      W[i] = rotate_left(wi, 11);
      }

   SBoxE4(W[  8],W[  9],W[ 10],W[ 11]); SBoxE3(W[ 12],W[ 13],W[ 14],W[ 15]);
   SBoxE2(W[ 16],W[ 17],W[ 18],W[ 19]); SBoxE1(W[ 20],W[ 21],W[ 22],W[ 23]);
   SBoxE8(W[ 24],W[ 25],W[ 26],W[ 27]); SBoxE7(W[ 28],W[ 29],W[ 30],W[ 31]);
   SBoxE6(W[ 32],W[ 33],W[ 34],W[ 35]); SBoxE5(W[ 36],W[ 37],W[ 38],W[ 39]);
   SBoxE4(W[ 40],W[ 41],W[ 42],W[ 43]); SBoxE3(W[ 44],W[ 45],W[ 46],W[ 47]);
   SBoxE2(W[ 48],W[ 49],W[ 50],W[ 51]); SBoxE1(W[ 52],W[ 53],W[ 54],W[ 55]);
   SBoxE8(W[ 56],W[ 57],W[ 58],W[ 59]); SBoxE7(W[ 60],W[ 61],W[ 62],W[ 63]);
   SBoxE6(W[ 64],W[ 65],W[ 66],W[ 67]); SBoxE5(W[ 68],W[ 69],W[ 70],W[ 71]);
   SBoxE4(W[ 72],W[ 73],W[ 74],W[ 75]); SBoxE3(W[ 76],W[ 77],W[ 78],W[ 79]);
   SBoxE2(W[ 80],W[ 81],W[ 82],W[ 83]); SBoxE1(W[ 84],W[ 85],W[ 86],W[ 87]);
   SBoxE8(W[ 88],W[ 89],W[ 90],W[ 91]); SBoxE7(W[ 92],W[ 93],W[ 94],W[ 95]);
   SBoxE6(W[ 96],W[ 97],W[ 98],W[ 99]); SBoxE5(W[100],W[101],W[102],W[103]);
   SBoxE4(W[104],W[105],W[106],W[107]); SBoxE3(W[108],W[109],W[110],W[111]);
   SBoxE2(W[112],W[113],W[114],W[115]); SBoxE1(W[116],W[117],W[118],W[119]);
   SBoxE8(W[120],W[121],W[122],W[123]); SBoxE7(W[124],W[125],W[126],W[127]);
   SBoxE6(W[128],W[129],W[130],W[131]); SBoxE5(W[132],W[133],W[134],W[135]);
   SBoxE4(W[136],W[137],W[138],W[139]);

   round_key.assign(&W[8], &W[140]);
   }

void Serpent::clear()
   {
   zap(round_key);
   }

}
/*
* Serpent (SIMD)
* (C) 2009,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

#define key_xor(round, B0, B1, B2, B3)                             \
   do {                                                            \
      B0 ^= SIMD_32(keys[4*round  ]);                              \
      B1 ^= SIMD_32(keys[4*round+1]);                              \
      B2 ^= SIMD_32(keys[4*round+2]);                              \
      B3 ^= SIMD_32(keys[4*round+3]);                              \
   } while(0);

/*
* Serpent's linear transformations
*/
#define transform(B0, B1, B2, B3)                                  \
   do {                                                            \
      B0.rotate_left(13);                                          \
      B2.rotate_left(3);                                           \
      B1 ^= B0 ^ B2;                                               \
      B3 ^= B2 ^ (B0 << 3);                                        \
      B1.rotate_left(1);                                           \
      B3.rotate_left(7);                                           \
      B0 ^= B1 ^ B3;                                               \
      B2 ^= B3 ^ (B1 << 7);                                        \
      B0.rotate_left(5);                                           \
      B2.rotate_left(22);                                          \
   } while(0);

#define i_transform(B0, B1, B2, B3)                                \
   do {                                                            \
      B2.rotate_right(22);                                         \
      B0.rotate_right(5);                                          \
      B2 ^= B3 ^ (B1 << 7);                                        \
      B0 ^= B1 ^ B3;                                               \
      B3.rotate_right(7);                                          \
      B1.rotate_right(1);                                          \
      B3 ^= B2 ^ (B0 << 3);                                        \
      B1 ^= B0 ^ B2;                                               \
      B2.rotate_right(3);                                          \
      B0.rotate_right(13);                                         \
   } while(0);

/*
* SIMD Serpent Encryption of 4 blocks in parallel
*/
void serpent_encrypt_4(const byte in[64],
                       byte out[64],
                       const u32bit keys[132])
   {
   SIMD_32 B0 = SIMD_32::load_le(in);
   SIMD_32 B1 = SIMD_32::load_le(in + 16);
   SIMD_32 B2 = SIMD_32::load_le(in + 32);
   SIMD_32 B3 = SIMD_32::load_le(in + 48);

   SIMD_32::transpose(B0, B1, B2, B3);

   key_xor( 0,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 1,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 2,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 3,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 4,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 5,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 6,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 7,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);

   key_xor( 8,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 9,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(10,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(11,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(12,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(13,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(14,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(15,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);

   key_xor(16,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(17,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(18,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(19,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(20,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(21,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(22,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(23,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);

   key_xor(24,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(25,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(26,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(27,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(28,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(29,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(30,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(31,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); key_xor(32,B0,B1,B2,B3);

   SIMD_32::transpose(B0, B1, B2, B3);

   B0.store_le(out);
   B1.store_le(out + 16);
   B2.store_le(out + 32);
   B3.store_le(out + 48);
   }

/*
* SIMD Serpent Decryption of 4 blocks in parallel
*/
void serpent_decrypt_4(const byte in[64],
                       byte out[64],
                       const u32bit keys[132])
   {
   SIMD_32 B0 = SIMD_32::load_le(in);
   SIMD_32 B1 = SIMD_32::load_le(in + 16);
   SIMD_32 B2 = SIMD_32::load_le(in + 32);
   SIMD_32 B3 = SIMD_32::load_le(in + 48);

   SIMD_32::transpose(B0, B1, B2, B3);

   key_xor(32,B0,B1,B2,B3);  SBoxD8(B0,B1,B2,B3); key_xor(31,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(30,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(29,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(28,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(27,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(26,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(25,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(24,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(23,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(22,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(21,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(20,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(19,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(18,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(17,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(16,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(15,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(14,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(13,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(12,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(11,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(10,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 9,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 8,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor( 7,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor( 6,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor( 5,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor( 4,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor( 3,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor( 2,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 1,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 0,B0,B1,B2,B3);

   SIMD_32::transpose(B0, B1, B2, B3);

   B0.store_le(out);
   B1.store_le(out + 16);
   B2.store_le(out + 32);
   B3.store_le(out + 48);
   }

}

#undef key_xor
#undef transform
#undef i_transform

/*
* Serpent Encryption
*/
void Serpent_SIMD::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u32bit* KS = &(this->get_round_keys()[0]);

   while(blocks >= 4)
      {
      serpent_encrypt_4(in, out, KS);
      in += 4 * BLOCK_SIZE;
      out += 4 * BLOCK_SIZE;
      blocks -= 4;
      }

   if(blocks)
     Serpent::encrypt_n(in, out, blocks);
   }

/*
* Serpent Decryption
*/
void Serpent_SIMD::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   const u32bit* KS = &(this->get_round_keys()[0]);

   while(blocks >= 4)
      {
      serpent_decrypt_4(in, out, KS);
      in += 4 * BLOCK_SIZE;
      out += 4 * BLOCK_SIZE;
      blocks -= 4;
      }

   if(blocks)
     Serpent::decrypt_n(in, out, blocks);
   }

}
/*
* Base64 Encoding and Decoding
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

static const byte BIN_TO_BASE64[64] = {
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
   'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
   'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

void do_base64_encode(char out[4], const byte in[3])
   {
   out[0] = BIN_TO_BASE64[((in[0] & 0xFC) >> 2)];
   out[1] = BIN_TO_BASE64[((in[0] & 0x03) << 4) | (in[1] >> 4)];
   out[2] = BIN_TO_BASE64[((in[1] & 0x0F) << 2) | (in[2] >> 6)];
   out[3] = BIN_TO_BASE64[((in[2] & 0x3F)     )];
   }

}

size_t base64_encode(char out[],
                     const byte in[],
                     size_t input_length,
                     size_t& input_consumed,
                     bool final_inputs)
   {
   input_consumed = 0;

   size_t input_remaining = input_length;
   size_t output_produced = 0;

   while(input_remaining >= 3)
      {
      do_base64_encode(out + output_produced, in + input_consumed);

      input_consumed += 3;
      output_produced += 4;
      input_remaining -= 3;
      }

   if(final_inputs && input_remaining)
      {
      byte remainder[3] = { 0 };
      for(size_t i = 0; i != input_remaining; ++i)
         remainder[i] = in[input_consumed + i];

      do_base64_encode(out + output_produced, remainder);

      size_t empty_bits = 8 * (3 - input_remaining);
      size_t index = output_produced + 4 - 1;
      while(empty_bits >= 8)
         {
         out[index--] = '=';
         empty_bits -= 6;
         }

      input_consumed += input_remaining;
      output_produced += 4;
      }

   return output_produced;
   }

std::string base64_encode(const byte input[],
                          size_t input_length)
   {
   std::string output((round_up<size_t>(input_length, 3) / 3) * 4, 0);

   size_t consumed = 0;
   size_t produced = base64_encode(&output[0],
                                   input, input_length,
                                   consumed, true);

   BOTAN_ASSERT_EQUAL(consumed, input_length, "Consumed the entire input");
   BOTAN_ASSERT_EQUAL(produced, output.size(), "Produced expected size");

   return output;
   }

size_t base64_decode(byte output[],
                     const char input[],
                     size_t input_length,
                     size_t& input_consumed,
                     bool final_inputs,
                     bool ignore_ws)
   {
   /*
   * Base64 Decoder Lookup Table
   * Warning: assumes ASCII encodings
   */
   static const byte BASE64_TO_BIN[256] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80,
      0x80, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35,
      0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF,
      0xFF, 0x81, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04,
      0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
      0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
      0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C,
      0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
      0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
      0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

   byte* out_ptr = output;
   byte decode_buf[4];
   size_t decode_buf_pos = 0;
   size_t final_truncate = 0;

   clear_mem(output, input_length * 3 / 4);

   for(size_t i = 0; i != input_length; ++i)
      {
      const byte bin = BASE64_TO_BIN[static_cast<byte>(input[i])];

      if(bin <= 0x3F)
         {
         decode_buf[decode_buf_pos] = bin;
         decode_buf_pos += 1;
         }
      else if(!(bin == 0x81 || (bin == 0x80 && ignore_ws)))
         {
         std::string bad_char(1, input[i]);
         if(bad_char == "\t")
           bad_char = "\\t";
         else if(bad_char == "\n")
           bad_char = "\\n";
         else if(bad_char == "\r")
           bad_char = "\\r";

         throw std::invalid_argument(
           std::string("base64_decode: invalid base64 character '") +
           bad_char + "'");
         }

      /*
      * If we're at the end of the input, pad with 0s and truncate
      */
      if(final_inputs && (i == input_length - 1))
         {
         if(decode_buf_pos)
            {
            for(size_t i = decode_buf_pos; i != 4; ++i)
               decode_buf[i] = 0;
            final_truncate = (4 - decode_buf_pos);
            decode_buf_pos = 4;
            }
         }

      if(decode_buf_pos == 4)
         {
         out_ptr[0] = (decode_buf[0] << 2) | (decode_buf[1] >> 4);
         out_ptr[1] = (decode_buf[1] << 4) | (decode_buf[2] >> 2);
         out_ptr[2] = (decode_buf[2] << 6) | decode_buf[3];

         out_ptr += 3;
         decode_buf_pos = 0;
         input_consumed = i+1;
         }
      }

   while(input_consumed < input_length &&
         BASE64_TO_BIN[static_cast<byte>(input[input_consumed])] == 0x80)
      {
      ++input_consumed;
      }

   size_t written = (out_ptr - output) - final_truncate;

   return written;
   }

size_t base64_decode(byte output[],
                     const char input[],
                     size_t input_length,
                     bool ignore_ws)
   {
   size_t consumed = 0;
   size_t written = base64_decode(output, input, input_length,
                                  consumed, true, ignore_ws);

   if(consumed != input_length)
      throw std::invalid_argument("base64_decode: input did not have full bytes");

   return written;
   }

size_t base64_decode(byte output[],
                     const std::string& input,
                     bool ignore_ws)
   {
   return base64_decode(output, &input[0], input.length(), ignore_ws);
   }

secure_vector<byte> base64_decode(const char input[],
                                 size_t input_length,
                                 bool ignore_ws)
   {
   secure_vector<byte> bin((round_up<size_t>(input_length, 4) * 3) / 4);

   size_t written = base64_decode(&bin[0],
                                  input,
                                  input_length,
                                  ignore_ws);

   bin.resize(written);
   return bin;
   }

secure_vector<byte> base64_decode(const std::string& input,
                                 bool ignore_ws)
   {
   return base64_decode(&input[0], input.size(), ignore_ws);
   }


}
/*
* Hex Encoding and Decoding
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

void hex_encode(char output[],
                const byte input[],
                size_t input_length,
                bool uppercase)
   {
   static const byte BIN_TO_HEX_UPPER[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'A', 'B', 'C', 'D', 'E', 'F' };

   static const byte BIN_TO_HEX_LOWER[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f' };

   const byte* tbl = uppercase ? BIN_TO_HEX_UPPER : BIN_TO_HEX_LOWER;

   for(size_t i = 0; i != input_length; ++i)
      {
      byte x = input[i];
      output[2*i  ] = tbl[(x >> 4) & 0x0F];
      output[2*i+1] = tbl[(x     ) & 0x0F];
      }
   }

std::string hex_encode(const byte input[],
                       size_t input_length,
                       bool uppercase)
   {
   std::string output(2 * input_length, 0);

   if(input_length)
      hex_encode(&output[0], input, input_length, uppercase);

   return output;
   }

size_t hex_decode(byte output[],
                  const char input[],
                  size_t input_length,
                  size_t& input_consumed,
                  bool ignore_ws)
   {
   /*
   * Mapping of hex characters to either their binary equivalent
   * or to an error code.
   *  If valid hex (0-9 A-F a-f), the value.
   *  If whitespace, then 0x80
   *  Otherwise 0xFF
   * Warning: this table assumes ASCII character encodings
   */

   static const byte HEX_TO_BIN[256] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80,
      0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01,
      0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
      0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C,
      0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

   byte* out_ptr = output;
   bool top_nibble = true;

   clear_mem(output, input_length / 2);

   for(size_t i = 0; i != input_length; ++i)
      {
      const byte bin = HEX_TO_BIN[static_cast<byte>(input[i])];

      if(bin >= 0x10)
         {
         if(bin == 0x80 && ignore_ws)
            continue;

         std::string bad_char(1, input[i]);
         if(bad_char == "\t")
           bad_char = "\\t";
         else if(bad_char == "\n")
           bad_char = "\\n";

         throw std::invalid_argument(
           std::string("hex_decode: invalid hex character '") +
           bad_char + "'");
         }

      *out_ptr |= bin << (top_nibble*4);

      top_nibble = !top_nibble;
      if(top_nibble)
         ++out_ptr;
      }

   input_consumed = input_length;
   size_t written = (out_ptr - output);

   /*
   * We only got half of a byte at the end; zap the half-written
   * output and mark it as unread
   */
   if(!top_nibble)
      {
      *out_ptr = 0;
      input_consumed -= 1;
      }

   return written;
   }

size_t hex_decode(byte output[],
                  const char input[],
                  size_t input_length,
                  bool ignore_ws)
   {
   size_t consumed = 0;
   size_t written = hex_decode(output, input, input_length,
                               consumed, ignore_ws);

   if(consumed != input_length)
      throw std::invalid_argument("hex_decode: input did not have full bytes");

   return written;
   }

size_t hex_decode(byte output[],
                  const std::string& input,
                  bool ignore_ws)
   {
   return hex_decode(output, &input[0], input.length(), ignore_ws);
   }

secure_vector<byte> hex_decode_locked(const char input[],
                                      size_t input_length,
                                      bool ignore_ws)
   {
   secure_vector<byte> bin(1 + input_length / 2);

   size_t written = hex_decode(&bin[0],
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

secure_vector<byte> hex_decode_locked(const std::string& input,
                                      bool ignore_ws)
   {
   return hex_decode_locked(&input[0], input.size(), ignore_ws);
   }

std::vector<byte> hex_decode(const char input[],
                             size_t input_length,
                             bool ignore_ws)
   {
   std::vector<byte> bin(1 + input_length / 2);

   size_t written = hex_decode(&bin[0],
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

std::vector<byte> hex_decode(const std::string& input,
                             bool ignore_ws)
   {
   return hex_decode(&input[0], input.size(), ignore_ws);
   }

}
/*
* PEM Encoding/Decoding
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace PEM_Code {

/*
* PEM encode BER/DER-encoded objects
*/
std::string encode(const byte der[], size_t length, const std::string& label,
                   size_t width)
   {
   const std::string PEM_HEADER = "-----BEGIN " + label + "-----\n";
   const std::string PEM_TRAILER = "-----END " + label + "-----\n";

   Pipe pipe(new Base64_Encoder(true, width));
   pipe.process_msg(der, length);
   return (PEM_HEADER + pipe.read_all_as_string() + PEM_TRAILER);
   }

/*
* Decode PEM down to raw BER/DER
*/
secure_vector<byte> decode_check_label(DataSource& source,
                                      const std::string& label_want)
   {
   std::string label_got;
   secure_vector<byte> ber = decode(source, label_got);
   if(label_got != label_want)
      throw Decoding_Error("PEM: Label mismatch, wanted " + label_want +
                           ", got " + label_got);
   return ber;
   }

/*
* Decode PEM down to raw BER/DER
*/
secure_vector<byte> decode(DataSource& source, std::string& label)
   {
   const size_t RANDOM_CHAR_LIMIT = 8;

   const std::string PEM_HEADER1 = "-----BEGIN ";
   const std::string PEM_HEADER2 = "-----";
   size_t position = 0;

   while(position != PEM_HEADER1.length())
      {
      byte b;
      if(!source.read_byte(b))
         throw Decoding_Error("PEM: No PEM header found");
      if(b == PEM_HEADER1[position])
         ++position;
      else if(position >= RANDOM_CHAR_LIMIT)
         throw Decoding_Error("PEM: Malformed PEM header");
      else
         position = 0;
      }
   position = 0;
   while(position != PEM_HEADER2.length())
      {
      byte b;
      if(!source.read_byte(b))
         throw Decoding_Error("PEM: No PEM header found");
      if(b == PEM_HEADER2[position])
         ++position;
      else if(position)
         throw Decoding_Error("PEM: Malformed PEM header");

      if(position == 0)
         label += static_cast<char>(b);
      }

   Pipe base64(new Base64_Decoder);
   base64.start_msg();

   const std::string PEM_TRAILER = "-----END " + label + "-----";
   position = 0;
   while(position != PEM_TRAILER.length())
      {
      byte b;
      if(!source.read_byte(b))
         throw Decoding_Error("PEM: No PEM trailer found");
      if(b == PEM_TRAILER[position])
         ++position;
      else if(position)
         throw Decoding_Error("PEM: Malformed PEM trailer");

      if(position == 0)
         base64.write(b);
      }
   base64.end_msg();
   return base64.read_all();
   }

secure_vector<byte> decode_check_label(const std::string& pem,
                                      const std::string& label_want)
   {
   DataSource_Memory src(pem);
   return decode_check_label(src, label_want);
   }

secure_vector<byte> decode(const std::string& pem, std::string& label)
   {
   DataSource_Memory src(pem);
   return decode(src, label);
   }

/*
* Search for a PEM signature
*/
bool matches(DataSource& source, const std::string& extra,
             size_t search_range)
   {
   const std::string PEM_HEADER = "-----BEGIN " + extra;

   secure_vector<byte> search_buf(search_range);
   size_t got = source.peek(&search_buf[0], search_buf.size(), 0);

   if(got < PEM_HEADER.length())
      return false;

   size_t index = 0;

   for(size_t j = 0; j != got; ++j)
      {
      if(search_buf[j] == PEM_HEADER[index])
         ++index;
      else
         index = 0;
      if(index == PEM_HEADER.size())
         return true;
      }
   return false;
   }

}

}
/*
* Core Engine
* (C) 1999-2007,2011,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_MODE_CFB)
#endif

#if defined(BOTAN_HAS_MODE_ECB)
#endif

#if defined(BOTAN_HAS_MODE_CBC)
#endif

#if defined(BOTAN_HAS_MODE_XTS)
#endif

#if defined(BOTAN_HAS_OFB)
#endif

#if defined(BOTAN_HAS_CTR_BE)
#endif

#if defined(BOTAN_HAS_AEAD_FILTER)


#if defined(BOTAN_HAS_AEAD_CCM)
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
#endif

#endif

namespace Botan {

namespace {

/**
* Get a block cipher padding method by name
*/
BlockCipherModePaddingMethod* get_bc_pad(const std::string& algo_spec,
                                         const std::string& def_if_empty)
   {
#if defined(BOTAN_HAS_CIPHER_MODE_PADDING)
   if(algo_spec == "NoPadding" || (algo_spec == "" && def_if_empty == "NoPadding"))
      return new Null_Padding;

   if(algo_spec == "PKCS7" || (algo_spec == "" && def_if_empty == "PKCS7"))
      return new PKCS7_Padding;

   if(algo_spec == "OneAndZeros")
      return new OneAndZeros_Padding;

   if(algo_spec == "X9.23")
      return new ANSI_X923_Padding;

#endif

   throw Algorithm_Not_Found(algo_spec);
   }

}

Keyed_Filter* get_cipher_mode(const BlockCipher* block_cipher,
                              Cipher_Dir direction,
                              const std::string& mode,
                              const std::string& padding)
   {
#if defined(BOTAN_HAS_OFB)
   if(mode == "OFB")
      return new StreamCipher_Filter(new OFB(block_cipher->clone()));
#endif

#if defined(BOTAN_HAS_CTR_BE)
   if(mode == "CTR-BE")
      return new StreamCipher_Filter(new CTR_BE(block_cipher->clone()));
#endif

#if defined(BOTAN_HAS_MODE_ECB)
   if(mode == "ECB" || mode == "")
      {
      if(direction == ENCRYPTION)
         return new Transformation_Filter(
            new ECB_Encryption(block_cipher->clone(), get_bc_pad(padding, "NoPadding")));
      else
         return new Transformation_Filter(
            new ECB_Decryption(block_cipher->clone(), get_bc_pad(padding, "NoPadding")));
      }
#endif

   if(mode == "CBC")
      {
#if defined(BOTAN_HAS_MODE_CBC)
      if(padding == "CTS")
         {
         if(direction == ENCRYPTION)
            return new Transformation_Filter(new CTS_Encryption(block_cipher->clone()));
         else
            return new Transformation_Filter(new CTS_Decryption(block_cipher->clone()));
         }

      if(direction == ENCRYPTION)
         return new Transformation_Filter(
            new CBC_Encryption(block_cipher->clone(), get_bc_pad(padding, "PKCS7")));
      else
         return new Transformation_Filter(
            new CBC_Decryption(block_cipher->clone(), get_bc_pad(padding, "PKCS7")));
#else
      return nullptr;
#endif
      }

#if defined(BOTAN_HAS_MODE_XTS)
   if(mode == "XTS")
      {
      if(direction == ENCRYPTION)
         return new Transformation_Filter(new XTS_Encryption(block_cipher->clone()));
      else
         return new Transformation_Filter(new XTS_Decryption(block_cipher->clone()));
      }
#endif

   if(mode.find("CFB") != std::string::npos ||
      mode.find("EAX") != std::string::npos ||
      mode.find("GCM") != std::string::npos ||
      mode.find("OCB") != std::string::npos ||
      mode.find("CCM") != std::string::npos)
      {
      std::vector<std::string> algo_info = parse_algorithm_name(mode);
      const std::string mode_name = algo_info[0];

      size_t bits = 8 * block_cipher->block_size();
      if(algo_info.size() > 1)
         bits = to_u32bit(algo_info[1]);

#if defined(BOTAN_HAS_MODE_CFB)
      if(mode_name == "CFB")
         {
         if(direction == ENCRYPTION)
            return new Transformation_Filter(new CFB_Encryption(block_cipher->clone(), bits));
         else
            return new Transformation_Filter(new CFB_Decryption(block_cipher->clone(), bits));
         }
#endif

      if(bits % 8 != 0)
         throw std::invalid_argument("AEAD interface does not support non-octet length tags");

#if defined(BOTAN_HAS_AEAD_FILTER)

      const size_t tag_size = bits / 8;

#if defined(BOTAN_HAS_AEAD_CCM)
      if(mode_name == "CCM")
         {
         const size_t L = (algo_info.size() == 3) ? to_u32bit(algo_info[2]) : 3;
         if(direction == ENCRYPTION)
            return new AEAD_Filter(new CCM_Encryption(block_cipher->clone(), tag_size, L));
         else
            return new AEAD_Filter(new CCM_Decryption(block_cipher->clone(), tag_size, L));
         }
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
      if(mode_name == "EAX")
         {
         if(direction == ENCRYPTION)
            return new AEAD_Filter(new EAX_Encryption(block_cipher->clone(), tag_size));
         else
            return new AEAD_Filter(new EAX_Decryption(block_cipher->clone(), tag_size));
         }
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
   if(mode_name == "OCB")
      {
      if(direction == ENCRYPTION)
         return new AEAD_Filter(new OCB_Encryption(block_cipher->clone(), tag_size));
      else
         return new AEAD_Filter(new OCB_Decryption(block_cipher->clone(), tag_size));
      }
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
   if(mode_name == "GCM")
      {
      if(direction == ENCRYPTION)
         return new AEAD_Filter(new GCM_Encryption(block_cipher->clone(), tag_size));
      else
         return new AEAD_Filter(new GCM_Decryption(block_cipher->clone(), tag_size));
      }
#endif

#endif
      }

   return nullptr;
   }

/*
* Get a cipher object
*/
Keyed_Filter* Core_Engine::get_cipher(const std::string& algo_spec,
                                      Cipher_Dir direction,
                                      Algorithm_Factory& af)
   {
   std::vector<std::string> algo_parts = split_on(algo_spec, '/');
   if(algo_parts.empty())
      throw Invalid_Algorithm_Name(algo_spec);

   const std::string cipher_name = algo_parts[0];

   // check if it is a stream cipher first (easy case)
   const StreamCipher* stream_cipher = af.prototype_stream_cipher(cipher_name);
   if(stream_cipher)
      return new StreamCipher_Filter(stream_cipher->clone());

   const BlockCipher* block_cipher = af.prototype_block_cipher(cipher_name);
   if(!block_cipher)
      return nullptr;

   if(algo_parts.size() >= 4)
      return nullptr; // 4 part mode, not something we know about

   if(algo_parts.size() < 2)
      throw Lookup_Error("Cipher specification '" + algo_spec +
                         "' is missing mode identifier");

   std::string mode = algo_parts[1];

   std::string padding;
   if(algo_parts.size() == 3)
      padding = algo_parts[2];
   else
      padding = (mode == "CBC") ? "PKCS7" : "NoPadding";

   if(mode == "ECB" && padding == "CTS")
      return nullptr;
   else if((mode != "CBC" && mode != "ECB") && padding != "NoPadding")
      throw Invalid_Algorithm_Name(algo_spec);

   Keyed_Filter* filt = get_cipher_mode(block_cipher, direction, mode, padding);
   if(filt)
      return filt;

   if(padding != "NoPadding")
      throw Algorithm_Not_Found(cipher_name + "/" + mode + "/" + padding);
   else
      throw Algorithm_Not_Found(cipher_name + "/" + mode);
   }

}
/*
* PK Operations
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_RSA)
#endif

#if defined(BOTAN_HAS_RW)
#endif

#if defined(BOTAN_HAS_DSA)
#endif

#if defined(BOTAN_HAS_ECDSA)
#endif

#if defined(BOTAN_HAS_ELGAMAL)
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
#endif

#if defined(BOTAN_HAS_ECDH)
#endif

namespace Botan {

PK_Ops::Encryption*
Core_Engine::get_encryption_op(const Public_Key& key, RandomNumberGenerator&) const
   {
#if defined(BOTAN_HAS_RSA)
   if(const RSA_PublicKey* s = dynamic_cast<const RSA_PublicKey*>(&key))
      return new RSA_Public_Operation(*s);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
   if(const ElGamal_PublicKey* s = dynamic_cast<const ElGamal_PublicKey*>(&key))
      return new ElGamal_Encryption_Operation(*s);
#endif

   return nullptr;
   }

PK_Ops::Decryption*
Core_Engine::get_decryption_op(const Private_Key& key, RandomNumberGenerator& rng) const
   {
#if defined(BOTAN_HAS_RSA)
   if(const RSA_PrivateKey* s = dynamic_cast<const RSA_PrivateKey*>(&key))
      return new RSA_Private_Operation(*s, rng);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
   if(const ElGamal_PrivateKey* s = dynamic_cast<const ElGamal_PrivateKey*>(&key))
      return new ElGamal_Decryption_Operation(*s, rng);
#endif

   return nullptr;
   }

PK_Ops::Key_Agreement*
Core_Engine::get_key_agreement_op(const Private_Key& key, RandomNumberGenerator& rng) const
   {
#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
   if(const DH_PrivateKey* dh = dynamic_cast<const DH_PrivateKey*>(&key))
      return new DH_KA_Operation(*dh, rng);
#endif

#if defined(BOTAN_HAS_ECDH)
   if(const ECDH_PrivateKey* ecdh = dynamic_cast<const ECDH_PrivateKey*>(&key))
      return new ECDH_KA_Operation(*ecdh);
#endif

   return nullptr;
   }

PK_Ops::Signature*
Core_Engine::get_signature_op(const Private_Key& key, RandomNumberGenerator& rng) const
   {
#if defined(BOTAN_HAS_RSA)
   if(const RSA_PrivateKey* s = dynamic_cast<const RSA_PrivateKey*>(&key))
      return new RSA_Private_Operation(*s, rng);
#endif

#if defined(BOTAN_HAS_RW)
   if(const RW_PrivateKey* s = dynamic_cast<const RW_PrivateKey*>(&key))
      return new RW_Signature_Operation(*s);
#endif

#if defined(BOTAN_HAS_DSA)
   if(const DSA_PrivateKey* s = dynamic_cast<const DSA_PrivateKey*>(&key))
      return new DSA_Signature_Operation(*s);
#endif

#if defined(BOTAN_HAS_ECDSA)
   if(const ECDSA_PrivateKey* s = dynamic_cast<const ECDSA_PrivateKey*>(&key))
      return new ECDSA_Signature_Operation(*s);
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
   if(const GOST_3410_PrivateKey* s =
         dynamic_cast<const GOST_3410_PrivateKey*>(&key))
      return new GOST_3410_Signature_Operation(*s);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
   if(const NR_PrivateKey* s = dynamic_cast<const NR_PrivateKey*>(&key))
      return new NR_Signature_Operation(*s);
#endif

   return nullptr;
   }

PK_Ops::Verification*
Core_Engine::get_verify_op(const Public_Key& key, RandomNumberGenerator&) const
   {
#if defined(BOTAN_HAS_RSA)
   if(const RSA_PublicKey* s = dynamic_cast<const RSA_PublicKey*>(&key))
      return new RSA_Public_Operation(*s);
#endif

#if defined(BOTAN_HAS_RW)
   if(const RW_PublicKey* s = dynamic_cast<const RW_PublicKey*>(&key))
      return new RW_Verification_Operation(*s);
#endif

#if defined(BOTAN_HAS_DSA)
   if(const DSA_PublicKey* s = dynamic_cast<const DSA_PublicKey*>(&key))
      return new DSA_Verification_Operation(*s);
#endif

#if defined(BOTAN_HAS_ECDSA)
   if(const ECDSA_PublicKey* s = dynamic_cast<const ECDSA_PublicKey*>(&key))
      return new ECDSA_Verification_Operation(*s);
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
   if(const GOST_3410_PublicKey* s =
         dynamic_cast<const GOST_3410_PublicKey*>(&key))
      return new GOST_3410_Verification_Operation(*s);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
   if(const NR_PublicKey* s = dynamic_cast<const NR_PublicKey*>(&key))
      return new NR_Verification_Operation(*s);
#endif

   return nullptr;
   }

}
/*
* Modular Exponentiation
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Choose a modular exponentation algorithm
*/
Modular_Exponentiator*
Core_Engine::mod_exp(const BigInt& n, Power_Mod::Usage_Hints hints) const
   {
   if(n.is_odd())
      return new Montgomery_Exponentiator(n, hints);
   return new Fixed_Window_Exponentiator(n, hints);
   }

}
/*
* Block Cipher Lookup
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_AES)
#endif

#if defined(BOTAN_HAS_BLOWFISH)
#endif

#if defined(BOTAN_HAS_CAMELLIA)
#endif

#if defined(BOTAN_HAS_CAST)
#endif

#if defined(BOTAN_HAS_CASCADE)
#endif

#if defined(BOTAN_HAS_DES)
#endif

#if defined(BOTAN_HAS_GOST_28147_89)
#endif

#if defined(BOTAN_HAS_IDEA)
#endif

#if defined(BOTAN_HAS_KASUMI)
#endif

#if defined(BOTAN_HAS_LION)
#endif

#if defined(BOTAN_HAS_MARS)
#endif

#if defined(BOTAN_HAS_MISTY1)
#endif

#if defined(BOTAN_HAS_NOEKEON)
#endif

#if defined(BOTAN_HAS_RC2)
#endif

#if defined(BOTAN_HAS_RC5)
#endif

#if defined(BOTAN_HAS_RC6)
#endif

#if defined(BOTAN_HAS_SAFER)
#endif

#if defined(BOTAN_HAS_SEED)
#endif

#if defined(BOTAN_HAS_SERPENT)
#endif

#if defined(BOTAN_HAS_TEA)
#endif

#if defined(BOTAN_HAS_TWOFISH)
#endif

#if defined(BOTAN_HAS_THREEFISH_512)
#endif

#if defined(BOTAN_HAS_XTEA)
#endif

namespace Botan {

/*
* Look for an algorithm with this name
*/
BlockCipher* Core_Engine::find_block_cipher(const SCAN_Name& request,
                                            Algorithm_Factory& af) const
   {

#if defined(BOTAN_HAS_AES)
   if(request.algo_name() == "AES-128")
      return new AES_128;
   if(request.algo_name() == "AES-192")
      return new AES_192;
   if(request.algo_name() == "AES-256")
      return new AES_256;
#endif

#if defined(BOTAN_HAS_BLOWFISH)
   if(request.algo_name() == "Blowfish")
      return new Blowfish;
#endif

#if defined(BOTAN_HAS_CAMELLIA)
   if(request.algo_name() == "Camellia-128")
      return new Camellia_128;
   if(request.algo_name() == "Camellia-192")
      return new Camellia_192;
   if(request.algo_name() == "Camellia-256")
      return new Camellia_256;
#endif

#if defined(BOTAN_HAS_CAST)
   if(request.algo_name() == "CAST-128")
      return new CAST_128;
   if(request.algo_name() == "CAST-256")
      return new CAST_256;
#endif

#if defined(BOTAN_HAS_DES)
   if(request.algo_name() == "DES")
      return new DES;
   if(request.algo_name() == "DESX")
      return new DESX;
   if(request.algo_name() == "TripleDES")
      return new TripleDES;
#endif

#if defined(BOTAN_HAS_GOST_28147_89)
   if(request.algo_name() == "GOST-28147-89")
      return new GOST_28147_89(request.arg(0, "R3411_94_TestParam"));
#endif

#if defined(BOTAN_HAS_IDEA)
   if(request.algo_name() == "IDEA")
      return new IDEA;
#endif

#if defined(BOTAN_HAS_KASUMI)
   if(request.algo_name() == "KASUMI")
      return new KASUMI;
#endif

#if defined(BOTAN_HAS_MARS)
   if(request.algo_name() == "MARS")
      return new MARS;
#endif

#if defined(BOTAN_HAS_MISTY1)
   if(request.algo_name() == "MISTY1")
      return new MISTY1(request.arg_as_integer(0, 8));
#endif

#if defined(BOTAN_HAS_NOEKEON)
   if(request.algo_name() == "Noekeon")
      return new Noekeon;
#endif

#if defined(BOTAN_HAS_RC2)
   if(request.algo_name() == "RC2")
      return new RC2;
#endif

#if defined(BOTAN_HAS_RC5)
   if(request.algo_name() == "RC5")
      return new RC5(request.arg_as_integer(0, 12));
#endif

#if defined(BOTAN_HAS_RC6)
   if(request.algo_name() == "RC6")
      return new RC6;
#endif

#if defined(BOTAN_HAS_SAFER)
   if(request.algo_name() == "SAFER-SK")
      return new SAFER_SK(request.arg_as_integer(0, 10));
#endif

#if defined(BOTAN_HAS_SEED)
   if(request.algo_name() == "SEED")
      return new SEED;
#endif

#if defined(BOTAN_HAS_SERPENT)
   if(request.algo_name() == "Serpent")
      return new Serpent;
#endif

#if defined(BOTAN_HAS_TEA)
   if(request.algo_name() == "TEA")
      return new TEA;
#endif

#if defined(BOTAN_HAS_TWOFISH)
   if(request.algo_name() == "Twofish")
      return new Twofish;
#endif

#if defined(BOTAN_HAS_TWOFISH)
   if(request.algo_name() == "Threefish-512")
      return new Threefish_512;
#endif

#if defined(BOTAN_HAS_XTEA)
   if(request.algo_name() == "XTEA")
      return new XTEA;
#endif

#if defined(BOTAN_HAS_CASCADE)
   if(request.algo_name() == "Cascade" && request.arg_count() == 2)
      {
      const BlockCipher* c1 = af.prototype_block_cipher(request.arg(0));
      const BlockCipher* c2 = af.prototype_block_cipher(request.arg(1));

      if(c1 && c2)
         return new Cascade_Cipher(c1->clone(), c2->clone());
      }
#endif

#if defined(BOTAN_HAS_LION)
   if(request.algo_name() == "Lion" && request.arg_count_between(2, 3))
      {
      const size_t block_size = request.arg_as_integer(2, 1024);

      const HashFunction* hash =
         af.prototype_hash_function(request.arg(0));

      const StreamCipher* stream_cipher =
         af.prototype_stream_cipher(request.arg(1));

      if(!hash || !stream_cipher)
         return nullptr;

      return new Lion(hash->clone(), stream_cipher->clone(), block_size);
      }
#endif

   return nullptr;
   }

}
/*
* Hash Algorithms Lookup
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_ADLER32)
#endif

#if defined(BOTAN_HAS_CRC24)
#endif

#if defined(BOTAN_HAS_CRC32)
#endif

#if defined(BOTAN_HAS_GOST_34_11)
#endif

#if defined(BOTAN_HAS_HAS_160)
#endif

#if defined(BOTAN_HAS_KECCAK)
#endif

#if defined(BOTAN_HAS_MD2)
#endif

#if defined(BOTAN_HAS_MD4)
#endif

#if defined(BOTAN_HAS_MD5)
#endif

#if defined(BOTAN_HAS_RIPEMD_128)
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
#endif

#if defined(BOTAN_HAS_SHA1)
#endif

#if defined(BOTAN_HAS_SHA2_32)
#endif

#if defined(BOTAN_HAS_SHA2_64)
#endif

#if defined(BOTAN_HAS_SKEIN_512)
#endif

#if defined(BOTAN_HAS_TIGER)
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
#endif

#if defined(BOTAN_HAS_COMB4P)
#endif

namespace Botan {

/*
* Look for an algorithm with this name
*/
HashFunction* Core_Engine::find_hash(const SCAN_Name& request,
                                     Algorithm_Factory& af) const
   {
#if defined(BOTAN_HAS_ADLER32)
   if(request.algo_name() == "Adler32")
      return new Adler32;
#endif

#if defined(BOTAN_HAS_CRC24)
   if(request.algo_name() == "CRC24")
      return new CRC24;
#endif

#if defined(BOTAN_HAS_CRC32)
   if(request.algo_name() == "CRC32")
      return new CRC32;
#endif

#if defined(BOTAN_HAS_GOST_34_11)
   if(request.algo_name() == "GOST-R-34.11-94")
      return new GOST_34_11;
#endif

#if defined(BOTAN_HAS_HAS_160)
   if(request.algo_name() == "HAS-160")
      return new HAS_160;
#endif

#if defined(BOTAN_HAS_KECCAK)
   if(request.algo_name() == "Keccak-1600")
      return new Keccak_1600(request.arg_as_integer(0, 512));
#endif

#if defined(BOTAN_HAS_MD2)
   if(request.algo_name() == "MD2")
      return new MD2;
#endif

#if defined(BOTAN_HAS_MD4)
   if(request.algo_name() == "MD4")
      return new MD4;
#endif

#if defined(BOTAN_HAS_MD5)
   if(request.algo_name() == "MD5")
      return new MD5;
#endif

#if defined(BOTAN_HAS_RIPEMD_128)
   if(request.algo_name() == "RIPEMD-128")
      return new RIPEMD_128;
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
   if(request.algo_name() == "RIPEMD-160")
      return new RIPEMD_160;
#endif

#if defined(BOTAN_HAS_SHA1)
   if(request.algo_name() == "SHA-160")
      return new SHA_160;
#endif

#if defined(BOTAN_HAS_SHA2_32)
   if(request.algo_name() == "SHA-224")
      return new SHA_224;
   if(request.algo_name() == "SHA-256")
      return new SHA_256;
#endif

#if defined(BOTAN_HAS_SHA2_64)
   if(request.algo_name() == "SHA-384")
      return new SHA_384;
   if(request.algo_name() == "SHA-512")
      return new SHA_512;
#endif

#if defined(BOTAN_HAS_TIGER)
   if(request.algo_name() == "Tiger")
      return new Tiger(request.arg_as_integer(0, 24), // hash output
                       request.arg_as_integer(1, 3)); // # passes
#endif

#if defined(BOTAN_HAS_SKEIN_512)
   if(request.algo_name() == "Skein-512")
      return new Skein_512(request.arg_as_integer(0, 512),
                           request.arg(1, ""));
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
   if(request.algo_name() == "Whirlpool")
      return new Whirlpool;
#endif

#if defined(BOTAN_HAS_COMB4P)
   if(request.algo_name() == "Comb4P" && request.arg_count() == 2)
      {
      const HashFunction* h1 = af.prototype_hash_function(request.arg(0));
      const HashFunction* h2 = af.prototype_hash_function(request.arg(1));

      if(h1 && h2)
         return new Comb4P(h1->clone(), h2->clone());
      }
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)

   if(request.algo_name() == "Parallel")
      {
      std::vector<const HashFunction*> hash_prototypes;

      /* First pass, just get the prototypes (no memory allocation). Then
         if all were found, replace each prototype with a newly created clone
      */
      for(size_t i = 0; i != request.arg_count(); ++i)
         {
         const HashFunction* hash = af.prototype_hash_function(request.arg(i));
         if(!hash)
            return nullptr;

         hash_prototypes.push_back(hash);
         }

      std::vector<HashFunction*> hashes;
      for(size_t i = 0; i != hash_prototypes.size(); ++i)
         hashes.push_back(hash_prototypes[i]->clone());

      return new Parallel(hashes);
      }

#endif

   return nullptr;
   }

}
/*
* MAC Lookup
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_CBC_MAC)
#endif

#if defined(BOTAN_HAS_CMAC)
#endif

#if defined(BOTAN_HAS_HMAC)
#endif

#if defined(BOTAN_HAS_SSL3_MAC)
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
#endif

namespace Botan {

/*
* Look for an algorithm with this name
*/
MessageAuthenticationCode*
Core_Engine::find_mac(const SCAN_Name& request,
                      Algorithm_Factory& af) const
   {

#if defined(BOTAN_HAS_CBC_MAC)
   if(request.algo_name() == "CBC-MAC" && request.arg_count() == 1)
      return new CBC_MAC(af.make_block_cipher(request.arg(0)));
#endif

#if defined(BOTAN_HAS_CMAC)
   if(request.algo_name() == "CMAC" && request.arg_count() == 1)
      return new CMAC(af.make_block_cipher(request.arg(0)));
#endif

#if defined(BOTAN_HAS_HMAC)
   if(request.algo_name() == "HMAC" && request.arg_count() == 1)
      return new HMAC(af.make_hash_function(request.arg(0)));
#endif

#if defined(BOTAN_HAS_SSL3_MAC)
   if(request.algo_name() == "SSL3-MAC" && request.arg_count() == 1)
      return new SSL3_MAC(af.make_hash_function(request.arg(0)));
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
   if(request.algo_name() == "X9.19-MAC" && request.arg_count() == 0)
      return new ANSI_X919_MAC(af.make_block_cipher("DES"));
#endif

   return nullptr;
   }

}
/*
* PBKDF Lookup
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_PBKDF1)
#endif

#if defined(BOTAN_HAS_PBKDF2)
#endif

namespace Botan {

PBKDF* Core_Engine::find_pbkdf(const SCAN_Name& algo_spec,
                               Algorithm_Factory& af) const
   {
#if defined(BOTAN_HAS_PBKDF1)
   if(algo_spec.algo_name() == "PBKDF1" && algo_spec.arg_count() == 1)
      return new PKCS5_PBKDF1(af.make_hash_function(algo_spec.arg(0)));
#endif

#if defined(BOTAN_HAS_PBKDF2)
   if(algo_spec.algo_name() == "PBKDF2" && algo_spec.arg_count() == 1)
      {
      if(const MessageAuthenticationCode* mac_proto = af.prototype_mac(algo_spec.arg(0)))
         return new PKCS5_PBKDF2(mac_proto->clone());

      return new PKCS5_PBKDF2(af.make_mac("HMAC(" + algo_spec.arg(0) + ")"));
      }
#endif

   return nullptr;
   }

}
/*
* Stream Cipher Lookup
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_OFB)
#endif

#if defined(BOTAN_HAS_CTR_BE)
#endif

#if defined(BOTAN_HAS_RC4)
#endif

#if defined(BOTAN_HAS_CHACHA)
#endif

#if defined(BOTAN_HAS_SALSA20)
#endif

namespace Botan {

/*
* Look for an algorithm with this name
*/
StreamCipher*
Core_Engine::find_stream_cipher(const SCAN_Name& request,
                                Algorithm_Factory& af) const
   {
#if defined(BOTAN_HAS_OFB)
   if(request.algo_name() == "OFB" && request.arg_count() == 1)
      {
      if(auto proto = af.prototype_block_cipher(request.arg(0)))
         return new OFB(proto->clone());
      }
#endif

#if defined(BOTAN_HAS_CTR_BE)
   if(request.algo_name() == "CTR-BE" && request.arg_count() == 1)
      {
      if(auto proto = af.prototype_block_cipher(request.arg(0)))
         return new CTR_BE(proto->clone());
      }
#endif

#if defined(BOTAN_HAS_RC4)
   if(request.algo_name() == "RC4")
      return new RC4(request.arg_as_integer(0, 0));
   if(request.algo_name() == "RC4_drop")
      return new RC4(768);
#endif

#if defined(BOTAN_HAS_CHACHA)
   if(request.algo_name() == "ChaCha")
      return new ChaCha;
#endif

#if defined(BOTAN_HAS_SALSA20)
   if(request.algo_name() == "Salsa20")
      return new Salsa20;
#endif

   return nullptr;
   }

}
/*
* Engine
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

BlockCipher*
Engine::find_block_cipher(const SCAN_Name&,
                          Algorithm_Factory&) const
   {
   return nullptr;
   }

StreamCipher*
Engine::find_stream_cipher(const SCAN_Name&,
                           Algorithm_Factory&) const
   {
   return nullptr;
   }

HashFunction*
Engine::find_hash(const SCAN_Name&,
                  Algorithm_Factory&) const
   {
   return nullptr;
   }

MessageAuthenticationCode*
Engine::find_mac(const SCAN_Name&,
                 Algorithm_Factory&) const
   {
   return nullptr;
   }

PBKDF*
Engine::find_pbkdf(const SCAN_Name&,
                   Algorithm_Factory&) const
   {
   return nullptr;
   }

Modular_Exponentiator*
Engine::mod_exp(const BigInt&,
                Power_Mod::Usage_Hints) const
   {
   return nullptr;
   }

Keyed_Filter* Engine::get_cipher(const std::string&,
                                 Cipher_Dir,
                                 Algorithm_Factory&)
   {
   return nullptr;
   }

PK_Ops::Key_Agreement*
Engine::get_key_agreement_op(const Private_Key&, RandomNumberGenerator&) const
   {
   return nullptr;
   }

PK_Ops::Signature*
Engine::get_signature_op(const Private_Key&, RandomNumberGenerator&) const
   {
   return nullptr;
   }

PK_Ops::Verification*
Engine::get_verify_op(const Public_Key&, RandomNumberGenerator&) const
   {
   return nullptr;
   }

PK_Ops::Encryption*
Engine::get_encryption_op(const Public_Key&, RandomNumberGenerator&) const
   {
   return nullptr;
   }

PK_Ops::Decryption*
Engine::get_decryption_op(const Private_Key&, RandomNumberGenerator&) const
   {
   return nullptr;
   }

}
/*
* SIMD Engine
* (C) 1999-2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_AES_SSSE3)
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
#endif

#if defined(BOTAN_HAS_THREEFISH_512_AVX2)
#endif

#if defined(BOTAN_HAS_NOEKEON_SIMD)
#endif

#if defined(BOTAN_HAS_XTEA_SIMD)
#endif

#if defined(BOTAN_HAS_IDEA_SSE2)
#endif

#if defined(BOTAN_HAS_SHA1_SSE2)
#endif

namespace Botan {

BlockCipher*
SIMD_Engine::find_block_cipher(const SCAN_Name& request,
                               Algorithm_Factory&) const
   {
#if defined(BOTAN_HAS_AES_SSSE3)
   if(request.algo_name() == "AES-128" && CPUID::has_ssse3())
      return new AES_128_SSSE3;
   if(request.algo_name() == "AES-192" && CPUID::has_ssse3())
      return new AES_192_SSSE3;
   if(request.algo_name() == "AES-256" && CPUID::has_ssse3())
      return new AES_256_SSSE3;
#endif

#if defined(BOTAN_HAS_IDEA_SSE2)
   if(request.algo_name() == "IDEA" && CPUID::has_sse2())
      return new IDEA_SSE2;
#endif

#if defined(BOTAN_HAS_NOEKEON_SIMD)
   if(request.algo_name() == "Noekeon" && SIMD_32::enabled())
      return new Noekeon_SIMD;
#endif

#if defined(BOTAN_HAS_THREEFISH_512_AVX2)
   if(request.algo_name() == "Threefish-512" && CPUID::has_avx2())
      return new Threefish_512_AVX2;
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
   if(request.algo_name() == "Serpent" && SIMD_32::enabled())
      return new Serpent_SIMD;
#endif

#if defined(BOTAN_HAS_XTEA_SIMD)
   if(request.algo_name() == "XTEA" && SIMD_32::enabled())
      return new XTEA_SIMD;
#endif

   return nullptr;
   }

HashFunction*
SIMD_Engine::find_hash(const SCAN_Name& request,
                       Algorithm_Factory&) const
   {
#if defined(BOTAN_HAS_SHA1_SSE2)
   if(request.algo_name() == "SHA-160" && CPUID::has_sse2())
      return new SHA_160_SSE2;
#endif

   BOTAN_UNUSED(request);

   return nullptr;
   }

}
/*
* Reader of /dev/random and company
* (C) 1999-2009,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace Botan {

/**
Device_EntropySource constructor
Open a file descriptor to each (available) device in fsnames
*/
Device_EntropySource::Device_EntropySource(const std::vector<std::string>& fsnames)
   {
#ifndef O_NONBLOCK
  #define O_NONBLOCK 0
#endif

#ifndef O_NOCTTY
  #define O_NOCTTY 0
#endif

   const int flags = O_RDONLY | O_NONBLOCK | O_NOCTTY;

   for(auto fsname : fsnames)
      {
      fd_type fd = ::open(fsname.c_str(), flags);

      if(fd >= 0 && fd < FD_SETSIZE)
         m_devices.push_back(fd);
      else if(fd >= 0)
         ::close(fd);
      }
   }

/**
Device_EntropySource destructor: close all open devices
*/
Device_EntropySource::~Device_EntropySource()
   {
   for(size_t i = 0; i != m_devices.size(); ++i)
      ::close(m_devices[i]);
   }

/**
* Gather entropy from a RNG device
*/
void Device_EntropySource::poll(Entropy_Accumulator& accum)
   {
   if(m_devices.empty())
      return;

   const size_t ENTROPY_BITS_PER_BYTE = 8;
   const size_t MS_WAIT_TIME = 32;
   const size_t READ_ATTEMPT = 32;

   int max_fd = m_devices[0];
   fd_set read_set;
   FD_ZERO(&read_set);
   for(size_t i = 0; i != m_devices.size(); ++i)
      {
      FD_SET(m_devices[i], &read_set);
      max_fd = std::max(m_devices[i], max_fd);
      }

   struct ::timeval timeout;

   timeout.tv_sec = (MS_WAIT_TIME / 1000);
   timeout.tv_usec = (MS_WAIT_TIME % 1000) * 1000;

   if(::select(max_fd + 1, &read_set, nullptr, nullptr, &timeout) < 0)
      return;

   secure_vector<byte>& io_buffer = accum.get_io_buffer(READ_ATTEMPT);

   for(size_t i = 0; i != m_devices.size(); ++i)
      {
      if(FD_ISSET(m_devices[i], &read_set))
         {
         const ssize_t got = ::read(m_devices[i], &io_buffer[0], io_buffer.size());
         accum.add(&io_buffer[0], got, ENTROPY_BITS_PER_BYTE);
         }
      }
   }

}
/*
* Filters
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* StreamCipher_Filter Constructor
*/
StreamCipher_Filter::StreamCipher_Filter(StreamCipher* stream_cipher) :
   buffer(DEFAULT_BUFFERSIZE)
   {
   cipher = stream_cipher;
   }

/*
* StreamCipher_Filter Constructor
*/
StreamCipher_Filter::StreamCipher_Filter(StreamCipher* stream_cipher,
                                         const SymmetricKey& key) :
   buffer(DEFAULT_BUFFERSIZE)
   {
   cipher = stream_cipher;
   cipher->set_key(key);
   }

/*
* StreamCipher_Filter Constructor
*/
StreamCipher_Filter::StreamCipher_Filter(const std::string& sc_name) :
   buffer(DEFAULT_BUFFERSIZE)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   cipher = af.make_stream_cipher(sc_name);
   }

/*
* StreamCipher_Filter Constructor
*/
StreamCipher_Filter::StreamCipher_Filter(const std::string& sc_name,
                                         const SymmetricKey& key) :
   buffer(DEFAULT_BUFFERSIZE)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   cipher = af.make_stream_cipher(sc_name);
   cipher->set_key(key);
   }

/*
* Set the IV of a stream cipher
*/
void StreamCipher_Filter::set_iv(const InitializationVector& iv)
   {
   cipher->set_iv(iv.begin(), iv.length());
   }

/*
* Write data into a StreamCipher_Filter
*/
void StreamCipher_Filter::write(const byte input[], size_t length)
   {
   while(length)
      {
      size_t copied = std::min<size_t>(length, buffer.size());
      cipher->cipher(input, &buffer[0], copied);
      send(buffer, copied);
      input += copied;
      length -= copied;
      }
   }

/*
* Hash_Filter Constructor
*/
Hash_Filter::Hash_Filter(const std::string& algo_spec,
                         size_t len) :
   OUTPUT_LENGTH(len)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   hash = af.make_hash_function(algo_spec);
   }

/*
* Complete a calculation by a Hash_Filter
*/
void Hash_Filter::end_msg()
   {
   secure_vector<byte> output = hash->final();
   if(OUTPUT_LENGTH)
      send(output, std::min<size_t>(OUTPUT_LENGTH, output.size()));
   else
      send(output);
   }

/*
* MAC_Filter Constructor
*/
MAC_Filter::MAC_Filter(const std::string& mac_name, size_t len) :
   OUTPUT_LENGTH(len)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   mac = af.make_mac(mac_name);
   }

/*
* MAC_Filter Constructor
*/
MAC_Filter::MAC_Filter(const std::string& mac_name, const SymmetricKey& key,
                       size_t len) : OUTPUT_LENGTH(len)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   mac = af.make_mac(mac_name);
   mac->set_key(key);
   }

/*
* Complete a calculation by a MAC_Filter
*/
void MAC_Filter::end_msg()
   {
   secure_vector<byte> output = mac->final();
   if(OUTPUT_LENGTH)
      send(output, std::min<size_t>(OUTPUT_LENGTH, output.size()));
   else
      send(output);
   }

}
/*
* Basic Filters
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

void Keyed_Filter::set_iv(const InitializationVector& iv)
   {
   if(iv.length() != 0)
      throw Invalid_IV_Length(name(), iv.length());
   }

/*
* Chain Constructor
*/
Chain::Chain(Filter* f1, Filter* f2, Filter* f3, Filter* f4)
   {
   if(f1) { attach(f1); incr_owns(); }
   if(f2) { attach(f2); incr_owns(); }
   if(f3) { attach(f3); incr_owns(); }
   if(f4) { attach(f4); incr_owns(); }
   }

/*
* Chain Constructor
*/
Chain::Chain(Filter* filters[], size_t count)
   {
   for(size_t j = 0; j != count; ++j)
      if(filters[j])
         {
         attach(filters[j]);
         incr_owns();
         }
   }

std::string Chain::name() const
   {
   return "Chain";
   }

/*
* Fork Constructor
*/
Fork::Fork(Filter* f1, Filter* f2, Filter* f3, Filter* f4)
   {
   Filter* filters[4] = { f1, f2, f3, f4 };
   set_next(filters, 4);
   }

/*
* Fork Constructor
*/
Fork::Fork(Filter* filters[], size_t count)
   {
   set_next(filters, count);
   }

std::string Fork::name() const
   {
   return "Fork";
   }

}
/*
* Buffered Filter
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Buffered_Filter Constructor
*/
Buffered_Filter::Buffered_Filter(size_t b, size_t f) :
   main_block_mod(b), final_minimum(f)
   {
   if(main_block_mod == 0)
      throw std::invalid_argument("main_block_mod == 0");

   if(final_minimum > main_block_mod)
      throw std::invalid_argument("final_minimum > main_block_mod");

   buffer.resize(2 * main_block_mod);
   buffer_pos = 0;
   }

/*
* Buffer input into blocks, trying to minimize copying
*/
void Buffered_Filter::write(const byte input[], size_t input_size)
   {
   if(!input_size)
      return;

   if(buffer_pos + input_size >= main_block_mod + final_minimum)
      {
      size_t to_copy = std::min<size_t>(buffer.size() - buffer_pos, input_size);

      copy_mem(&buffer[buffer_pos], input, to_copy);
      buffer_pos += to_copy;

      input += to_copy;
      input_size -= to_copy;

      size_t total_to_consume =
         round_down(std::min(buffer_pos,
                             buffer_pos + input_size - final_minimum),
                    main_block_mod);

      buffered_block(&buffer[0], total_to_consume);

      buffer_pos -= total_to_consume;

      copy_mem(&buffer[0], &buffer[total_to_consume], buffer_pos);
      }

   if(input_size >= final_minimum)
      {
      size_t full_blocks = (input_size - final_minimum) / main_block_mod;
      size_t to_copy = full_blocks * main_block_mod;

      if(to_copy)
         {
         buffered_block(input, to_copy);

         input += to_copy;
         input_size -= to_copy;
         }
      }

   copy_mem(&buffer[buffer_pos], input, input_size);
   buffer_pos += input_size;
   }

/*
* Finish/flush operation
*/
void Buffered_Filter::end_msg()
   {
   if(buffer_pos < final_minimum)
      throw std::runtime_error("Buffered filter end_msg without enough input");

   size_t spare_blocks = (buffer_pos - final_minimum) / main_block_mod;

   if(spare_blocks)
      {
      size_t spare_bytes = main_block_mod * spare_blocks;
      buffered_block(&buffer[0], spare_bytes);
      buffered_final(&buffer[spare_bytes], buffer_pos - spare_bytes);
      }
   else
      {
      buffered_final(&buffer[0], buffer_pos);
      }

   buffer_pos = 0;
   }

}
/*
* Base64 Encoder/Decoder
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Base64_Encoder Constructor
*/
Base64_Encoder::Base64_Encoder(bool breaks, size_t length, bool t_n) :
   line_length(breaks ? length : 0),
   trailing_newline(t_n && breaks),
   in(48),
   out(64),
   position(0),
   out_position(0)
   {
   }

/*
* Encode and send a block
*/
void Base64_Encoder::encode_and_send(const byte input[], size_t length,
                                     bool final_inputs)
   {
   while(length)
      {
      const size_t proc = std::min(length, in.size());

      size_t consumed = 0;
      size_t produced = base64_encode(reinterpret_cast<char*>(&out[0]), input,
                                      proc, consumed, final_inputs);

      do_output(&out[0], produced);

      // FIXME: s/proc/consumed/?
      input += proc;
      length -= proc;
      }
   }

/*
* Handle the output
*/
void Base64_Encoder::do_output(const byte input[], size_t length)
   {
   if(line_length == 0)
      send(input, length);
   else
      {
      size_t remaining = length, offset = 0;
      while(remaining)
         {
         size_t sent = std::min(line_length - out_position, remaining);
         send(input + offset, sent);
         out_position += sent;
         remaining -= sent;
         offset += sent;
         if(out_position == line_length)
            {
            send('\n');
            out_position = 0;
            }
         }
      }
   }

/*
* Convert some data into Base64
*/
void Base64_Encoder::write(const byte input[], size_t length)
   {
   buffer_insert(in, position, input, length);
   if(position + length >= in.size())
      {
      encode_and_send(&in[0], in.size());
      input += (in.size() - position);
      length -= (in.size() - position);
      while(length >= in.size())
         {
         encode_and_send(input, in.size());
         input += in.size();
         length -= in.size();
         }
      copy_mem(&in[0], input, length);
      position = 0;
      }
   position += length;
   }

/*
* Flush buffers
*/
void Base64_Encoder::end_msg()
   {
   encode_and_send(&in[0], position, true);

   if(trailing_newline || (out_position && line_length))
      send('\n');

   out_position = position = 0;
   }

/*
* Base64_Decoder Constructor
*/
Base64_Decoder::Base64_Decoder(Decoder_Checking c) :
   checking(c), in(64), out(48), position(0)
   {
   }

/*
* Convert some data from Base64
*/
void Base64_Decoder::write(const byte input[], size_t length)
   {
   while(length)
      {
      size_t to_copy = std::min<size_t>(length, in.size() - position);
      copy_mem(&in[position], input, to_copy);
      position += to_copy;

      size_t consumed = 0;
      size_t written = base64_decode(&out[0],
                                     reinterpret_cast<const char*>(&in[0]),
                                     position,
                                     consumed,
                                     false,
                                     checking != FULL_CHECK);

      send(out, written);

      if(consumed != position)
         {
         copy_mem(&in[0], &in[consumed], position - consumed);
         position = position - consumed;
         }
      else
         position = 0;

      length -= to_copy;
      input += to_copy;
      }
   }

/*
* Flush buffers
*/
void Base64_Decoder::end_msg()
   {
   size_t consumed = 0;
   size_t written = base64_decode(&out[0],
                                  reinterpret_cast<const char*>(&in[0]),
                                  position,
                                  consumed,
                                  true,
                                  checking != FULL_CHECK);

   send(out, written);

   const bool not_full_bytes = consumed != position;

   position = 0;

   if(not_full_bytes)
      throw std::invalid_argument("Base64_Decoder: Input not full bytes");
   }

}
/*
* Hex Encoder/Decoder
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/**
* Size used for internal buffer in hex encoder/decoder
*/
const size_t HEX_CODEC_BUFFER_SIZE = 256;

/*
* Hex_Encoder Constructor
*/
Hex_Encoder::Hex_Encoder(bool breaks, size_t length, Case c) :
   casing(c), line_length(breaks ? length : 0)
   {
   in.resize(HEX_CODEC_BUFFER_SIZE);
   out.resize(2*in.size());
   counter = position = 0;
   }

/*
* Hex_Encoder Constructor
*/
Hex_Encoder::Hex_Encoder(Case c) : casing(c), line_length(0)
   {
   in.resize(HEX_CODEC_BUFFER_SIZE);
   out.resize(2*in.size());
   counter = position = 0;
   }

/*
* Encode and send a block
*/
void Hex_Encoder::encode_and_send(const byte block[], size_t length)
   {
   hex_encode(reinterpret_cast<char*>(&out[0]),
              block, length,
              casing == Uppercase);

   if(line_length == 0)
      send(out, 2*length);
   else
      {
      size_t remaining = 2*length, offset = 0;
      while(remaining)
         {
         size_t sent = std::min(line_length - counter, remaining);
         send(&out[offset], sent);
         counter += sent;
         remaining -= sent;
         offset += sent;
         if(counter == line_length)
            {
            send('\n');
            counter = 0;
            }
         }
      }
   }

/*
* Convert some data into hex format
*/
void Hex_Encoder::write(const byte input[], size_t length)
   {
   buffer_insert(in, position, input, length);
   if(position + length >= in.size())
      {
      encode_and_send(&in[0], in.size());
      input += (in.size() - position);
      length -= (in.size() - position);
      while(length >= in.size())
         {
         encode_and_send(input, in.size());
         input += in.size();
         length -= in.size();
         }
      copy_mem(&in[0], input, length);
      position = 0;
      }
   position += length;
   }

/*
* Flush buffers
*/
void Hex_Encoder::end_msg()
   {
   encode_and_send(&in[0], position);
   if(counter && line_length)
      send('\n');
   counter = position = 0;
   }

/*
* Hex_Decoder Constructor
*/
Hex_Decoder::Hex_Decoder(Decoder_Checking c) : checking(c)
   {
   in.resize(HEX_CODEC_BUFFER_SIZE);
   out.resize(in.size() / 2);
   position = 0;
   }

/*
* Convert some data from hex format
*/
void Hex_Decoder::write(const byte input[], size_t length)
   {
   while(length)
      {
      size_t to_copy = std::min<size_t>(length, in.size() - position);
      copy_mem(&in[position], input, to_copy);
      position += to_copy;

      size_t consumed = 0;
      size_t written = hex_decode(&out[0],
                                  reinterpret_cast<const char*>(&in[0]),
                                  position,
                                  consumed,
                                  checking != FULL_CHECK);

      send(out, written);

      if(consumed != position)
         {
         copy_mem(&in[0], &in[consumed], position - consumed);
         position = position - consumed;
         }
      else
         position = 0;

      length -= to_copy;
      input += to_copy;
      }
   }

/*
* Flush buffers
*/
void Hex_Decoder::end_msg()
   {
   size_t consumed = 0;
   size_t written = hex_decode(&out[0],
                               reinterpret_cast<const char*>(&in[0]),
                               position,
                               consumed,
                               checking != FULL_CHECK);

   send(out, written);

   const bool not_full_bytes = consumed != position;

   position = 0;

   if(not_full_bytes)
      throw std::invalid_argument("Hex_Decoder: Input not full bytes");
   }

}
/*
* DataSink
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Distributed under the terms of the Botan license
*/

#include <fstream>

namespace Botan {

/*
* Write to a stream
*/
void DataSink_Stream::write(const byte out[], size_t length)
   {
   sink.write(reinterpret_cast<const char*>(out), length);
   if(!sink.good())
      throw Stream_IO_Error("DataSink_Stream: Failure writing to " +
                            identifier);
   }

/*
* DataSink_Stream Constructor
*/
DataSink_Stream::DataSink_Stream(std::ostream& out,
                                 const std::string& name) :
   identifier(name),
   sink_p(nullptr),
   sink(out)
   {
   }

/*
* DataSink_Stream Constructor
*/
DataSink_Stream::DataSink_Stream(const std::string& path,
                                 bool use_binary) :
   identifier(path),
   sink_p(new std::ofstream(
             path.c_str(),
             use_binary ? std::ios::binary : std::ios::out)),
   sink(*sink_p)
   {
   if(!sink.good())
      {
      delete sink_p;
      throw Stream_IO_Error("DataSink_Stream: Failure opening " + path);
      }
   }

/*
* DataSink_Stream Destructor
*/
DataSink_Stream::~DataSink_Stream()
   {
   delete sink_p;
   }

}
/*
* DataSource
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Read a single byte from the DataSource
*/
size_t DataSource::read_byte(byte& out)
   {
   return read(&out, 1);
   }

/*
* Peek a single byte from the DataSource
*/
size_t DataSource::peek_byte(byte& out) const
   {
   return peek(&out, 1, 0);
   }

/*
* Discard the next N bytes of the data
*/
size_t DataSource::discard_next(size_t n)
   {
   size_t discarded = 0;
   byte dummy;
   for(size_t j = 0; j != n; ++j)
      discarded += read_byte(dummy);
   return discarded;
   }

/*
* Read from a memory buffer
*/
size_t DataSource_Memory::read(byte out[], size_t length)
   {
   size_t got = std::min<size_t>(source.size() - offset, length);
   copy_mem(out, &source[offset], got);
   offset += got;
   return got;
   }

/*
* Peek into a memory buffer
*/
size_t DataSource_Memory::peek(byte out[], size_t length,
                               size_t peek_offset) const
   {
   const size_t bytes_left = source.size() - offset;
   if(peek_offset >= bytes_left) return 0;

   size_t got = std::min(bytes_left - peek_offset, length);
   copy_mem(out, &source[offset + peek_offset], got);
   return got;
   }

/*
* Check if the memory buffer is empty
*/
bool DataSource_Memory::end_of_data() const
   {
   return (offset == source.size());
   }

/*
* DataSource_Memory Constructor
*/
DataSource_Memory::DataSource_Memory(const std::string& in) :
   source(reinterpret_cast<const byte*>(in.data()),
          reinterpret_cast<const byte*>(in.data()) + in.length()),
   offset(0)
   {
   offset = 0;
   }

/*
* Read from a stream
*/
size_t DataSource_Stream::read(byte out[], size_t length)
   {
   source.read(reinterpret_cast<char*>(out), length);
   if(source.bad())
      throw Stream_IO_Error("DataSource_Stream::read: Source failure");

   size_t got = source.gcount();
   total_read += got;
   return got;
   }

/*
* Peek into a stream
*/
size_t DataSource_Stream::peek(byte out[], size_t length, size_t offset) const
   {
   if(end_of_data())
      throw Invalid_State("DataSource_Stream: Cannot peek when out of data");

   size_t got = 0;

   if(offset)
      {
      secure_vector<byte> buf(offset);
      source.read(reinterpret_cast<char*>(&buf[0]), buf.size());
      if(source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = source.gcount();
      }

   if(got == offset)
      {
      source.read(reinterpret_cast<char*>(out), length);
      if(source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = source.gcount();
      }

   if(source.eof())
      source.clear();
   source.seekg(total_read, std::ios::beg);

   return got;
   }

/*
* Check if the stream is empty or in error
*/
bool DataSource_Stream::end_of_data() const
   {
   return (!source.good());
   }

/*
* Return a human-readable ID for this stream
*/
std::string DataSource_Stream::id() const
   {
   return identifier;
   }

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(const std::string& path,
                                     bool use_binary) :
   identifier(path),
   source_p(new std::ifstream(
               path.c_str(),
               use_binary ? std::ios::binary : std::ios::in)),
   source(*source_p),
   total_read(0)
   {
   if(!source.good())
      {
      delete source_p;
      throw Stream_IO_Error("DataSource: Failure opening file " + path);
      }
   }

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(std::istream& in,
                                     const std::string& name) :
   identifier(name),
   source_p(nullptr),
   source(in),
   total_read(0)
   {
   }

/*
* DataSource_Stream Destructor
*/
DataSource_Stream::~DataSource_Stream()
   {
   delete source_p;
   }

}
/*
* Filter
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Filter Constructor
*/
Filter::Filter()
   {
   next.resize(1);
   port_num = 0;
   filter_owns = 0;
   owned = false;
   }

/*
* Send data to all ports
*/
void Filter::send(const byte input[], size_t length)
   {
   if(!length)
      return;

   bool nothing_attached = true;
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         {
         if(write_queue.size())
            next[j]->write(&write_queue[0], write_queue.size());
         next[j]->write(input, length);
         nothing_attached = false;
         }

   if(nothing_attached)
      write_queue += std::make_pair(input, length);
   else
      write_queue.clear();
   }

/*
* Start a new message
*/
void Filter::new_msg()
   {
   start_msg();
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         next[j]->new_msg();
   }

/*
* End the current message
*/
void Filter::finish_msg()
   {
   end_msg();
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         next[j]->finish_msg();
   }

/*
* Attach a filter to the current port
*/
void Filter::attach(Filter* new_filter)
   {
   if(new_filter)
      {
      Filter* last = this;
      while(last->get_next())
         last = last->get_next();
      last->next[last->current_port()] = new_filter;
      }
   }

/*
* Set the active port on a filter
*/
void Filter::set_port(size_t new_port)
   {
   if(new_port >= total_ports())
      throw Invalid_Argument("Filter: Invalid port number");
   port_num = new_port;
   }

/*
* Return the next Filter in the logical chain
*/
Filter* Filter::get_next() const
   {
   if(port_num < next.size())
      return next[port_num];
   return nullptr;
   }

/*
* Set the next Filters
*/
void Filter::set_next(Filter* filters[], size_t size)
   {
   next.clear();

   port_num = 0;
   filter_owns = 0;

   while(size && filters && (filters[size-1] == nullptr))
      --size;

   if(filters && size)
      next.assign(filters, filters + size);
   }

/*
* Return the total number of ports
*/
size_t Filter::total_ports() const
   {
   return next.size();
   }

}
/*
* Pipe Output Buffer
* (C) 1999-2007,2011 Jack Lloyd
*     2012 Markus Wanner
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Read data from a message
*/
size_t Output_Buffers::read(byte output[], size_t length,
                            Pipe::message_id msg)
   {
   SecureQueue* q = get(msg);
   if(q)
      return q->read(output, length);
   return 0;
   }

/*
* Peek at data in a message
*/
size_t Output_Buffers::peek(byte output[], size_t length,
                            size_t stream_offset,
                            Pipe::message_id msg) const
   {
   SecureQueue* q = get(msg);
   if(q)
      return q->peek(output, length, stream_offset);
   return 0;
   }

/*
* Check available bytes in a message
*/
size_t Output_Buffers::remaining(Pipe::message_id msg) const
   {
   SecureQueue* q = get(msg);
   if(q)
      return q->size();
   return 0;
   }

/*
* Return the total bytes of a message that have already been read.
*/
size_t Output_Buffers::get_bytes_read(Pipe::message_id msg) const
   {
   SecureQueue* q = get(msg);
   if (q)
      return q->get_bytes_read();
   return 0;
   }

/*
* Add a new output queue
*/
void Output_Buffers::add(SecureQueue* queue)
   {
   BOTAN_ASSERT(queue, "queue was provided");

   BOTAN_ASSERT(buffers.size() < buffers.max_size(),
                "Room was available in container");

   buffers.push_back(queue);
   }

/*
* Retire old output queues
*/
void Output_Buffers::retire()
   {
   for(size_t i = 0; i != buffers.size(); ++i)
      if(buffers[i] && buffers[i]->size() == 0)
         {
         delete buffers[i];
         buffers[i] = nullptr;
         }

   while(buffers.size() && !buffers[0])
      {
      buffers.pop_front();
      offset = offset + Pipe::message_id(1);
      }
   }

/*
* Get a particular output queue
*/
SecureQueue* Output_Buffers::get(Pipe::message_id msg) const
   {
   if(msg < offset)
      return nullptr;

   BOTAN_ASSERT(msg < message_count(), "Message number is in range");

   return buffers[msg-offset];
   }

/*
* Return the total number of messages
*/
Pipe::message_id Output_Buffers::message_count() const
   {
   return (offset + buffers.size());
   }

/*
* Output_Buffers Constructor
*/
Output_Buffers::Output_Buffers()
   {
   offset = 0;
   }

/*
* Output_Buffers Destructor
*/
Output_Buffers::~Output_Buffers()
   {
   for(size_t j = 0; j != buffers.size(); ++j)
      delete buffers[j];
   }

}
/*
* Pipe
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* A Filter that does nothing
*/
class Null_Filter : public Filter
   {
   public:
      void write(const byte input[], size_t length)
         { send(input, length); }

      std::string name() const { return "Null"; }
   };

}

/*
* Pipe Constructor
*/
Pipe::Pipe(Filter* f1, Filter* f2, Filter* f3, Filter* f4)
   {
   init();
   append(f1);
   append(f2);
   append(f3);
   append(f4);
   }

/*
* Pipe Constructor
*/
Pipe::Pipe(std::initializer_list<Filter*> args)
   {
   init();

   for(auto i = args.begin(); i != args.end(); ++i)
      append(*i);
   }

/*
* Pipe Destructor
*/
Pipe::~Pipe()
   {
   destruct(pipe);
   delete outputs;
   }

/*
* Initialize the Pipe
*/
void Pipe::init()
   {
   outputs = new Output_Buffers;
   pipe = nullptr;
   default_read = 0;
   inside_msg = false;
   }

/*
* Reset the Pipe
*/
void Pipe::reset()
   {
   destruct(pipe);
   pipe = nullptr;
   inside_msg = false;
   }

/*
* Destroy the Pipe
*/
void Pipe::destruct(Filter* to_kill)
   {
   if(!to_kill || dynamic_cast<SecureQueue*>(to_kill))
      return;
   for(size_t j = 0; j != to_kill->total_ports(); ++j)
      destruct(to_kill->next[j]);
   delete to_kill;
   }

/*
* Test if the Pipe has any data in it
*/
bool Pipe::end_of_data() const
   {
   return (remaining() == 0);
   }

/*
* Set the default read message
*/
void Pipe::set_default_msg(message_id msg)
   {
   if(msg >= message_count())
      throw Invalid_Argument("Pipe::set_default_msg: msg number is too high");
   default_read = msg;
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(const byte input[], size_t length)
   {
   start_msg();
   write(input, length);
   end_msg();
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(const secure_vector<byte>& input)
   {
   process_msg(&input[0], input.size());
   }

void Pipe::process_msg(const std::vector<byte>& input)
   {
   process_msg(&input[0], input.size());
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(const std::string& input)
   {
   process_msg(reinterpret_cast<const byte*>(input.data()), input.length());
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(DataSource& input)
   {
   start_msg();
   write(input);
   end_msg();
   }

/*
* Start a new message
*/
void Pipe::start_msg()
   {
   if(inside_msg)
      throw Invalid_State("Pipe::start_msg: Message was already started");
   if(pipe == nullptr)
      pipe = new Null_Filter;
   find_endpoints(pipe);
   pipe->new_msg();
   inside_msg = true;
   }

/*
* End the current message
*/
void Pipe::end_msg()
   {
   if(!inside_msg)
      throw Invalid_State("Pipe::end_msg: Message was already ended");
   pipe->finish_msg();
   clear_endpoints(pipe);
   if(dynamic_cast<Null_Filter*>(pipe))
      {
      delete pipe;
      pipe = nullptr;
      }
   inside_msg = false;

   outputs->retire();
   }

/*
* Find the endpoints of the Pipe
*/
void Pipe::find_endpoints(Filter* f)
   {
   for(size_t j = 0; j != f->total_ports(); ++j)
      if(f->next[j] && !dynamic_cast<SecureQueue*>(f->next[j]))
         find_endpoints(f->next[j]);
      else
         {
         SecureQueue* q = new SecureQueue;
         f->next[j] = q;
         outputs->add(q);
         }
   }

/*
* Remove the SecureQueues attached to the Filter
*/
void Pipe::clear_endpoints(Filter* f)
   {
   if(!f) return;
   for(size_t j = 0; j != f->total_ports(); ++j)
      {
      if(f->next[j] && dynamic_cast<SecureQueue*>(f->next[j]))
         f->next[j] = nullptr;
      clear_endpoints(f->next[j]);
      }
   }

/*
* Append a Filter to the Pipe
*/
void Pipe::append(Filter* filter)
   {
   if(inside_msg)
      throw Invalid_State("Cannot append to a Pipe while it is processing");
   if(!filter)
      return;
   if(dynamic_cast<SecureQueue*>(filter))
      throw Invalid_Argument("Pipe::append: SecureQueue cannot be used");
   if(filter->owned)
      throw Invalid_Argument("Filters cannot be shared among multiple Pipes");

   filter->owned = true;

   if(!pipe) pipe = filter;
   else      pipe->attach(filter);
   }

/*
* Prepend a Filter to the Pipe
*/
void Pipe::prepend(Filter* filter)
   {
   if(inside_msg)
      throw Invalid_State("Cannot prepend to a Pipe while it is processing");
   if(!filter)
      return;
   if(dynamic_cast<SecureQueue*>(filter))
      throw Invalid_Argument("Pipe::prepend: SecureQueue cannot be used");
   if(filter->owned)
      throw Invalid_Argument("Filters cannot be shared among multiple Pipes");

   filter->owned = true;

   if(pipe) filter->attach(pipe);
   pipe = filter;
   }

/*
* Pop a Filter off the Pipe
*/
void Pipe::pop()
   {
   if(inside_msg)
      throw Invalid_State("Cannot pop off a Pipe while it is processing");

   if(!pipe)
      return;

   if(pipe->total_ports() > 1)
      throw Invalid_State("Cannot pop off a Filter with multiple ports");

   Filter* f = pipe;
   size_t owns = f->owns();
   pipe = pipe->next[0];
   delete f;

   while(owns--)
      {
      f = pipe;
      pipe = pipe->next[0];
      delete f;
      }
   }

/*
* Return the number of messages in this Pipe
*/
Pipe::message_id Pipe::message_count() const
   {
   return outputs->message_count();
   }

/*
* Static Member Variables
*/
const Pipe::message_id Pipe::LAST_MESSAGE =
   static_cast<Pipe::message_id>(-2);

const Pipe::message_id Pipe::DEFAULT_MESSAGE =
   static_cast<Pipe::message_id>(-1);

}
/*
* Pipe I/O
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <iostream>

namespace Botan {

/*
* Write data from a pipe into an ostream
*/
std::ostream& operator<<(std::ostream& stream, Pipe& pipe)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   while(stream.good() && pipe.remaining())
      {
      size_t got = pipe.read(&buffer[0], buffer.size());
      stream.write(reinterpret_cast<const char*>(&buffer[0]), got);
      }
   if(!stream.good())
      throw Stream_IO_Error("Pipe output operator (iostream) has failed");
   return stream;
   }

/*
* Read data from an istream into a pipe
*/
std::istream& operator>>(std::istream& stream, Pipe& pipe)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   while(stream.good())
      {
      stream.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());
      pipe.write(&buffer[0], stream.gcount());
      }
   if(stream.bad() || (stream.fail() && !stream.eof()))
      throw Stream_IO_Error("Pipe input operator (iostream) has failed");
   return stream;
   }

}
/*
* Pipe Reading/Writing
* (C) 1999-2007 Jack Lloyd
*     2012 Markus Wanner
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Look up the canonical ID for a queue
*/
Pipe::message_id Pipe::get_message_no(const std::string& func_name,
                                      message_id msg) const
   {
   if(msg == DEFAULT_MESSAGE)
      msg = default_msg();
   else if(msg == LAST_MESSAGE)
      msg = message_count() - 1;

   if(msg >= message_count())
      throw Invalid_Message_Number(func_name, msg);

   return msg;
   }

/*
* Write into a Pipe
*/
void Pipe::write(const byte input[], size_t length)
   {
   if(!inside_msg)
      throw Invalid_State("Cannot write to a Pipe while it is not processing");
   pipe->write(input, length);
   }

/*
* Write a string into a Pipe
*/
void Pipe::write(const std::string& str)
   {
   write(reinterpret_cast<const byte*>(str.data()), str.size());
   }

/*
* Write a single byte into a Pipe
*/
void Pipe::write(byte input)
   {
   write(&input, 1);
   }

/*
* Write the contents of a DataSource into a Pipe
*/
void Pipe::write(DataSource& source)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   while(!source.end_of_data())
      {
      size_t got = source.read(&buffer[0], buffer.size());
      write(&buffer[0], got);
      }
   }

/*
* Read some data from the pipe
*/
size_t Pipe::read(byte output[], size_t length, message_id msg)
   {
   return outputs->read(output, length, get_message_no("read", msg));
   }

/*
* Read some data from the pipe
*/
size_t Pipe::read(byte output[], size_t length)
   {
   return read(output, length, DEFAULT_MESSAGE);
   }

/*
* Read a single byte from the pipe
*/
size_t Pipe::read(byte& out, message_id msg)
   {
   return read(&out, 1, msg);
   }

/*
* Return all data in the pipe
*/
secure_vector<byte> Pipe::read_all(message_id msg)
   {
   msg = ((msg != DEFAULT_MESSAGE) ? msg : default_msg());
   secure_vector<byte> buffer(remaining(msg));
   size_t got = read(&buffer[0], buffer.size(), msg);
   buffer.resize(got);
   return buffer;
   }

/*
* Return all data in the pipe as a string
*/
std::string Pipe::read_all_as_string(message_id msg)
   {
   msg = ((msg != DEFAULT_MESSAGE) ? msg : default_msg());
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   std::string str;
   str.reserve(remaining(msg));

   while(true)
      {
      size_t got = read(&buffer[0], buffer.size(), msg);
      if(got == 0)
         break;
      str.append(reinterpret_cast<const char*>(&buffer[0]), got);
      }

   return str;
   }

/*
* Find out how many bytes are ready to read
*/
size_t Pipe::remaining(message_id msg) const
   {
   return outputs->remaining(get_message_no("remaining", msg));
   }

/*
* Peek at some data in the pipe
*/
size_t Pipe::peek(byte output[], size_t length,
                  size_t offset, message_id msg) const
   {
   return outputs->peek(output, length, offset, get_message_no("peek", msg));
   }

/*
* Peek at some data in the pipe
*/
size_t Pipe::peek(byte output[], size_t length, size_t offset) const
   {
   return peek(output, length, offset, DEFAULT_MESSAGE);
   }

/*
* Peek at a byte in the pipe
*/
size_t Pipe::peek(byte& out, size_t offset, message_id msg) const
   {
   return peek(&out, 1, offset, msg);
   }

size_t Pipe::get_bytes_read() const
   {
   return outputs->get_bytes_read(DEFAULT_MESSAGE);
   }

size_t Pipe::get_bytes_read(message_id msg) const
   {
   return outputs->get_bytes_read(msg);
   }

}
/*
* SecureQueue
* (C) 1999-2007 Jack Lloyd
*     2012 Markus Wanner
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/**
* A node in a SecureQueue
*/
class SecureQueueNode
   {
   public:
      SecureQueueNode() : buffer(DEFAULT_BUFFERSIZE)
         { next = nullptr; start = end = 0; }

      ~SecureQueueNode() { next = nullptr; start = end = 0; }

      size_t write(const byte input[], size_t length)
         {
         size_t copied = std::min<size_t>(length, buffer.size() - end);
         copy_mem(&buffer[end], input, copied);
         end += copied;
         return copied;
         }

      size_t read(byte output[], size_t length)
         {
         size_t copied = std::min(length, end - start);
         copy_mem(output, &buffer[start], copied);
         start += copied;
         return copied;
         }

      size_t peek(byte output[], size_t length, size_t offset = 0)
         {
         const size_t left = end - start;
         if(offset >= left) return 0;
         size_t copied = std::min(length, left - offset);
         copy_mem(output, &buffer[start + offset], copied);
         return copied;
         }

      size_t size() const { return (end - start); }
   private:
      friend class SecureQueue;
      SecureQueueNode* next;
      secure_vector<byte> buffer;
      size_t start, end;
   };

/*
* Create a SecureQueue
*/
SecureQueue::SecureQueue()
   {
   bytes_read = 0;
   set_next(nullptr, 0);
   head = tail = new SecureQueueNode;
   }

/*
* Copy a SecureQueue
*/
SecureQueue::SecureQueue(const SecureQueue& input) :
   Fanout_Filter(), DataSource()
   {
   bytes_read = 0;
   set_next(nullptr, 0);

   head = tail = new SecureQueueNode;
   SecureQueueNode* temp = input.head;
   while(temp)
      {
      write(&temp->buffer[temp->start], temp->end - temp->start);
      temp = temp->next;
      }
   }

/*
* Destroy this SecureQueue
*/
void SecureQueue::destroy()
   {
   SecureQueueNode* temp = head;
   while(temp)
      {
      SecureQueueNode* holder = temp->next;
      delete temp;
      temp = holder;
      }
   head = tail = nullptr;
   }

/*
* Copy a SecureQueue
*/
SecureQueue& SecureQueue::operator=(const SecureQueue& input)
   {
   destroy();
   head = tail = new SecureQueueNode;
   SecureQueueNode* temp = input.head;
   while(temp)
      {
      write(&temp->buffer[temp->start], temp->end - temp->start);
      temp = temp->next;
      }
   return (*this);
   }

/*
* Add some bytes to the queue
*/
void SecureQueue::write(const byte input[], size_t length)
   {
   if(!head)
      head = tail = new SecureQueueNode;
   while(length)
      {
      const size_t n = tail->write(input, length);
      input += n;
      length -= n;
      if(length)
         {
         tail->next = new SecureQueueNode;
         tail = tail->next;
         }
      }
   }

/*
* Read some bytes from the queue
*/
size_t SecureQueue::read(byte output[], size_t length)
   {
   size_t got = 0;
   while(length && head)
      {
      const size_t n = head->read(output, length);
      output += n;
      got += n;
      length -= n;
      if(head->size() == 0)
         {
         SecureQueueNode* holder = head->next;
         delete head;
         head = holder;
         }
      }
   bytes_read += got;
   return got;
   }

/*
* Read data, but do not remove it from queue
*/
size_t SecureQueue::peek(byte output[], size_t length, size_t offset) const
   {
   SecureQueueNode* current = head;

   while(offset && current)
      {
      if(offset >= current->size())
         {
         offset -= current->size();
         current = current->next;
         }
      else
         break;
      }

   size_t got = 0;
   while(length && current)
      {
      const size_t n = current->peek(output, length, offset);
      offset = 0;
      output += n;
      got += n;
      length -= n;
      current = current->next;
      }
   return got;
   }

/**
* Return how many bytes have been read so far.
*/
size_t SecureQueue::get_bytes_read() const
   {
   return bytes_read;
   }

/*
* Return how many bytes the queue holds
*/
size_t SecureQueue::size() const
   {
   SecureQueueNode* current = head;
   size_t count = 0;

   while(current)
      {
      count += current->size();
      current = current->next;
      }
   return count;
   }

/*
* Test if the queue has any data in it
*/
bool SecureQueue::end_of_data() const
   {
   return (size() == 0);
   }

bool SecureQueue::empty() const
   {
   return (size() == 0);
   }

}
/*
* Threaded Fork
* (C) 2013 Joel Low
*     2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

struct Threaded_Fork_Data
   {
   /*
   * Semaphore for indicating that there is work to be done (or to
   * quit)
   */
   Semaphore m_input_ready_semaphore;

   /*
   * Ensures that all threads have completed processing data.
   */
   Semaphore m_input_complete_semaphore;

   /*
   * The work that needs to be done. This should be only when the threads
   * are NOT running (i.e. before notifying the work condition, after
   * the input_complete_semaphore is completely reset.)
   */
   const byte* m_input = nullptr;

   /*
   * The length of the work that needs to be done.
   */
   size_t m_input_length = 0;
   };

/*
* Threaded_Fork constructor
*/
Threaded_Fork::Threaded_Fork(Filter* f1, Filter* f2, Filter* f3, Filter* f4) :
   Fork(nullptr, static_cast<size_t>(0)),
   m_thread_data(new Threaded_Fork_Data)
   {
   Filter* filters[4] = { f1, f2, f3, f4 };
   set_next(filters, 4);
   }

/*
* Threaded_Fork constructor
*/
Threaded_Fork::Threaded_Fork(Filter* filters[], size_t count) :
   Fork(nullptr, static_cast<size_t>(0)),
   m_thread_data(new Threaded_Fork_Data)
   {
   set_next(filters, count);
   }

Threaded_Fork::~Threaded_Fork()
   {
   m_thread_data->m_input = nullptr;
   m_thread_data->m_input_length = 0;

   m_thread_data->m_input_ready_semaphore.release(m_threads.size());

   for(auto& thread : m_threads)
     thread->join();
   }

std::string Threaded_Fork::name() const
   {
   return "Threaded Fork";
   }

void Threaded_Fork::set_next(Filter* f[], size_t n)
   {
   Fork::set_next(f, n);
   n = next.size();

   if(n < m_threads.size())
      m_threads.resize(n);
   else
      {
      m_threads.reserve(n);
      for(size_t i = m_threads.size(); i != n; ++i)
         {
         m_threads.push_back(
            std::shared_ptr<std::thread>(
               new std::thread(
                  std::bind(&Threaded_Fork::thread_entry, this, next[i]))));
         }
      }
   }

void Threaded_Fork::send(const byte input[], size_t length)
   {
   if(write_queue.size())
      thread_delegate_work(&write_queue[0], write_queue.size());
   thread_delegate_work(input, length);

   bool nothing_attached = true;
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         nothing_attached = false;

   if(nothing_attached)
      write_queue += std::make_pair(input, length);
   else
      write_queue.clear();
   }

void Threaded_Fork::thread_delegate_work(const byte input[], size_t length)
   {
   //Set the data to do.
   m_thread_data->m_input = input;
   m_thread_data->m_input_length = length;

   //Let the workers start processing.
   m_thread_data->m_input_ready_semaphore.release(total_ports());

   //Wait for all the filters to finish processing.
   for(size_t i = 0; i != total_ports(); ++i)
      m_thread_data->m_input_complete_semaphore.acquire();

   //Reset the thread data
   m_thread_data->m_input = nullptr;
   m_thread_data->m_input_length = 0;
   }

void Threaded_Fork::thread_entry(Filter* filter)
   {
   while(true)
      {
      m_thread_data->m_input_ready_semaphore.acquire();

      if(!m_thread_data->m_input)
         break;

      filter->write(m_thread_data->m_input, m_thread_data->m_input_length);
      m_thread_data->m_input_complete_semaphore.release();
      }
   }

}
/*
* Filter interface for Transformations
* (C) 2013,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

size_t choose_update_size(size_t update_granularity)
   {
   const size_t target_size = 1024;

   if(update_granularity >= target_size)
      return update_granularity;

   return round_up(target_size, update_granularity);
   }

}

Transformation_Filter::Transformation_Filter(Transformation* transform) :
   Buffered_Filter(choose_update_size(transform->update_granularity()),
                   transform->minimum_final_size()),
   m_nonce(transform->default_nonce_length() == 0),
   m_transform(transform),
   m_buffer(m_transform->update_granularity())
   {
   }

std::string Transformation_Filter::name() const
   {
   return m_transform->name();
   }

void Transformation_Filter::Nonce_State::update(const InitializationVector& iv)
   {
   m_nonce = unlock(iv.bits_of());
   m_fresh_nonce = true;
   }

std::vector<byte> Transformation_Filter::Nonce_State::get()
   {
   BOTAN_ASSERT(m_fresh_nonce, "The nonce is fresh for this message");

   if(!m_nonce.empty())
      m_fresh_nonce = false;
   return m_nonce;
   }

void Transformation_Filter::set_iv(const InitializationVector& iv)
   {
   m_nonce.update(iv);
   }

void Transformation_Filter::set_key(const SymmetricKey& key)
   {
   if(Keyed_Transform* keyed = dynamic_cast<Keyed_Transform*>(m_transform.get()))
      keyed->set_key(key);
   else if(key.length() != 0)
      throw std::runtime_error("Transformation " + name() + " does not accept keys");
   }

Key_Length_Specification Transformation_Filter::key_spec() const
   {
   if(Keyed_Transform* keyed = dynamic_cast<Keyed_Transform*>(m_transform.get()))
      return keyed->key_spec();
   return Key_Length_Specification(0);
   }

bool Transformation_Filter::valid_iv_length(size_t length) const
   {
   return m_transform->valid_nonce_length(length);
   }

void Transformation_Filter::write(const byte input[], size_t input_length)
   {
   Buffered_Filter::write(input, input_length);
   }

void Transformation_Filter::end_msg()
   {
   Buffered_Filter::end_msg();
   }

void Transformation_Filter::start_msg()
   {
   send(m_transform->start_vec(m_nonce.get()));
   }

void Transformation_Filter::buffered_block(const byte input[], size_t input_length)
   {
   while(input_length)
      {
      const size_t take = std::min(m_transform->update_granularity(), input_length);

      m_buffer.assign(input, input + take);
      m_transform->update(m_buffer);

      send(m_buffer);

      input += take;
      input_length -= take;
      }
   }

void Transformation_Filter::buffered_final(const byte input[], size_t input_length)
   {
   secure_vector<byte> buf(input, input + input_length);
   m_transform->finish(buf);
   send(buf);
   }

}
/*
* Keccak
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

void keccak_f_1600(u64bit A[25])
   {
   static const u64bit RC[24] = {
      0x0000000000000001, 0x0000000000008082, 0x800000000000808A,
      0x8000000080008000, 0x000000000000808B, 0x0000000080000001,
      0x8000000080008081, 0x8000000000008009, 0x000000000000008A,
      0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
      0x000000008000808B, 0x800000000000008B, 0x8000000000008089,
      0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
      0x000000000000800A, 0x800000008000000A, 0x8000000080008081,
      0x8000000000008080, 0x0000000080000001, 0x8000000080008008
   };

   for(size_t i = 0; i != 24; ++i)
      {
      const u64bit C0 = A[0] ^ A[5] ^ A[10] ^ A[15] ^ A[20];
      const u64bit C1 = A[1] ^ A[6] ^ A[11] ^ A[16] ^ A[21];
      const u64bit C2 = A[2] ^ A[7] ^ A[12] ^ A[17] ^ A[22];
      const u64bit C3 = A[3] ^ A[8] ^ A[13] ^ A[18] ^ A[23];
      const u64bit C4 = A[4] ^ A[9] ^ A[14] ^ A[19] ^ A[24];

      const u64bit D0 = rotate_left(C0, 1) ^ C3;
      const u64bit D1 = rotate_left(C1, 1) ^ C4;
      const u64bit D2 = rotate_left(C2, 1) ^ C0;
      const u64bit D3 = rotate_left(C3, 1) ^ C1;
      const u64bit D4 = rotate_left(C4, 1) ^ C2;

      const u64bit B00 = A[ 0] ^ D1;
      const u64bit B01 = rotate_left(A[ 6] ^ D2, 44);
      const u64bit B02 = rotate_left(A[12] ^ D3, 43);
      const u64bit B03 = rotate_left(A[18] ^ D4, 21);
      const u64bit B04 = rotate_left(A[24] ^ D0, 14);
      const u64bit B05 = rotate_left(A[ 3] ^ D4, 28);
      const u64bit B06 = rotate_left(A[ 9] ^ D0, 20);
      const u64bit B07 = rotate_left(A[10] ^ D1, 3);
      const u64bit B08 = rotate_left(A[16] ^ D2, 45);
      const u64bit B09 = rotate_left(A[22] ^ D3, 61);
      const u64bit B10 = rotate_left(A[ 1] ^ D2, 1);
      const u64bit B11 = rotate_left(A[ 7] ^ D3, 6);
      const u64bit B12 = rotate_left(A[13] ^ D4, 25);
      const u64bit B13 = rotate_left(A[19] ^ D0, 8);
      const u64bit B14 = rotate_left(A[20] ^ D1, 18);
      const u64bit B15 = rotate_left(A[ 4] ^ D0, 27);
      const u64bit B16 = rotate_left(A[ 5] ^ D1, 36);
      const u64bit B17 = rotate_left(A[11] ^ D2, 10);
      const u64bit B18 = rotate_left(A[17] ^ D3, 15);
      const u64bit B19 = rotate_left(A[23] ^ D4, 56);
      const u64bit B20 = rotate_left(A[ 2] ^ D3, 62);
      const u64bit B21 = rotate_left(A[ 8] ^ D4, 55);
      const u64bit B22 = rotate_left(A[14] ^ D0, 39);
      const u64bit B23 = rotate_left(A[15] ^ D1, 41);
      const u64bit B24 = rotate_left(A[21] ^ D2, 2);

      A[ 0] = B00 ^ (~B01 & B02);
      A[ 1] = B01 ^ (~B02 & B03);
      A[ 2] = B02 ^ (~B03 & B04);
      A[ 3] = B03 ^ (~B04 & B00);
      A[ 4] = B04 ^ (~B00 & B01);
      A[ 5] = B05 ^ (~B06 & B07);
      A[ 6] = B06 ^ (~B07 & B08);
      A[ 7] = B07 ^ (~B08 & B09);
      A[ 8] = B08 ^ (~B09 & B05);
      A[ 9] = B09 ^ (~B05 & B06);
      A[10] = B10 ^ (~B11 & B12);
      A[11] = B11 ^ (~B12 & B13);
      A[12] = B12 ^ (~B13 & B14);
      A[13] = B13 ^ (~B14 & B10);
      A[14] = B14 ^ (~B10 & B11);
      A[15] = B15 ^ (~B16 & B17);
      A[16] = B16 ^ (~B17 & B18);
      A[17] = B17 ^ (~B18 & B19);
      A[18] = B18 ^ (~B19 & B15);
      A[19] = B19 ^ (~B15 & B16);
      A[20] = B20 ^ (~B21 & B22);
      A[21] = B21 ^ (~B22 & B23);
      A[22] = B22 ^ (~B23 & B24);
      A[23] = B23 ^ (~B24 & B20);
      A[24] = B24 ^ (~B20 & B21);

      A[0] ^= RC[i];
      }
   }

}

Keccak_1600::Keccak_1600(size_t output_bits) :
   output_bits(output_bits),
   bitrate(1600 - 2*output_bits),
   S(25),
   S_pos(0)
   {
   // We only support the parameters for the SHA-3 proposal

   if(output_bits != 224 && output_bits != 256 &&
      output_bits != 384 && output_bits != 512)
      throw Invalid_Argument("Keccak_1600: Invalid output length " +
                             std::to_string(output_bits));
   }

std::string Keccak_1600::name() const
   {
   return "Keccak-1600(" + std::to_string(output_bits) + ")";
   }

HashFunction* Keccak_1600::clone() const
   {
   return new Keccak_1600(output_bits);
   }

void Keccak_1600::clear()
   {
   zeroise(S);
   S_pos = 0;
   }

void Keccak_1600::add_data(const byte input[], size_t length)
   {
   if(length == 0)
      return;

   while(length)
      {
      size_t to_take = std::min(length, bitrate / 8 - S_pos);

      length -= to_take;

      while(to_take && S_pos % 8)
         {
         S[S_pos / 8] ^= static_cast<u64bit>(input[0]) << (8 * (S_pos % 8));

         ++S_pos;
         ++input;
         --to_take;
         }

      while(to_take && to_take % 8 == 0)
         {
         S[S_pos / 8] ^= load_le<u64bit>(input, 0);
         S_pos += 8;
         input += 8;
         to_take -= 8;
         }

      while(to_take)
         {
         S[S_pos / 8] ^= static_cast<u64bit>(input[0]) << (8 * (S_pos % 8));

         ++S_pos;
         ++input;
         --to_take;
         }

      if(S_pos == bitrate / 8)
         {
         keccak_f_1600(&S[0]);
         S_pos = 0;
         }
      }
   }

void Keccak_1600::final_result(byte output[])
   {
   std::vector<byte> padding(bitrate / 8 - S_pos);

   padding[0] = 0x01;
   padding[padding.size()-1] |= 0x80;

   add_data(&padding[0], padding.size());

   /*
   * We never have to run the permutation again because we only support
   * limited output lengths
   */
   for(size_t i = 0; i != output_bits/8; ++i)
      output[i] = get_byte(7 - (i % 8), S[i/8]);

   clear();
   }

}
/*
* Merkle-Damgard Hash Function
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* MDx_HashFunction Constructor
*/
MDx_HashFunction::MDx_HashFunction(size_t block_len,
                                   bool byte_end,
                                   bool bit_end,
                                   size_t cnt_size) :
   buffer(block_len),
   BIG_BYTE_ENDIAN(byte_end),
   BIG_BIT_ENDIAN(bit_end),
   COUNT_SIZE(cnt_size)
   {
   count = position = 0;
   }

/*
* Clear memory of sensitive data
*/
void MDx_HashFunction::clear()
   {
   zeroise(buffer);
   count = position = 0;
   }

/*
* Update the hash
*/
void MDx_HashFunction::add_data(const byte input[], size_t length)
   {
   count += length;

   if(position)
      {
      buffer_insert(buffer, position, input, length);

      if(position + length >= buffer.size())
         {
         compress_n(&buffer[0], 1);
         input += (buffer.size() - position);
         length -= (buffer.size() - position);
         position = 0;
         }
      }

   const size_t full_blocks = length / buffer.size();
   const size_t remaining   = length % buffer.size();

   if(full_blocks)
      compress_n(input, full_blocks);

   buffer_insert(buffer, position, input + full_blocks * buffer.size(), remaining);
   position += remaining;
   }

/*
* Finalize a hash
*/
void MDx_HashFunction::final_result(byte output[])
   {
   buffer[position] = (BIG_BIT_ENDIAN ? 0x80 : 0x01);
   for(size_t i = position+1; i != buffer.size(); ++i)
      buffer[i] = 0;

   if(position >= buffer.size() - COUNT_SIZE)
      {
      compress_n(&buffer[0], 1);
      zeroise(buffer);
      }

   write_count(&buffer[buffer.size() - COUNT_SIZE]);

   compress_n(&buffer[0], 1);
   copy_out(output);
   clear();
   }

/*
* Write the count bits to the buffer
*/
void MDx_HashFunction::write_count(byte out[])
   {
   if(COUNT_SIZE < 8)
      throw Invalid_State("MDx_HashFunction::write_count: COUNT_SIZE < 8");
   if(COUNT_SIZE >= output_length() || COUNT_SIZE >= hash_block_size())
      throw Invalid_Argument("MDx_HashFunction: COUNT_SIZE is too big");

   const u64bit bit_count = count * 8;

   if(BIG_BYTE_ENDIAN)
      store_be(bit_count, out + COUNT_SIZE - 8);
   else
      store_le(bit_count, out + COUNT_SIZE - 8);
   }

}
/*
* SHA-{224,256}
* (C) 1999-2010 Jack Lloyd
*     2007 FlexSecure GmbH
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

namespace SHA2_32 {

/*
* SHA-256 Rho Function
*/
inline u32bit rho(u32bit X, u32bit rot1, u32bit rot2, u32bit rot3)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^
           rotate_right(X, rot3));
   }

/*
* SHA-256 Sigma Function
*/
inline u32bit sigma(u32bit X, u32bit rot1, u32bit rot2, u32bit shift)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^ (X >> shift));
   }

/*
* SHA-256 F1 Function
*
* Use a macro as many compilers won't inline a function this big,
* even though it is much faster if inlined.
*/
#define SHA2_32_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic)   \
   do {                                                            \
      H += magic + rho(E, 6, 11, 25) + ((E & F) ^ (~E & G)) + M1;  \
      D += H;                                                      \
      H += rho(A, 2, 13, 22) + ((A & B) | ((A | B) & C));          \
      M1 += sigma(M2, 17, 19, 10) + M3 + sigma(M4, 7, 18, 3);      \
   } while(0);

/*
* SHA-224 / SHA-256 compression function
*/
void compress(secure_vector<u32bit>& digest,
              const byte input[], size_t blocks)
   {
   u32bit A = digest[0], B = digest[1], C = digest[2],
          D = digest[3], E = digest[4], F = digest[5],
          G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit W00 = load_be<u32bit>(input,  0);
      u32bit W01 = load_be<u32bit>(input,  1);
      u32bit W02 = load_be<u32bit>(input,  2);
      u32bit W03 = load_be<u32bit>(input,  3);
      u32bit W04 = load_be<u32bit>(input,  4);
      u32bit W05 = load_be<u32bit>(input,  5);
      u32bit W06 = load_be<u32bit>(input,  6);
      u32bit W07 = load_be<u32bit>(input,  7);
      u32bit W08 = load_be<u32bit>(input,  8);
      u32bit W09 = load_be<u32bit>(input,  9);
      u32bit W10 = load_be<u32bit>(input, 10);
      u32bit W11 = load_be<u32bit>(input, 11);
      u32bit W12 = load_be<u32bit>(input, 12);
      u32bit W13 = load_be<u32bit>(input, 13);
      u32bit W14 = load_be<u32bit>(input, 14);
      u32bit W15 = load_be<u32bit>(input, 15);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x71374491);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCF);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25B);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B01);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174);
      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC6);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DC);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C8);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x14292967);
      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A85);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B2138);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D13);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A7354);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C85);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664B);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD6990624);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E3585);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA070);
      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C08);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774C);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4A);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC70208);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 64;
      }
   }

}

}

/*
* SHA-224 compression function
*/
void SHA_224::compress_n(const byte input[], size_t blocks)
   {
   SHA2_32::compress(digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_224::copy_out(byte output[])
   {
   for(size_t i = 0; i != output_length(); i += 4)
      store_be(digest[i/4], output + i);
   }

/*
* Clear memory of sensitive data
*/
void SHA_224::clear()
   {
   MDx_HashFunction::clear();
   digest[0] = 0xC1059ED8;
   digest[1] = 0x367CD507;
   digest[2] = 0x3070DD17;
   digest[3] = 0xF70E5939;
   digest[4] = 0xFFC00B31;
   digest[5] = 0x68581511;
   digest[6] = 0x64F98FA7;
   digest[7] = 0xBEFA4FA4;
   }

/*
* SHA-256 compression function
*/
void SHA_256::compress_n(const byte input[], size_t blocks)
   {
   SHA2_32::compress(digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_256::copy_out(byte output[])
   {
   for(size_t i = 0; i != output_length(); i += 4)
      store_be(digest[i/4], output + i);
   }

/*
* Clear memory of sensitive data
*/
void SHA_256::clear()
   {
   MDx_HashFunction::clear();
   digest[0] = 0x6A09E667;
   digest[1] = 0xBB67AE85;
   digest[2] = 0x3C6EF372;
   digest[3] = 0xA54FF53A;
   digest[4] = 0x510E527F;
   digest[5] = 0x9B05688C;
   digest[6] = 0x1F83D9AB;
   digest[7] = 0x5BE0CD19;
   }

}
/*
* SHA-{384,512}
* (C) 1999-2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

namespace SHA2_64 {

/*
* SHA-{384,512} Rho Function
*/
inline u64bit rho(u64bit X, u32bit rot1, u32bit rot2, u32bit rot3)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^
           rotate_right(X, rot3));
   }

/*
* SHA-{384,512} Sigma Function
*/
inline u64bit sigma(u64bit X, u32bit rot1, u32bit rot2, u32bit shift)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^ (X >> shift));
   }

/*
* SHA-512 F1 Function
*
* Use a macro as many compilers won't inline a function this big,
* even though it is much faster if inlined.
*/
#define SHA2_64_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic)   \
   do {                                                            \
      H += magic + rho(E, 14, 18, 41) + ((E & F) ^ (~E & G)) + M1; \
      D += H;                                                      \
      H += rho(A, 28, 34, 39) + ((A & B) | ((A | B) & C));         \
      M1 += sigma(M2, 19, 61, 6) + M3 + sigma(M4, 1, 8, 7);        \
   } while(0);

/*
* SHA-{384,512} Compression Function
*/
void compress(secure_vector<u64bit>& digest,
              const byte input[], size_t blocks)
   {
   u64bit A = digest[0], B = digest[1], C = digest[2],
          D = digest[3], E = digest[4], F = digest[5],
          G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      u64bit W00 = load_be<u64bit>(input,  0);
      u64bit W01 = load_be<u64bit>(input,  1);
      u64bit W02 = load_be<u64bit>(input,  2);
      u64bit W03 = load_be<u64bit>(input,  3);
      u64bit W04 = load_be<u64bit>(input,  4);
      u64bit W05 = load_be<u64bit>(input,  5);
      u64bit W06 = load_be<u64bit>(input,  6);
      u64bit W07 = load_be<u64bit>(input,  7);
      u64bit W08 = load_be<u64bit>(input,  8);
      u64bit W09 = load_be<u64bit>(input,  9);
      u64bit W10 = load_be<u64bit>(input, 10);
      u64bit W11 = load_be<u64bit>(input, 11);
      u64bit W12 = load_be<u64bit>(input, 12);
      u64bit W13 = load_be<u64bit>(input, 13);
      u64bit W14 = load_be<u64bit>(input, 14);
      u64bit W15 = load_be<u64bit>(input, 15);

      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98D728AE22);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x7137449123EF65CD);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCFEC4D3B2F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA58189DBBC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25BF348B538);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1B605D019);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4AF194F9B);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5DA6D8118);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98A3030242);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B0145706FBE);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE4EE4B28C);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3D5FFB4E2);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74F27B896F);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE3B1696B1);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A725C71235);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174CF692694);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C19EF14AD2);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786384F25E3);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC68B8CD5B5);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC77AC9C65);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F592B0275);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA6EA6E483);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DCBD41FBD4);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA831153B5);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152EE66DFAB);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D2DB43210);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C898FB213F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7BEEF0EE4);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF33DA88FC2);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147930AA725);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351E003826F);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x142929670A0E6E70);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A8546D22FFC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B21385C26C926);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC5AC42AED);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D139D95B3DF);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A73548BAF63DE);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB3C77B2A8);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E47EDAEE6);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C851482353B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A14CF10364);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664BBC423001);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70D0F89791);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A30654BE30);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819D6EF5218);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD69906245565A910);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E35855771202A);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA07032BBD1B8);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116B8D2D0C8);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C085141AB53);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774CDF8EEB99);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5E19B48A8);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3C5C95A63);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4AE3418ACB);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F7763E373);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3D6B2B8A3);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE5DEFB2FC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F43172F60);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814A1F0AB72);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC702081A6439EC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA23631E28);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEBDE82BDE9);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7B2C67915);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2E372532B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xCA273ECEEA26619C);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xD186B8C721C0C207);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xEADA7DD6CDE0EB1E);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xF57D4F7FEE6ED178);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x06F067AA72176FBA);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x0A637DC5A2C898A6);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x113F9804BEF90DAE);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x1B710B35131C471B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x28DB77F523047D84);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x32CAAB7B40C72493);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x3C9EBE0A15C9BEBC);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x431D67C49C100D4C);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x4CC5D4BECB3E42B6);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x597F299CFC657E2A);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x5FCB6FAB3AD6FAEC);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x6C44198C4A475817);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 128;
      }
   }

}

}

/*
* SHA-384 compression function
*/
void SHA_384::compress_n(const byte input[], size_t blocks)
   {
   SHA2_64::compress(digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_384::copy_out(byte output[])
   {
   for(size_t i = 0; i != output_length(); i += 8)
      store_be(digest[i/8], output + i);
   }

/*
* Clear memory of sensitive data
*/
void SHA_384::clear()
   {
   MDx_HashFunction::clear();
   digest[0] = 0xCBBB9D5DC1059ED8;
   digest[1] = 0x629A292A367CD507;
   digest[2] = 0x9159015A3070DD17;
   digest[3] = 0x152FECD8F70E5939;
   digest[4] = 0x67332667FFC00B31;
   digest[5] = 0x8EB44A8768581511;
   digest[6] = 0xDB0C2E0D64F98FA7;
   digest[7] = 0x47B5481DBEFA4FA4;
   }

/*
* SHA-512 compression function
*/
void SHA_512::compress_n(const byte input[], size_t blocks)
   {
   SHA2_64::compress(digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_512::copy_out(byte output[])
   {
   for(size_t i = 0; i != output_length(); i += 8)
      store_be(digest[i/8], output + i);
   }

/*
* Clear memory of sensitive data
*/
void SHA_512::clear()
   {
   MDx_HashFunction::clear();
   digest[0] = 0x6A09E667F3BCC908;
   digest[1] = 0xBB67AE8584CAA73B;
   digest[2] = 0x3C6EF372FE94F82B;
   digest[3] = 0xA54FF53A5F1D36F1;
   digest[4] = 0x510E527FADE682D1;
   digest[5] = 0x9B05688C2B3E6C1F;
   digest[6] = 0x1F83D9ABFB41BD6B;
   digest[7] = 0x5BE0CD19137E2179;
   }

}
/*
* KDF Retrieval
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_KDF1)
#endif

#if defined(BOTAN_HAS_KDF2)
#endif

#if defined(BOTAN_HAS_X942_PRF)
#endif

#if defined(BOTAN_HAS_SSL_V3_PRF)
#endif

#if defined(BOTAN_HAS_TLS_V10_PRF)
#endif

namespace Botan {

KDF* get_kdf(const std::string& algo_spec)
   {
   SCAN_Name request(algo_spec);

   Algorithm_Factory& af = global_state().algorithm_factory();

   if(request.algo_name() == "Raw")
      return nullptr; // No KDF

#if defined(BOTAN_HAS_KDF1)
   if(request.algo_name() == "KDF1" && request.arg_count() == 1)
      return new KDF1(af.make_hash_function(request.arg(0)));
#endif

#if defined(BOTAN_HAS_KDF2)
   if(request.algo_name() == "KDF2" && request.arg_count() == 1)
      return new KDF2(af.make_hash_function(request.arg(0)));
#endif

#if defined(BOTAN_HAS_X942_PRF)
   if(request.algo_name() == "X9.42-PRF" && request.arg_count() == 1)
      return new X942_PRF(request.arg(0)); // OID
#endif

#if defined(BOTAN_HAS_SSL_V3_PRF)
   if(request.algo_name() == "SSL3-PRF" && request.arg_count() == 0)
      return new SSL3_PRF;
#endif

#if defined(BOTAN_HAS_TLS_V10_PRF)
   if(request.algo_name() == "TLS-PRF" && request.arg_count() == 0)
      return new TLS_PRF;
#endif

#if defined(BOTAN_HAS_TLS_V10_PRF)
   if(request.algo_name() == "TLS-PRF" && request.arg_count() == 0)
      return new TLS_PRF;
#endif

#if defined(BOTAN_HAS_TLS_V12_PRF)
   if(request.algo_name() == "TLS-12-PRF" && request.arg_count() == 1)
      return new TLS_12_PRF(af.make_mac("HMAC(" + request.arg(0) + ")"));
#endif

   throw Algorithm_Not_Found(algo_spec);
   }

}
/*
* KDF2
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* KDF2 Key Derivation Mechanism
*/
secure_vector<byte> KDF2::derive(size_t out_len,
                                const byte secret[], size_t secret_len,
                                const byte P[], size_t P_len) const
   {
   secure_vector<byte> output;
   u32bit counter = 1;

   while(out_len && counter)
      {
      hash->update(secret, secret_len);
      hash->update_be(counter);
      hash->update(P, P_len);

      secure_vector<byte> hash_result = hash->final();

      size_t added = std::min(hash_result.size(), out_len);
      output += std::make_pair(&hash_result[0], added);
      out_len -= added;

      ++counter;
      }

   return output;
   }

}
/*
* Global PRNG
* (C) 2008-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDRAND)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_EGD)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_BEOS)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_CAPI)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_PROC_WALKER)
#endif

namespace Botan {

std::vector<std::unique_ptr<EntropySource>> Library_State::entropy_sources()
   {
   std::vector<std::unique_ptr<EntropySource>> sources;

#if defined(BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER)
   sources.push_back(std::unique_ptr<EntropySource>(new High_Resolution_Timestamp));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDRAND)
   sources.push_back(std::unique_ptr<EntropySource>(new Intel_Rdrand));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER)
   sources.push_back(std::unique_ptr<EntropySource>(new UnixProcessInfo_EntropySource));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM)
   sources.push_back(std::unique_ptr<EntropySource>(new Device_EntropySource(
      { "/dev/random", "/dev/srandom", "/dev/urandom" }
   )));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_CAPI)
   sources.push_back(std::unique_ptr<EntropySource>(new Win32_CAPI_EntropySource));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_PROC_WALKER)
   sources.push_back(std::unique_ptr<EntropySource>(
      new ProcWalking_EntropySource("/proc")));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
   sources.push_back(std::unique_ptr<EntropySource>(new Win32_EntropySource));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_BEOS)
   sources.push_back(std::unique_ptr<EntropySource>(new BeOS_EntropySource));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER)
   sources.push_back(std::unique_ptr<EntropySource>(
      new Unix_EntropySource(
         { "/bin", "/sbin", "/usr/bin", "/usr/sbin" }
      )));
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_EGD)
   sources.push_back(std::unique_ptr<EntropySource>(
      new EGD_EntropySource({ "/var/run/egd-pool", "/dev/egd-pool" })
      ));
#endif

   return sources;
   }

void Library_State::poll_available_sources(class Entropy_Accumulator& accum)
   {
   std::lock_guard<std::mutex> lock(m_entropy_src_mutex);

   if(m_sources.empty())
      throw std::runtime_error("No entropy sources enabled at build time, poll failed");

   size_t poll_attempt = 0;

   while(!accum.polling_goal_achieved() && poll_attempt < 16)
      {
      const size_t src_idx = poll_attempt % m_sources.size();
      m_sources[src_idx]->poll(accum);
      ++poll_attempt;
      }
   }

}

/*
* Global State Management
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* @todo There should probably be a lock to avoid racy manipulation
* of the state among different threads
*/

namespace Global_State_Management {

/*
* Botan's global state
*/
namespace {

Library_State* global_lib_state = nullptr;

}

/*
* Access the global state object
*/
Library_State& global_state()
   {
   /* Lazy initialization. Botan still needs to be deinitialized later
      on or memory might leak.
   */
   if(!global_lib_state)
      {
      global_lib_state = new Library_State;
      global_lib_state->initialize();
      }

   return (*global_lib_state);
   }

/*
* Set a new global state object
*/
void set_global_state(Library_State* new_state)
   {
   delete swap_global_state(new_state);
   }

/*
* Set a new global state object unless one already existed
*/
bool set_global_state_unless_set(Library_State* new_state)
   {
   if(global_lib_state)
      {
      delete new_state;
      return false;
      }
   else
      {
      delete swap_global_state(new_state);
      return true;
      }
   }

/*
* Swap two global state objects
*/
Library_State* swap_global_state(Library_State* new_state)
   {
   Library_State* old_state = global_lib_state;
   global_lib_state = new_state;
   return old_state;
   }

/*
* Query if library is initialized
*/
bool global_state_exists()
   {
   return (global_lib_state != nullptr);
   }

}

}
/*
* Default Initialization Function
* (C) 1999-2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Library Initialization
*/
void LibraryInitializer::initialize(const std::string&)
   {

   try
      {
      /*
      This two stage initialization process is because Library_State's
      constructor will implicitly refer to global state through the
      allocators and so forth, so global_state() has to be a valid
      reference before initialize() can be called. Yeah, gross.
      */
      Global_State_Management::set_global_state(new Library_State);

      global_state().initialize();
      }
   catch(...)
      {
      deinitialize();
      throw;
      }
   }

/*
* Library Shutdown
*/
void LibraryInitializer::deinitialize()
   {
   Global_State_Management::set_global_state(nullptr);
   }

}
/*
* Library Internal/Global State
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_SELFTESTS)
#endif

#if defined(BOTAN_HAS_ENGINE_ASSEMBLER)
#endif

#if defined(BOTAN_HAS_ENGINE_AES_ISA)
#endif

#if defined(BOTAN_HAS_ENGINE_SIMD)
#endif

#if defined(BOTAN_HAS_ENGINE_GNU_MP)
#endif

#if defined(BOTAN_HAS_ENGINE_OPENSSL)
#endif

namespace Botan {

/*
* Return a reference to the Algorithm_Factory
*/
Algorithm_Factory& Library_State::algorithm_factory() const
   {
   if(!m_algorithm_factory)
      throw Invalid_State("Uninitialized in Library_State::algorithm_factory");
   return *m_algorithm_factory;
   }

/*
* Return a reference to the global PRNG
*/
RandomNumberGenerator& Library_State::global_rng()
   {
   return *m_global_prng;
   }

void Library_State::initialize()
   {
   if(m_algorithm_factory.get())
      throw Invalid_State("Library_State has already been initialized");

   CPUID::initialize();

   SCAN_Name::set_default_aliases();
   OIDS::set_defaults();

   m_algorithm_factory.reset(new Algorithm_Factory());

#if defined(BOTAN_HAS_ENGINE_GNU_MP)
   algorithm_factory().add_engine(new GMP_Engine);
#endif

#if defined(BOTAN_HAS_ENGINE_OPENSSL)
   algorithm_factory().add_engine(new OpenSSL_Engine);
#endif

#if defined(BOTAN_HAS_ENGINE_AES_ISA)
   algorithm_factory().add_engine(new AES_ISA_Engine);
#endif

#if defined(BOTAN_HAS_ENGINE_SIMD)
   algorithm_factory().add_engine(new SIMD_Engine);
#endif

#if defined(BOTAN_HAS_ENGINE_ASSEMBLER)
   algorithm_factory().add_engine(new Assembler_Engine);
#endif

   algorithm_factory().add_engine(new Core_Engine);

   m_sources = entropy_sources();

   m_global_prng.reset(new Serialized_RNG());

#if defined(BOTAN_HAS_SELFTESTS)
   confirm_startup_self_tests(algorithm_factory());
#endif
   }

}
/*
* Algorithm Retrieval
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Get a PBKDF algorithm by name
*/
PBKDF* get_pbkdf(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();

   if(PBKDF* pbkdf = af.make_pbkdf(algo_spec))
      return pbkdf;

   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Query if an algorithm exists
*/
bool have_algorithm(const std::string& name)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();

   if(af.prototype_block_cipher(name))
      return true;
   if(af.prototype_stream_cipher(name))
      return true;
   if(af.prototype_hash_function(name))
      return true;
   if(af.prototype_mac(name))
      return true;
   return false;
   }

/*
* Query the block size of a cipher or hash
*/
size_t block_size_of(const std::string& name)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();

   if(const BlockCipher* cipher = af.prototype_block_cipher(name))
      return cipher->block_size();

   if(const HashFunction* hash = af.prototype_hash_function(name))
      return hash->hash_block_size();

   throw Algorithm_Not_Found(name);
   }

/*
* Query the output_length() of a hash or MAC
*/
size_t output_length_of(const std::string& name)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();

   if(const HashFunction* hash = af.prototype_hash_function(name))
      return hash->output_length();

   if(const MessageAuthenticationCode* mac = af.prototype_mac(name))
      return mac->output_length();

   throw Algorithm_Not_Found(name);
   }

/*
* Get a cipher object
*/
Keyed_Filter* get_cipher(const std::string& algo_spec,
                         Cipher_Dir direction)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();

   Algorithm_Factory::Engine_Iterator i(af);

   while(Engine* engine = i.next())
      {
      if(Keyed_Filter* algo = engine->get_cipher(algo_spec, direction, af))
         return algo;
      }

   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Get a cipher object
*/
Keyed_Filter* get_cipher(const std::string& algo_spec,
                         const SymmetricKey& key,
                         const InitializationVector& iv,
                         Cipher_Dir direction)
   {
   Keyed_Filter* cipher = get_cipher(algo_spec, direction);
   cipher->set_key(key);

   if(iv.length())
      cipher->set_iv(iv);

   return cipher;
   }

/*
* Get a cipher object
*/
Keyed_Filter* get_cipher(const std::string& algo_spec,
                         const SymmetricKey& key,
                         Cipher_Dir direction)
   {
   return get_cipher(algo_spec,
                     key, InitializationVector(), direction);
   }

}
/*
* CMAC
* (C) 1999-2007,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Perform CMAC's multiplication in GF(2^n)
*/
secure_vector<byte> CMAC::poly_double(const secure_vector<byte>& in)
   {
   const bool top_carry = (in[0] & 0x80);

   secure_vector<byte> out = in;

   byte carry = 0;
   for(size_t i = out.size(); i != 0; --i)
      {
      byte temp = out[i-1];
      out[i-1] = (temp << 1) | carry;
      carry = (temp >> 7);
      }

   if(top_carry)
      {
      switch(in.size())
         {
         case 8:
            out[out.size()-1] ^= 0x1B;
            break;
         case 16:
            out[out.size()-1] ^= 0x87;
            break;
         case 32:
            out[out.size()-2] ^= 0x4;
            out[out.size()-1] ^= 0x25;
            break;
         case 64:
            out[out.size()-2] ^= 0x1;
            out[out.size()-1] ^= 0x25;
            break;
         }
      }

   return out;
   }

/*
* Update an CMAC Calculation
*/
void CMAC::add_data(const byte input[], size_t length)
   {
   buffer_insert(m_buffer, m_position, input, length);
   if(m_position + length > output_length())
      {
      xor_buf(m_state, m_buffer, output_length());
      m_cipher->encrypt(m_state);
      input += (output_length() - m_position);
      length -= (output_length() - m_position);
      while(length > output_length())
         {
         xor_buf(m_state, input, output_length());
         m_cipher->encrypt(m_state);
         input += output_length();
         length -= output_length();
         }
      copy_mem(&m_buffer[0], input, length);
      m_position = 0;
      }
   m_position += length;
   }

/*
* Finalize an CMAC Calculation
*/
void CMAC::final_result(byte mac[])
   {
   xor_buf(m_state, m_buffer, m_position);

   if(m_position == output_length())
      {
      xor_buf(m_state, m_B, output_length());
      }
   else
      {
      m_state[m_position] ^= 0x80;
      xor_buf(m_state, m_P, output_length());
      }

   m_cipher->encrypt(m_state);

   for(size_t i = 0; i != output_length(); ++i)
      mac[i] = m_state[i];

   zeroise(m_state);
   zeroise(m_buffer);
   m_position = 0;
   }

/*
* CMAC Key Schedule
*/
void CMAC::key_schedule(const byte key[], size_t length)
   {
   clear();
   m_cipher->set_key(key, length);
   m_cipher->encrypt(m_B);
   m_B = poly_double(m_B);
   m_P = poly_double(m_B);
   }

/*
* Clear memory of sensitive data
*/
void CMAC::clear()
   {
   m_cipher->clear();
   zeroise(m_state);
   zeroise(m_buffer);
   zeroise(m_B);
   zeroise(m_P);
   m_position = 0;
   }

/*
* Return the name of this type
*/
std::string CMAC::name() const
   {
   return "CMAC(" + m_cipher->name() + ")";
   }

/*
* Return a clone of this object
*/
MessageAuthenticationCode* CMAC::clone() const
   {
   return new CMAC(m_cipher->clone());
   }

/*
* CMAC Constructor
*/
CMAC::CMAC(BlockCipher* cipher) : m_cipher(cipher)
   {
   if(m_cipher->block_size() !=  8 && m_cipher->block_size() != 16 &&
      m_cipher->block_size() != 32 && m_cipher->block_size() != 64)
      {
      throw Invalid_Argument("CMAC cannot use the " +
                             std::to_string(m_cipher->block_size() * 8) +
                             " bit cipher " + m_cipher->name());
      }

   m_state.resize(output_length());
   m_buffer.resize(output_length());
   m_B.resize(output_length());
   m_P.resize(output_length());
   m_position = 0;
   }

}
/*
* HMAC
* (C) 1999-2007,2014 Jack Lloyd
*     2007 Yves Jerschow
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Update a HMAC Calculation
*/
void HMAC::add_data(const byte input[], size_t length)
   {
   m_hash->update(input, length);
   }

/*
* Finalize a HMAC Calculation
*/
void HMAC::final_result(byte mac[])
   {
   m_hash->final(mac);
   m_hash->update(m_okey);
   m_hash->update(mac, output_length());
   m_hash->final(mac);
   m_hash->update(m_ikey);
   }

/*
* HMAC Key Schedule
*/
void HMAC::key_schedule(const byte key[], size_t length)
   {
   m_hash->clear();

   m_ikey.resize(m_hash->hash_block_size());
   m_okey.resize(m_hash->hash_block_size());

   std::fill(m_ikey.begin(), m_ikey.end(), 0x36);
   std::fill(m_okey.begin(), m_okey.end(), 0x5C);

   if(length > m_hash->hash_block_size())
      {
      secure_vector<byte> hmac_key = m_hash->process(key, length);
      xor_buf(m_ikey, hmac_key, hmac_key.size());
      xor_buf(m_okey, hmac_key, hmac_key.size());
      }
   else
      {
      xor_buf(m_ikey, key, length);
      xor_buf(m_okey, key, length);
      }

   m_hash->update(m_ikey);
   }

/*
* Clear memory of sensitive data
*/
void HMAC::clear()
   {
   m_hash->clear();
   zap(m_ikey);
   zap(m_okey);
   }

/*
* Return the name of this type
*/
std::string HMAC::name() const
   {
   return "HMAC(" + m_hash->name() + ")";
   }

/*
* Return a clone of this object
*/
MessageAuthenticationCode* HMAC::clone() const
   {
   return new HMAC(m_hash->clone());
   }

/*
* HMAC Constructor
*/
HMAC::HMAC(HashFunction* hash) : m_hash(hash)
   {
   if(m_hash->hash_block_size() == 0)
      throw Invalid_Argument("HMAC cannot be used with " + m_hash->name());
   }

}
/*
* Message Authentication Code base class
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Default (deterministic) MAC verification operation
*/
bool MessageAuthenticationCode::verify_mac(const byte mac[], size_t length)
   {
   secure_vector<byte> our_mac = final();

   if(our_mac.size() != length)
      return false;

   return same_mem(&our_mac[0], &mac[0], length);
   }

}
/*
* BigInt Encoding/Decoding
* (C) 1999-2010,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Encode a BigInt
*/
void BigInt::encode(byte output[], const BigInt& n, Base base)
   {
   if(base == Binary)
      {
      n.binary_encode(output);
      }
   else if(base == Hexadecimal)
      {
      secure_vector<byte> binary(n.encoded_size(Binary));
      n.binary_encode(&binary[0]);

      hex_encode(reinterpret_cast<char*>(output),
                 &binary[0], binary.size());
      }
   else if(base == Decimal)
      {
      BigInt copy = n;
      BigInt remainder;
      copy.set_sign(Positive);
      const size_t output_size = n.encoded_size(Decimal);
      for(size_t j = 0; j != output_size; ++j)
         {
         divide(copy, 10, copy, remainder);
         output[output_size - 1 - j] =
            Charset::digit2char(static_cast<byte>(remainder.word_at(0)));
         if(copy.is_zero())
            break;
         }
      }
   else
      throw Invalid_Argument("Unknown BigInt encoding method");
   }

/*
* Encode a BigInt
*/
std::vector<byte> BigInt::encode(const BigInt& n, Base base)
   {
   std::vector<byte> output(n.encoded_size(base));
   encode(&output[0], n, base);
   if(base != Binary)
      for(size_t j = 0; j != output.size(); ++j)
         if(output[j] == 0)
            output[j] = '0';
   return output;
   }

/*
* Encode a BigInt
*/
secure_vector<byte> BigInt::encode_locked(const BigInt& n, Base base)
   {
   secure_vector<byte> output(n.encoded_size(base));
   encode(&output[0], n, base);
   if(base != Binary)
      for(size_t j = 0; j != output.size(); ++j)
         if(output[j] == 0)
            output[j] = '0';
   return output;
   }

/*
* Encode a BigInt, with leading 0s if needed
*/
secure_vector<byte> BigInt::encode_1363(const BigInt& n, size_t bytes)
   {
   const size_t n_bytes = n.bytes();
   if(n_bytes > bytes)
      throw Encoding_Error("encode_1363: n is too large to encode properly");

   const size_t leading_0s = bytes - n_bytes;

   secure_vector<byte> output(bytes);
   encode(&output[leading_0s], n, Binary);
   return output;
   }

/*
* Decode a BigInt
*/
BigInt BigInt::decode(const byte buf[], size_t length, Base base)
   {
   BigInt r;
   if(base == Binary)
      r.binary_decode(buf, length);
   else if(base == Hexadecimal)
      {
      secure_vector<byte> binary;

      if(length % 2)
         {
         // Handle lack of leading 0
         const char buf0_with_leading_0[2] =
            { '0', static_cast<char>(buf[0]) };

         binary = hex_decode_locked(buf0_with_leading_0, 2);

         binary += hex_decode_locked(reinterpret_cast<const char*>(&buf[1]),
                                     length - 1,
                                     false);
         }
      else
         binary = hex_decode_locked(reinterpret_cast<const char*>(buf),
                                    length, false);

      r.binary_decode(&binary[0], binary.size());
      }
   else if(base == Decimal)
      {
      for(size_t i = 0; i != length; ++i)
         {
         if(Charset::is_space(buf[i]))
            continue;

         if(!Charset::is_digit(buf[i]))
            throw Invalid_Argument("BigInt::decode: "
                                   "Invalid character in decimal input");

         const byte x = Charset::char2digit(buf[i]);

         if(x >= 10)
            throw Invalid_Argument("BigInt: Invalid decimal string");

         r *= 10;
         r += x;
         }
      }
   else
      throw Invalid_Argument("Unknown BigInt decoding method");
   return r;
   }

}
/*
* BigInt Input/Output
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Write the BigInt into a stream
*/
std::ostream& operator<<(std::ostream& stream, const BigInt& n)
   {
   BigInt::Base base = BigInt::Decimal;
   if(stream.flags() & std::ios::hex)
      base = BigInt::Hexadecimal;
   else if(stream.flags() & std::ios::oct)
      throw std::runtime_error("Octal output of BigInt not supported");

   if(n == 0)
      stream.write("0", 1);
   else
      {
      if(n < 0)
         stream.write("-", 1);
      const std::vector<byte> buffer = BigInt::encode(n, base);
      size_t skip = 0;
      while(skip < buffer.size() && buffer[skip] == '0')
         ++skip;
      stream.write(reinterpret_cast<const char*>(&buffer[0]) + skip,
                   buffer.size() - skip);
      }
   if(!stream.good())
      throw Stream_IO_Error("BigInt output operator has failed");
   return stream;
   }

/*
* Read the BigInt from a stream
*/
std::istream& operator>>(std::istream& stream, BigInt& n)
   {
   std::string str;
   std::getline(stream, str);
   if(stream.bad() || (stream.fail() && !stream.eof()))
      throw Stream_IO_Error("BigInt input operator has failed");
   n = BigInt(str);
   return stream;
   }

}
/*
* BigInt Assignment Operators
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Addition Operator
*/
BigInt& BigInt::operator+=(const BigInt& y)
   {
   const size_t x_sw = sig_words(), y_sw = y.sig_words();

   const size_t reg_size = std::max(x_sw, y_sw) + 1;
   grow_to(reg_size);

   if(sign() == y.sign())
      bigint_add2(mutable_data(), reg_size - 1, y.data(), y_sw);
   else
      {
      s32bit relative_size = bigint_cmp(data(), x_sw, y.data(), y_sw);

      if(relative_size < 0)
         {
         secure_vector<word> z(reg_size - 1);
         bigint_sub3(&z[0], y.data(), reg_size - 1, data(), x_sw);
         std::swap(m_reg, z);
         set_sign(y.sign());
         }
      else if(relative_size == 0)
         {
         zeroise(m_reg);
         set_sign(Positive);
         }
      else if(relative_size > 0)
         bigint_sub2(mutable_data(), x_sw, y.data(), y_sw);
      }

   return (*this);
   }

/*
* Subtraction Operator
*/
BigInt& BigInt::operator-=(const BigInt& y)
   {
   const size_t x_sw = sig_words(), y_sw = y.sig_words();

   s32bit relative_size = bigint_cmp(data(), x_sw, y.data(), y_sw);

   const size_t reg_size = std::max(x_sw, y_sw) + 1;
   grow_to(reg_size);

   if(relative_size < 0)
      {
      if(sign() == y.sign())
         bigint_sub2_rev(mutable_data(), y.data(), y_sw);
      else
         bigint_add2(mutable_data(), reg_size - 1, y.data(), y_sw);

      set_sign(y.reverse_sign());
      }
   else if(relative_size == 0)
      {
      if(sign() == y.sign())
         {
         clear();
         set_sign(Positive);
         }
      else
         bigint_shl1(mutable_data(), x_sw, 0, 1);
      }
   else if(relative_size > 0)
      {
      if(sign() == y.sign())
         bigint_sub2(mutable_data(), x_sw, y.data(), y_sw);
      else
         bigint_add2(mutable_data(), reg_size - 1, y.data(), y_sw);
      }

   return (*this);
   }

/*
* Multiplication Operator
*/
BigInt& BigInt::operator*=(const BigInt& y)
   {
   const size_t x_sw = sig_words(), y_sw = y.sig_words();
   set_sign((sign() == y.sign()) ? Positive : Negative);

   if(x_sw == 0 || y_sw == 0)
      {
      clear();
      set_sign(Positive);
      }
   else if(x_sw == 1 && y_sw)
      {
      grow_to(y_sw + 2);
      bigint_linmul3(mutable_data(), y.data(), y_sw, word_at(0));
      }
   else if(y_sw == 1 && x_sw)
      {
      grow_to(x_sw + 2);
      bigint_linmul2(mutable_data(), x_sw, y.word_at(0));
      }
   else
      {
      grow_to(size() + y.size());

      secure_vector<word> z(data(), data() + x_sw);
      secure_vector<word> workspace(size());

      bigint_mul(mutable_data(), size(), &workspace[0],
                 &z[0], z.size(), x_sw,
                 y.data(), y.size(), y_sw);
      }

   return (*this);
   }

/*
* Division Operator
*/
BigInt& BigInt::operator/=(const BigInt& y)
   {
   if(y.sig_words() == 1 && is_power_of_2(y.word_at(0)))
      (*this) >>= (y.bits() - 1);
   else
      (*this) = (*this) / y;
   return (*this);
   }

/*
* Modulo Operator
*/
BigInt& BigInt::operator%=(const BigInt& mod)
   {
   return (*this = (*this) % mod);
   }

/*
* Modulo Operator
*/
word BigInt::operator%=(word mod)
   {
   if(mod == 0)
      throw BigInt::DivideByZero();

   if(is_power_of_2(mod))
       {
       word result = (word_at(0) & (mod - 1));
       clear();
       grow_to(2);
       m_reg[0] = result;
       return result;
       }

   word remainder = 0;

   for(size_t j = sig_words(); j > 0; --j)
      remainder = bigint_modop(remainder, word_at(j-1), mod);
   clear();
   grow_to(2);

   if(remainder && sign() == BigInt::Negative)
      m_reg[0] = mod - remainder;
   else
      m_reg[0] = remainder;

   set_sign(BigInt::Positive);

   return word_at(0);
   }

/*
* Left Shift Operator
*/
BigInt& BigInt::operator<<=(size_t shift)
   {
   if(shift)
      {
      const size_t shift_words = shift / MP_WORD_BITS,
                   shift_bits  = shift % MP_WORD_BITS,
                   words = sig_words();

      grow_to(words + shift_words + (shift_bits ? 1 : 0));
      bigint_shl1(mutable_data(), words, shift_words, shift_bits);
      }

   return (*this);
   }

/*
* Right Shift Operator
*/
BigInt& BigInt::operator>>=(size_t shift)
   {
   if(shift)
      {
      const size_t shift_words = shift / MP_WORD_BITS,
                   shift_bits  = shift % MP_WORD_BITS;

      bigint_shr1(mutable_data(), sig_words(), shift_words, shift_bits);

      if(is_zero())
         set_sign(Positive);
      }

   return (*this);
   }

}
/*
* BigInt Binary Operators
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Addition Operator
*/
BigInt operator+(const BigInt& x, const BigInt& y)
   {
   const size_t x_sw = x.sig_words(), y_sw = y.sig_words();

   BigInt z(x.sign(), std::max(x_sw, y_sw) + 1);

   if((x.sign() == y.sign()))
      bigint_add3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
   else
      {
      s32bit relative_size = bigint_cmp(x.data(), x_sw, y.data(), y_sw);

      if(relative_size < 0)
         {
         bigint_sub3(z.mutable_data(), y.data(), y_sw, x.data(), x_sw);
         z.set_sign(y.sign());
         }
      else if(relative_size == 0)
         z.set_sign(BigInt::Positive);
      else if(relative_size > 0)
         bigint_sub3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      }

   return z;
   }

/*
* Subtraction Operator
*/
BigInt operator-(const BigInt& x, const BigInt& y)
   {
   const size_t x_sw = x.sig_words(), y_sw = y.sig_words();

   s32bit relative_size = bigint_cmp(x.data(), x_sw, y.data(), y_sw);

   BigInt z(BigInt::Positive, std::max(x_sw, y_sw) + 1);

   if(relative_size < 0)
      {
      if(x.sign() == y.sign())
         bigint_sub3(z.mutable_data(), y.data(), y_sw, x.data(), x_sw);
      else
         bigint_add3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      z.set_sign(y.reverse_sign());
      }
   else if(relative_size == 0)
      {
      if(x.sign() != y.sign())
         bigint_shl2(z.mutable_data(), x.data(), x_sw, 0, 1);
      }
   else if(relative_size > 0)
      {
      if(x.sign() == y.sign())
         bigint_sub3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      else
         bigint_add3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      z.set_sign(x.sign());
      }
   return z;
   }

/*
* Multiplication Operator
*/
BigInt operator*(const BigInt& x, const BigInt& y)
   {
   const size_t x_sw = x.sig_words(), y_sw = y.sig_words();

   BigInt z(BigInt::Positive, x.size() + y.size());

   if(x_sw == 1 && y_sw)
      bigint_linmul3(z.mutable_data(), y.data(), y_sw, x.word_at(0));
   else if(y_sw == 1 && x_sw)
      bigint_linmul3(z.mutable_data(), x.data(), x_sw, y.word_at(0));
   else if(x_sw && y_sw)
      {
      secure_vector<word> workspace(z.size());
      bigint_mul(z.mutable_data(), z.size(), &workspace[0],
                 x.data(), x.size(), x_sw,
                 y.data(), y.size(), y_sw);
      }

   if(x_sw && y_sw && x.sign() != y.sign())
      z.flip_sign();
   return z;
   }

/*
* Division Operator
*/
BigInt operator/(const BigInt& x, const BigInt& y)
   {
   BigInt q, r;
   divide(x, y, q, r);
   return q;
   }

/*
* Modulo Operator
*/
BigInt operator%(const BigInt& n, const BigInt& mod)
   {
   if(mod.is_zero())
      throw BigInt::DivideByZero();
   if(mod.is_negative())
      throw Invalid_Argument("BigInt::operator%: modulus must be > 0");
   if(n.is_positive() && mod.is_positive() && n < mod)
      return n;

   BigInt q, r;
   divide(n, mod, q, r);
   return r;
   }

/*
* Modulo Operator
*/
word operator%(const BigInt& n, word mod)
   {
   if(mod == 0)
      throw BigInt::DivideByZero();

   if(is_power_of_2(mod))
      return (n.word_at(0) & (mod - 1));

   word remainder = 0;

   for(size_t j = n.sig_words(); j > 0; --j)
      remainder = bigint_modop(remainder, n.word_at(j-1), mod);

   if(remainder && n.sign() == BigInt::Negative)
      return mod - remainder;
   return remainder;
   }

/*
* Left Shift Operator
*/
BigInt operator<<(const BigInt& x, size_t shift)
   {
   if(shift == 0)
      return x;

   const size_t shift_words = shift / MP_WORD_BITS,
                shift_bits  = shift % MP_WORD_BITS;

   const size_t x_sw = x.sig_words();

   BigInt y(x.sign(), x_sw + shift_words + (shift_bits ? 1 : 0));
   bigint_shl2(y.mutable_data(), x.data(), x_sw, shift_words, shift_bits);
   return y;
   }

/*
* Right Shift Operator
*/
BigInt operator>>(const BigInt& x, size_t shift)
   {
   if(shift == 0)
      return x;
   if(x.bits() <= shift)
      return 0;

   const size_t shift_words = shift / MP_WORD_BITS,
                shift_bits  = shift % MP_WORD_BITS,
                x_sw = x.sig_words();

   BigInt y(x.sign(), x_sw - shift_words);
   bigint_shr2(y.mutable_data(), x.data(), x_sw, shift_words, shift_bits);
   return y;
   }

}
/*
* BigInt Random Generation
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Randomize this number
*/
void BigInt::randomize(RandomNumberGenerator& rng,
                       size_t bitsize)
   {
   set_sign(Positive);

   if(bitsize == 0)
      clear();
   else
      {
      secure_vector<byte> array = rng.random_vec((bitsize + 7) / 8);

      if(bitsize % 8)
         array[0] &= 0xFF >> (8 - (bitsize % 8));
      array[0] |= 0x80 >> ((bitsize % 8) ? (8 - bitsize % 8) : 0);
      binary_decode(&array[0], array.size());
      }
   }

/*
* Generate a random integer within given range
*/
BigInt BigInt::random_integer(RandomNumberGenerator& rng,
                              const BigInt& min, const BigInt& max)
   {
   BigInt range = max - min;

   if(range <= 0)
      throw Invalid_Argument("random_integer: invalid min/max values");

   return (min + (BigInt(rng, range.bits() + 2) % range));
   }

}
/*
* BigInt Base
* (C) 1999-2011,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Construct a BigInt from a regular number
*/
BigInt::BigInt(u64bit n)
   {
   if(n == 0)
      return;

   const size_t limbs_needed = sizeof(u64bit) / sizeof(word);

   m_reg.resize(4*limbs_needed);
   for(size_t i = 0; i != limbs_needed; ++i)
      m_reg[i] = ((n >> (i*MP_WORD_BITS)) & MP_WORD_MASK);
   }

/*
* Construct a BigInt of the specified size
*/
BigInt::BigInt(Sign s, size_t size)
   {
   m_reg.resize(round_up<size_t>(size, 8));
   m_signedness = s;
   }

/*
* Copy constructor
*/
BigInt::BigInt(const BigInt& other)
   {
   m_reg = other.m_reg;
   m_signedness = other.m_signedness;
   }

/*
* Construct a BigInt from a string
*/
BigInt::BigInt(const std::string& str)
   {
   Base base = Decimal;
   size_t markers = 0;
   bool negative = false;

   if(str.length() > 0 && str[0] == '-')
      {
      markers += 1;
      negative = true;
      }

   if(str.length() > markers + 2 && str[markers    ] == '0' &&
                                    str[markers + 1] == 'x')
      {
      markers += 2;
      base = Hexadecimal;
      }

   *this = decode(reinterpret_cast<const byte*>(str.data()) + markers,
                  str.length() - markers, base);

   if(negative) set_sign(Negative);
   else         set_sign(Positive);
   }

/*
* Construct a BigInt from an encoded BigInt
*/
BigInt::BigInt(const byte input[], size_t length, Base base)
   {
   *this = decode(input, length, base);
   }

/*
* Construct a BigInt from an encoded BigInt
*/
BigInt::BigInt(RandomNumberGenerator& rng, size_t bits)
   {
   randomize(rng, bits);
   }

/*
* Grow the internal storage
*/
void BigInt::grow_to(size_t n)
   {
   if(n > size())
      m_reg.resize(round_up<size_t>(n, 8));
   }

/*
* Comparison Function
*/
s32bit BigInt::cmp(const BigInt& other, bool check_signs) const
   {
   if(check_signs)
      {
      if(other.is_positive() && this->is_negative())
         return -1;

      if(other.is_negative() && this->is_positive())
         return 1;

      if(other.is_negative() && this->is_negative())
         return (-bigint_cmp(this->data(), this->sig_words(),
                             other.data(), other.sig_words()));
      }

   return bigint_cmp(this->data(), this->sig_words(),
                     other.data(), other.sig_words());
   }

/*
* Return byte n of this number
*/
byte BigInt::byte_at(size_t n) const
   {
   const size_t WORD_BYTES = sizeof(word);
   size_t word_num = n / WORD_BYTES, byte_num = n % WORD_BYTES;
   if(word_num >= size())
      return 0;
   else
      return get_byte(WORD_BYTES - byte_num - 1, m_reg[word_num]);
   }

/*
* Return bit n of this number
*/
bool BigInt::get_bit(size_t n) const
   {
   return ((word_at(n / MP_WORD_BITS) >> (n % MP_WORD_BITS)) & 1);
   }

/*
* Return bits {offset...offset+length}
*/
u32bit BigInt::get_substring(size_t offset, size_t length) const
   {
   if(length > 32)
      throw Invalid_Argument("BigInt::get_substring: Substring size too big");

   u64bit piece = 0;
   for(size_t i = 0; i != 8; ++i)
      {
      const byte part = byte_at((offset / 8) + (7-i));
      piece = (piece << 8) | part;
      }

   const u64bit mask = (static_cast<u64bit>(1) << length) - 1;
   const size_t shift = (offset % 8);

   return static_cast<u32bit>((piece >> shift) & mask);
   }

/*
* Convert this number to a u32bit, if possible
*/
u32bit BigInt::to_u32bit() const
   {
   if(is_negative())
      throw Encoding_Error("BigInt::to_u32bit: Number is negative");
   if(bits() >= 32)
      throw Encoding_Error("BigInt::to_u32bit: Number is too big to convert");

   u32bit out = 0;
   for(u32bit j = 0; j != 4; ++j)
      out = (out << 8) | byte_at(3-j);
   return out;
   }

/*
* Set bit number n
*/
void BigInt::set_bit(size_t n)
   {
   const size_t which = n / MP_WORD_BITS;
   const word mask = static_cast<word>(1) << (n % MP_WORD_BITS);
   if(which >= size()) grow_to(which + 1);
   m_reg[which] |= mask;
   }

/*
* Clear bit number n
*/
void BigInt::clear_bit(size_t n)
   {
   const size_t which = n / MP_WORD_BITS;
   const word mask = static_cast<word>(1) << (n % MP_WORD_BITS);
   if(which < size())
      m_reg[which] &= ~mask;
   }

/*
* Clear all but the lowest n bits
*/
void BigInt::mask_bits(size_t n)
   {
   if(n == 0) { clear(); return; }
   if(n >= bits()) return;

   const size_t top_word = n / MP_WORD_BITS;
   const word mask = (static_cast<word>(1) << (n % MP_WORD_BITS)) - 1;

   if(top_word < size())
      clear_mem(&m_reg[top_word+1], size() - (top_word + 1));

   m_reg[top_word] &= mask;
   }

/*
* Count how many bytes are being used
*/
size_t BigInt::bytes() const
   {
   return (bits() + 7) / 8;
   }

/*
* Count how many bits are being used
*/
size_t BigInt::bits() const
   {
   const size_t words = sig_words();

   if(words == 0)
      return 0;

   size_t full_words = words - 1, top_bits = MP_WORD_BITS;
   word top_word = word_at(full_words), mask = MP_WORD_TOP_BIT;

   while(top_bits && ((top_word & mask) == 0))
      { mask >>= 1; top_bits--; }

   return (full_words * MP_WORD_BITS + top_bits);
   }

/*
* Calcluate the size in a certain base
*/
size_t BigInt::encoded_size(Base base) const
   {
   static const double LOG_2_BASE_10 = 0.30102999566;

   if(base == Binary)
      return bytes();
   else if(base == Hexadecimal)
      return 2*bytes();
   else if(base == Decimal)
      return static_cast<size_t>((bits() * LOG_2_BASE_10) + 1);
   else
      throw Invalid_Argument("Unknown base for BigInt encoding");
   }

/*
* Set the sign
*/
void BigInt::set_sign(Sign s)
   {
   if(is_zero())
      m_signedness = Positive;
   else
      m_signedness = s;
   }

/*
* Reverse the value of the sign flag
*/
void BigInt::flip_sign()
   {
   set_sign(reverse_sign());
   }

/*
* Return the opposite value of the current sign
*/
BigInt::Sign BigInt::reverse_sign() const
   {
   if(sign() == Positive)
      return Negative;
   return Positive;
   }

/*
* Return the negation of this number
*/
BigInt BigInt::operator-() const
   {
   BigInt x = (*this);
   x.flip_sign();
   return x;
   }

/*
* Return the absolute value of this number
*/
BigInt BigInt::abs() const
   {
   BigInt x = (*this);
   x.set_sign(Positive);
   return x;
   }

/*
* Encode this number into bytes
*/
void BigInt::binary_encode(byte output[]) const
   {
   const size_t sig_bytes = bytes();
   for(size_t i = 0; i != sig_bytes; ++i)
      output[sig_bytes-i-1] = byte_at(i);
   }

/*
* Set this number to the value in buf
*/
void BigInt::binary_decode(const byte buf[], size_t length)
   {
   const size_t WORD_BYTES = sizeof(word);

   clear();
   m_reg.resize(round_up<size_t>((length / WORD_BYTES) + 1, 8));

   for(size_t i = 0; i != length / WORD_BYTES; ++i)
      {
      const size_t top = length - WORD_BYTES*i;
      for(size_t j = WORD_BYTES; j > 0; --j)
         m_reg[i] = (m_reg[i] << 8) | buf[top - j];
      }

   for(size_t i = 0; i != length % WORD_BYTES; ++i)
      m_reg[length / WORD_BYTES] = (m_reg[length / WORD_BYTES] << 8) | buf[i];
   }

}
/*
* Division Algorithm
* (C) 1999-2007,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* Handle signed operands, if necessary
*/
void sign_fixup(const BigInt& x, const BigInt& y, BigInt& q, BigInt& r)
   {
   if(x.sign() == BigInt::Negative)
      {
      q.flip_sign();
      if(r.is_nonzero()) { --q; r = y.abs() - r; }
      }
   if(y.sign() == BigInt::Negative)
      q.flip_sign();
   }

bool division_check(word q, word y2, word y1,
                    word x3, word x2, word x1)
   {
   // Compute (y3,y2,y1) = (y2,y1) * q

   word y3 = 0;
   y1 = word_madd2(q, y1, &y3);
   y2 = word_madd2(q, y2, &y3);

   // Return (y3,y2,y1) >? (x3,x2,x1)

   if(y3 > x3) return true;
   if(y3 < x3) return false;

   if(y2 > x2) return true;
   if(y2 < x2) return false;

   if(y1 > x1) return true;
   if(y1 < x1) return false;

   return false;
   }

}

/*
* Solve x = q * y + r
*/
void divide(const BigInt& x, const BigInt& y_arg, BigInt& q, BigInt& r)
   {
   if(y_arg.is_zero())
      throw BigInt::DivideByZero();

   BigInt y = y_arg;
   const size_t y_words = y.sig_words();

   r = x;
   q = 0;

   r.set_sign(BigInt::Positive);
   y.set_sign(BigInt::Positive);

   s32bit compare = r.cmp(y);

   if(compare == 0)
      {
      q = 1;
      r = 0;
      }
   else if(compare > 0)
      {
      size_t shifts = 0;
      word y_top = y.word_at(y.sig_words()-1);
      while(y_top < MP_WORD_TOP_BIT) { y_top <<= 1; ++shifts; }
      y <<= shifts;
      r <<= shifts;

      const size_t n = r.sig_words() - 1, t = y_words - 1;

      if(n < t)
         throw Internal_Error("BigInt division word sizes");

      q.grow_to(n - t + 1);

      word* q_words = q.mutable_data();

      if(n <= t)
         {
         while(r > y) { r -= y; ++q; }
         r >>= shifts;
         sign_fixup(x, y_arg, q, r);
         return;
         }

      BigInt temp = y << (MP_WORD_BITS * (n-t));

      while(r >= temp) { r -= temp; q_words[n-t] += 1; }

      for(size_t j = n; j != t; --j)
         {
         const word x_j0  = r.word_at(j);
         const word x_j1 = r.word_at(j-1);
         const word y_t  = y.word_at(t);

         if(x_j0 == y_t)
            q_words[j-t-1] = MP_WORD_MAX;
         else
            q_words[j-t-1] = bigint_divop(x_j0, x_j1, y_t);

         while(division_check(q_words[j-t-1],
                              y_t, y.word_at(t-1),
                              x_j0, x_j1, r.word_at(j-2)))
            {
            q_words[j-t-1] -= 1;
            }

         r -= (q_words[j-t-1] * y) << (MP_WORD_BITS * (j-t-1));

         if(r.is_negative())
            {
            r += y << (MP_WORD_BITS * (j-t-1));
            q_words[j-t-1] -= 1;
            }
         }
      r >>= shifts;
      }

   sign_fixup(x, y_arg, q, r);
   }

}
/*
* Lowest Level MPI Algorithms
* (C) 1999-2010 Jack Lloyd
*     2006 Luca Piccarreta
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

extern "C" {

/*
* Two Operand Addition, No Carry
*/
word bigint_add2_nc(word x[], size_t x_size, const word y[], size_t y_size)
   {
   word carry = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_add2(x + i, y + i, carry);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_add(x[i], y[i], &carry);

   for(size_t i = y_size; i != x_size; ++i)
      x[i] = word_add(x[i], 0, &carry);

   return carry;
   }

/*
* Three Operand Addition, No Carry
*/
word bigint_add3_nc(word z[], const word x[], size_t x_size,
                              const word y[], size_t y_size)
   {
   if(x_size < y_size)
      { return bigint_add3_nc(z, y, y_size, x, x_size); }

   word carry = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_add3(z + i, x + i, y + i, carry);

   for(size_t i = blocks; i != y_size; ++i)
      z[i] = word_add(x[i], y[i], &carry);

   for(size_t i = y_size; i != x_size; ++i)
      z[i] = word_add(x[i], 0, &carry);

   return carry;
   }

/*
* Two Operand Addition
*/
void bigint_add2(word x[], size_t x_size, const word y[], size_t y_size)
   {
   if(bigint_add2_nc(x, x_size, y, y_size))
      x[x_size] += 1;
   }

/*
* Three Operand Addition
*/
void bigint_add3(word z[], const word x[], size_t x_size,
                           const word y[], size_t y_size)
   {
   z[(x_size > y_size ? x_size : y_size)] +=
      bigint_add3_nc(z, x, x_size, y, y_size);
   }

/*
* Two Operand Subtraction
*/
word bigint_sub2(word x[], size_t x_size, const word y[], size_t y_size)
   {
   word borrow = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub2(x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_sub(x[i], y[i], &borrow);

   for(size_t i = y_size; i != x_size; ++i)
      x[i] = word_sub(x[i], 0, &borrow);

   return borrow;
   }

/*
* Two Operand Subtraction x = y - x
*/
void bigint_sub2_rev(word x[],  const word y[], size_t y_size)
   {
   word borrow = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub2_rev(x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_sub(y[i], x[i], &borrow);

   if(borrow)
      throw Internal_Error("bigint_sub2_rev: x >= y");
   }

/*
* Three Operand Subtraction
*/
word bigint_sub3(word z[], const word x[], size_t x_size,
                           const word y[], size_t y_size)
   {
   word borrow = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub3(z + i, x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      z[i] = word_sub(x[i], y[i], &borrow);

   for(size_t i = y_size; i != x_size; ++i)
      z[i] = word_sub(x[i], 0, &borrow);

   return borrow;
   }

/*
* Two Operand Linear Multiply
*/
void bigint_linmul2(word x[], size_t x_size, word y)
   {
   const size_t blocks = x_size - (x_size % 8);

   word carry = 0;

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_linmul2(x + i, y, carry);

   for(size_t i = blocks; i != x_size; ++i)
      x[i] = word_madd2(x[i], y, &carry);

   x[x_size] = carry;
   }

/*
* Three Operand Linear Multiply
*/
void bigint_linmul3(word z[], const word x[], size_t x_size, word y)
   {
   const size_t blocks = x_size - (x_size % 8);

   word carry = 0;

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_linmul3(z + i, x + i, y, carry);

   for(size_t i = blocks; i != x_size; ++i)
      z[i] = word_madd2(x[i], y, &carry);

   z[x_size] = carry;
   }

}

}
/*
* Comba Multiplication and Squaring
* (C) 1999-2007,2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

extern "C" {

/*
* Comba 4x4 Squaring
*/
void bigint_comba_sqr4(word z[8], const word x[4])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0;
   z[ 7] = w1;
   }

/*
* Comba 4x4 Multiplication
*/
void bigint_comba_mul4(word z[8], const word x[4], const word y[4])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   z[ 6] = w0;
   z[ 7] = w1;
   }

/*
* Comba 6x6 Squaring
*/
void bigint_comba_sqr6(word z[12], const word x[6])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1;
   z[11] = w2;
   }

/*
* Comba 6x6 Multiplication
*/
void bigint_comba_mul6(word z[12], const word x[6], const word y[6])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   z[10] = w1;
   z[11] = w2;
   }

/*
* Comba 8x8 Squaring
*/
void bigint_comba_sqr8(word z[16], const word x[8])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 6]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 6]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 7]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 6]);
   z[11] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], x[ 6]);
   z[12] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[ 7]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 7], x[ 7]);
   z[14] = w2;
   z[15] = w0;
   }

/*
* Comba 8x8 Multiplication
*/
void bigint_comba_mul8(word z[16], const word x[8], const word y[8])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 0]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 0]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 1]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 2]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 3]);
   z[10] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 4]);
   z[11] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 5]);
   z[12] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 6]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 7]);
   z[14] = w2;
   z[15] = w0;
   }

/*
* Comba 16x16 Squaring
*/
void bigint_comba_sqr16(word z[32], const word x[16])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 6]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 8]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 6]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 9]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 8]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 7]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[10]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 9]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 8]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[11]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[10]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 9]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 8]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 6]);
   z[11] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[11]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[10]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 9]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 8]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], x[ 6]);
   z[12] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[12]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[11]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[10]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 9]);
   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[ 8]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[ 7]);
   z[13] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[12]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[11]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[10]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 9]);
   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 7], x[ 7]);
   z[14] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[13]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[11]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[10]);
   word3_muladd_2(&w2, &w1, &w0, x[ 6], x[ 9]);
   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[ 8]);
   z[15] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[12]);
   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[11]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[10]);
   word3_muladd_2(&w0, &w2, &w1, x[ 7], x[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 8], x[ 8]);
   z[16] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[12]);
   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[11]);
   word3_muladd_2(&w1, &w0, &w2, x[ 7], x[10]);
   word3_muladd_2(&w1, &w0, &w2, x[ 8], x[ 9]);
   z[17] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[13]);
   word3_muladd_2(&w2, &w1, &w0, x[ 6], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[11]);
   word3_muladd_2(&w2, &w1, &w0, x[ 8], x[10]);
   word3_muladd(&w2, &w1, &w0, x[ 9], x[ 9]);
   z[18] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[ 7], x[12]);
   word3_muladd_2(&w0, &w2, &w1, x[ 8], x[11]);
   word3_muladd_2(&w0, &w2, &w1, x[ 9], x[10]);
   z[19] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[ 7], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[ 8], x[12]);
   word3_muladd_2(&w1, &w0, &w2, x[ 9], x[11]);
   word3_muladd(&w1, &w0, &w2, x[10], x[10]);
   z[20] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 6], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[ 8], x[13]);
   word3_muladd_2(&w2, &w1, &w0, x[ 9], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[10], x[11]);
   z[21] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 7], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[ 8], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[ 9], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[10], x[12]);
   word3_muladd(&w0, &w2, &w1, x[11], x[11]);
   z[22] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 8], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[ 9], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[10], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[11], x[12]);
   z[23] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 9], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[10], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[11], x[13]);
   word3_muladd(&w2, &w1, &w0, x[12], x[12]);
   z[24] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[10], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[11], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[12], x[13]);
   z[25] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[11], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[12], x[14]);
   word3_muladd(&w1, &w0, &w2, x[13], x[13]);
   z[26] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[12], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[13], x[14]);
   z[27] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[13], x[15]);
   word3_muladd(&w0, &w2, &w1, x[14], x[14]);
   z[28] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[14], x[15]);
   z[29] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[15], x[15]);
   z[30] = w0;
   z[31] = w1;
   }

/*
* Comba 16x16 Multiplication
*/
void bigint_comba_mul16(word z[32], const word x[16], const word y[16])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 0]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 0]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 0]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 0]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[10]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 0]);
   z[10] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[11]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[10]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[10], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 0]);
   z[11] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[12]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[11]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[10]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[10], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[11], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 0]);
   z[12] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[13]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[12]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[11]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[10]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[11], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[12], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 0]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[14]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[13]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[12]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[11]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[10]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[10], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[12], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[13], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 0]);
   z[14] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[15]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[14]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[13]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[12]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[11]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[10]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[10], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[11], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[13], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[14], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 0]);
   z[15] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 1], y[15]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[14]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[13]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[12]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[11]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[10]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[11], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[12], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[14], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[15], y[ 1]);
   z[16] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 2], y[15]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[14]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[13]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[12]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[11]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[10]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[10], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[12], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[13], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[15], y[ 2]);
   z[17] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 3], y[15]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[14]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[13]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[12]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[11]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[10]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[10], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[11], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[13], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[14], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 3]);
   z[18] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 4], y[15]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[14]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[13]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[12]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[11]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[10]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[11], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[12], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[14], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[15], y[ 4]);
   z[19] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 5], y[15]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[14]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[13]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[12]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[11]);
   word3_muladd(&w1, &w0, &w2, x[10], y[10]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[12], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[13], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[15], y[ 5]);
   z[20] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 6], y[15]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[14]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[13]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[12]);
   word3_muladd(&w2, &w1, &w0, x[10], y[11]);
   word3_muladd(&w2, &w1, &w0, x[11], y[10]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[13], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[14], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 6]);
   z[21] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 7], y[15]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[14]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[13]);
   word3_muladd(&w0, &w2, &w1, x[10], y[12]);
   word3_muladd(&w0, &w2, &w1, x[11], y[11]);
   word3_muladd(&w0, &w2, &w1, x[12], y[10]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[14], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[15], y[ 7]);
   z[22] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 8], y[15]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[14]);
   word3_muladd(&w1, &w0, &w2, x[10], y[13]);
   word3_muladd(&w1, &w0, &w2, x[11], y[12]);
   word3_muladd(&w1, &w0, &w2, x[12], y[11]);
   word3_muladd(&w1, &w0, &w2, x[13], y[10]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[15], y[ 8]);
   z[23] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 9], y[15]);
   word3_muladd(&w2, &w1, &w0, x[10], y[14]);
   word3_muladd(&w2, &w1, &w0, x[11], y[13]);
   word3_muladd(&w2, &w1, &w0, x[12], y[12]);
   word3_muladd(&w2, &w1, &w0, x[13], y[11]);
   word3_muladd(&w2, &w1, &w0, x[14], y[10]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 9]);
   z[24] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[10], y[15]);
   word3_muladd(&w0, &w2, &w1, x[11], y[14]);
   word3_muladd(&w0, &w2, &w1, x[12], y[13]);
   word3_muladd(&w0, &w2, &w1, x[13], y[12]);
   word3_muladd(&w0, &w2, &w1, x[14], y[11]);
   word3_muladd(&w0, &w2, &w1, x[15], y[10]);
   z[25] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[11], y[15]);
   word3_muladd(&w1, &w0, &w2, x[12], y[14]);
   word3_muladd(&w1, &w0, &w2, x[13], y[13]);
   word3_muladd(&w1, &w0, &w2, x[14], y[12]);
   word3_muladd(&w1, &w0, &w2, x[15], y[11]);
   z[26] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[12], y[15]);
   word3_muladd(&w2, &w1, &w0, x[13], y[14]);
   word3_muladd(&w2, &w1, &w0, x[14], y[13]);
   word3_muladd(&w2, &w1, &w0, x[15], y[12]);
   z[27] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[13], y[15]);
   word3_muladd(&w0, &w2, &w1, x[14], y[14]);
   word3_muladd(&w0, &w2, &w1, x[15], y[13]);
   z[28] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[14], y[15]);
   word3_muladd(&w1, &w0, &w2, x[15], y[14]);
   z[29] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[15], y[15]);
   z[30] = w0;
   z[31] = w1;
   }

}

}
/*
* Karatsuba Multiplication/Squaring
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

static const size_t KARATSUBA_MULTIPLY_THRESHOLD = 32;
static const size_t KARATSUBA_SQUARE_THRESHOLD = 32;

/*
* Karatsuba Multiplication Operation
*/
void karatsuba_mul(word z[], const word x[], const word y[], size_t N,
                   word workspace[])
   {
   if(N < KARATSUBA_MULTIPLY_THRESHOLD || N % 2)
      {
      if(N == 6)
         return bigint_comba_mul6(z, x, y);
      else if(N == 8)
         return bigint_comba_mul8(z, x, y);
      else if(N == 16)
         return bigint_comba_mul16(z, x, y);
      else
         return bigint_simple_mul(z, x, N, y, N);
      }

   const size_t N2 = N / 2;

   const word* x0 = x;
   const word* x1 = x + N2;
   const word* y0 = y;
   const word* y1 = y + N2;
   word* z0 = z;
   word* z1 = z + N;

   const s32bit cmp0 = bigint_cmp(x0, N2, x1, N2);
   const s32bit cmp1 = bigint_cmp(y1, N2, y0, N2);

   clear_mem(workspace, 2*N);

   //if(cmp0 && cmp1)
      {
      if(cmp0 > 0)
         bigint_sub3(z0, x0, N2, x1, N2);
      else
         bigint_sub3(z0, x1, N2, x0, N2);

      if(cmp1 > 0)
         bigint_sub3(z1, y1, N2, y0, N2);
      else
         bigint_sub3(z1, y0, N2, y1, N2);

      karatsuba_mul(workspace, z0, z1, N2, workspace+N);
      }

   karatsuba_mul(z0, x0, y0, N2, workspace+N);
   karatsuba_mul(z1, x1, y1, N2, workspace+N);

   const word ws_carry = bigint_add3_nc(workspace + N, z0, N, z1, N);
   word z_carry = bigint_add2_nc(z + N2, N, workspace + N, N);

   z_carry += bigint_add2_nc(z + N + N2, N2, &ws_carry, 1);
   bigint_add2_nc(z + N + N2, N2, &z_carry, 1);

   if((cmp0 == cmp1) || (cmp0 == 0) || (cmp1 == 0))
      bigint_add2(z + N2, 2*N-N2, workspace, N);
   else
      bigint_sub2(z + N2, 2*N-N2, workspace, N);
   }

/*
* Karatsuba Squaring Operation
*/
void karatsuba_sqr(word z[], const word x[], size_t N, word workspace[])
   {
   if(N < KARATSUBA_SQUARE_THRESHOLD || N % 2)
      {
      if(N == 6)
         return bigint_comba_sqr6(z, x);
      else if(N == 8)
         return bigint_comba_sqr8(z, x);
      else if(N == 16)
         return bigint_comba_sqr16(z, x);
      else
         return bigint_simple_sqr(z, x, N);
      }

   const size_t N2 = N / 2;

   const word* x0 = x;
   const word* x1 = x + N2;
   word* z0 = z;
   word* z1 = z + N;

   const s32bit cmp = bigint_cmp(x0, N2, x1, N2);

   clear_mem(workspace, 2*N);

   //if(cmp)
      {
      if(cmp > 0)
         bigint_sub3(z0, x0, N2, x1, N2);
      else
         bigint_sub3(z0, x1, N2, x0, N2);

      karatsuba_sqr(workspace, z0, N2, workspace+N);
      }

   karatsuba_sqr(z0, x0, N2, workspace+N);
   karatsuba_sqr(z1, x1, N2, workspace+N);

   const word ws_carry = bigint_add3_nc(workspace + N, z0, N, z1, N);
   word z_carry = bigint_add2_nc(z + N2, N, workspace + N, N);

   z_carry += bigint_add2_nc(z + N + N2, N2, &ws_carry, 1);
   bigint_add2_nc(z + N + N2, N2, &z_carry, 1);

   /*
   * This is only actually required if cmp is != 0, however
   * if cmp==0 then workspace[0:N] == 0 and avoiding the jump
   * hides a timing channel.
   */
   bigint_sub2(z + N2, 2*N-N2, workspace, N);
   }

/*
* Pick a good size for the Karatsuba multiply
*/
size_t karatsuba_size(size_t z_size,
                      size_t x_size, size_t x_sw,
                      size_t y_size, size_t y_sw)
   {
   if(x_sw > x_size || x_sw > y_size || y_sw > x_size || y_sw > y_size)
      return 0;

   if(((x_size == x_sw) && (x_size % 2)) ||
      ((y_size == y_sw) && (y_size % 2)))
      return 0;

   const size_t start = (x_sw > y_sw) ? x_sw : y_sw;
   const size_t end = (x_size < y_size) ? x_size : y_size;

   if(start == end)
      {
      if(start % 2)
         return 0;
      return start;
      }

   for(size_t j = start; j <= end; ++j)
      {
      if(j % 2)
         continue;

      if(2*j > z_size)
         return 0;

      if(x_sw <= j && j <= x_size && y_sw <= j && j <= y_size)
         {
         if(j % 4 == 2 &&
            (j+2) <= x_size && (j+2) <= y_size && 2*(j+2) <= z_size)
            return j+2;
         return j;
         }
      }

   return 0;
   }

/*
* Pick a good size for the Karatsuba squaring
*/
size_t karatsuba_size(size_t z_size, size_t x_size, size_t x_sw)
   {
   if(x_sw == x_size)
      {
      if(x_sw % 2)
         return 0;
      return x_sw;
      }

   for(size_t j = x_sw; j <= x_size; ++j)
      {
      if(j % 2)
         continue;

      if(2*j > z_size)
         return 0;

      if(j % 4 == 2 && (j+2) <= x_size && 2*(j+2) <= z_size)
         return j+2;
      return j;
      }

   return 0;
   }

}

/*
* Multiplication Algorithm Dispatcher
*/
void bigint_mul(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw,
                const word y[], size_t y_size, size_t y_sw)
   {
   if(x_sw == 1)
      {
      bigint_linmul3(z, y, y_sw, x[0]);
      }
   else if(y_sw == 1)
      {
      bigint_linmul3(z, x, x_sw, y[0]);
      }
   else if(x_sw <= 4 && x_size >= 4 &&
           y_sw <= 4 && y_size >= 4 && z_size >= 8)
      {
      bigint_comba_mul4(z, x, y);
      }
   else if(x_sw <= 6 && x_size >= 6 &&
           y_sw <= 6 && y_size >= 6 && z_size >= 12)
      {
      bigint_comba_mul6(z, x, y);
      }
   else if(x_sw <= 8 && x_size >= 8 &&
           y_sw <= 8 && y_size >= 8 && z_size >= 16)
      {
      bigint_comba_mul8(z, x, y);
      }
   else if(x_sw <= 16 && x_size >= 16 &&
           y_sw <= 16 && y_size >= 16 && z_size >= 32)
      {
      bigint_comba_mul16(z, x, y);
      }
   else if(x_sw < KARATSUBA_MULTIPLY_THRESHOLD ||
           y_sw < KARATSUBA_MULTIPLY_THRESHOLD ||
           !workspace)
      {
      bigint_simple_mul(z, x, x_sw, y, y_sw);
      }
   else
      {
      const size_t N = karatsuba_size(z_size, x_size, x_sw, y_size, y_sw);

      if(N)
         karatsuba_mul(z, x, y, N, workspace);
      else
         bigint_simple_mul(z, x, x_sw, y, y_sw);
      }
   }

/*
* Squaring Algorithm Dispatcher
*/
void bigint_sqr(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw)
   {
   if(x_sw == 1)
      {
      bigint_linmul3(z, x, x_sw, x[0]);
      }
   else if(x_sw <= 4 && x_size >= 4 && z_size >= 8)
      {
      bigint_comba_sqr4(z, x);
      }
   else if(x_sw <= 6 && x_size >= 6 && z_size >= 12)
      {
      bigint_comba_sqr6(z, x);
      }
   else if(x_sw <= 8 && x_size >= 8 && z_size >= 16)
      {
      bigint_comba_sqr8(z, x);
      }
   else if(x_sw <= 16 && x_size >= 16 && z_size >= 32)
      {
      bigint_comba_sqr16(z, x);
      }
   else if(x_size < KARATSUBA_SQUARE_THRESHOLD || !workspace)
      {
      bigint_simple_sqr(z, x, x_sw);
      }
   else
      {
      const size_t N = karatsuba_size(z_size, x_size, x_sw);

      if(N)
         karatsuba_sqr(z, x, N, workspace);
      else
         bigint_simple_sqr(z, x, x_sw);
      }
   }

}
/*
* MP Misc Functions
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

extern "C" {

/*
* Compare two MP integers
*/
s32bit bigint_cmp(const word x[], size_t x_size,
                  const word y[], size_t y_size)
   {
   if(x_size < y_size) { return (-bigint_cmp(y, y_size, x, x_size)); }

   while(x_size > y_size)
      {
      if(x[x_size-1])
         return 1;
      x_size--;
      }

   for(size_t i = x_size; i > 0; --i)
      {
      if(x[i-1] > y[i-1])
         return 1;
      if(x[i-1] < y[i-1])
         return -1;
      }

   return 0;
   }

/*
* Do a 2-word/1-word Division
*/
word bigint_divop(word n1, word n0, word d)
   {
   word high = n1 % d, quotient = 0;

   for(size_t i = 0; i != MP_WORD_BITS; ++i)
      {
      word high_top_bit = (high & MP_WORD_TOP_BIT);

      high <<= 1;
      high |= (n0 >> (MP_WORD_BITS-1-i)) & 1;
      quotient <<= 1;

      if(high_top_bit || high >= d)
         {
         high -= d;
         quotient |= 1;
         }
      }

   return quotient;
   }

/*
* Do a 2-word/1-word Modulo
*/
word bigint_modop(word n1, word n0, word d)
   {
   word z = bigint_divop(n1, n0, d);
   word dummy = 0;
   z = word_madd2(z, d, &dummy);
   return (n0-z);
   }

}

}
/*
* Montgomery Reduction
* (C) 1999-2011 Jack Lloyd
*     2006 Luca Piccarreta
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

extern "C" {

/*
* Montgomery Reduction Algorithm
*/
void bigint_monty_redc(word z[],
                       const word p[], size_t p_size,
                       word p_dash, word ws[])
   {
   const size_t z_size = 2*(p_size+1);

   const size_t blocks_of_8 = p_size - (p_size % 8);

   for(size_t i = 0; i != p_size; ++i)
      {
      word* z_i = z + i;

      const word y = z_i[0] * p_dash;

      /*
      bigint_linmul3(ws, p, p_size, y);
      bigint_add2(z_i, z_size - i, ws, p_size+1);
      */

      word carry = 0;

      for(size_t j = 0; j != blocks_of_8; j += 8)
         carry = word8_madd3(z_i + j, p + j, y, carry);

      for(size_t j = blocks_of_8; j != p_size; ++j)
         z_i[j] = word_madd3(p[j], y, z_i[j], &carry);

      word z_sum = z_i[p_size] + carry;
      carry = (z_sum < z_i[p_size]);
      z_i[p_size] = z_sum;

      for(size_t j = p_size + 1; carry && j != z_size - i; ++j)
         {
         ++z_i[j];
         carry = !z_i[j];
         }
      }

   word borrow = 0;
   for(size_t i = 0; i != p_size; ++i)
      ws[i] = word_sub(z[p_size + i], p[i], &borrow);

   ws[p_size] = word_sub(z[p_size+p_size], 0, &borrow);

   copy_mem(ws + p_size + 1, z + p_size, p_size + 1);

   copy_mem(z, ws + borrow*(p_size+1), p_size + 1);
   clear_mem(z + p_size + 1, z_size - p_size - 1);
   }

void bigint_monty_mul(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word y[], size_t y_size, size_t y_sw,
                      const word p[], size_t p_size, word p_dash,
                      word ws[])
   {
   bigint_mul(&z[0], z_size, &ws[0],
              &x[0], x_size, x_sw,
              &y[0], y_size, y_sw);

   bigint_monty_redc(&z[0],
                     &p[0], p_size, p_dash,
                     &ws[0]);
   }

void bigint_monty_sqr(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word p[], size_t p_size, word p_dash,
                      word ws[])
   {
   bigint_sqr(&z[0], z_size, &ws[0],
              &x[0], x_size, x_sw);

   bigint_monty_redc(&z[0],
                     &p[0], p_size, p_dash,
                     &ws[0]);
   }

}

}
/*
* Simple O(N^2) Multiplication and Squaring
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

extern "C" {

/*
* Simple O(N^2) Multiplication
*/
void bigint_simple_mul(word z[], const word x[], size_t x_size,
                                 const word y[], size_t y_size)
   {
   const size_t x_size_8 = x_size - (x_size % 8);

   clear_mem(z, x_size + y_size);

   for(size_t i = 0; i != y_size; ++i)
      {
      const word y_i = y[i];

      word carry = 0;

      for(size_t j = 0; j != x_size_8; j += 8)
         carry = word8_madd3(z + i + j, x + j, y_i, carry);

      for(size_t j = x_size_8; j != x_size; ++j)
         z[i+j] = word_madd3(x[j], y_i, z[i+j], &carry);

      z[x_size+i] = carry;
      }
   }

/*
* Simple O(N^2) Squaring
*
* This is exactly the same algorithm as bigint_simple_mul, however
* because C/C++ compilers suck at alias analysis it is good to have
* the version where the compiler knows that x == y
*
* There is an O(n^1.5) squaring algorithm specified in Handbook of
* Applied Cryptography, chapter 14
*
*/
void bigint_simple_sqr(word z[], const word x[], size_t x_size)
   {
   const size_t x_size_8 = x_size - (x_size % 8);

   clear_mem(z, 2*x_size);

   for(size_t i = 0; i != x_size; ++i)
      {
      const word x_i = x[i];
      word carry = 0;

      for(size_t j = 0; j != x_size_8; j += 8)
         carry = word8_madd3(z + i + j, x + j, x_i, carry);

      for(size_t j = x_size_8; j != x_size; ++j)
         z[i+j] = word_madd3(x[j], x_i, z[i+j], &carry);

      z[x_size+i] = carry;
      }
   }

}

}
/*
* MP Shift Algorithms
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

extern "C" {

/*
* Single Operand Left Shift
*/
void bigint_shl1(word x[], size_t x_size, size_t word_shift, size_t bit_shift)
   {
   if(word_shift)
      {
      for(size_t j = 1; j != x_size + 1; ++j)
         x[(x_size - j) + word_shift] = x[x_size - j];
      clear_mem(x, word_shift);
      }

   if(bit_shift)
      {
      word carry = 0;
      for(size_t j = word_shift; j != x_size + word_shift + 1; ++j)
         {
         word temp = x[j];
         x[j] = (temp << bit_shift) | carry;
         carry = (temp >> (MP_WORD_BITS - bit_shift));
         }
      }
   }

/*
* Single Operand Right Shift
*/
void bigint_shr1(word x[], size_t x_size, size_t word_shift, size_t bit_shift)
   {
   if(x_size < word_shift)
      {
      clear_mem(x, x_size);
      return;
      }

   if(word_shift)
      {
      copy_mem(x, x + word_shift, x_size - word_shift);
      clear_mem(x + x_size - word_shift, word_shift);
      }

   if(bit_shift)
      {
      word carry = 0;

      size_t top = x_size - word_shift;

      while(top >= 4)
         {
         word w = x[top-1];
         x[top-1] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         w = x[top-2];
         x[top-2] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         w = x[top-3];
         x[top-3] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         w = x[top-4];
         x[top-4] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         top -= 4;
         }

      while(top)
         {
         word w = x[top-1];
         x[top-1] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         top--;
         }
      }
   }

/*
* Two Operand Left Shift
*/
void bigint_shl2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift)
   {
   for(size_t j = 0; j != x_size; ++j)
      y[j + word_shift] = x[j];
   if(bit_shift)
      {
      word carry = 0;
      for(size_t j = word_shift; j != x_size + word_shift + 1; ++j)
         {
         word w = y[j];
         y[j] = (w << bit_shift) | carry;
         carry = (w >> (MP_WORD_BITS - bit_shift));
         }
      }
   }

/*
* Two Operand Right Shift
*/
void bigint_shr2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift)
   {
   if(x_size < word_shift) return;

   for(size_t j = 0; j != x_size - word_shift; ++j)
      y[j] = x[j + word_shift];
   if(bit_shift)
      {
      word carry = 0;
      for(size_t j = x_size - word_shift; j > 0; --j)
         {
         word w = y[j-1];
         y[j-1] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));
         }
      }
   }

}

}
/*
* DSA Parameter Generation
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* Check if this size is allowed by FIPS 186-3
*/
bool fips186_3_valid_size(size_t pbits, size_t qbits)
   {
   if(qbits == 160)
      return (pbits == 512 || pbits == 768 || pbits == 1024);

   if(qbits == 224)
      return (pbits == 2048);

   if(qbits == 256)
      return (pbits == 2048 || pbits == 3072);

   return false;
   }

}

/*
* Attempt DSA prime generation with given seed
*/
bool generate_dsa_primes(RandomNumberGenerator& rng,
                         Algorithm_Factory& af,
                         BigInt& p, BigInt& q,
                         size_t pbits, size_t qbits,
                         const std::vector<byte>& seed_c)
   {
   if(!fips186_3_valid_size(pbits, qbits))
      throw Invalid_Argument(
         "FIPS 186-3 does not allow DSA domain parameters of " +
         std::to_string(pbits) + "/" + std::to_string(qbits) + " bits long");

   if(seed_c.size() * 8 < qbits)
      throw Invalid_Argument(
         "Generating a DSA parameter set with a " + std::to_string(qbits) +
         "long q requires a seed at least as many bits long");

   std::unique_ptr<HashFunction> hash(
      af.make_hash_function("SHA-" + std::to_string(qbits)));

   const size_t HASH_SIZE = hash->output_length();

   class Seed
      {
      public:
         Seed(const std::vector<byte>& s) : seed(s) {}

         operator std::vector<byte>& () { return seed; }

         Seed& operator++()
            {
            for(size_t j = seed.size(); j > 0; --j)
               if(++seed[j-1])
                  break;
            return (*this);
            }
      private:
         std::vector<byte> seed;
      };

   Seed seed(seed_c);

   q.binary_decode(hash->process(seed));
   q.set_bit(qbits-1);
   q.set_bit(0);

   if(!check_prime(q, rng))
      return false;

   const size_t n = (pbits-1) / (HASH_SIZE * 8),
                b = (pbits-1) % (HASH_SIZE * 8);

   BigInt X;
   std::vector<byte> V(HASH_SIZE * (n+1));

   for(size_t j = 0; j != 4096; ++j)
      {
      for(size_t k = 0; k <= n; ++k)
         {
         ++seed;
         hash->update(seed);
         hash->final(&V[HASH_SIZE * (n-k)]);
         }

      X.binary_decode(&V[HASH_SIZE - 1 - b/8],
                      V.size() - (HASH_SIZE - 1 - b/8));
      X.set_bit(pbits-1);

      p = X - (X % (2*q) - 1);

      if(p.bits() == pbits && check_prime(p, rng))
         return true;
      }
   return false;
   }

/*
* Generate DSA Primes
*/
std::vector<byte> generate_dsa_primes(RandomNumberGenerator& rng,
                                      Algorithm_Factory& af,
                                      BigInt& p, BigInt& q,
                                      size_t pbits, size_t qbits)
   {
   while(true)
      {
      std::vector<byte> seed(qbits / 8);
      rng.randomize(&seed[0], seed.size());

      if(generate_dsa_primes(rng, af, p, q, pbits, qbits, seed))
         return seed;
      }
   }

}
/*
* Jacobi Function
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Calculate the Jacobi symbol
*/
s32bit jacobi(const BigInt& a, const BigInt& n)
   {
   if(a.is_negative())
      throw Invalid_Argument("jacobi: first argument must be non-negative");
   if(n.is_even() || n < 2)
      throw Invalid_Argument("jacobi: second argument must be odd and > 1");

   BigInt x = a, y = n;
   s32bit J = 1;

   while(y > 1)
      {
      x %= y;
      if(x > y / 2)
         {
         x = y - x;
         if(y % 4 == 3)
            J = -J;
         }
      if(x.is_zero())
         return 0;

      size_t shifts = low_zero_bits(x);
      x >>= shifts;
      if(shifts % 2)
         {
         word y_mod_8 = y % 8;
         if(y_mod_8 == 3 || y_mod_8 == 5)
            J = -J;
         }

      if(x % 4 == 3 && y % 4 == 3)
         J = -J;
      std::swap(x, y);
      }
   return J;
   }

}
/*
* Prime Generation
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Generate a random prime
*/
BigInt random_prime(RandomNumberGenerator& rng,
                    size_t bits, const BigInt& coprime,
                    size_t equiv, size_t modulo)
   {
   if(bits <= 1)
      throw Invalid_Argument("random_prime: Can't make a prime of " +
                             std::to_string(bits) + " bits");
   else if(bits == 2)
      return ((rng.next_byte() % 2) ? 2 : 3);
   else if(bits == 3)
      return ((rng.next_byte() % 2) ? 5 : 7);
   else if(bits == 4)
      return ((rng.next_byte() % 2) ? 11 : 13);

   if(coprime <= 0)
      throw Invalid_Argument("random_prime: coprime must be > 0");
   if(modulo % 2 == 1 || modulo == 0)
      throw Invalid_Argument("random_prime: Invalid modulo value");
   if(equiv >= modulo || equiv % 2 == 0)
      throw Invalid_Argument("random_prime: equiv must be < modulo, and odd");

   while(true)
      {
      BigInt p(rng, bits);

      // Force lowest and two top bits on
      p.set_bit(bits - 1);
      p.set_bit(bits - 2);
      p.set_bit(0);

      if(p % modulo != equiv)
         p += (modulo - p % modulo) + equiv;

      const size_t sieve_size = std::min(bits / 2, PRIME_TABLE_SIZE);
      secure_vector<u16bit> sieve(sieve_size);

      for(size_t j = 0; j != sieve.size(); ++j)
         sieve[j] = p % PRIMES[j];

      size_t counter = 0;
      while(true)
         {
         if(counter == 4096 || p.bits() > bits)
            break;

         bool passes_sieve = true;
         ++counter;
         p += modulo;

         if(p.bits() > bits)
            break;

         for(size_t j = 0; j != sieve.size(); ++j)
            {
            sieve[j] = (sieve[j] + modulo) % PRIMES[j];
            if(sieve[j] == 0)
               passes_sieve = false;
            }

         if(!passes_sieve || gcd(p - 1, coprime) != 1)
            continue;
         if(check_prime(p, rng))
            return p;
         }
      }
   }

/*
* Generate a random safe prime
*/
BigInt random_safe_prime(RandomNumberGenerator& rng, size_t bits)
   {
   if(bits <= 64)
      throw Invalid_Argument("random_safe_prime: Can't make a prime of " +
                             std::to_string(bits) + " bits");

   BigInt p;
   do
      p = (random_prime(rng, bits - 1) << 1) + 1;
   while(!check_prime(p, rng));
   return p;
   }

}
/*
* Fused and Important MP Algorithms
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Square a BigInt
*/
BigInt square(const BigInt& x)
   {
   const size_t x_sw = x.sig_words();

   BigInt z(BigInt::Positive, round_up<size_t>(2*x_sw, 16));
   secure_vector<word> workspace(z.size());

   bigint_sqr(z.mutable_data(), z.size(),
              &workspace[0],
              x.data(), x.size(), x_sw);
   return z;
   }

/*
* Multiply-Add Operation
*/
BigInt mul_add(const BigInt& a, const BigInt& b, const BigInt& c)
   {
   if(c.is_negative() || c.is_zero())
      throw Invalid_Argument("mul_add: Third argument must be > 0");

   BigInt::Sign sign = BigInt::Positive;
   if(a.sign() != b.sign())
      sign = BigInt::Negative;

   const size_t a_sw = a.sig_words();
   const size_t b_sw = b.sig_words();
   const size_t c_sw = c.sig_words();

   BigInt r(sign, std::max(a.size() + b.size(), c_sw) + 1);
   secure_vector<word> workspace(r.size());

   bigint_mul(r.mutable_data(), r.size(),
              &workspace[0],
              a.data(), a.size(), a_sw,
              b.data(), b.size(), b_sw);

   const size_t r_size = std::max(r.sig_words(), c_sw);
   bigint_add2(r.mutable_data(), r_size, c.data(), c_sw);
   return r;
   }

/*
* Subtract-Multiply Operation
*/
BigInt sub_mul(const BigInt& a, const BigInt& b, const BigInt& c)
   {
   if(a.is_negative() || b.is_negative())
      throw Invalid_Argument("sub_mul: First two arguments must be >= 0");

   BigInt r = a;
   r -= b;
   r *= c;
   return r;
   }

}
/*
* Number Theory Functions
* (C) 1999-2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* Miller-Rabin Primality Tester
*/
class MillerRabin_Test
   {
   public:
      bool is_witness(const BigInt& nonce);
      MillerRabin_Test(const BigInt& num);
   private:
      BigInt n, r, n_minus_1;
      size_t s;
      Fixed_Exponent_Power_Mod pow_mod;
      Modular_Reducer reducer;
   };

/*
* Miller-Rabin Test, as described in Handbook of Applied Cryptography
* section 4.24
*/
bool MillerRabin_Test::is_witness(const BigInt& a)
   {
   if(a < 2 || a >= n_minus_1)
      throw Invalid_Argument("Bad size for nonce in Miller-Rabin test");

   BigInt y = pow_mod(a);
   if(y == 1 || y == n_minus_1)
      return false;

   for(size_t i = 1; i != s; ++i)
      {
      y = reducer.square(y);

      if(y == 1) // found a non-trivial square root
         return true;

      if(y == n_minus_1) // -1, trivial square root, so give up
         return false;
      }

   return true; // fails Fermat test
   }

/*
* Miller-Rabin Constructor
*/
MillerRabin_Test::MillerRabin_Test(const BigInt& num)
   {
   if(num.is_even() || num < 3)
      throw Invalid_Argument("MillerRabin_Test: Invalid number for testing");

   n = num;
   n_minus_1 = n - 1;
   s = low_zero_bits(n_minus_1);
   r = n_minus_1 >> s;

   pow_mod = Fixed_Exponent_Power_Mod(r, n);
   reducer = Modular_Reducer(n);
   }

/*
* Miller-Rabin Iterations
*/
size_t miller_rabin_test_iterations(size_t bits, size_t level)
   {
   struct mapping { size_t bits; size_t verify_iter; size_t check_iter; };

   const mapping tests[] = {
      {   50, 55, 25 },
      {  100, 38, 22 },
      {  160, 32, 18 },
      {  163, 31, 17 },
      {  168, 30, 16 },
      {  177, 29, 16 },
      {  181, 28, 15 },
      {  185, 27, 15 },
      {  190, 26, 15 },
      {  195, 25, 14 },
      {  201, 24, 14 },
      {  208, 23, 14 },
      {  215, 22, 13 },
      {  222, 21, 13 },
      {  231, 20, 13 },
      {  241, 19, 12 },
      {  252, 18, 12 },
      {  264, 17, 12 },
      {  278, 16, 11 },
      {  294, 15, 10 },
      {  313, 14,  9 },
      {  334, 13,  8 },
      {  360, 12,  8 },
      {  392, 11,  7 },
      {  430, 10,  7 },
      {  479,  9,  6 },
      {  542,  8,  6 },
      {  626,  7,  5 },
      {  746,  6,  4 },
      {  926,  5,  3 },
      { 1232,  4,  2 },
      { 1853,  3,  2 },
      {    0,  0,  0 }
   };

   for(size_t i = 0; tests[i].bits; ++i)
      {
      if(bits <= tests[i].bits)
         {
         if(level >= 2)
            return tests[i].verify_iter;
         else if(level == 1)
            return tests[i].check_iter;
         else if(level == 0)
            return std::max<size_t>(tests[i].check_iter / 4, 1);
         }
      }

   return level > 0 ? 2 : 1; // for large inputs
   }

}

/*
* Return the number of 0 bits at the end of n
*/
size_t low_zero_bits(const BigInt& n)
   {
   size_t low_zero = 0;

   if(n.is_positive() && n.is_nonzero())
      {
      for(size_t i = 0; i != n.size(); ++i)
         {
         const word x = n.word_at(i);

         if(x)
            {
            low_zero += ctz(x);
            break;
            }
         else
            low_zero += BOTAN_MP_WORD_BITS;
         }
      }

   return low_zero;
   }

/*
* Calculate the GCD
*/
BigInt gcd(const BigInt& a, const BigInt& b)
   {
   if(a.is_zero() || b.is_zero()) return 0;
   if(a == 1 || b == 1)           return 1;

   BigInt x = a, y = b;
   x.set_sign(BigInt::Positive);
   y.set_sign(BigInt::Positive);
   size_t shift = std::min(low_zero_bits(x), low_zero_bits(y));

   x >>= shift;
   y >>= shift;

   while(x.is_nonzero())
      {
      x >>= low_zero_bits(x);
      y >>= low_zero_bits(y);
      if(x >= y) { x -= y; x >>= 1; }
      else       { y -= x; y >>= 1; }
      }

   return (y << shift);
   }

/*
* Calculate the LCM
*/
BigInt lcm(const BigInt& a, const BigInt& b)
   {
   return ((a * b) / gcd(a, b));
   }

namespace {

/*
* If the modulus is odd, then we can avoid computing A and C. This is
* a critical path algorithm in some instances and an odd modulus is
* the common case for crypto, so worth special casing. See note 14.64
* in Handbook of Applied Cryptography for more details.
*/
BigInt inverse_mod_odd_modulus(const BigInt& n, const BigInt& mod)
   {
   BigInt u = mod, v = n;
   BigInt B = 0, D = 1;

   while(u.is_nonzero())
      {
      const size_t u_zero_bits = low_zero_bits(u);
      u >>= u_zero_bits;
      for(size_t i = 0; i != u_zero_bits; ++i)
         {
         if(B.is_odd())
            { B -= mod; }
         B >>= 1;
         }

      const size_t v_zero_bits = low_zero_bits(v);
      v >>= v_zero_bits;
      for(size_t i = 0; i != v_zero_bits; ++i)
         {
         if(D.is_odd())
            { D -= mod; }
         D >>= 1;
         }

      if(u >= v) { u -= v; B -= D; }
      else       { v -= u; D -= B; }
      }

   if(v != 1)
      return 0; // no modular inverse

   while(D.is_negative()) D += mod;
   while(D >= mod) D -= mod;

   return D;
   }

}

/*
* Find the Modular Inverse
*/
BigInt inverse_mod(const BigInt& n, const BigInt& mod)
   {
   if(mod.is_zero())
      throw BigInt::DivideByZero();
   if(mod.is_negative() || n.is_negative())
      throw Invalid_Argument("inverse_mod: arguments must be non-negative");

   if(n.is_zero() || (n.is_even() && mod.is_even()))
      return 0; // fast fail checks

   if(mod.is_odd())
      return inverse_mod_odd_modulus(n, mod);

   BigInt u = mod, v = n;
   BigInt A = 1, B = 0, C = 0, D = 1;

   while(u.is_nonzero())
      {
      const size_t u_zero_bits = low_zero_bits(u);
      u >>= u_zero_bits;
      for(size_t i = 0; i != u_zero_bits; ++i)
         {
         if(A.is_odd() || B.is_odd())
            { A += n; B -= mod; }
         A >>= 1; B >>= 1;
         }

      const size_t v_zero_bits = low_zero_bits(v);
      v >>= v_zero_bits;
      for(size_t i = 0; i != v_zero_bits; ++i)
         {
         if(C.is_odd() || D.is_odd())
            { C += n; D -= mod; }
         C >>= 1; D >>= 1;
         }

      if(u >= v) { u -= v; A -= C; B -= D; }
      else       { v -= u; C -= A; D -= B; }
      }

   if(v != 1)
      return 0; // no modular inverse

   while(D.is_negative()) D += mod;
   while(D >= mod) D -= mod;

   return D;
   }

word monty_inverse(word input)
   {
   word b = input;
   word x2 = 1, x1 = 0, y2 = 0, y1 = 1;

   // First iteration, a = n+1
   word q = bigint_divop(1, 0, b);
   word r = (MP_WORD_MAX - q*b) + 1;
   word x = x2 - q*x1;
   word y = y2 - q*y1;

   word a = b;
   b = r;
   x2 = x1;
   x1 = x;
   y2 = y1;
   y1 = y;

   while(b > 0)
      {
      q = a / b;
      r = a - q*b;
      x = x2 - q*x1;
      y = y2 - q*y1;

      a = b;
      b = r;
      x2 = x1;
      x1 = x;
      y2 = y1;
      y1 = y;
      }

   // Now invert in addition space
   y2 = (MP_WORD_MAX - y2) + 1;

   return y2;
   }

/*
* Modular Exponentiation
*/
BigInt power_mod(const BigInt& base, const BigInt& exp, const BigInt& mod)
   {
   Power_Mod pow_mod(mod);

   /*
   * Calling set_base before set_exponent means we end up using a
   * minimal window. This makes sense given that here we know that any
   * precomputation is wasted.
   */
   pow_mod.set_base(base);
   pow_mod.set_exponent(exp);
   return pow_mod.execute();
   }

/*
* Test for primaility using Miller-Rabin
*/
bool primality_test(const BigInt& n,
                    RandomNumberGenerator& rng,
                    size_t level)
   {
   if(n == 2)
      return true;
   if(n <= 1 || n.is_even())
      return false;

   // Fast path testing for small numbers (<= 65521)
   if(n <= PRIMES[PRIME_TABLE_SIZE-1])
      {
      const word num = n.word_at(0);

      for(size_t i = 0; PRIMES[i]; ++i)
         {
         if(num == PRIMES[i])
            return true;
         if(num < PRIMES[i])
            return false;
         }

      return false;
      }

   if(level > 2)
      level = 2;

   const size_t PREF_NONCE_BITS = 192;

   const size_t NONCE_BITS = std::min(n.bits() - 2, PREF_NONCE_BITS);

   MillerRabin_Test mr(n);

   if(mr.is_witness(2))
      return false;

   const size_t tests = miller_rabin_test_iterations(n.bits(), level);

   for(size_t i = 0; i != tests; ++i)
      {
      BigInt nonce;
      while(nonce < 2 || nonce >= (n-1))
         nonce.randomize(rng, NONCE_BITS);

      if(mr.is_witness(nonce))
         return false;
      }

   return true;
   }

}
/*
* Modular Exponentiation Proxy
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Power_Mod Constructor
*/
Power_Mod::Power_Mod(const BigInt& n, Usage_Hints hints)
   {
   core = nullptr;
   set_modulus(n, hints);
   }

/*
* Power_Mod Copy Constructor
*/
Power_Mod::Power_Mod(const Power_Mod& other)
   {
   core = nullptr;
   if(other.core)
      core = other.core->copy();
   }

/*
* Power_Mod Assignment Operator
*/
Power_Mod& Power_Mod::operator=(const Power_Mod& other)
   {
   delete core;
   core = nullptr;
   if(other.core)
      core = other.core->copy();
   return (*this);
   }

/*
* Power_Mod Destructor
*/
Power_Mod::~Power_Mod()
   {
   delete core;
   }

/*
* Set the modulus
*/
void Power_Mod::set_modulus(const BigInt& n, Usage_Hints hints) const
   {
   delete core;
   core = nullptr;

   if(n != 0)
      {
      Algorithm_Factory::Engine_Iterator i(global_state().algorithm_factory());

      while(const Engine* engine = i.next())
         {
         core = engine->mod_exp(n, hints);

         if(core)
            break;
         }

      if(!core)
         throw Lookup_Error("Power_Mod: Unable to find a working engine");
      }
   }

/*
* Set the base
*/
void Power_Mod::set_base(const BigInt& b) const
   {
   if(b.is_zero() || b.is_negative())
      throw Invalid_Argument("Power_Mod::set_base: arg must be > 0");

   if(!core)
      throw Internal_Error("Power_Mod::set_base: core was NULL");
   core->set_base(b);
   }

/*
* Set the exponent
*/
void Power_Mod::set_exponent(const BigInt& e) const
   {
   if(e.is_negative())
      throw Invalid_Argument("Power_Mod::set_exponent: arg must be > 0");

   if(!core)
      throw Internal_Error("Power_Mod::set_exponent: core was NULL");
   core->set_exponent(e);
   }

/*
* Compute the result
*/
BigInt Power_Mod::execute() const
   {
   if(!core)
      throw Internal_Error("Power_Mod::execute: core was NULL");
   return core->execute();
   }

/*
* Try to choose a good window size
*/
size_t Power_Mod::window_bits(size_t exp_bits, size_t,
                              Power_Mod::Usage_Hints hints)
   {
   static const size_t wsize[][2] = {
      { 1434, 7 },
      {  539, 6 },
      {  197, 4 },
      {   70, 3 },
      {   25, 2 },
      {    0, 0 }
   };

   size_t window_bits = 1;

   if(exp_bits)
      {
      for(size_t j = 0; wsize[j][0]; ++j)
         {
         if(exp_bits >= wsize[j][0])
            {
            window_bits += wsize[j][1];
            break;
            }
         }
      }

   if(hints & Power_Mod::BASE_IS_FIXED)
      window_bits += 2;
   if(hints & Power_Mod::EXP_IS_LARGE)
      ++window_bits;

   return window_bits;
   }

namespace {

/*
* Choose potentially useful hints
*/
Power_Mod::Usage_Hints choose_base_hints(const BigInt& b, const BigInt& n)
   {
   if(b == 2)
      return Power_Mod::Usage_Hints(Power_Mod::BASE_IS_2 |
                                    Power_Mod::BASE_IS_SMALL);

   const size_t b_bits = b.bits();
   const size_t n_bits = n.bits();

   if(b_bits < n_bits / 32)
      return Power_Mod::BASE_IS_SMALL;
   if(b_bits > n_bits / 4)
      return Power_Mod::BASE_IS_LARGE;

   return Power_Mod::NO_HINTS;
   }

/*
* Choose potentially useful hints
*/
Power_Mod::Usage_Hints choose_exp_hints(const BigInt& e, const BigInt& n)
   {
   const size_t e_bits = e.bits();
   const size_t n_bits = n.bits();

   if(e_bits < n_bits / 32)
      return Power_Mod::BASE_IS_SMALL;
   if(e_bits > n_bits / 4)
      return Power_Mod::BASE_IS_LARGE;
   return Power_Mod::NO_HINTS;
   }

}

/*
* Fixed_Exponent_Power_Mod Constructor
*/
Fixed_Exponent_Power_Mod::Fixed_Exponent_Power_Mod(const BigInt& e,
                                                   const BigInt& n,
                                                   Usage_Hints hints) :
   Power_Mod(n, Usage_Hints(hints | EXP_IS_FIXED | choose_exp_hints(e, n)))
   {
   set_exponent(e);
   }

/*
* Fixed_Base_Power_Mod Constructor
*/
Fixed_Base_Power_Mod::Fixed_Base_Power_Mod(const BigInt& b, const BigInt& n,
                                           Usage_Hints hints) :
   Power_Mod(n, Usage_Hints(hints | BASE_IS_FIXED | choose_base_hints(b, n)))
   {
   set_base(b);
   }

}
/*
* Fixed Window Exponentiation
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Set the exponent
*/
void Fixed_Window_Exponentiator::set_exponent(const BigInt& e)
   {
   exp = e;
   }

/*
* Set the base
*/
void Fixed_Window_Exponentiator::set_base(const BigInt& base)
   {
   window_bits = Power_Mod::window_bits(exp.bits(), base.bits(), hints);

   g.resize((1 << window_bits));
   g[0] = 1;
   g[1] = base;

   for(size_t i = 2; i != g.size(); ++i)
      g[i] = reducer.multiply(g[i-1], g[0]);
   }

/*
* Compute the result
*/
BigInt Fixed_Window_Exponentiator::execute() const
   {
   const size_t exp_nibbles = (exp.bits() + window_bits - 1) / window_bits;

   BigInt x = 1;

   for(size_t i = exp_nibbles; i > 0; --i)
      {
      for(size_t j = 0; j != window_bits; ++j)
         x = reducer.square(x);

      const u32bit nibble = exp.get_substring(window_bits*(i-1), window_bits);

      x = reducer.multiply(x, g[nibble]);
      }
   return x;
   }

/*
* Fixed_Window_Exponentiator Constructor
*/
Fixed_Window_Exponentiator::Fixed_Window_Exponentiator(const BigInt& n,
                                                       Power_Mod::Usage_Hints hints)
   {
   reducer = Modular_Reducer(n);
   this->hints = hints;
   window_bits = 0;
   }

}
/*
* Montgomery Exponentiation
* (C) 1999-2010,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Set the exponent
*/
void Montgomery_Exponentiator::set_exponent(const BigInt& exp)
   {
   m_exp = exp;
   m_exp_bits = exp.bits();
   }

/*
* Set the base
*/
void Montgomery_Exponentiator::set_base(const BigInt& base)
   {
   m_window_bits = Power_Mod::window_bits(m_exp.bits(), base.bits(), m_hints);

   m_g.resize((1 << m_window_bits));

   BigInt z(BigInt::Positive, 2 * (m_mod_words + 1));
   secure_vector<word> workspace(z.size());

   m_g[0] = 1;

   bigint_monty_mul(z.mutable_data(), z.size(),
                    m_g[0].data(), m_g[0].size(), m_g[0].sig_words(),
                    m_R2_mod.data(), m_R2_mod.size(), m_R2_mod.sig_words(),
                    m_modulus.data(), m_mod_words, m_mod_prime,
                    &workspace[0]);

   m_g[0] = z;

   m_g[1] = (base >= m_modulus) ? (base % m_modulus) : base;

   bigint_monty_mul(z.mutable_data(), z.size(),
                    m_g[1].data(), m_g[1].size(), m_g[1].sig_words(),
                    m_R2_mod.data(), m_R2_mod.size(), m_R2_mod.sig_words(),
                    m_modulus.data(), m_mod_words, m_mod_prime,
                    &workspace[0]);

   m_g[1] = z;

   const BigInt& x = m_g[1];
   const size_t x_sig = x.sig_words();

   for(size_t i = 2; i != m_g.size(); ++i)
      {
      const BigInt& y = m_g[i-1];
      const size_t y_sig = y.sig_words();

      bigint_monty_mul(z.mutable_data(), z.size(),
                       x.data(), x.size(), x_sig,
                       y.data(), y.size(), y_sig,
                       m_modulus.data(), m_mod_words, m_mod_prime,
                       &workspace[0]);

      m_g[i] = z;
      }
   }

/*
* Compute the result
*/
BigInt Montgomery_Exponentiator::execute() const
   {
   const size_t exp_nibbles = (m_exp_bits + m_window_bits - 1) / m_window_bits;

   BigInt x = m_R_mod;

   const size_t z_size = 2*(m_mod_words + 1);

   BigInt z(BigInt::Positive, z_size);
   secure_vector<word> workspace(z_size);

   for(size_t i = exp_nibbles; i > 0; --i)
      {
      for(size_t k = 0; k != m_window_bits; ++k)
         {
         bigint_monty_sqr(z.mutable_data(), z_size,
                          x.data(), x.size(), x.sig_words(),
                          m_modulus.data(), m_mod_words, m_mod_prime,
                          &workspace[0]);

         x = z;
         }

      const u32bit nibble = m_exp.get_substring(m_window_bits*(i-1), m_window_bits);

      const BigInt& y = m_g[nibble];

      bigint_monty_mul(z.mutable_data(), z_size,
                       x.data(), x.size(), x.sig_words(),
                       y.data(), y.size(), y.sig_words(),
                       m_modulus.data(), m_mod_words, m_mod_prime,
                       &workspace[0]);

      x = z;
      }

   x.grow_to(2*m_mod_words + 1);

   bigint_monty_redc(x.mutable_data(),
                     m_modulus.data(), m_mod_words, m_mod_prime,
                     &workspace[0]);

   return x;
   }

/*
* Montgomery_Exponentiator Constructor
*/
Montgomery_Exponentiator::Montgomery_Exponentiator(const BigInt& mod,
                                                   Power_Mod::Usage_Hints hints) :
   m_modulus(mod),
   m_mod_words(m_modulus.sig_words()),
   m_window_bits(1),
   m_hints(hints)
   {
   // Montgomery reduction only works for positive odd moduli
   if(!m_modulus.is_positive() || m_modulus.is_even())
      throw Invalid_Argument("Montgomery_Exponentiator: invalid modulus");

   m_mod_prime = monty_inverse(mod.word_at(0));

   const BigInt r = BigInt::power_of_2(m_mod_words * BOTAN_MP_WORD_BITS);
   m_R_mod = r % m_modulus;
   m_R2_mod = (m_R_mod * m_R_mod) % m_modulus;
   }

}
/*
* Small Primes Table
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

const u16bit PRIMES[PRIME_TABLE_SIZE+1] = {
    3,     5,     7,    11,    13,    17,    19,    23,    29,    31,    37,
   41,    43,    47,    53,    59,    61,    67,    71,    73,    79,    83,
   89,    97,   101,   103,   107,   109,   113,   127,   131,   137,   139,
  149,   151,   157,   163,   167,   173,   179,   181,   191,   193,   197,
  199,   211,   223,   227,   229,   233,   239,   241,   251,   257,   263,
  269,   271,   277,   281,   283,   293,   307,   311,   313,   317,   331,
  337,   347,   349,   353,   359,   367,   373,   379,   383,   389,   397,
  401,   409,   419,   421,   431,   433,   439,   443,   449,   457,   461,
  463,   467,   479,   487,   491,   499,   503,   509,   521,   523,   541,
  547,   557,   563,   569,   571,   577,   587,   593,   599,   601,   607,
  613,   617,   619,   631,   641,   643,   647,   653,   659,   661,   673,
  677,   683,   691,   701,   709,   719,   727,   733,   739,   743,   751,
  757,   761,   769,   773,   787,   797,   809,   811,   821,   823,   827,
  829,   839,   853,   857,   859,   863,   877,   881,   883,   887,   907,
  911,   919,   929,   937,   941,   947,   953,   967,   971,   977,   983,
  991,   997,  1009,  1013,  1019,  1021,  1031,  1033,  1039,  1049,  1051,
 1061,  1063,  1069,  1087,  1091,  1093,  1097,  1103,  1109,  1117,  1123,
 1129,  1151,  1153,  1163,  1171,  1181,  1187,  1193,  1201,  1213,  1217,
 1223,  1229,  1231,  1237,  1249,  1259,  1277,  1279,  1283,  1289,  1291,
 1297,  1301,  1303,  1307,  1319,  1321,  1327,  1361,  1367,  1373,  1381,
 1399,  1409,  1423,  1427,  1429,  1433,  1439,  1447,  1451,  1453,  1459,
 1471,  1481,  1483,  1487,  1489,  1493,  1499,  1511,  1523,  1531,  1543,
 1549,  1553,  1559,  1567,  1571,  1579,  1583,  1597,  1601,  1607,  1609,
 1613,  1619,  1621,  1627,  1637,  1657,  1663,  1667,  1669,  1693,  1697,
 1699,  1709,  1721,  1723,  1733,  1741,  1747,  1753,  1759,  1777,  1783,
 1787,  1789,  1801,  1811,  1823,  1831,  1847,  1861,  1867,  1871,  1873,
 1877,  1879,  1889,  1901,  1907,  1913,  1931,  1933,  1949,  1951,  1973,
 1979,  1987,  1993,  1997,  1999,  2003,  2011,  2017,  2027,  2029,  2039,
 2053,  2063,  2069,  2081,  2083,  2087,  2089,  2099,  2111,  2113,  2129,
 2131,  2137,  2141,  2143,  2153,  2161,  2179,  2203,  2207,  2213,  2221,
 2237,  2239,  2243,  2251,  2267,  2269,  2273,  2281,  2287,  2293,  2297,
 2309,  2311,  2333,  2339,  2341,  2347,  2351,  2357,  2371,  2377,  2381,
 2383,  2389,  2393,  2399,  2411,  2417,  2423,  2437,  2441,  2447,  2459,
 2467,  2473,  2477,  2503,  2521,  2531,  2539,  2543,  2549,  2551,  2557,
 2579,  2591,  2593,  2609,  2617,  2621,  2633,  2647,  2657,  2659,  2663,
 2671,  2677,  2683,  2687,  2689,  2693,  2699,  2707,  2711,  2713,  2719,
 2729,  2731,  2741,  2749,  2753,  2767,  2777,  2789,  2791,  2797,  2801,
 2803,  2819,  2833,  2837,  2843,  2851,  2857,  2861,  2879,  2887,  2897,
 2903,  2909,  2917,  2927,  2939,  2953,  2957,  2963,  2969,  2971,  2999,
 3001,  3011,  3019,  3023,  3037,  3041,  3049,  3061,  3067,  3079,  3083,
 3089,  3109,  3119,  3121,  3137,  3163,  3167,  3169,  3181,  3187,  3191,
 3203,  3209,  3217,  3221,  3229,  3251,  3253,  3257,  3259,  3271,  3299,
 3301,  3307,  3313,  3319,  3323,  3329,  3331,  3343,  3347,  3359,  3361,
 3371,  3373,  3389,  3391,  3407,  3413,  3433,  3449,  3457,  3461,  3463,
 3467,  3469,  3491,  3499,  3511,  3517,  3527,  3529,  3533,  3539,  3541,
 3547,  3557,  3559,  3571,  3581,  3583,  3593,  3607,  3613,  3617,  3623,
 3631,  3637,  3643,  3659,  3671,  3673,  3677,  3691,  3697,  3701,  3709,
 3719,  3727,  3733,  3739,  3761,  3767,  3769,  3779,  3793,  3797,  3803,
 3821,  3823,  3833,  3847,  3851,  3853,  3863,  3877,  3881,  3889,  3907,
 3911,  3917,  3919,  3923,  3929,  3931,  3943,  3947,  3967,  3989,  4001,
 4003,  4007,  4013,  4019,  4021,  4027,  4049,  4051,  4057,  4073,  4079,
 4091,  4093,  4099,  4111,  4127,  4129,  4133,  4139,  4153,  4157,  4159,
 4177,  4201,  4211,  4217,  4219,  4229,  4231,  4241,  4243,  4253,  4259,
 4261,  4271,  4273,  4283,  4289,  4297,  4327,  4337,  4339,  4349,  4357,
 4363,  4373,  4391,  4397,  4409,  4421,  4423,  4441,  4447,  4451,  4457,
 4463,  4481,  4483,  4493,  4507,  4513,  4517,  4519,  4523,  4547,  4549,
 4561,  4567,  4583,  4591,  4597,  4603,  4621,  4637,  4639,  4643,  4649,
 4651,  4657,  4663,  4673,  4679,  4691,  4703,  4721,  4723,  4729,  4733,
 4751,  4759,  4783,  4787,  4789,  4793,  4799,  4801,  4813,  4817,  4831,
 4861,  4871,  4877,  4889,  4903,  4909,  4919,  4931,  4933,  4937,  4943,
 4951,  4957,  4967,  4969,  4973,  4987,  4993,  4999,  5003,  5009,  5011,
 5021,  5023,  5039,  5051,  5059,  5077,  5081,  5087,  5099,  5101,  5107,
 5113,  5119,  5147,  5153,  5167,  5171,  5179,  5189,  5197,  5209,  5227,
 5231,  5233,  5237,  5261,  5273,  5279,  5281,  5297,  5303,  5309,  5323,
 5333,  5347,  5351,  5381,  5387,  5393,  5399,  5407,  5413,  5417,  5419,
 5431,  5437,  5441,  5443,  5449,  5471,  5477,  5479,  5483,  5501,  5503,
 5507,  5519,  5521,  5527,  5531,  5557,  5563,  5569,  5573,  5581,  5591,
 5623,  5639,  5641,  5647,  5651,  5653,  5657,  5659,  5669,  5683,  5689,
 5693,  5701,  5711,  5717,  5737,  5741,  5743,  5749,  5779,  5783,  5791,
 5801,  5807,  5813,  5821,  5827,  5839,  5843,  5849,  5851,  5857,  5861,
 5867,  5869,  5879,  5881,  5897,  5903,  5923,  5927,  5939,  5953,  5981,
 5987,  6007,  6011,  6029,  6037,  6043,  6047,  6053,  6067,  6073,  6079,
 6089,  6091,  6101,  6113,  6121,  6131,  6133,  6143,  6151,  6163,  6173,
 6197,  6199,  6203,  6211,  6217,  6221,  6229,  6247,  6257,  6263,  6269,
 6271,  6277,  6287,  6299,  6301,  6311,  6317,  6323,  6329,  6337,  6343,
 6353,  6359,  6361,  6367,  6373,  6379,  6389,  6397,  6421,  6427,  6449,
 6451,  6469,  6473,  6481,  6491,  6521,  6529,  6547,  6551,  6553,  6563,
 6569,  6571,  6577,  6581,  6599,  6607,  6619,  6637,  6653,  6659,  6661,
 6673,  6679,  6689,  6691,  6701,  6703,  6709,  6719,  6733,  6737,  6761,
 6763,  6779,  6781,  6791,  6793,  6803,  6823,  6827,  6829,  6833,  6841,
 6857,  6863,  6869,  6871,  6883,  6899,  6907,  6911,  6917,  6947,  6949,
 6959,  6961,  6967,  6971,  6977,  6983,  6991,  6997,  7001,  7013,  7019,
 7027,  7039,  7043,  7057,  7069,  7079,  7103,  7109,  7121,  7127,  7129,
 7151,  7159,  7177,  7187,  7193,  7207,  7211,  7213,  7219,  7229,  7237,
 7243,  7247,  7253,  7283,  7297,  7307,  7309,  7321,  7331,  7333,  7349,
 7351,  7369,  7393,  7411,  7417,  7433,  7451,  7457,  7459,  7477,  7481,
 7487,  7489,  7499,  7507,  7517,  7523,  7529,  7537,  7541,  7547,  7549,
 7559,  7561,  7573,  7577,  7583,  7589,  7591,  7603,  7607,  7621,  7639,
 7643,  7649,  7669,  7673,  7681,  7687,  7691,  7699,  7703,  7717,  7723,
 7727,  7741,  7753,  7757,  7759,  7789,  7793,  7817,  7823,  7829,  7841,
 7853,  7867,  7873,  7877,  7879,  7883,  7901,  7907,  7919,  7927,  7933,
 7937,  7949,  7951,  7963,  7993,  8009,  8011,  8017,  8039,  8053,  8059,
 8069,  8081,  8087,  8089,  8093,  8101,  8111,  8117,  8123,  8147,  8161,
 8167,  8171,  8179,  8191,  8209,  8219,  8221,  8231,  8233,  8237,  8243,
 8263,  8269,  8273,  8287,  8291,  8293,  8297,  8311,  8317,  8329,  8353,
 8363,  8369,  8377,  8387,  8389,  8419,  8423,  8429,  8431,  8443,  8447,
 8461,  8467,  8501,  8513,  8521,  8527,  8537,  8539,  8543,  8563,  8573,
 8581,  8597,  8599,  8609,  8623,  8627,  8629,  8641,  8647,  8663,  8669,
 8677,  8681,  8689,  8693,  8699,  8707,  8713,  8719,  8731,  8737,  8741,
 8747,  8753,  8761,  8779,  8783,  8803,  8807,  8819,  8821,  8831,  8837,
 8839,  8849,  8861,  8863,  8867,  8887,  8893,  8923,  8929,  8933,  8941,
 8951,  8963,  8969,  8971,  8999,  9001,  9007,  9011,  9013,  9029,  9041,
 9043,  9049,  9059,  9067,  9091,  9103,  9109,  9127,  9133,  9137,  9151,
 9157,  9161,  9173,  9181,  9187,  9199,  9203,  9209,  9221,  9227,  9239,
 9241,  9257,  9277,  9281,  9283,  9293,  9311,  9319,  9323,  9337,  9341,
 9343,  9349,  9371,  9377,  9391,  9397,  9403,  9413,  9419,  9421,  9431,
 9433,  9437,  9439,  9461,  9463,  9467,  9473,  9479,  9491,  9497,  9511,
 9521,  9533,  9539,  9547,  9551,  9587,  9601,  9613,  9619,  9623,  9629,
 9631,  9643,  9649,  9661,  9677,  9679,  9689,  9697,  9719,  9721,  9733,
 9739,  9743,  9749,  9767,  9769,  9781,  9787,  9791,  9803,  9811,  9817,
 9829,  9833,  9839,  9851,  9857,  9859,  9871,  9883,  9887,  9901,  9907,
 9923,  9929,  9931,  9941,  9949,  9967,  9973, 10007, 10009, 10037, 10039,
10061, 10067, 10069, 10079, 10091, 10093, 10099, 10103, 10111, 10133, 10139,
10141, 10151, 10159, 10163, 10169, 10177, 10181, 10193, 10211, 10223, 10243,
10247, 10253, 10259, 10267, 10271, 10273, 10289, 10301, 10303, 10313, 10321,
10331, 10333, 10337, 10343, 10357, 10369, 10391, 10399, 10427, 10429, 10433,
10453, 10457, 10459, 10463, 10477, 10487, 10499, 10501, 10513, 10529, 10531,
10559, 10567, 10589, 10597, 10601, 10607, 10613, 10627, 10631, 10639, 10651,
10657, 10663, 10667, 10687, 10691, 10709, 10711, 10723, 10729, 10733, 10739,
10753, 10771, 10781, 10789, 10799, 10831, 10837, 10847, 10853, 10859, 10861,
10867, 10883, 10889, 10891, 10903, 10909, 10937, 10939, 10949, 10957, 10973,
10979, 10987, 10993, 11003, 11027, 11047, 11057, 11059, 11069, 11071, 11083,
11087, 11093, 11113, 11117, 11119, 11131, 11149, 11159, 11161, 11171, 11173,
11177, 11197, 11213, 11239, 11243, 11251, 11257, 11261, 11273, 11279, 11287,
11299, 11311, 11317, 11321, 11329, 11351, 11353, 11369, 11383, 11393, 11399,
11411, 11423, 11437, 11443, 11447, 11467, 11471, 11483, 11489, 11491, 11497,
11503, 11519, 11527, 11549, 11551, 11579, 11587, 11593, 11597, 11617, 11621,
11633, 11657, 11677, 11681, 11689, 11699, 11701, 11717, 11719, 11731, 11743,
11777, 11779, 11783, 11789, 11801, 11807, 11813, 11821, 11827, 11831, 11833,
11839, 11863, 11867, 11887, 11897, 11903, 11909, 11923, 11927, 11933, 11939,
11941, 11953, 11959, 11969, 11971, 11981, 11987, 12007, 12011, 12037, 12041,
12043, 12049, 12071, 12073, 12097, 12101, 12107, 12109, 12113, 12119, 12143,
12149, 12157, 12161, 12163, 12197, 12203, 12211, 12227, 12239, 12241, 12251,
12253, 12263, 12269, 12277, 12281, 12289, 12301, 12323, 12329, 12343, 12347,
12373, 12377, 12379, 12391, 12401, 12409, 12413, 12421, 12433, 12437, 12451,
12457, 12473, 12479, 12487, 12491, 12497, 12503, 12511, 12517, 12527, 12539,
12541, 12547, 12553, 12569, 12577, 12583, 12589, 12601, 12611, 12613, 12619,
12637, 12641, 12647, 12653, 12659, 12671, 12689, 12697, 12703, 12713, 12721,
12739, 12743, 12757, 12763, 12781, 12791, 12799, 12809, 12821, 12823, 12829,
12841, 12853, 12889, 12893, 12899, 12907, 12911, 12917, 12919, 12923, 12941,
12953, 12959, 12967, 12973, 12979, 12983, 13001, 13003, 13007, 13009, 13033,
13037, 13043, 13049, 13063, 13093, 13099, 13103, 13109, 13121, 13127, 13147,
13151, 13159, 13163, 13171, 13177, 13183, 13187, 13217, 13219, 13229, 13241,
13249, 13259, 13267, 13291, 13297, 13309, 13313, 13327, 13331, 13337, 13339,
13367, 13381, 13397, 13399, 13411, 13417, 13421, 13441, 13451, 13457, 13463,
13469, 13477, 13487, 13499, 13513, 13523, 13537, 13553, 13567, 13577, 13591,
13597, 13613, 13619, 13627, 13633, 13649, 13669, 13679, 13681, 13687, 13691,
13693, 13697, 13709, 13711, 13721, 13723, 13729, 13751, 13757, 13759, 13763,
13781, 13789, 13799, 13807, 13829, 13831, 13841, 13859, 13873, 13877, 13879,
13883, 13901, 13903, 13907, 13913, 13921, 13931, 13933, 13963, 13967, 13997,
13999, 14009, 14011, 14029, 14033, 14051, 14057, 14071, 14081, 14083, 14087,
14107, 14143, 14149, 14153, 14159, 14173, 14177, 14197, 14207, 14221, 14243,
14249, 14251, 14281, 14293, 14303, 14321, 14323, 14327, 14341, 14347, 14369,
14387, 14389, 14401, 14407, 14411, 14419, 14423, 14431, 14437, 14447, 14449,
14461, 14479, 14489, 14503, 14519, 14533, 14537, 14543, 14549, 14551, 14557,
14561, 14563, 14591, 14593, 14621, 14627, 14629, 14633, 14639, 14653, 14657,
14669, 14683, 14699, 14713, 14717, 14723, 14731, 14737, 14741, 14747, 14753,
14759, 14767, 14771, 14779, 14783, 14797, 14813, 14821, 14827, 14831, 14843,
14851, 14867, 14869, 14879, 14887, 14891, 14897, 14923, 14929, 14939, 14947,
14951, 14957, 14969, 14983, 15013, 15017, 15031, 15053, 15061, 15073, 15077,
15083, 15091, 15101, 15107, 15121, 15131, 15137, 15139, 15149, 15161, 15173,
15187, 15193, 15199, 15217, 15227, 15233, 15241, 15259, 15263, 15269, 15271,
15277, 15287, 15289, 15299, 15307, 15313, 15319, 15329, 15331, 15349, 15359,
15361, 15373, 15377, 15383, 15391, 15401, 15413, 15427, 15439, 15443, 15451,
15461, 15467, 15473, 15493, 15497, 15511, 15527, 15541, 15551, 15559, 15569,
15581, 15583, 15601, 15607, 15619, 15629, 15641, 15643, 15647, 15649, 15661,
15667, 15671, 15679, 15683, 15727, 15731, 15733, 15737, 15739, 15749, 15761,
15767, 15773, 15787, 15791, 15797, 15803, 15809, 15817, 15823, 15859, 15877,
15881, 15887, 15889, 15901, 15907, 15913, 15919, 15923, 15937, 15959, 15971,
15973, 15991, 16001, 16007, 16033, 16057, 16061, 16063, 16067, 16069, 16073,
16087, 16091, 16097, 16103, 16111, 16127, 16139, 16141, 16183, 16187, 16189,
16193, 16217, 16223, 16229, 16231, 16249, 16253, 16267, 16273, 16301, 16319,
16333, 16339, 16349, 16361, 16363, 16369, 16381, 16411, 16417, 16421, 16427,
16433, 16447, 16451, 16453, 16477, 16481, 16487, 16493, 16519, 16529, 16547,
16553, 16561, 16567, 16573, 16603, 16607, 16619, 16631, 16633, 16649, 16651,
16657, 16661, 16673, 16691, 16693, 16699, 16703, 16729, 16741, 16747, 16759,
16763, 16787, 16811, 16823, 16829, 16831, 16843, 16871, 16879, 16883, 16889,
16901, 16903, 16921, 16927, 16931, 16937, 16943, 16963, 16979, 16981, 16987,
16993, 17011, 17021, 17027, 17029, 17033, 17041, 17047, 17053, 17077, 17093,
17099, 17107, 17117, 17123, 17137, 17159, 17167, 17183, 17189, 17191, 17203,
17207, 17209, 17231, 17239, 17257, 17291, 17293, 17299, 17317, 17321, 17327,
17333, 17341, 17351, 17359, 17377, 17383, 17387, 17389, 17393, 17401, 17417,
17419, 17431, 17443, 17449, 17467, 17471, 17477, 17483, 17489, 17491, 17497,
17509, 17519, 17539, 17551, 17569, 17573, 17579, 17581, 17597, 17599, 17609,
17623, 17627, 17657, 17659, 17669, 17681, 17683, 17707, 17713, 17729, 17737,
17747, 17749, 17761, 17783, 17789, 17791, 17807, 17827, 17837, 17839, 17851,
17863, 17881, 17891, 17903, 17909, 17911, 17921, 17923, 17929, 17939, 17957,
17959, 17971, 17977, 17981, 17987, 17989, 18013, 18041, 18043, 18047, 18049,
18059, 18061, 18077, 18089, 18097, 18119, 18121, 18127, 18131, 18133, 18143,
18149, 18169, 18181, 18191, 18199, 18211, 18217, 18223, 18229, 18233, 18251,
18253, 18257, 18269, 18287, 18289, 18301, 18307, 18311, 18313, 18329, 18341,
18353, 18367, 18371, 18379, 18397, 18401, 18413, 18427, 18433, 18439, 18443,
18451, 18457, 18461, 18481, 18493, 18503, 18517, 18521, 18523, 18539, 18541,
18553, 18583, 18587, 18593, 18617, 18637, 18661, 18671, 18679, 18691, 18701,
18713, 18719, 18731, 18743, 18749, 18757, 18773, 18787, 18793, 18797, 18803,
18839, 18859, 18869, 18899, 18911, 18913, 18917, 18919, 18947, 18959, 18973,
18979, 19001, 19009, 19013, 19031, 19037, 19051, 19069, 19073, 19079, 19081,
19087, 19121, 19139, 19141, 19157, 19163, 19181, 19183, 19207, 19211, 19213,
19219, 19231, 19237, 19249, 19259, 19267, 19273, 19289, 19301, 19309, 19319,
19333, 19373, 19379, 19381, 19387, 19391, 19403, 19417, 19421, 19423, 19427,
19429, 19433, 19441, 19447, 19457, 19463, 19469, 19471, 19477, 19483, 19489,
19501, 19507, 19531, 19541, 19543, 19553, 19559, 19571, 19577, 19583, 19597,
19603, 19609, 19661, 19681, 19687, 19697, 19699, 19709, 19717, 19727, 19739,
19751, 19753, 19759, 19763, 19777, 19793, 19801, 19813, 19819, 19841, 19843,
19853, 19861, 19867, 19889, 19891, 19913, 19919, 19927, 19937, 19949, 19961,
19963, 19973, 19979, 19991, 19993, 19997, 20011, 20021, 20023, 20029, 20047,
20051, 20063, 20071, 20089, 20101, 20107, 20113, 20117, 20123, 20129, 20143,
20147, 20149, 20161, 20173, 20177, 20183, 20201, 20219, 20231, 20233, 20249,
20261, 20269, 20287, 20297, 20323, 20327, 20333, 20341, 20347, 20353, 20357,
20359, 20369, 20389, 20393, 20399, 20407, 20411, 20431, 20441, 20443, 20477,
20479, 20483, 20507, 20509, 20521, 20533, 20543, 20549, 20551, 20563, 20593,
20599, 20611, 20627, 20639, 20641, 20663, 20681, 20693, 20707, 20717, 20719,
20731, 20743, 20747, 20749, 20753, 20759, 20771, 20773, 20789, 20807, 20809,
20849, 20857, 20873, 20879, 20887, 20897, 20899, 20903, 20921, 20929, 20939,
20947, 20959, 20963, 20981, 20983, 21001, 21011, 21013, 21017, 21019, 21023,
21031, 21059, 21061, 21067, 21089, 21101, 21107, 21121, 21139, 21143, 21149,
21157, 21163, 21169, 21179, 21187, 21191, 21193, 21211, 21221, 21227, 21247,
21269, 21277, 21283, 21313, 21317, 21319, 21323, 21341, 21347, 21377, 21379,
21383, 21391, 21397, 21401, 21407, 21419, 21433, 21467, 21481, 21487, 21491,
21493, 21499, 21503, 21517, 21521, 21523, 21529, 21557, 21559, 21563, 21569,
21577, 21587, 21589, 21599, 21601, 21611, 21613, 21617, 21647, 21649, 21661,
21673, 21683, 21701, 21713, 21727, 21737, 21739, 21751, 21757, 21767, 21773,
21787, 21799, 21803, 21817, 21821, 21839, 21841, 21851, 21859, 21863, 21871,
21881, 21893, 21911, 21929, 21937, 21943, 21961, 21977, 21991, 21997, 22003,
22013, 22027, 22031, 22037, 22039, 22051, 22063, 22067, 22073, 22079, 22091,
22093, 22109, 22111, 22123, 22129, 22133, 22147, 22153, 22157, 22159, 22171,
22189, 22193, 22229, 22247, 22259, 22271, 22273, 22277, 22279, 22283, 22291,
22303, 22307, 22343, 22349, 22367, 22369, 22381, 22391, 22397, 22409, 22433,
22441, 22447, 22453, 22469, 22481, 22483, 22501, 22511, 22531, 22541, 22543,
22549, 22567, 22571, 22573, 22613, 22619, 22621, 22637, 22639, 22643, 22651,
22669, 22679, 22691, 22697, 22699, 22709, 22717, 22721, 22727, 22739, 22741,
22751, 22769, 22777, 22783, 22787, 22807, 22811, 22817, 22853, 22859, 22861,
22871, 22877, 22901, 22907, 22921, 22937, 22943, 22961, 22963, 22973, 22993,
23003, 23011, 23017, 23021, 23027, 23029, 23039, 23041, 23053, 23057, 23059,
23063, 23071, 23081, 23087, 23099, 23117, 23131, 23143, 23159, 23167, 23173,
23189, 23197, 23201, 23203, 23209, 23227, 23251, 23269, 23279, 23291, 23293,
23297, 23311, 23321, 23327, 23333, 23339, 23357, 23369, 23371, 23399, 23417,
23431, 23447, 23459, 23473, 23497, 23509, 23531, 23537, 23539, 23549, 23557,
23561, 23563, 23567, 23581, 23593, 23599, 23603, 23609, 23623, 23627, 23629,
23633, 23663, 23669, 23671, 23677, 23687, 23689, 23719, 23741, 23743, 23747,
23753, 23761, 23767, 23773, 23789, 23801, 23813, 23819, 23827, 23831, 23833,
23857, 23869, 23873, 23879, 23887, 23893, 23899, 23909, 23911, 23917, 23929,
23957, 23971, 23977, 23981, 23993, 24001, 24007, 24019, 24023, 24029, 24043,
24049, 24061, 24071, 24077, 24083, 24091, 24097, 24103, 24107, 24109, 24113,
24121, 24133, 24137, 24151, 24169, 24179, 24181, 24197, 24203, 24223, 24229,
24239, 24247, 24251, 24281, 24317, 24329, 24337, 24359, 24371, 24373, 24379,
24391, 24407, 24413, 24419, 24421, 24439, 24443, 24469, 24473, 24481, 24499,
24509, 24517, 24527, 24533, 24547, 24551, 24571, 24593, 24611, 24623, 24631,
24659, 24671, 24677, 24683, 24691, 24697, 24709, 24733, 24749, 24763, 24767,
24781, 24793, 24799, 24809, 24821, 24841, 24847, 24851, 24859, 24877, 24889,
24907, 24917, 24919, 24923, 24943, 24953, 24967, 24971, 24977, 24979, 24989,
25013, 25031, 25033, 25037, 25057, 25073, 25087, 25097, 25111, 25117, 25121,
25127, 25147, 25153, 25163, 25169, 25171, 25183, 25189, 25219, 25229, 25237,
25243, 25247, 25253, 25261, 25301, 25303, 25307, 25309, 25321, 25339, 25343,
25349, 25357, 25367, 25373, 25391, 25409, 25411, 25423, 25439, 25447, 25453,
25457, 25463, 25469, 25471, 25523, 25537, 25541, 25561, 25577, 25579, 25583,
25589, 25601, 25603, 25609, 25621, 25633, 25639, 25643, 25657, 25667, 25673,
25679, 25693, 25703, 25717, 25733, 25741, 25747, 25759, 25763, 25771, 25793,
25799, 25801, 25819, 25841, 25847, 25849, 25867, 25873, 25889, 25903, 25913,
25919, 25931, 25933, 25939, 25943, 25951, 25969, 25981, 25997, 25999, 26003,
26017, 26021, 26029, 26041, 26053, 26083, 26099, 26107, 26111, 26113, 26119,
26141, 26153, 26161, 26171, 26177, 26183, 26189, 26203, 26209, 26227, 26237,
26249, 26251, 26261, 26263, 26267, 26293, 26297, 26309, 26317, 26321, 26339,
26347, 26357, 26371, 26387, 26393, 26399, 26407, 26417, 26423, 26431, 26437,
26449, 26459, 26479, 26489, 26497, 26501, 26513, 26539, 26557, 26561, 26573,
26591, 26597, 26627, 26633, 26641, 26647, 26669, 26681, 26683, 26687, 26693,
26699, 26701, 26711, 26713, 26717, 26723, 26729, 26731, 26737, 26759, 26777,
26783, 26801, 26813, 26821, 26833, 26839, 26849, 26861, 26863, 26879, 26881,
26891, 26893, 26903, 26921, 26927, 26947, 26951, 26953, 26959, 26981, 26987,
26993, 27011, 27017, 27031, 27043, 27059, 27061, 27067, 27073, 27077, 27091,
27103, 27107, 27109, 27127, 27143, 27179, 27191, 27197, 27211, 27239, 27241,
27253, 27259, 27271, 27277, 27281, 27283, 27299, 27329, 27337, 27361, 27367,
27397, 27407, 27409, 27427, 27431, 27437, 27449, 27457, 27479, 27481, 27487,
27509, 27527, 27529, 27539, 27541, 27551, 27581, 27583, 27611, 27617, 27631,
27647, 27653, 27673, 27689, 27691, 27697, 27701, 27733, 27737, 27739, 27743,
27749, 27751, 27763, 27767, 27773, 27779, 27791, 27793, 27799, 27803, 27809,
27817, 27823, 27827, 27847, 27851, 27883, 27893, 27901, 27917, 27919, 27941,
27943, 27947, 27953, 27961, 27967, 27983, 27997, 28001, 28019, 28027, 28031,
28051, 28057, 28069, 28081, 28087, 28097, 28099, 28109, 28111, 28123, 28151,
28163, 28181, 28183, 28201, 28211, 28219, 28229, 28277, 28279, 28283, 28289,
28297, 28307, 28309, 28319, 28349, 28351, 28387, 28393, 28403, 28409, 28411,
28429, 28433, 28439, 28447, 28463, 28477, 28493, 28499, 28513, 28517, 28537,
28541, 28547, 28549, 28559, 28571, 28573, 28579, 28591, 28597, 28603, 28607,
28619, 28621, 28627, 28631, 28643, 28649, 28657, 28661, 28663, 28669, 28687,
28697, 28703, 28711, 28723, 28729, 28751, 28753, 28759, 28771, 28789, 28793,
28807, 28813, 28817, 28837, 28843, 28859, 28867, 28871, 28879, 28901, 28909,
28921, 28927, 28933, 28949, 28961, 28979, 29009, 29017, 29021, 29023, 29027,
29033, 29059, 29063, 29077, 29101, 29123, 29129, 29131, 29137, 29147, 29153,
29167, 29173, 29179, 29191, 29201, 29207, 29209, 29221, 29231, 29243, 29251,
29269, 29287, 29297, 29303, 29311, 29327, 29333, 29339, 29347, 29363, 29383,
29387, 29389, 29399, 29401, 29411, 29423, 29429, 29437, 29443, 29453, 29473,
29483, 29501, 29527, 29531, 29537, 29567, 29569, 29573, 29581, 29587, 29599,
29611, 29629, 29633, 29641, 29663, 29669, 29671, 29683, 29717, 29723, 29741,
29753, 29759, 29761, 29789, 29803, 29819, 29833, 29837, 29851, 29863, 29867,
29873, 29879, 29881, 29917, 29921, 29927, 29947, 29959, 29983, 29989, 30011,
30013, 30029, 30047, 30059, 30071, 30089, 30091, 30097, 30103, 30109, 30113,
30119, 30133, 30137, 30139, 30161, 30169, 30181, 30187, 30197, 30203, 30211,
30223, 30241, 30253, 30259, 30269, 30271, 30293, 30307, 30313, 30319, 30323,
30341, 30347, 30367, 30389, 30391, 30403, 30427, 30431, 30449, 30467, 30469,
30491, 30493, 30497, 30509, 30517, 30529, 30539, 30553, 30557, 30559, 30577,
30593, 30631, 30637, 30643, 30649, 30661, 30671, 30677, 30689, 30697, 30703,
30707, 30713, 30727, 30757, 30763, 30773, 30781, 30803, 30809, 30817, 30829,
30839, 30841, 30851, 30853, 30859, 30869, 30871, 30881, 30893, 30911, 30931,
30937, 30941, 30949, 30971, 30977, 30983, 31013, 31019, 31033, 31039, 31051,
31063, 31069, 31079, 31081, 31091, 31121, 31123, 31139, 31147, 31151, 31153,
31159, 31177, 31181, 31183, 31189, 31193, 31219, 31223, 31231, 31237, 31247,
31249, 31253, 31259, 31267, 31271, 31277, 31307, 31319, 31321, 31327, 31333,
31337, 31357, 31379, 31387, 31391, 31393, 31397, 31469, 31477, 31481, 31489,
31511, 31513, 31517, 31531, 31541, 31543, 31547, 31567, 31573, 31583, 31601,
31607, 31627, 31643, 31649, 31657, 31663, 31667, 31687, 31699, 31721, 31723,
31727, 31729, 31741, 31751, 31769, 31771, 31793, 31799, 31817, 31847, 31849,
31859, 31873, 31883, 31891, 31907, 31957, 31963, 31973, 31981, 31991, 32003,
32009, 32027, 32029, 32051, 32057, 32059, 32063, 32069, 32077, 32083, 32089,
32099, 32117, 32119, 32141, 32143, 32159, 32173, 32183, 32189, 32191, 32203,
32213, 32233, 32237, 32251, 32257, 32261, 32297, 32299, 32303, 32309, 32321,
32323, 32327, 32341, 32353, 32359, 32363, 32369, 32371, 32377, 32381, 32401,
32411, 32413, 32423, 32429, 32441, 32443, 32467, 32479, 32491, 32497, 32503,
32507, 32531, 32533, 32537, 32561, 32563, 32569, 32573, 32579, 32587, 32603,
32609, 32611, 32621, 32633, 32647, 32653, 32687, 32693, 32707, 32713, 32717,
32719, 32749, 32771, 32779, 32783, 32789, 32797, 32801, 32803, 32831, 32833,
32839, 32843, 32869, 32887, 32909, 32911, 32917, 32933, 32939, 32941, 32957,
32969, 32971, 32983, 32987, 32993, 32999, 33013, 33023, 33029, 33037, 33049,
33053, 33071, 33073, 33083, 33091, 33107, 33113, 33119, 33149, 33151, 33161,
33179, 33181, 33191, 33199, 33203, 33211, 33223, 33247, 33287, 33289, 33301,
33311, 33317, 33329, 33331, 33343, 33347, 33349, 33353, 33359, 33377, 33391,
33403, 33409, 33413, 33427, 33457, 33461, 33469, 33479, 33487, 33493, 33503,
33521, 33529, 33533, 33547, 33563, 33569, 33577, 33581, 33587, 33589, 33599,
33601, 33613, 33617, 33619, 33623, 33629, 33637, 33641, 33647, 33679, 33703,
33713, 33721, 33739, 33749, 33751, 33757, 33767, 33769, 33773, 33791, 33797,
33809, 33811, 33827, 33829, 33851, 33857, 33863, 33871, 33889, 33893, 33911,
33923, 33931, 33937, 33941, 33961, 33967, 33997, 34019, 34031, 34033, 34039,
34057, 34061, 34123, 34127, 34129, 34141, 34147, 34157, 34159, 34171, 34183,
34211, 34213, 34217, 34231, 34253, 34259, 34261, 34267, 34273, 34283, 34297,
34301, 34303, 34313, 34319, 34327, 34337, 34351, 34361, 34367, 34369, 34381,
34403, 34421, 34429, 34439, 34457, 34469, 34471, 34483, 34487, 34499, 34501,
34511, 34513, 34519, 34537, 34543, 34549, 34583, 34589, 34591, 34603, 34607,
34613, 34631, 34649, 34651, 34667, 34673, 34679, 34687, 34693, 34703, 34721,
34729, 34739, 34747, 34757, 34759, 34763, 34781, 34807, 34819, 34841, 34843,
34847, 34849, 34871, 34877, 34883, 34897, 34913, 34919, 34939, 34949, 34961,
34963, 34981, 35023, 35027, 35051, 35053, 35059, 35069, 35081, 35083, 35089,
35099, 35107, 35111, 35117, 35129, 35141, 35149, 35153, 35159, 35171, 35201,
35221, 35227, 35251, 35257, 35267, 35279, 35281, 35291, 35311, 35317, 35323,
35327, 35339, 35353, 35363, 35381, 35393, 35401, 35407, 35419, 35423, 35437,
35447, 35449, 35461, 35491, 35507, 35509, 35521, 35527, 35531, 35533, 35537,
35543, 35569, 35573, 35591, 35593, 35597, 35603, 35617, 35671, 35677, 35729,
35731, 35747, 35753, 35759, 35771, 35797, 35801, 35803, 35809, 35831, 35837,
35839, 35851, 35863, 35869, 35879, 35897, 35899, 35911, 35923, 35933, 35951,
35963, 35969, 35977, 35983, 35993, 35999, 36007, 36011, 36013, 36017, 36037,
36061, 36067, 36073, 36083, 36097, 36107, 36109, 36131, 36137, 36151, 36161,
36187, 36191, 36209, 36217, 36229, 36241, 36251, 36263, 36269, 36277, 36293,
36299, 36307, 36313, 36319, 36341, 36343, 36353, 36373, 36383, 36389, 36433,
36451, 36457, 36467, 36469, 36473, 36479, 36493, 36497, 36523, 36527, 36529,
36541, 36551, 36559, 36563, 36571, 36583, 36587, 36599, 36607, 36629, 36637,
36643, 36653, 36671, 36677, 36683, 36691, 36697, 36709, 36713, 36721, 36739,
36749, 36761, 36767, 36779, 36781, 36787, 36791, 36793, 36809, 36821, 36833,
36847, 36857, 36871, 36877, 36887, 36899, 36901, 36913, 36919, 36923, 36929,
36931, 36943, 36947, 36973, 36979, 36997, 37003, 37013, 37019, 37021, 37039,
37049, 37057, 37061, 37087, 37097, 37117, 37123, 37139, 37159, 37171, 37181,
37189, 37199, 37201, 37217, 37223, 37243, 37253, 37273, 37277, 37307, 37309,
37313, 37321, 37337, 37339, 37357, 37361, 37363, 37369, 37379, 37397, 37409,
37423, 37441, 37447, 37463, 37483, 37489, 37493, 37501, 37507, 37511, 37517,
37529, 37537, 37547, 37549, 37561, 37567, 37571, 37573, 37579, 37589, 37591,
37607, 37619, 37633, 37643, 37649, 37657, 37663, 37691, 37693, 37699, 37717,
37747, 37781, 37783, 37799, 37811, 37813, 37831, 37847, 37853, 37861, 37871,
37879, 37889, 37897, 37907, 37951, 37957, 37963, 37967, 37987, 37991, 37993,
37997, 38011, 38039, 38047, 38053, 38069, 38083, 38113, 38119, 38149, 38153,
38167, 38177, 38183, 38189, 38197, 38201, 38219, 38231, 38237, 38239, 38261,
38273, 38281, 38287, 38299, 38303, 38317, 38321, 38327, 38329, 38333, 38351,
38371, 38377, 38393, 38431, 38447, 38449, 38453, 38459, 38461, 38501, 38543,
38557, 38561, 38567, 38569, 38593, 38603, 38609, 38611, 38629, 38639, 38651,
38653, 38669, 38671, 38677, 38693, 38699, 38707, 38711, 38713, 38723, 38729,
38737, 38747, 38749, 38767, 38783, 38791, 38803, 38821, 38833, 38839, 38851,
38861, 38867, 38873, 38891, 38903, 38917, 38921, 38923, 38933, 38953, 38959,
38971, 38977, 38993, 39019, 39023, 39041, 39043, 39047, 39079, 39089, 39097,
39103, 39107, 39113, 39119, 39133, 39139, 39157, 39161, 39163, 39181, 39191,
39199, 39209, 39217, 39227, 39229, 39233, 39239, 39241, 39251, 39293, 39301,
39313, 39317, 39323, 39341, 39343, 39359, 39367, 39371, 39373, 39383, 39397,
39409, 39419, 39439, 39443, 39451, 39461, 39499, 39503, 39509, 39511, 39521,
39541, 39551, 39563, 39569, 39581, 39607, 39619, 39623, 39631, 39659, 39667,
39671, 39679, 39703, 39709, 39719, 39727, 39733, 39749, 39761, 39769, 39779,
39791, 39799, 39821, 39827, 39829, 39839, 39841, 39847, 39857, 39863, 39869,
39877, 39883, 39887, 39901, 39929, 39937, 39953, 39971, 39979, 39983, 39989,
40009, 40013, 40031, 40037, 40039, 40063, 40087, 40093, 40099, 40111, 40123,
40127, 40129, 40151, 40153, 40163, 40169, 40177, 40189, 40193, 40213, 40231,
40237, 40241, 40253, 40277, 40283, 40289, 40343, 40351, 40357, 40361, 40387,
40423, 40427, 40429, 40433, 40459, 40471, 40483, 40487, 40493, 40499, 40507,
40519, 40529, 40531, 40543, 40559, 40577, 40583, 40591, 40597, 40609, 40627,
40637, 40639, 40693, 40697, 40699, 40709, 40739, 40751, 40759, 40763, 40771,
40787, 40801, 40813, 40819, 40823, 40829, 40841, 40847, 40849, 40853, 40867,
40879, 40883, 40897, 40903, 40927, 40933, 40939, 40949, 40961, 40973, 40993,
41011, 41017, 41023, 41039, 41047, 41051, 41057, 41077, 41081, 41113, 41117,
41131, 41141, 41143, 41149, 41161, 41177, 41179, 41183, 41189, 41201, 41203,
41213, 41221, 41227, 41231, 41233, 41243, 41257, 41263, 41269, 41281, 41299,
41333, 41341, 41351, 41357, 41381, 41387, 41389, 41399, 41411, 41413, 41443,
41453, 41467, 41479, 41491, 41507, 41513, 41519, 41521, 41539, 41543, 41549,
41579, 41593, 41597, 41603, 41609, 41611, 41617, 41621, 41627, 41641, 41647,
41651, 41659, 41669, 41681, 41687, 41719, 41729, 41737, 41759, 41761, 41771,
41777, 41801, 41809, 41813, 41843, 41849, 41851, 41863, 41879, 41887, 41893,
41897, 41903, 41911, 41927, 41941, 41947, 41953, 41957, 41959, 41969, 41981,
41983, 41999, 42013, 42017, 42019, 42023, 42043, 42061, 42071, 42073, 42083,
42089, 42101, 42131, 42139, 42157, 42169, 42179, 42181, 42187, 42193, 42197,
42209, 42221, 42223, 42227, 42239, 42257, 42281, 42283, 42293, 42299, 42307,
42323, 42331, 42337, 42349, 42359, 42373, 42379, 42391, 42397, 42403, 42407,
42409, 42433, 42437, 42443, 42451, 42457, 42461, 42463, 42467, 42473, 42487,
42491, 42499, 42509, 42533, 42557, 42569, 42571, 42577, 42589, 42611, 42641,
42643, 42649, 42667, 42677, 42683, 42689, 42697, 42701, 42703, 42709, 42719,
42727, 42737, 42743, 42751, 42767, 42773, 42787, 42793, 42797, 42821, 42829,
42839, 42841, 42853, 42859, 42863, 42899, 42901, 42923, 42929, 42937, 42943,
42953, 42961, 42967, 42979, 42989, 43003, 43013, 43019, 43037, 43049, 43051,
43063, 43067, 43093, 43103, 43117, 43133, 43151, 43159, 43177, 43189, 43201,
43207, 43223, 43237, 43261, 43271, 43283, 43291, 43313, 43319, 43321, 43331,
43391, 43397, 43399, 43403, 43411, 43427, 43441, 43451, 43457, 43481, 43487,
43499, 43517, 43541, 43543, 43573, 43577, 43579, 43591, 43597, 43607, 43609,
43613, 43627, 43633, 43649, 43651, 43661, 43669, 43691, 43711, 43717, 43721,
43753, 43759, 43777, 43781, 43783, 43787, 43789, 43793, 43801, 43853, 43867,
43889, 43891, 43913, 43933, 43943, 43951, 43961, 43963, 43969, 43973, 43987,
43991, 43997, 44017, 44021, 44027, 44029, 44041, 44053, 44059, 44071, 44087,
44089, 44101, 44111, 44119, 44123, 44129, 44131, 44159, 44171, 44179, 44189,
44201, 44203, 44207, 44221, 44249, 44257, 44263, 44267, 44269, 44273, 44279,
44281, 44293, 44351, 44357, 44371, 44381, 44383, 44389, 44417, 44449, 44453,
44483, 44491, 44497, 44501, 44507, 44519, 44531, 44533, 44537, 44543, 44549,
44563, 44579, 44587, 44617, 44621, 44623, 44633, 44641, 44647, 44651, 44657,
44683, 44687, 44699, 44701, 44711, 44729, 44741, 44753, 44771, 44773, 44777,
44789, 44797, 44809, 44819, 44839, 44843, 44851, 44867, 44879, 44887, 44893,
44909, 44917, 44927, 44939, 44953, 44959, 44963, 44971, 44983, 44987, 45007,
45013, 45053, 45061, 45077, 45083, 45119, 45121, 45127, 45131, 45137, 45139,
45161, 45179, 45181, 45191, 45197, 45233, 45247, 45259, 45263, 45281, 45289,
45293, 45307, 45317, 45319, 45329, 45337, 45341, 45343, 45361, 45377, 45389,
45403, 45413, 45427, 45433, 45439, 45481, 45491, 45497, 45503, 45523, 45533,
45541, 45553, 45557, 45569, 45587, 45589, 45599, 45613, 45631, 45641, 45659,
45667, 45673, 45677, 45691, 45697, 45707, 45737, 45751, 45757, 45763, 45767,
45779, 45817, 45821, 45823, 45827, 45833, 45841, 45853, 45863, 45869, 45887,
45893, 45943, 45949, 45953, 45959, 45971, 45979, 45989, 46021, 46027, 46049,
46051, 46061, 46073, 46091, 46093, 46099, 46103, 46133, 46141, 46147, 46153,
46171, 46181, 46183, 46187, 46199, 46219, 46229, 46237, 46261, 46271, 46273,
46279, 46301, 46307, 46309, 46327, 46337, 46349, 46351, 46381, 46399, 46411,
46439, 46441, 46447, 46451, 46457, 46471, 46477, 46489, 46499, 46507, 46511,
46523, 46549, 46559, 46567, 46573, 46589, 46591, 46601, 46619, 46633, 46639,
46643, 46649, 46663, 46679, 46681, 46687, 46691, 46703, 46723, 46727, 46747,
46751, 46757, 46769, 46771, 46807, 46811, 46817, 46819, 46829, 46831, 46853,
46861, 46867, 46877, 46889, 46901, 46919, 46933, 46957, 46993, 46997, 47017,
47041, 47051, 47057, 47059, 47087, 47093, 47111, 47119, 47123, 47129, 47137,
47143, 47147, 47149, 47161, 47189, 47207, 47221, 47237, 47251, 47269, 47279,
47287, 47293, 47297, 47303, 47309, 47317, 47339, 47351, 47353, 47363, 47381,
47387, 47389, 47407, 47417, 47419, 47431, 47441, 47459, 47491, 47497, 47501,
47507, 47513, 47521, 47527, 47533, 47543, 47563, 47569, 47581, 47591, 47599,
47609, 47623, 47629, 47639, 47653, 47657, 47659, 47681, 47699, 47701, 47711,
47713, 47717, 47737, 47741, 47743, 47777, 47779, 47791, 47797, 47807, 47809,
47819, 47837, 47843, 47857, 47869, 47881, 47903, 47911, 47917, 47933, 47939,
47947, 47951, 47963, 47969, 47977, 47981, 48017, 48023, 48029, 48049, 48073,
48079, 48091, 48109, 48119, 48121, 48131, 48157, 48163, 48179, 48187, 48193,
48197, 48221, 48239, 48247, 48259, 48271, 48281, 48299, 48311, 48313, 48337,
48341, 48353, 48371, 48383, 48397, 48407, 48409, 48413, 48437, 48449, 48463,
48473, 48479, 48481, 48487, 48491, 48497, 48523, 48527, 48533, 48539, 48541,
48563, 48571, 48589, 48593, 48611, 48619, 48623, 48647, 48649, 48661, 48673,
48677, 48679, 48731, 48733, 48751, 48757, 48761, 48767, 48779, 48781, 48787,
48799, 48809, 48817, 48821, 48823, 48847, 48857, 48859, 48869, 48871, 48883,
48889, 48907, 48947, 48953, 48973, 48989, 48991, 49003, 49009, 49019, 49031,
49033, 49037, 49043, 49057, 49069, 49081, 49103, 49109, 49117, 49121, 49123,
49139, 49157, 49169, 49171, 49177, 49193, 49199, 49201, 49207, 49211, 49223,
49253, 49261, 49277, 49279, 49297, 49307, 49331, 49333, 49339, 49363, 49367,
49369, 49391, 49393, 49409, 49411, 49417, 49429, 49433, 49451, 49459, 49463,
49477, 49481, 49499, 49523, 49529, 49531, 49537, 49547, 49549, 49559, 49597,
49603, 49613, 49627, 49633, 49639, 49663, 49667, 49669, 49681, 49697, 49711,
49727, 49739, 49741, 49747, 49757, 49783, 49787, 49789, 49801, 49807, 49811,
49823, 49831, 49843, 49853, 49871, 49877, 49891, 49919, 49921, 49927, 49937,
49939, 49943, 49957, 49991, 49993, 49999, 50021, 50023, 50033, 50047, 50051,
50053, 50069, 50077, 50087, 50093, 50101, 50111, 50119, 50123, 50129, 50131,
50147, 50153, 50159, 50177, 50207, 50221, 50227, 50231, 50261, 50263, 50273,
50287, 50291, 50311, 50321, 50329, 50333, 50341, 50359, 50363, 50377, 50383,
50387, 50411, 50417, 50423, 50441, 50459, 50461, 50497, 50503, 50513, 50527,
50539, 50543, 50549, 50551, 50581, 50587, 50591, 50593, 50599, 50627, 50647,
50651, 50671, 50683, 50707, 50723, 50741, 50753, 50767, 50773, 50777, 50789,
50821, 50833, 50839, 50849, 50857, 50867, 50873, 50891, 50893, 50909, 50923,
50929, 50951, 50957, 50969, 50971, 50989, 50993, 51001, 51031, 51043, 51047,
51059, 51061, 51071, 51109, 51131, 51133, 51137, 51151, 51157, 51169, 51193,
51197, 51199, 51203, 51217, 51229, 51239, 51241, 51257, 51263, 51283, 51287,
51307, 51329, 51341, 51343, 51347, 51349, 51361, 51383, 51407, 51413, 51419,
51421, 51427, 51431, 51437, 51439, 51449, 51461, 51473, 51479, 51481, 51487,
51503, 51511, 51517, 51521, 51539, 51551, 51563, 51577, 51581, 51593, 51599,
51607, 51613, 51631, 51637, 51647, 51659, 51673, 51679, 51683, 51691, 51713,
51719, 51721, 51749, 51767, 51769, 51787, 51797, 51803, 51817, 51827, 51829,
51839, 51853, 51859, 51869, 51871, 51893, 51899, 51907, 51913, 51929, 51941,
51949, 51971, 51973, 51977, 51991, 52009, 52021, 52027, 52051, 52057, 52067,
52069, 52081, 52103, 52121, 52127, 52147, 52153, 52163, 52177, 52181, 52183,
52189, 52201, 52223, 52237, 52249, 52253, 52259, 52267, 52289, 52291, 52301,
52313, 52321, 52361, 52363, 52369, 52379, 52387, 52391, 52433, 52453, 52457,
52489, 52501, 52511, 52517, 52529, 52541, 52543, 52553, 52561, 52567, 52571,
52579, 52583, 52609, 52627, 52631, 52639, 52667, 52673, 52691, 52697, 52709,
52711, 52721, 52727, 52733, 52747, 52757, 52769, 52783, 52807, 52813, 52817,
52837, 52859, 52861, 52879, 52883, 52889, 52901, 52903, 52919, 52937, 52951,
52957, 52963, 52967, 52973, 52981, 52999, 53003, 53017, 53047, 53051, 53069,
53077, 53087, 53089, 53093, 53101, 53113, 53117, 53129, 53147, 53149, 53161,
53171, 53173, 53189, 53197, 53201, 53231, 53233, 53239, 53267, 53269, 53279,
53281, 53299, 53309, 53323, 53327, 53353, 53359, 53377, 53381, 53401, 53407,
53411, 53419, 53437, 53441, 53453, 53479, 53503, 53507, 53527, 53549, 53551,
53569, 53591, 53593, 53597, 53609, 53611, 53617, 53623, 53629, 53633, 53639,
53653, 53657, 53681, 53693, 53699, 53717, 53719, 53731, 53759, 53773, 53777,
53783, 53791, 53813, 53819, 53831, 53849, 53857, 53861, 53881, 53887, 53891,
53897, 53899, 53917, 53923, 53927, 53939, 53951, 53959, 53987, 53993, 54001,
54011, 54013, 54037, 54049, 54059, 54083, 54091, 54101, 54121, 54133, 54139,
54151, 54163, 54167, 54181, 54193, 54217, 54251, 54269, 54277, 54287, 54293,
54311, 54319, 54323, 54331, 54347, 54361, 54367, 54371, 54377, 54401, 54403,
54409, 54413, 54419, 54421, 54437, 54443, 54449, 54469, 54493, 54497, 54499,
54503, 54517, 54521, 54539, 54541, 54547, 54559, 54563, 54577, 54581, 54583,
54601, 54617, 54623, 54629, 54631, 54647, 54667, 54673, 54679, 54709, 54713,
54721, 54727, 54751, 54767, 54773, 54779, 54787, 54799, 54829, 54833, 54851,
54869, 54877, 54881, 54907, 54917, 54919, 54941, 54949, 54959, 54973, 54979,
54983, 55001, 55009, 55021, 55049, 55051, 55057, 55061, 55073, 55079, 55103,
55109, 55117, 55127, 55147, 55163, 55171, 55201, 55207, 55213, 55217, 55219,
55229, 55243, 55249, 55259, 55291, 55313, 55331, 55333, 55337, 55339, 55343,
55351, 55373, 55381, 55399, 55411, 55439, 55441, 55457, 55469, 55487, 55501,
55511, 55529, 55541, 55547, 55579, 55589, 55603, 55609, 55619, 55621, 55631,
55633, 55639, 55661, 55663, 55667, 55673, 55681, 55691, 55697, 55711, 55717,
55721, 55733, 55763, 55787, 55793, 55799, 55807, 55813, 55817, 55819, 55823,
55829, 55837, 55843, 55849, 55871, 55889, 55897, 55901, 55903, 55921, 55927,
55931, 55933, 55949, 55967, 55987, 55997, 56003, 56009, 56039, 56041, 56053,
56081, 56087, 56093, 56099, 56101, 56113, 56123, 56131, 56149, 56167, 56171,
56179, 56197, 56207, 56209, 56237, 56239, 56249, 56263, 56267, 56269, 56299,
56311, 56333, 56359, 56369, 56377, 56383, 56393, 56401, 56417, 56431, 56437,
56443, 56453, 56467, 56473, 56477, 56479, 56489, 56501, 56503, 56509, 56519,
56527, 56531, 56533, 56543, 56569, 56591, 56597, 56599, 56611, 56629, 56633,
56659, 56663, 56671, 56681, 56687, 56701, 56711, 56713, 56731, 56737, 56747,
56767, 56773, 56779, 56783, 56807, 56809, 56813, 56821, 56827, 56843, 56857,
56873, 56891, 56893, 56897, 56909, 56911, 56921, 56923, 56929, 56941, 56951,
56957, 56963, 56983, 56989, 56993, 56999, 57037, 57041, 57047, 57059, 57073,
57077, 57089, 57097, 57107, 57119, 57131, 57139, 57143, 57149, 57163, 57173,
57179, 57191, 57193, 57203, 57221, 57223, 57241, 57251, 57259, 57269, 57271,
57283, 57287, 57301, 57329, 57331, 57347, 57349, 57367, 57373, 57383, 57389,
57397, 57413, 57427, 57457, 57467, 57487, 57493, 57503, 57527, 57529, 57557,
57559, 57571, 57587, 57593, 57601, 57637, 57641, 57649, 57653, 57667, 57679,
57689, 57697, 57709, 57713, 57719, 57727, 57731, 57737, 57751, 57773, 57781,
57787, 57791, 57793, 57803, 57809, 57829, 57839, 57847, 57853, 57859, 57881,
57899, 57901, 57917, 57923, 57943, 57947, 57973, 57977, 57991, 58013, 58027,
58031, 58043, 58049, 58057, 58061, 58067, 58073, 58099, 58109, 58111, 58129,
58147, 58151, 58153, 58169, 58171, 58189, 58193, 58199, 58207, 58211, 58217,
58229, 58231, 58237, 58243, 58271, 58309, 58313, 58321, 58337, 58363, 58367,
58369, 58379, 58391, 58393, 58403, 58411, 58417, 58427, 58439, 58441, 58451,
58453, 58477, 58481, 58511, 58537, 58543, 58549, 58567, 58573, 58579, 58601,
58603, 58613, 58631, 58657, 58661, 58679, 58687, 58693, 58699, 58711, 58727,
58733, 58741, 58757, 58763, 58771, 58787, 58789, 58831, 58889, 58897, 58901,
58907, 58909, 58913, 58921, 58937, 58943, 58963, 58967, 58979, 58991, 58997,
59009, 59011, 59021, 59023, 59029, 59051, 59053, 59063, 59069, 59077, 59083,
59093, 59107, 59113, 59119, 59123, 59141, 59149, 59159, 59167, 59183, 59197,
59207, 59209, 59219, 59221, 59233, 59239, 59243, 59263, 59273, 59281, 59333,
59341, 59351, 59357, 59359, 59369, 59377, 59387, 59393, 59399, 59407, 59417,
59419, 59441, 59443, 59447, 59453, 59467, 59471, 59473, 59497, 59509, 59513,
59539, 59557, 59561, 59567, 59581, 59611, 59617, 59621, 59627, 59629, 59651,
59659, 59663, 59669, 59671, 59693, 59699, 59707, 59723, 59729, 59743, 59747,
59753, 59771, 59779, 59791, 59797, 59809, 59833, 59863, 59879, 59887, 59921,
59929, 59951, 59957, 59971, 59981, 59999, 60013, 60017, 60029, 60037, 60041,
60077, 60083, 60089, 60091, 60101, 60103, 60107, 60127, 60133, 60139, 60149,
60161, 60167, 60169, 60209, 60217, 60223, 60251, 60257, 60259, 60271, 60289,
60293, 60317, 60331, 60337, 60343, 60353, 60373, 60383, 60397, 60413, 60427,
60443, 60449, 60457, 60493, 60497, 60509, 60521, 60527, 60539, 60589, 60601,
60607, 60611, 60617, 60623, 60631, 60637, 60647, 60649, 60659, 60661, 60679,
60689, 60703, 60719, 60727, 60733, 60737, 60757, 60761, 60763, 60773, 60779,
60793, 60811, 60821, 60859, 60869, 60887, 60889, 60899, 60901, 60913, 60917,
60919, 60923, 60937, 60943, 60953, 60961, 61001, 61007, 61027, 61031, 61043,
61051, 61057, 61091, 61099, 61121, 61129, 61141, 61151, 61153, 61169, 61211,
61223, 61231, 61253, 61261, 61283, 61291, 61297, 61331, 61333, 61339, 61343,
61357, 61363, 61379, 61381, 61403, 61409, 61417, 61441, 61463, 61469, 61471,
61483, 61487, 61493, 61507, 61511, 61519, 61543, 61547, 61553, 61559, 61561,
61583, 61603, 61609, 61613, 61627, 61631, 61637, 61643, 61651, 61657, 61667,
61673, 61681, 61687, 61703, 61717, 61723, 61729, 61751, 61757, 61781, 61813,
61819, 61837, 61843, 61861, 61871, 61879, 61909, 61927, 61933, 61949, 61961,
61967, 61979, 61981, 61987, 61991, 62003, 62011, 62017, 62039, 62047, 62053,
62057, 62071, 62081, 62099, 62119, 62129, 62131, 62137, 62141, 62143, 62171,
62189, 62191, 62201, 62207, 62213, 62219, 62233, 62273, 62297, 62299, 62303,
62311, 62323, 62327, 62347, 62351, 62383, 62401, 62417, 62423, 62459, 62467,
62473, 62477, 62483, 62497, 62501, 62507, 62533, 62539, 62549, 62563, 62581,
62591, 62597, 62603, 62617, 62627, 62633, 62639, 62653, 62659, 62683, 62687,
62701, 62723, 62731, 62743, 62753, 62761, 62773, 62791, 62801, 62819, 62827,
62851, 62861, 62869, 62873, 62897, 62903, 62921, 62927, 62929, 62939, 62969,
62971, 62981, 62983, 62987, 62989, 63029, 63031, 63059, 63067, 63073, 63079,
63097, 63103, 63113, 63127, 63131, 63149, 63179, 63197, 63199, 63211, 63241,
63247, 63277, 63281, 63299, 63311, 63313, 63317, 63331, 63337, 63347, 63353,
63361, 63367, 63377, 63389, 63391, 63397, 63409, 63419, 63421, 63439, 63443,
63463, 63467, 63473, 63487, 63493, 63499, 63521, 63527, 63533, 63541, 63559,
63577, 63587, 63589, 63599, 63601, 63607, 63611, 63617, 63629, 63647, 63649,
63659, 63667, 63671, 63689, 63691, 63697, 63703, 63709, 63719, 63727, 63737,
63743, 63761, 63773, 63781, 63793, 63799, 63803, 63809, 63823, 63839, 63841,
63853, 63857, 63863, 63901, 63907, 63913, 63929, 63949, 63977, 63997, 64007,
64013, 64019, 64033, 64037, 64063, 64067, 64081, 64091, 64109, 64123, 64151,
64153, 64157, 64171, 64187, 64189, 64217, 64223, 64231, 64237, 64271, 64279,
64283, 64301, 64303, 64319, 64327, 64333, 64373, 64381, 64399, 64403, 64433,
64439, 64451, 64453, 64483, 64489, 64499, 64513, 64553, 64567, 64577, 64579,
64591, 64601, 64609, 64613, 64621, 64627, 64633, 64661, 64663, 64667, 64679,
64693, 64709, 64717, 64747, 64763, 64781, 64783, 64793, 64811, 64817, 64849,
64853, 64871, 64877, 64879, 64891, 64901, 64919, 64921, 64927, 64937, 64951,
64969, 64997, 65003, 65011, 65027, 65029, 65033, 65053, 65063, 65071, 65089,
65099, 65101, 65111, 65119, 65123, 65129, 65141, 65147, 65167, 65171, 65173,
65179, 65183, 65203, 65213, 65239, 65257, 65267, 65269, 65287, 65293, 65309,
65323, 65327, 65353, 65357, 65371, 65381, 65393, 65407, 65413, 65419, 65423,
65437, 65447, 65449, 65479, 65497, 65519, 65521, 0 };

}
/*
* Modular Reducer
* (C) 1999-2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Modular_Reducer Constructor
*/
Modular_Reducer::Modular_Reducer(const BigInt& mod)
   {
   if(mod <= 0)
      throw Invalid_Argument("Modular_Reducer: modulus must be positive");

   modulus = mod;
   mod_words = modulus.sig_words();

   modulus_2 = Botan::square(modulus);

   mu = BigInt::power_of_2(2 * MP_WORD_BITS * mod_words) / modulus;
   }

/*
* Barrett Reduction
*/
BigInt Modular_Reducer::reduce(const BigInt& x) const
   {
   if(mod_words == 0)
      throw Invalid_State("Modular_Reducer: Never initalized");

   if(x.cmp(modulus, false) < 0)
      {
      if(x.is_negative())
         return x + modulus; // make positive
      return x;
      }
   else if(x.cmp(modulus_2, false) < 0)
      {
      BigInt t1 = x;
      t1.set_sign(BigInt::Positive);
      t1 >>= (MP_WORD_BITS * (mod_words - 1));
      t1 *= mu;

      t1 >>= (MP_WORD_BITS * (mod_words + 1));
      t1 *= modulus;

      t1.mask_bits(MP_WORD_BITS * (mod_words + 1));

      BigInt t2 = x;
      t2.set_sign(BigInt::Positive);
      t2.mask_bits(MP_WORD_BITS * (mod_words + 1));

      t2 -= t1;

      if(t2.is_negative())
         {
         t2 += BigInt::power_of_2(MP_WORD_BITS * (mod_words + 1));
         }

      while(t2 >= modulus)
         t2 -= modulus;

      if(x.is_positive())
         return t2;
      else
         return (modulus - t2);
      }
   else
      {
      // too big, fall back to normal division
      return (x % modulus);
      }
   }

}
/*
* Shanks-Tonnelli (RESSOL)
* (C) 2007-2008 Falko Strenzke, FlexSecure GmbH
* (C) 2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Shanks-Tonnelli algorithm
*/
BigInt ressol(const BigInt& a, const BigInt& p)
   {
   if(a < 0)
      throw Invalid_Argument("ressol(): a to solve for must be positive");
   if(p <= 1)
      throw Invalid_Argument("ressol(): prime must be > 1");

   if(a == 0)
      return 0;
   if(p == 2)
      return a;

   if(jacobi(a, p) != 1) // not a quadratic residue
      return -BigInt(1);

   if(p % 4 == 3)
      return power_mod(a, ((p+1) >> 2), p);

   size_t s = low_zero_bits(p - 1);
   BigInt q = p >> s;

   q -= 1;
   q >>= 1;

   Modular_Reducer mod_p(p);

   BigInt r = power_mod(a, q, p);
   BigInt n = mod_p.multiply(a, mod_p.square(r));
   r = mod_p.multiply(r, a);

   if(n == 1)
      return r;

   // find random non quadratic residue z
   BigInt z = 2;
   while(jacobi(z, p) == 1) // while z quadratic residue
      ++z;

   BigInt c = power_mod(z, (q << 1) + 1, p);

   while(n > 1)
      {
      q = n;

      size_t i = 0;
      while(q != 1)
         {
         q = mod_p.square(q);
         ++i;
         }

      if(s <= i)
         return -BigInt(1);

      c = power_mod(c, BigInt::power_of_2(s-i-1), p);
      r = mod_p.multiply(r, c);
      c = mod_p.square(c);
      n = mod_p.multiply(n, c);
      s = i;
      }

   return r;
   }

}
/*
* Interface for AEAD modes
* (C) 2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_AEAD_CCM)
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
#endif

#if defined(BOTAN_HAS_AEAD_SIV)
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
#endif

namespace Botan {

AEAD_Mode* get_aead(const std::string& algo_spec, Cipher_Dir direction)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();

   const std::vector<std::string> algo_parts = split_on(algo_spec, '/');
   if(algo_parts.empty())
      throw Invalid_Algorithm_Name(algo_spec);

   if(algo_parts.size() < 2)
      return nullptr;

   const std::string cipher_name = algo_parts[0];
   const BlockCipher* cipher = af.prototype_block_cipher(cipher_name);
   if(!cipher)
      return nullptr;

   const std::vector<std::string> mode_info = parse_algorithm_name(algo_parts[1]);

   if(mode_info.empty())
      return nullptr;

   const std::string mode_name = mode_info[0];

   const size_t tag_size = (mode_info.size() > 1) ? to_u32bit(mode_info[1]) : cipher->block_size();

#if defined(BOTAN_HAS_AEAD_CCM)
   if(mode_name == "CCM-8")
      {
      if(direction == ENCRYPTION)
         return new CCM_Encryption(cipher->clone(), 8, 3);
      else
         return new CCM_Decryption(cipher->clone(), 8, 3);
      }

   if(mode_name == "CCM" || mode_name == "CCM-8")
      {
      const size_t L = (mode_info.size() > 2) ? to_u32bit(mode_info[2]) : 3;

      if(direction == ENCRYPTION)
         return new CCM_Encryption(cipher->clone(), tag_size, L);
      else
         return new CCM_Decryption(cipher->clone(), tag_size, L);
      }
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
   if(mode_name == "EAX")
      {
      if(direction == ENCRYPTION)
         return new EAX_Encryption(cipher->clone(), tag_size);
      else
         return new EAX_Decryption(cipher->clone(), tag_size);
      }
#endif

#if defined(BOTAN_HAS_AEAD_SIV)
   if(mode_name == "SIV")
      {
      BOTAN_ASSERT(tag_size == 16, "Valid tag size for SIV");
      if(direction == ENCRYPTION)
         return new SIV_Encryption(cipher->clone());
      else
         return new SIV_Decryption(cipher->clone());
      }
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
   if(mode_name == "GCM")
      {
      if(direction == ENCRYPTION)
         return new GCM_Encryption(cipher->clone(), tag_size);
      else
         return new GCM_Decryption(cipher->clone(), tag_size);
      }
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
   if(mode_name == "OCB")
      {
      if(direction == ENCRYPTION)
         return new OCB_Encryption(cipher->clone(), tag_size);
      else
         return new OCB_Decryption(cipher->clone(), tag_size);
      }
#endif

   return nullptr;
   }

}
/*
* EAX Mode Encryption
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

/*
* EAX MAC-based PRF
*/
secure_vector<byte> eax_prf(byte tag, size_t block_size,
                           MessageAuthenticationCode& mac,
                           const byte in[], size_t length)
   {
   for(size_t i = 0; i != block_size - 1; ++i)
      mac.update(0);
   mac.update(tag);
   mac.update(in, length);
   return mac.final();
   }

}

/*
* EAX_Mode Constructor
*/
EAX_Mode::EAX_Mode(BlockCipher* cipher, size_t tag_size) :
   m_tag_size(tag_size ? tag_size : cipher->block_size()),
   m_cipher(cipher),
   m_ctr(new CTR_BE(m_cipher->clone())),
   m_cmac(new CMAC(m_cipher->clone()))
   {
   if(m_tag_size < 8 || m_tag_size > m_cmac->output_length())
      throw Invalid_Argument(name() + ": Bad tag size " + std::to_string(tag_size));
   }

void EAX_Mode::clear()
   {
   m_cipher.reset();
   m_ctr.reset();
   m_cmac.reset();
   zeroise(m_ad_mac);
   zeroise(m_nonce_mac);
   }

std::string EAX_Mode::name() const
   {
   return (m_cipher->name() + "/EAX");
   }

size_t EAX_Mode::update_granularity() const
   {
   return 8 * m_cipher->parallel_bytes();
   }

Key_Length_Specification EAX_Mode::key_spec() const
   {
   return m_cipher->key_spec();
   }

/*
* Set the EAX key
*/
void EAX_Mode::key_schedule(const byte key[], size_t length)
   {
   /*
   * These could share the key schedule, which is one nice part of EAX,
   * but it's much easier to ignore that here...
   */
   m_ctr->set_key(key, length);
   m_cmac->set_key(key, length);

   m_ad_mac = eax_prf(1, block_size(), *m_cmac, nullptr, 0);
   }

/*
* Set the EAX associated data
*/
void EAX_Mode::set_associated_data(const byte ad[], size_t length)
   {
   m_ad_mac = eax_prf(1, block_size(), *m_cmac, ad, length);
   }

secure_vector<byte> EAX_Mode::start(const byte nonce[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   m_nonce_mac = eax_prf(0, block_size(), *m_cmac, nonce, nonce_len);

   m_ctr->set_iv(&m_nonce_mac[0], m_nonce_mac.size());

   for(size_t i = 0; i != block_size() - 1; ++i)
      m_cmac->update(0);
   m_cmac->update(2);

   return secure_vector<byte>();
   }

void EAX_Encryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = &buffer[offset];

   m_ctr->cipher(buf, buf, sz);
   m_cmac->update(buf, sz);
   }

void EAX_Encryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   update(buffer, offset);

   secure_vector<byte> data_mac = m_cmac->final();
   xor_buf(data_mac, m_nonce_mac, data_mac.size());
   xor_buf(data_mac, m_ad_mac, data_mac.size());

   buffer += std::make_pair(&data_mac[0], tag_size());
   }

void EAX_Decryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = &buffer[offset];

   m_cmac->update(buf, sz);
   m_ctr->cipher(buf, buf, sz);
   }

void EAX_Decryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = &buffer[offset];

   BOTAN_ASSERT(sz >= tag_size(), "Have the tag as part of final input");

   const size_t remaining = sz - tag_size();

   if(remaining)
      {
      m_cmac->update(buf, remaining);
      m_ctr->cipher(buf, buf, remaining);
      }

   const byte* included_tag = &buf[remaining];

   secure_vector<byte> mac = m_cmac->final();
   mac ^= m_nonce_mac;
   mac ^= m_ad_mac;

   if(!same_mem(&mac[0], included_tag, tag_size()))
      throw Integrity_Failure("EAX tag check failed");

   buffer.resize(offset + remaining);
   }

}
/*
* GCM Mode Encryption
* (C) 2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_GCM_CLMUL)
#endif

namespace Botan {

void GHASH::gcm_multiply(secure_vector<byte>& x) const
   {
#if defined(BOTAN_HAS_GCM_CLMUL)
   if(CPUID::has_clmul())
      return gcm_multiply_clmul(&x[0], &m_H[0]);
#endif

   static const u64bit R = 0xE100000000000000;

   u64bit H[2] = {
      load_be<u64bit>(&m_H[0], 0),
      load_be<u64bit>(&m_H[0], 1)
   };

   u64bit Z[2] = { 0, 0 };

   // SSE2 might be useful here

   for(size_t i = 0; i != 2; ++i)
      {
      const u64bit X = load_be<u64bit>(&x[0], i);

      for(size_t j = 0; j != 64; ++j)
         {
         if((X >> (63-j)) & 1)
            {
            Z[0] ^= H[0];
            Z[1] ^= H[1];
            }

         const u64bit r = (H[1] & 1) ? R : 0;

         H[1] = (H[0] << 63) | (H[1] >> 1);
         H[0] = (H[0] >> 1) ^ r;
         }
      }

   store_be<u64bit>(&x[0], Z[0], Z[1]);
   }

void GHASH::ghash_update(secure_vector<byte>& ghash,
                         const byte input[], size_t length)
   {
   const size_t BS = 16;

   /*
   This assumes if less than block size input then we're just on the
   final block and should pad with zeros
   */
   while(length)
      {
      const size_t to_proc = std::min(length, BS);

      xor_buf(&ghash[0], &input[0], to_proc);

      gcm_multiply(ghash);

      input += to_proc;
      length -= to_proc;
      }
   }

void GHASH::key_schedule(const byte key[], size_t length)
   {
   m_H.assign(key, key+length);
   m_H_ad.resize(16);
   m_ad_len = 0;
   m_text_len = 0;
   }

void GHASH::start(const byte nonce[], size_t len)
   {
   m_nonce.assign(nonce, nonce + len);
   m_ghash = m_H_ad;
   }

void GHASH::set_associated_data(const byte input[], size_t length)
   {
   zeroise(m_H_ad);

   ghash_update(m_H_ad, input, length);
   m_ad_len = length;
   }

void GHASH::update(const byte input[], size_t length)
   {
   BOTAN_ASSERT(m_ghash.size() == 16, "Key was set");

   m_text_len += length;

   ghash_update(m_ghash, input, length);
   }

void GHASH::add_final_block(secure_vector<byte>& hash,
                            size_t ad_len, size_t text_len)
   {
   secure_vector<byte> final_block(16);
   store_be<u64bit>(&final_block[0], 8*ad_len, 8*text_len);
   ghash_update(hash, &final_block[0], final_block.size());
   }

secure_vector<byte> GHASH::final()
   {
   add_final_block(m_ghash, m_ad_len, m_text_len);

   secure_vector<byte> mac;
   mac.swap(m_ghash);

   mac ^= m_nonce;
   m_text_len = 0;
   return mac;
   }

secure_vector<byte> GHASH::nonce_hash(const byte nonce[], size_t nonce_len)
   {
   BOTAN_ASSERT(m_ghash.size() == 0, "nonce_hash called during wrong time");
   secure_vector<byte> y0(16);

   ghash_update(y0, nonce, nonce_len);
   add_final_block(y0, 0, nonce_len);

   return y0;
   }

void GHASH::clear()
   {
   zeroise(m_H);
   zeroise(m_H_ad);
   m_ghash.clear();
   m_text_len = m_ad_len = 0;
   }

/*
* GCM_Mode Constructor
*/
GCM_Mode::GCM_Mode(BlockCipher* cipher, size_t tag_size) :
   m_tag_size(tag_size),
   m_cipher_name(cipher->name())
   {
   if(cipher->block_size() != BS)
      throw std::invalid_argument("GCM requires a 128 bit cipher so cannot be used with " +
                                  cipher->name());

   m_ghash.reset(new GHASH);

   m_ctr.reset(new CTR_BE(cipher)); // CTR_BE takes ownership of cipher

   if(m_tag_size != 8 && m_tag_size != 16)
      throw Invalid_Argument(name() + ": Bad tag size " + std::to_string(m_tag_size));
   }

void GCM_Mode::clear()
   {
   m_ctr->clear();
   m_ghash->clear();
   }

std::string GCM_Mode::name() const
   {
   return (m_cipher_name + "/GCM");
   }

size_t GCM_Mode::update_granularity() const
   {
   return 4096; // CTR-BE's internal block size
   }

Key_Length_Specification GCM_Mode::key_spec() const
   {
   return m_ctr->key_spec();
   }

void GCM_Mode::key_schedule(const byte key[], size_t keylen)
   {
   m_ctr->set_key(key, keylen);

   const std::vector<byte> zeros(BS);
   m_ctr->set_iv(&zeros[0], zeros.size());

   secure_vector<byte> H(BS);
   m_ctr->encipher(H);
   m_ghash->set_key(H);
   }

void GCM_Mode::set_associated_data(const byte ad[], size_t ad_len)
   {
   m_ghash->set_associated_data(ad, ad_len);
   }

secure_vector<byte> GCM_Mode::start(const byte nonce[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   secure_vector<byte> y0(BS);

   if(nonce_len == 12)
      {
      copy_mem(&y0[0], nonce, nonce_len);
      y0[15] = 1;
      }
   else
      {
      y0 = m_ghash->nonce_hash(nonce, nonce_len);
      }

   m_ctr->set_iv(&y0[0], y0.size());

   secure_vector<byte> m_enc_y0(BS);
   m_ctr->encipher(m_enc_y0);

   m_ghash->start(&m_enc_y0[0], m_enc_y0.size());

   return secure_vector<byte>();
   }

void GCM_Encryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = &buffer[offset];

   m_ctr->cipher(buf, buf, sz);
   m_ghash->update(buf, sz);
   }

void GCM_Encryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   update(buffer, offset);
   auto mac = m_ghash->final();
   buffer += std::make_pair(&mac[0], tag_size());
   }

void GCM_Decryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = &buffer[offset];

   m_ghash->update(buf, sz);
   m_ctr->cipher(buf, buf, sz);
   }

void GCM_Decryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = &buffer[offset];

   BOTAN_ASSERT(sz >= tag_size(), "Have the tag as part of final input");

   const size_t remaining = sz - tag_size();

   // handle any final input before the tag
   if(remaining)
      {
      m_ghash->update(buf, remaining);
      m_ctr->cipher(buf, buf, remaining);
      }

   auto mac = m_ghash->final();

   const byte* included_tag = &buffer[remaining];

   if(!same_mem(&mac[0], included_tag, tag_size()))
      throw Integrity_Failure("GCM tag check failed");

   buffer.resize(offset + remaining);
   }

}
/*
* CBC Padding Methods
* (C) 1999-2007,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Pad with PKCS #7 Method
*/
void PKCS7_Padding::add_padding(secure_vector<byte>& buffer,
                                size_t last_byte_pos,
                                size_t block_size) const
   {
   const byte pad_value = block_size - last_byte_pos;

   for(size_t i = 0; i != pad_value; ++i)
      buffer.push_back(pad_value);
   }

/*
* Unpad with PKCS #7 Method
*/
size_t PKCS7_Padding::unpad(const byte block[], size_t size) const
   {
   size_t position = block[size-1];

   if(position > size)
      throw Decoding_Error("Bad padding in " + name());

   for(size_t j = size-position; j != size-1; ++j)
      if(block[j] != position)
         throw Decoding_Error("Bad padding in " + name());

   return (size-position);
   }

/*
* Pad with ANSI X9.23 Method
*/
void ANSI_X923_Padding::add_padding(secure_vector<byte>& buffer,
                                    size_t last_byte_pos,
                                    size_t block_size) const
   {
   const byte pad_value = block_size - last_byte_pos;

   for(size_t i = last_byte_pos; i < block_size; ++i)
      buffer.push_back(0);
   buffer.push_back(pad_value);
   }

/*
* Unpad with ANSI X9.23 Method
*/
size_t ANSI_X923_Padding::unpad(const byte block[], size_t size) const
   {
   size_t position = block[size-1];
   if(position > size)
      throw Decoding_Error(name());
   for(size_t j = size-position; j != size-1; ++j)
      if(block[j] != 0)
         throw Decoding_Error(name());
   return (size-position);
   }

/*
* Pad with One and Zeros Method
*/
void OneAndZeros_Padding::add_padding(secure_vector<byte>& buffer,
                                      size_t last_byte_pos,
                                      size_t block_size) const
   {
   buffer.push_back(0x80);

   for(size_t i = last_byte_pos + 1; i % block_size; ++i)
      buffer.push_back(0x00);
   }

/*
* Unpad with One and Zeros Method
*/
size_t OneAndZeros_Padding::unpad(const byte block[], size_t size) const
   {
   while(size)
      {
      if(block[size-1] == 0x80)
         break;
      if(block[size-1] != 0x00)
         throw Decoding_Error(name());
      size--;
      }
   if(!size)
      throw Decoding_Error(name());
   return (size-1);
   }


}
/*
* PBE Retrieval
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_PBE_PKCS_V20)
#endif

namespace Botan {

/*
* Get an encryption PBE, set new parameters
*/
PBE* get_pbe(const std::string& algo_spec,
             const std::string& passphrase,
             std::chrono::milliseconds msec,
             RandomNumberGenerator& rng)
   {
   SCAN_Name request(algo_spec);

   const std::string pbe = request.algo_name();
   std::string digest_name = request.arg(0);
   const std::string cipher = request.arg(1);

   std::vector<std::string> cipher_spec = split_on(cipher, '/');
   if(cipher_spec.size() != 2)
      throw Invalid_Argument("PBE: Invalid cipher spec " + cipher);

   const std::string cipher_algo = SCAN_Name::deref_alias(cipher_spec[0]);
   const std::string cipher_mode = cipher_spec[1];

   if(cipher_mode != "CBC")
      throw Invalid_Argument("PBE: Invalid cipher mode " + cipher);

   Algorithm_Factory& af = global_state().algorithm_factory();

   const BlockCipher* block_cipher = af.prototype_block_cipher(cipher_algo);
   if(!block_cipher)
      throw Algorithm_Not_Found(cipher_algo);

   const HashFunction* hash_function = af.prototype_hash_function(digest_name);
   if(!hash_function)
      throw Algorithm_Not_Found(digest_name);

   if(request.arg_count() != 2)
      throw Invalid_Algorithm_Name(algo_spec);

#if defined(BOTAN_HAS_PBE_PKCS_V20)
   if(pbe == "PBE-PKCS5v20")
      return new PBE_PKCS5v20(block_cipher->clone(),
                              new HMAC(hash_function->clone()),
                              passphrase,
                              msec,
                              rng);
#endif

   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Get a decryption PBE, decode parameters
*/
PBE* get_pbe(const OID& pbe_oid,
             const std::vector<byte>& params,
             const std::string& passphrase)
   {
   SCAN_Name request(OIDS::lookup(pbe_oid));

   const std::string pbe = request.algo_name();

#if defined(BOTAN_HAS_PBE_PKCS_V20)
   if(pbe == "PBE-PKCS5v20")
      return new PBE_PKCS5v20(params, passphrase);
#endif

   throw Algorithm_Not_Found(pbe_oid.as_string());
   }

}
/*
* PBKDF
* (C) 2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

OctetString PBKDF::derive_key(size_t output_len,
                              const std::string& passphrase,
                              const byte salt[], size_t salt_len,
                              size_t iterations) const
   {
   if(iterations == 0)
      throw std::invalid_argument(name() + ": Invalid iteration count");

   auto derived = key_derivation(output_len, passphrase,
                                 salt, salt_len, iterations,
                                 std::chrono::milliseconds(0));

   BOTAN_ASSERT(derived.first == iterations,
                "PBKDF used the correct number of iterations");

   return derived.second;
   }

OctetString PBKDF::derive_key(size_t output_len,
                              const std::string& passphrase,
                              const byte salt[], size_t salt_len,
                              std::chrono::milliseconds ms,
                              size_t& iterations) const
   {
   auto derived = key_derivation(output_len, passphrase, salt, salt_len, 0, ms);

   iterations = derived.first;

   return derived.second;
   }

}
/*
* PBKDF2
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Return a PKCS #5 PBKDF2 derived key
*/
std::pair<size_t, OctetString>
PKCS5_PBKDF2::key_derivation(size_t key_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             size_t iterations,
                             std::chrono::milliseconds msec) const
   {
   if(key_len == 0)
      return std::make_pair(iterations, OctetString());

   try
      {
      mac->set_key(reinterpret_cast<const byte*>(passphrase.data()),
                   passphrase.length());
      }
   catch(Invalid_Key_Length)
      {
      throw Exception(name() + " cannot accept passphrases of length " +
                      std::to_string(passphrase.length()));
      }

   secure_vector<byte> key(key_len);

   byte* T = &key[0];

   secure_vector<byte> U(mac->output_length());

   const size_t blocks_needed = round_up(key_len, mac->output_length()) / mac->output_length();

   std::chrono::microseconds usec_per_block =
      std::chrono::duration_cast<std::chrono::microseconds>(msec) / blocks_needed;

   u32bit counter = 1;
   while(key_len)
      {
      size_t T_size = std::min<size_t>(mac->output_length(), key_len);

      mac->update(salt, salt_len);
      mac->update_be(counter);
      mac->final(&U[0]);

      xor_buf(T, &U[0], T_size);

      if(iterations == 0)
         {
         /*
         If no iterations set, run the first block to calibrate based
         on how long hashing takes on whatever machine we're running on.
         */

         const auto start = std::chrono::high_resolution_clock::now();

         iterations = 1; // the first iteration we did above

         while(true)
            {
            mac->update(U);
            mac->final(&U[0]);
            xor_buf(T, &U[0], T_size);
            iterations++;

            /*
            Only break on relatively 'even' iterations. For one it
            avoids confusion, and likely some broken implementations
            break on getting completely randomly distributed values
            */
            if(iterations % 10000 == 0)
               {
               auto time_taken = std::chrono::high_resolution_clock::now() - start;
               auto usec_taken = std::chrono::duration_cast<std::chrono::microseconds>(time_taken);
               if(usec_taken > usec_per_block)
                  break;
               }
            }
         }
      else
         {
         for(size_t i = 1; i != iterations; ++i)
            {
            mac->update(U);
            mac->final(&U[0]);
            xor_buf(T, &U[0], T_size);
            }
         }

      key_len -= T_size;
      T += T_size;
      ++counter;
      }

   return std::make_pair(iterations, key);
   }

}
/*
* EME Base Class
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Encode a message
*/
secure_vector<byte> EME::encode(const byte msg[], size_t msg_len,
                               size_t key_bits,
                               RandomNumberGenerator& rng) const
   {
   return pad(msg, msg_len, key_bits, rng);
   }

/*
* Encode a message
*/
secure_vector<byte> EME::encode(const secure_vector<byte>& msg,
                               size_t key_bits,
                               RandomNumberGenerator& rng) const
   {
   return pad(&msg[0], msg.size(), key_bits, rng);
   }

/*
* Decode a message
*/
secure_vector<byte> EME::decode(const byte msg[], size_t msg_len,
                               size_t key_bits) const
   {
   return unpad(msg, msg_len, key_bits);
   }

/*
* Decode a message
*/
secure_vector<byte> EME::decode(const secure_vector<byte>& msg,
                               size_t key_bits) const
   {
   return unpad(&msg[0], msg.size(), key_bits);
   }

}
/*
* EMSA/EME Retrieval
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_EMSA1)
#endif

#if defined(BOTAN_HAS_EMSA1_BSI)
#endif

#if defined(BOTAN_HAS_EMSA_X931)
#endif

#if defined(BOTAN_HAS_EMSA_PKCS1)
#endif

#if defined(BOTAN_HAS_EMSA_PSSR)
#endif

#if defined(BOTAN_HAS_EMSA_RAW)
#endif

#if defined(BOTAN_HAS_EME_OAEP)
#endif

#if defined(BOTAN_HAS_EME_PKCS1v15)
#endif

namespace Botan {

/*
* Get an EMSA by name
*/
EMSA* get_emsa(const std::string& algo_spec)
   {
   SCAN_Name request(algo_spec);

   Algorithm_Factory& af = global_state().algorithm_factory();

#if defined(BOTAN_HAS_EMSA_RAW)
   if(request.algo_name() == "Raw" && request.arg_count() == 0)
      return new EMSA_Raw;
#endif

   if(request.algo_name() == "EMSA1" && request.arg_count() == 1)
      {
#if defined(BOTAN_HAS_EMSA_RAW)
      if(request.arg(0) == "Raw")
         return new EMSA_Raw;
#endif

#if defined(BOTAN_HAS_EMSA1)
      return new EMSA1(af.make_hash_function(request.arg(0)));
#endif
      }

#if defined(BOTAN_HAS_EMSA1_BSI)
   if(request.algo_name() == "EMSA1_BSI" && request.arg_count() == 1)
      return new EMSA1_BSI(af.make_hash_function(request.arg(0)));
#endif

#if defined(BOTAN_HAS_EMSA_X931)
   if(request.algo_name() == "EMSA_X931" && request.arg_count() == 1)
      return new EMSA_X931(af.make_hash_function(request.arg(0)));
#endif

#if defined(BOTAN_HAS_EMSA_PKCS1)
   if(request.algo_name() == "EMSA_PKCS1" && request.arg_count() == 1)
      {
      if(request.arg(0) == "Raw")
         return new EMSA_PKCS1v15_Raw;
      return new EMSA_PKCS1v15(af.make_hash_function(request.arg(0)));
      }
#endif

#if defined(BOTAN_HAS_EMSA_PSSR)
   if(request.algo_name() == "PSSR" && request.arg_count_between(1, 3))
      {
      // 3 args: Hash, MGF, salt size (MGF is hardcoded MGF1 in Botan)
      if(request.arg_count() == 1)
         return new PSSR(af.make_hash_function(request.arg(0)));

      if(request.arg_count() == 2 && request.arg(1) != "MGF1")
         return new PSSR(af.make_hash_function(request.arg(0)));

      if(request.arg_count() == 3)
         return new PSSR(af.make_hash_function(request.arg(0)),
                         request.arg_as_integer(2, 0));
      }
#endif

   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Get an EME by name
*/
EME* get_eme(const std::string& algo_spec)
   {
   SCAN_Name request(algo_spec);

   if(request.algo_name() == "Raw")
      return nullptr; // No padding

#if defined(BOTAN_HAS_EME_PKCS1v15)
   if(request.algo_name() == "PKCS1v15" && request.arg_count() == 0)
      return new EME_PKCS1v15;
#endif

#if defined(BOTAN_HAS_EME_OAEP)
   Algorithm_Factory& af = global_state().algorithm_factory();

   if(request.algo_name() == "OAEP" && request.arg_count_between(1, 2))
      {
      if(request.arg_count() == 1 ||
         (request.arg_count() == 2 && request.arg(1) == "MGF1"))
         {
         return new OAEP(af.make_hash_function(request.arg(0)));
         }
      }
#endif

   throw Algorithm_Not_Found(algo_spec);
   }

}
/*
* Blinding for public key operations
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Blinder Constructor
*/
Blinder::Blinder(const BigInt& e, const BigInt& d, const BigInt& n)
   {
   if(e < 1 || d < 1 || n < 1)
      throw Invalid_Argument("Blinder: Arguments too small");

   reducer = Modular_Reducer(n);
   this->e = e;
   this->d = d;
   }

/*
* Blind a number
*/
BigInt Blinder::blind(const BigInt& i) const
   {
   if(!reducer.initialized())
      return i;

   e = reducer.square(e);
   d = reducer.square(d);
   return reducer.multiply(i, e);
   }

/*
* Unblind a number
*/
BigInt Blinder::unblind(const BigInt& i) const
   {
   if(!reducer.initialized())
      return i;
   return reducer.multiply(i, d);
   }

}
/*
* PK Key
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_HAS_RSA)
#endif

#if defined(BOTAN_HAS_DSA)
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
#endif

#if defined(BOTAN_HAS_ECDSA)
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
#endif

#if defined(BOTAN_HAS_RW)
#endif

#if defined(BOTAN_HAS_ELGAMAL)
#endif

#if defined(BOTAN_HAS_ECDH)
#endif

namespace Botan {

Public_Key* make_public_key(const AlgorithmIdentifier& alg_id,
                            const secure_vector<byte>& key_bits)
   {
   const std::string alg_name = OIDS::lookup(alg_id.oid);
   if(alg_name == "")
      throw Decoding_Error("Unknown algorithm OID: " + alg_id.oid.as_string());

#if defined(BOTAN_HAS_RSA)
   if(alg_name == "RSA")
      return new RSA_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_RW)
   if(alg_name == "RW")
      return new RW_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_DSA)
   if(alg_name == "DSA")
      return new DSA_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
   if(alg_name == "DH")
      return new DH_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
   if(alg_name == "NR")
      return new NR_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
   if(alg_name == "ElGamal")
      return new ElGamal_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ECDSA)
   if(alg_name == "ECDSA")
      return new ECDSA_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
   if(alg_name == "GOST-34.10")
      return new GOST_3410_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ECDH)
   if(alg_name == "ECDH")
      return new ECDH_PublicKey(alg_id, key_bits);
#endif

   return nullptr;
   }

Private_Key* make_private_key(const AlgorithmIdentifier& alg_id,
                              const secure_vector<byte>& key_bits,
                              RandomNumberGenerator& rng)
   {
   const std::string alg_name = OIDS::lookup(alg_id.oid);
   if(alg_name == "")
      throw Decoding_Error("Unknown algorithm OID: " + alg_id.oid.as_string());

#if defined(BOTAN_HAS_RSA)
   if(alg_name == "RSA")
      return new RSA_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_RW)
   if(alg_name == "RW")
      return new RW_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_DSA)
   if(alg_name == "DSA")
      return new DSA_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
   if(alg_name == "DH")
      return new DH_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
   if(alg_name == "NR")
      return new NR_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
   if(alg_name == "ElGamal")
      return new ElGamal_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_ECDSA)
   if(alg_name == "ECDSA")
      return new ECDSA_PrivateKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
   if(alg_name == "GOST-34.10")
      return new GOST_3410_PrivateKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ECDH)
   if(alg_name == "ECDH")
      return new ECDH_PrivateKey(alg_id, key_bits);
#endif

   return nullptr;
   }

}
/*
* PK Key Types
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* Default OID access
*/
OID Public_Key::get_oid() const
   {
   try {
      return OIDS::lookup(algo_name());
      }
   catch(Lookup_Error)
      {
      throw Lookup_Error("PK algo " + algo_name() + " has no defined OIDs");
      }
   }

/*
* Run checks on a loaded public key
*/
void Public_Key::load_check(RandomNumberGenerator& rng) const
   {
   if(!check_key(rng, BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD))
      throw Invalid_Argument(algo_name() + ": Invalid public key");
   }

/*
* Run checks on a loaded private key
*/
void Private_Key::load_check(RandomNumberGenerator& rng) const
   {
   if(!check_key(rng, BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD))
      throw Invalid_Argument(algo_name() + ": Invalid private key");
   }

/*
* Run checks on a generated private key
*/
void Private_Key::gen_check(RandomNumberGenerator& rng) const
   {
   if(!check_key(rng, BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE))
      throw Self_Test_Failure(algo_name() + " private key generation failed");
   }

}
/*
* PKCS #8
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace PKCS8 {

namespace {

/*
* Get info from an EncryptedPrivateKeyInfo
*/
secure_vector<byte> PKCS8_extract(DataSource& source,
                                  AlgorithmIdentifier& pbe_alg_id)
   {
   secure_vector<byte> key_data;

   BER_Decoder(source)
      .start_cons(SEQUENCE)
         .decode(pbe_alg_id)
         .decode(key_data, OCTET_STRING)
      .verify_end();

   return key_data;
   }

/*
* PEM decode and/or decrypt a private key
*/
secure_vector<byte> PKCS8_decode(
   DataSource& source,
   std::function<std::pair<bool,std::string> ()> get_passphrase,
   AlgorithmIdentifier& pk_alg_id)
   {
   AlgorithmIdentifier pbe_alg_id;
   secure_vector<byte> key_data, key;
   bool is_encrypted = true;

   try {
      if(ASN1::maybe_BER(source) && !PEM_Code::matches(source))
         key_data = PKCS8_extract(source, pbe_alg_id);
      else
         {
         std::string label;
         key_data = PEM_Code::decode(source, label);
         if(label == "PRIVATE KEY")
            is_encrypted = false;
         else if(label == "ENCRYPTED PRIVATE KEY")
            {
            DataSource_Memory key_source(key_data);
            key_data = PKCS8_extract(key_source, pbe_alg_id);
            }
         else
            throw PKCS8_Exception("Unknown PEM label " + label);
         }

      if(key_data.empty())
         throw PKCS8_Exception("No key data found");
      }
   catch(Decoding_Error& e)
      {
      throw Decoding_Error("PKCS #8 private key decoding failed: " + std::string(e.what()));
      }

   if(!is_encrypted)
      key = key_data;

   const size_t MAX_TRIES = 3;

   size_t tries = 0;
   while(true)
      {
      try {
         if(MAX_TRIES && tries >= MAX_TRIES)
            break;

         if(is_encrypted)
            {
            std::pair<bool, std::string> pass = get_passphrase();

            if(pass.first == false)
               break;

            Pipe decryptor(get_pbe(pbe_alg_id.oid, pbe_alg_id.parameters, pass.second));

            decryptor.process_msg(key_data);
            key = decryptor.read_all();
            }

         BER_Decoder(key)
            .start_cons(SEQUENCE)
               .decode_and_check<size_t>(0, "Unknown PKCS #8 version number")
               .decode(pk_alg_id)
               .decode(key, OCTET_STRING)
               .discard_remaining()
            .end_cons();

         break;
         }
      catch(Decoding_Error)
         {
         ++tries;
         }
      }

   if(key.empty())
      throw Decoding_Error("PKCS #8 private key decoding failed");
   return key;
   }

}

/*
* BER encode a PKCS #8 private key, unencrypted
*/
secure_vector<byte> BER_encode(const Private_Key& key)
   {
   const size_t PKCS8_VERSION = 0;

   return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(PKCS8_VERSION)
            .encode(key.pkcs8_algorithm_identifier())
            .encode(key.pkcs8_private_key(), OCTET_STRING)
         .end_cons()
      .get_contents();
   }

/*
* PEM encode a PKCS #8 private key, unencrypted
*/
std::string PEM_encode(const Private_Key& key)
   {
   return PEM_Code::encode(PKCS8::BER_encode(key), "PRIVATE KEY");
   }

/*
* BER encode a PKCS #8 private key, encrypted
*/
std::vector<byte> BER_encode(const Private_Key& key,
                             RandomNumberGenerator& rng,
                             const std::string& pass,
                             std::chrono::milliseconds msec,
                             const std::string& pbe_algo)
   {
   const std::string DEFAULT_PBE = "PBE-PKCS5v20(SHA-1,AES-256/CBC)";

   std::unique_ptr<PBE> pbe(
      get_pbe(((pbe_algo != "") ? pbe_algo : DEFAULT_PBE),
              pass,
              msec,
              rng));

   AlgorithmIdentifier pbe_algid(pbe->get_oid(), pbe->encode_params());

   Pipe key_encrytor(pbe.release());
   key_encrytor.process_msg(PKCS8::BER_encode(key));

   return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(pbe_algid)
            .encode(key_encrytor.read_all(), OCTET_STRING)
         .end_cons()
      .get_contents_unlocked();
   }

/*
* PEM encode a PKCS #8 private key, encrypted
*/
std::string PEM_encode(const Private_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& pass,
                       std::chrono::milliseconds msec,
                       const std::string& pbe_algo)
   {
   if(pass == "")
      return PEM_encode(key);

   return PEM_Code::encode(PKCS8::BER_encode(key, rng, pass, msec, pbe_algo),
                           "ENCRYPTED PRIVATE KEY");
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(DataSource& source,
                      RandomNumberGenerator& rng,
                      std::function<std::pair<bool, std::string> ()> get_pass)
   {
   AlgorithmIdentifier alg_id;
   secure_vector<byte> pkcs8_key = PKCS8_decode(source, get_pass, alg_id);

   const std::string alg_name = OIDS::lookup(alg_id.oid);
   if(alg_name == "" || alg_name == alg_id.oid.as_string())
      throw PKCS8_Exception("Unknown algorithm OID: " +
                            alg_id.oid.as_string());

   return make_private_key(alg_id, pkcs8_key, rng);
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(const std::string& fsname,
                      RandomNumberGenerator& rng,
                      std::function<std::pair<bool, std::string> ()> get_pass)
   {
   DataSource_Stream source(fsname, true);
   return PKCS8::load_key(source, rng, get_pass);
   }

namespace {

class Single_Shot_Passphrase
   {
   public:
      Single_Shot_Passphrase(const std::string& pass) :
         passphrase(pass), first(true) {}

      std::pair<bool, std::string> operator()()
         {
         if(first)
            {
            first = false;
            return std::make_pair(true, passphrase);
            }
         else
            return std::make_pair(false, "");
         }

   private:
      std::string passphrase;
      bool first;
   };

}

/*
* Extract a private key and return it
*/
Private_Key* load_key(DataSource& source,
                      RandomNumberGenerator& rng,
                      const std::string& pass)
   {
   return PKCS8::load_key(source, rng, Single_Shot_Passphrase(pass));
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(const std::string& fsname,
                      RandomNumberGenerator& rng,
                      const std::string& pass)
   {
   return PKCS8::load_key(fsname, rng, Single_Shot_Passphrase(pass));
   }

/*
* Make a copy of this private key
*/
Private_Key* copy_key(const Private_Key& key,
                      RandomNumberGenerator& rng)
   {
   DataSource_Memory source(PEM_encode(key));
   return PKCS8::load_key(source, rng);
   }

}

}
/*
* Public Key Base
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
* PK_Encryptor_EME Constructor
*/
PK_Encryptor_EME::PK_Encryptor_EME(const Public_Key& key,
                                   const std::string& eme_name)
   {
   Algorithm_Factory::Engine_Iterator i(global_state().algorithm_factory());
   RandomNumberGenerator& rng = global_state().global_rng();

   while(const Engine* engine = i.next())
      {
      m_op.reset(engine->get_encryption_op(key, rng));
      if(m_op)
         break;
      }

   if(!m_op)
      throw Lookup_Error("Encryption with " + key.algo_name() + " not supported");

   m_eme.reset(get_eme(eme_name));
   }

/*
* Encrypt a message
*/
std::vector<byte>
PK_Encryptor_EME::enc(const byte in[],
                      size_t length,
                      RandomNumberGenerator& rng) const
   {
   if(m_eme)
      {
      secure_vector<byte> encoded =
         m_eme->encode(in, length, m_op->max_input_bits(), rng);

      if(8*(encoded.size() - 1) + high_bit(encoded[0]) > m_op->max_input_bits())
         throw Invalid_Argument("PK_Encryptor_EME: Input is too large");

      return unlock(m_op->encrypt(&encoded[0], encoded.size(), rng));
      }
   else
      {
      if(8*(length - 1) + high_bit(in[0]) > m_op->max_input_bits())
         throw Invalid_Argument("PK_Encryptor_EME: Input is too large");

      return unlock(m_op->encrypt(&in[0], length, rng));
      }
   }

/*
* Return the max size, in bytes, of a message
*/
size_t PK_Encryptor_EME::maximum_input_size() const
   {
   if(!m_eme)
      return (m_op->max_input_bits() / 8);
   else
      return m_eme->maximum_input_size(m_op->max_input_bits());
   }

/*
* PK_Decryptor_EME Constructor
*/
PK_Decryptor_EME::PK_Decryptor_EME(const Private_Key& key,
                                   const std::string& eme_name)
   {
   Algorithm_Factory::Engine_Iterator i(global_state().algorithm_factory());
   RandomNumberGenerator& rng = global_state().global_rng();

   while(const Engine* engine = i.next())
      {
      m_op.reset(engine->get_decryption_op(key, rng));
      if(m_op)
         break;
      }

   if(!m_op)
      throw Lookup_Error("Decryption with " + key.algo_name() + " not supported");

   m_eme.reset(get_eme(eme_name));
   }

/*
* Decrypt a message
*/
secure_vector<byte> PK_Decryptor_EME::dec(const byte msg[],
                                          size_t length) const
   {
   try {
      secure_vector<byte> decrypted = m_op->decrypt(msg, length);
      if(m_eme)
         return m_eme->decode(decrypted, m_op->max_input_bits());
      else
         return decrypted;
      }
   catch(Invalid_Argument)
      {
      throw Decoding_Error("PK_Decryptor_EME: Input is invalid");
      }
   }

/*
* PK_Signer Constructor
*/
PK_Signer::PK_Signer(const Private_Key& key,
                     const std::string& emsa_name,
                     Signature_Format format,
                     Fault_Protection prot)
   {
   Algorithm_Factory::Engine_Iterator i(global_state().algorithm_factory());
   RandomNumberGenerator& rng = global_state().global_rng();

   m_op = nullptr;
   m_verify_op = nullptr;

   while(const Engine* engine = i.next())
      {
      if(!m_op)
         m_op.reset(engine->get_signature_op(key, rng));

      if(!m_verify_op && prot == ENABLE_FAULT_PROTECTION)
         m_verify_op.reset(engine->get_verify_op(key, rng));

      if(m_op && (m_verify_op || prot == DISABLE_FAULT_PROTECTION))
         break;
      }

   if(!m_op || (!m_verify_op && prot == ENABLE_FAULT_PROTECTION))
      throw Lookup_Error("Signing with " + key.algo_name() + " not supported");

   m_emsa.reset(get_emsa(emsa_name));
   m_sig_format = format;
   }

/*
* Sign a message
*/
std::vector<byte> PK_Signer::sign_message(const byte msg[], size_t length,
                                           RandomNumberGenerator& rng)
   {
   update(msg, length);
   return signature(rng);
   }

/*
* Add more to the message to be signed
*/
void PK_Signer::update(const byte in[], size_t length)
   {
   m_emsa->update(in, length);
   }

/*
* Check the signature we just created, to help prevent fault attacks
*/
bool PK_Signer::self_test_signature(const std::vector<byte>& msg,
                                    const std::vector<byte>& sig) const
   {
   if(!m_verify_op)
      return true; // checking disabled, assume ok

   if(m_verify_op->with_recovery())
      {
      std::vector<byte> recovered =
         unlock(m_verify_op->verify_mr(&sig[0], sig.size()));

      if(msg.size() > recovered.size())
         {
         size_t extra_0s = msg.size() - recovered.size();

         for(size_t i = 0; i != extra_0s; ++i)
            if(msg[i] != 0)
               return false;

         return same_mem(&msg[extra_0s], &recovered[0], recovered.size());
         }

      return (recovered == msg);
      }
   else
      return m_verify_op->verify(&msg[0], msg.size(),
                               &sig[0], sig.size());
   }

/*
* Create a signature
*/
std::vector<byte> PK_Signer::signature(RandomNumberGenerator& rng)
   {
   std::vector<byte> encoded = unlock(m_emsa->encoding_of(m_emsa->raw_data(),
                                                 m_op->max_input_bits(),
                                                        rng));

   std::vector<byte> plain_sig = unlock(m_op->sign(&encoded[0], encoded.size(), rng));

   BOTAN_ASSERT(self_test_signature(encoded, plain_sig), "Signature was consistent");

   if(m_op->message_parts() == 1 || m_sig_format == IEEE_1363)
      return plain_sig;

   if(m_sig_format == DER_SEQUENCE)
      {
      if(plain_sig.size() % m_op->message_parts())
         throw Encoding_Error("PK_Signer: strange signature size found");
      const size_t SIZE_OF_PART = plain_sig.size() / m_op->message_parts();

      std::vector<BigInt> sig_parts(m_op->message_parts());
      for(size_t j = 0; j != sig_parts.size(); ++j)
         sig_parts[j].binary_decode(&plain_sig[SIZE_OF_PART*j], SIZE_OF_PART);

      return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode_list(sig_parts)
         .end_cons()
      .get_contents_unlocked();
      }
   else
      throw Encoding_Error("PK_Signer: Unknown signature format " +
                           std::to_string(m_sig_format));
   }

/*
* PK_Verifier Constructor
*/
PK_Verifier::PK_Verifier(const Public_Key& key,
                         const std::string& emsa_name,
                         Signature_Format format)
   {
   Algorithm_Factory::Engine_Iterator i(global_state().algorithm_factory());
   RandomNumberGenerator& rng = global_state().global_rng();

   while(const Engine* engine = i.next())
      {
      m_op.reset(engine->get_verify_op(key, rng));
      if(m_op)
         break;
      }

   if(!m_op)
      throw Lookup_Error("Verification with " + key.algo_name() + " not supported");

   m_emsa.reset(get_emsa(emsa_name));
   m_sig_format = format;
   }

/*
* Set the signature format
*/
void PK_Verifier::set_input_format(Signature_Format format)
   {
   if(m_op->message_parts() == 1 && format != IEEE_1363)
      throw Invalid_State("PK_Verifier: This algorithm always uses IEEE 1363");
   m_sig_format = format;
   }

/*
* Verify a message
*/
bool PK_Verifier::verify_message(const byte msg[], size_t msg_length,
                                 const byte sig[], size_t sig_length)
   {
   update(msg, msg_length);
   return check_signature(sig, sig_length);
   }

/*
* Append to the message
*/
void PK_Verifier::update(const byte in[], size_t length)
   {
   m_emsa->update(in, length);
   }

/*
* Check a signature
*/
bool PK_Verifier::check_signature(const byte sig[], size_t length)
   {
   try {
      if(m_sig_format == IEEE_1363)
         return validate_signature(m_emsa->raw_data(), sig, length);
      else if(m_sig_format == DER_SEQUENCE)
         {
         BER_Decoder decoder(sig, length);
         BER_Decoder ber_sig = decoder.start_cons(SEQUENCE);

         size_t count = 0;
         std::vector<byte> real_sig;
         while(ber_sig.more_items())
            {
            BigInt sig_part;
            ber_sig.decode(sig_part);
            real_sig += BigInt::encode_1363(sig_part, m_op->message_part_size());
            ++count;
            }

         if(count != m_op->message_parts())
            throw Decoding_Error("PK_Verifier: signature size invalid");

         return validate_signature(m_emsa->raw_data(),
                                   &real_sig[0], real_sig.size());
         }
      else
         throw Decoding_Error("PK_Verifier: Unknown signature format " +
                              std::to_string(m_sig_format));
      }
   catch(Invalid_Argument) { return false; }
   }

/*
* Verify a signature
*/
bool PK_Verifier::validate_signature(const secure_vector<byte>& msg,
                                     const byte sig[], size_t sig_len)
   {
   if(m_op->with_recovery())
      {
      secure_vector<byte> output_of_key = m_op->verify_mr(sig, sig_len);
      return m_emsa->verify(output_of_key, msg, m_op->max_input_bits());
      }
   else
      {
      RandomNumberGenerator& rng = global_state().global_rng();

      secure_vector<byte> encoded =
         m_emsa->encoding_of(msg, m_op->max_input_bits(), rng);

      return m_op->verify(&encoded[0], encoded.size(), sig, sig_len);
      }
   }

/*
* PK_Key_Agreement Constructor
*/
PK_Key_Agreement::PK_Key_Agreement(const PK_Key_Agreement_Key& key,
                                   const std::string& kdf_name)
   {
   Algorithm_Factory::Engine_Iterator i(global_state().algorithm_factory());
   RandomNumberGenerator& rng = global_state().global_rng();

   while(const Engine* engine = i.next())
      {
      m_op.reset(engine->get_key_agreement_op(key, rng));
      if(m_op)
         break;
      }

   if(!m_op)
      throw Lookup_Error("Key agreement with " + key.algo_name() + " not supported");

   m_kdf.reset(get_kdf(kdf_name));
   }

SymmetricKey PK_Key_Agreement::derive_key(size_t key_len, const byte in[],
                                          size_t in_len, const byte params[],
                                          size_t params_len) const
   {
   secure_vector<byte> z = m_op->agree(in, in_len);

   if(!m_kdf)
      return z;

   return m_kdf->derive_key(key_len, z, params, params_len);
   }

}
/*
* Public Key Work Factor Functions
* (C) 1999-2007,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <cmath>

namespace Botan {

size_t dl_work_factor(size_t bits)
   {
   /*
   Based on GNFS work factors. Constant is 1.43 times the asymptotic
   value; I'm not sure but I believe that came from a paper on 'real
   world' runtimes, but I don't remember where now.

   Sample return values:
      |512|  -> 64
      |1024| -> 86
      |1536| -> 102
      |2048| -> 116
      |3072| -> 138
      |4096| -> 155
      |8192| -> 206

   For DL algos, we use an exponent of twice the size of the result;
   the assumption is that an arbitrary discrete log on a group of size
   bits would take about 2^n effort, and thus using an exponent of
   size 2^(2*n) implies that all available attacks are about as easy
   (as e.g Pollard's kangaroo algorithm can compute the DL in sqrt(x)
   operations) while minimizing the exponent size for performance
   reasons.
   */

   const size_t MIN_WORKFACTOR = 64;

   // approximates natural logarithm of p
   const double log_p = bits / 1.4426;

   const double strength =
      2.76 * std::pow(log_p, 1.0/3.0) * std::pow(std::log(log_p), 2.0/3.0);

   return std::max(static_cast<size_t>(strength), MIN_WORKFACTOR);
   }

}
/*
* X.509 Public Key
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace X509 {

std::vector<byte> BER_encode(const Public_Key& key)
   {
   return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(key.algorithm_identifier())
            .encode(key.x509_subject_public_key(), BIT_STRING)
         .end_cons()
      .get_contents_unlocked();
   }

/*
* PEM encode a X.509 public key
*/
std::string PEM_encode(const Public_Key& key)
   {
   return PEM_Code::encode(X509::BER_encode(key),
                           "PUBLIC KEY");
   }

/*
* Extract a public key and return it
*/
Public_Key* load_key(DataSource& source)
   {
   try {
      AlgorithmIdentifier alg_id;
      secure_vector<byte> key_bits;

      if(ASN1::maybe_BER(source) && !PEM_Code::matches(source))
         {
         BER_Decoder(source)
            .start_cons(SEQUENCE)
            .decode(alg_id)
            .decode(key_bits, BIT_STRING)
            .verify_end()
         .end_cons();
         }
      else
         {
         DataSource_Memory ber(
            PEM_Code::decode_check_label(source, "PUBLIC KEY")
            );

         BER_Decoder(ber)
            .start_cons(SEQUENCE)
            .decode(alg_id)
            .decode(key_bits, BIT_STRING)
            .verify_end()
         .end_cons();
         }

      if(key_bits.empty())
         throw Decoding_Error("X.509 public key decoding failed");

      return make_public_key(alg_id, key_bits);
      }
   catch(Decoding_Error)
      {
      throw Decoding_Error("X.509 public key decoding failed");
      }
   }

/*
* Extract a public key and return it
*/
Public_Key* load_key(const std::string& fsname)
   {
   DataSource_Stream source(fsname, true);
   return X509::load_key(source);
   }

/*
* Extract a public key and return it
*/
Public_Key* load_key(const std::vector<byte>& mem)
   {
   DataSource_Memory source(mem);
   return X509::load_key(source);
   }

/*
* Make a copy of this public key
*/
Public_Key* copy_key(const Public_Key& key)
   {
   DataSource_Memory source(PEM_encode(key));
   return X509::load_key(source);
   }

}

}
/*
* HMAC_RNG
* (C) 2008-2009,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

namespace {

void hmac_prf(MessageAuthenticationCode& prf,
              secure_vector<byte>& K,
              u32bit& counter,
              const std::string& label)
   {
   typedef std::chrono::high_resolution_clock clock;

   auto timestamp = clock::now().time_since_epoch().count();

   prf.update(K);
   prf.update(label);
   prf.update_be(timestamp);
   prf.update_be(counter);
   prf.final(&K[0]);

   ++counter;
   }

}

/*
* HMAC_RNG Constructor
*/
HMAC_RNG::HMAC_RNG(MessageAuthenticationCode* extractor,
                   MessageAuthenticationCode* prf) :
   m_extractor(extractor), m_prf(prf)
   {
   if(!m_prf->valid_keylength(m_extractor->output_length()) ||
      !m_extractor->valid_keylength(m_prf->output_length()))
      throw Invalid_Argument("HMAC_RNG: Bad algo combination " +
                             m_extractor->name() + " and " +
                             m_prf->name());

   // First PRF inputs are all zero, as specified in section 2
   m_K.resize(m_prf->output_length());

   /*
   Normally we want to feedback PRF outputs to the extractor function
   to ensure a single bad poll does not reduce entropy. Thus in reseed
   we'll want to invoke the PRF before we reset the PRF key, but until
   the first reseed the PRF is unkeyed. Rather than trying to keep
   track of this, just set the initial PRF key to constant zero.
   Since all PRF inputs in the first reseed are constants, this
   amounts to suffixing the seed in the first poll with a fixed
   constant string.

   The PRF key will not be used to generate outputs until after reseed
   sets m_seeded to true.
   */
   secure_vector<byte> prf_key(m_extractor->output_length());
   m_prf->set_key(prf_key);

   /*
   Use PRF("Botan HMAC_RNG XTS") as the intitial XTS key.

   This will be used during the first extraction sequence; XTS values
   after this one are generated using the PRF.

   If I understand the E-t-E paper correctly (specifically Section 4),
   using this fixed extractor key is safe to do.
   */
   m_extractor->set_key(prf->process("Botan HMAC_RNG XTS"));
   }

/*
* Generate a buffer of random bytes
*/
void HMAC_RNG::randomize(byte out[], size_t length)
   {
   if(!is_seeded())
      {
      reseed(256);
      if(!is_seeded())
         throw PRNG_Unseeded(name());
      }

   const size_t max_per_prf_iter = m_prf->output_length() / 2;

   /*
    HMAC KDF as described in E-t-E, using a CTXinfo of "rng"
   */
   while(length)
      {
      hmac_prf(*m_prf, m_K, m_counter, "rng");

      const size_t copied = std::min<size_t>(length, max_per_prf_iter);

      copy_mem(out, &m_K[0], copied);
      out += copied;
      length -= copied;

      m_output_since_reseed += copied;

      if(m_output_since_reseed >= BOTAN_RNG_MAX_OUTPUT_BEFORE_RESEED)
         reseed(BOTAN_RNG_RESEED_POLL_BITS);
      }
   }

/*
* Poll for entropy and reset the internal keys
*/
void HMAC_RNG::reseed(size_t poll_bits)
   {
   /*
   Using the terminology of E-t-E, XTR is the MAC function (normally
   HMAC) seeded with XTS (below) and we form SKM, the key material, by
   polling as many sources as we think needed to reach our polling
   goal. We then also include feedback of the current PRK so that
   a bad poll doesn't wipe us out.
   */

   double bits_collected = 0;

   Entropy_Accumulator accum(
      [&](const byte in[], size_t in_len, double entropy_estimate)
      {
      m_extractor->update(in, in_len);
      bits_collected += entropy_estimate;
      return (bits_collected >= poll_bits);
      });

   global_state().poll_available_sources(accum);

   /*
   * It is necessary to feed forward poll data. Otherwise, a good poll
   * (collecting a large amount of conditional entropy) followed by a
   * bad one (collecting little) would be unsafe. Do this by
   * generating new PRF outputs using the previous key and feeding
   * them into the extractor function.
   *
   * Cycle the RNG once (CTXinfo="rng"), then generate a new PRF
   * output using the CTXinfo "reseed". Provide these values as input
   * to the extractor function.
   */
   hmac_prf(*m_prf, m_K, m_counter, "rng");
   m_extractor->update(m_K); // K is the CTXinfo=rng PRF output

   hmac_prf(*m_prf, m_K, m_counter, "reseed");
   m_extractor->update(m_K); // K is the CTXinfo=reseed PRF output

   /* Now derive the new PRK using everything that has been fed into
      the extractor, and set the PRF key to that */
   m_prf->set_key(m_extractor->final());

   // Now generate a new PRF output to use as the XTS extractor salt
   hmac_prf(*m_prf, m_K, m_counter, "xts");
   m_extractor->set_key(m_K);

   // Reset state
   zeroise(m_K);
   m_counter = 0;

   m_collected_entropy_estimate =
      std::min<size_t>(m_collected_entropy_estimate + bits_collected,
                       m_extractor->output_length() * 8);

   m_output_since_reseed = 0;
   }

bool HMAC_RNG::is_seeded() const
   {
   return (m_collected_entropy_estimate >= 256);
   }

/*
* Add user-supplied entropy to the extractor input
*/
void HMAC_RNG::add_entropy(const byte input[], size_t length)
   {
   m_extractor->update(input, length);
   reseed(BOTAN_RNG_RESEED_POLL_BITS);
   }

/*
* Clear memory of sensitive data
*/
void HMAC_RNG::clear()
   {
   m_collected_entropy_estimate = 0;
   m_extractor->clear();
   m_prf->clear();
   zeroise(m_K);
   m_counter = 0;
   }

/*
* Return the name of this type
*/
std::string HMAC_RNG::name() const
   {
   return "HMAC_RNG(" + m_extractor->name() + "," + m_prf->name() + ")";
   }

}
/*
* Random Number Generator Base
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

RandomNumberGenerator* RandomNumberGenerator::make_rng()
   {
   return make_rng(global_state().algorithm_factory()).release();
   }

/*
* Create and seed a new RNG object
*/
std::unique_ptr<RandomNumberGenerator> RandomNumberGenerator::make_rng(Algorithm_Factory& af)
   {
   std::unique_ptr<RandomNumberGenerator> rng(
      new HMAC_RNG(af.make_mac("HMAC(SHA-512)"),
                   af.make_mac("HMAC(SHA-256)"))
      );

   rng->reseed(256);

   return rng;
   }

}
/*
* Counter mode
* (C) 1999-2011,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

CTR_BE::CTR_BE(BlockCipher* ciph) :
   m_cipher(ciph),
   m_counter(256 * m_cipher->block_size()),
   m_pad(m_counter.size()),
   m_pad_pos(0)
   {
   }

void CTR_BE::clear()
   {
   m_cipher->clear();
   zeroise(m_pad);
   zeroise(m_counter);
   m_pad_pos = 0;
   }

void CTR_BE::key_schedule(const byte key[], size_t key_len)
   {
   m_cipher->set_key(key, key_len);

   // Set a default all-zeros IV
   set_iv(nullptr, 0);
   }

std::string CTR_BE::name() const
   {
   return ("CTR-BE(" + m_cipher->name() + ")");
   }

void CTR_BE::cipher(const byte in[], byte out[], size_t length)
   {
   while(length >= m_pad.size() - m_pad_pos)
      {
      xor_buf(out, in, &m_pad[m_pad_pos], m_pad.size() - m_pad_pos);
      length -= (m_pad.size() - m_pad_pos);
      in += (m_pad.size() - m_pad_pos);
      out += (m_pad.size() - m_pad_pos);
      increment_counter();
      }
   xor_buf(out, in, &m_pad[m_pad_pos], length);
   m_pad_pos += length;
   }

void CTR_BE::set_iv(const byte iv[], size_t iv_len)
   {
   if(!valid_iv_length(iv_len))
      throw Invalid_IV_Length(name(), iv_len);

   const size_t bs = m_cipher->block_size();

   zeroise(m_counter);

   buffer_insert(m_counter, 0, iv, iv_len);

   // Set m_counter blocks to IV, IV + 1, ... IV + 255
   for(size_t i = 1; i != 256; ++i)
      {
      buffer_insert(m_counter, i*bs, &m_counter[(i-1)*bs], bs);

      for(size_t j = 0; j != bs; ++j)
         if(++m_counter[i*bs + (bs - 1 - j)])
            break;
      }

   m_cipher->encrypt_n(&m_counter[0], &m_pad[0], 256);
   m_pad_pos = 0;
   }

/*
* Increment the counter and update the buffer
*/
void CTR_BE::increment_counter()
   {
   const size_t bs = m_cipher->block_size();

   /*
   * Each counter value always needs to be incremented by 256,
   * so we don't touch the lowest byte and instead treat it as
   * an increment of one starting with the next byte.
   */
   for(size_t i = 0; i != 256; ++i)
      {
      for(size_t j = 1; j != bs; ++j)
         if(++m_counter[i*bs + (bs - 1 - j)])
            break;
      }

   m_cipher->encrypt_n(&m_counter[0], &m_pad[0], 256);
   m_pad_pos = 0;
   }

}
/*
* Stream Cipher
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

void StreamCipher::set_iv(const byte[], size_t iv_len)
   {
   if(iv_len)
      throw Invalid_Argument("The stream cipher " + name() +
                             " does not support resyncronization");
   }

bool StreamCipher::valid_iv_length(size_t iv_len) const
   {
   return (iv_len == 0);
   }

}
/*
* Runtime assertion checking
* (C) 2010,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <sstream>

namespace Botan {

void assertion_failure(const char* expr_str,
                       const char* assertion_made,
                       const char* func,
                       const char* file,
                       int line)
   {
   std::ostringstream format;

   format << "False assertion ";

   if(assertion_made && assertion_made[0] != 0)
      format << "'" << assertion_made << "' (expression " << expr_str << ") ";
   else
      format << expr_str << " ";

   if(func)
      format << "in " << func << " ";

   format << "@" << file << ":" << line;

   throw std::runtime_error(format.str());
   }

}
/*
* Calendar Functions
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <ctime>

namespace Botan {

namespace {

std::tm do_gmtime(std::time_t time_val)
   {
   std::tm tm;

#if defined(BOTAN_TARGET_OS_HAS_GMTIME_S)
   gmtime_s(&tm, &time_val); // Windows
#elif defined(BOTAN_TARGET_OS_HAS_GMTIME_R)
   gmtime_r(&time_val, &tm); // Unix/SUSv2
#else
   std::tm* tm_p = std::gmtime(&time_val);
   if (tm_p == 0)
      throw Encoding_Error("time_t_to_tm could not convert");
   tm = *tm_p;
#endif

   return tm;
   }

}

/*
* Convert a time_point to a calendar_point
*/
calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point)
   {
   std::tm tm = do_gmtime(std::chrono::system_clock::to_time_t(time_point));

   return calendar_point(tm.tm_year + 1900,
                         tm.tm_mon + 1,
                         tm.tm_mday,
                         tm.tm_hour,
                         tm.tm_min,
                         tm.tm_sec);
   }

}
/*
* Character Set Handling
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <cctype>

namespace Botan {

namespace Charset {

namespace {

/*
* Convert from UCS-2 to ISO 8859-1
*/
std::string ucs2_to_latin1(const std::string& ucs2)
   {
   if(ucs2.size() % 2 == 1)
      throw Decoding_Error("UCS-2 string has an odd number of bytes");

   std::string latin1;

   for(size_t i = 0; i != ucs2.size(); i += 2)
      {
      const byte c1 = ucs2[i];
      const byte c2 = ucs2[i+1];

      if(c1 != 0)
         throw Decoding_Error("UCS-2 has non-Latin1 characters");

      latin1 += static_cast<char>(c2);
      }

   return latin1;
   }

/*
* Convert from UTF-8 to ISO 8859-1
*/
std::string utf8_to_latin1(const std::string& utf8)
   {
   std::string iso8859;

   size_t position = 0;
   while(position != utf8.size())
      {
      const byte c1 = static_cast<byte>(utf8[position++]);

      if(c1 <= 0x7F)
         iso8859 += static_cast<char>(c1);
      else if(c1 >= 0xC0 && c1 <= 0xC7)
         {
         if(position == utf8.size())
            throw Decoding_Error("UTF-8: sequence truncated");

         const byte c2 = static_cast<byte>(utf8[position++]);
         const byte iso_char = ((c1 & 0x07) << 6) | (c2 & 0x3F);

         if(iso_char <= 0x7F)
            throw Decoding_Error("UTF-8: sequence longer than needed");

         iso8859 += static_cast<char>(iso_char);
         }
      else
         throw Decoding_Error("UTF-8: Unicode chars not in Latin1 used");
      }

   return iso8859;
   }

/*
* Convert from ISO 8859-1 to UTF-8
*/
std::string latin1_to_utf8(const std::string& iso8859)
   {
   std::string utf8;
   for(size_t i = 0; i != iso8859.size(); ++i)
      {
      const byte c = static_cast<byte>(iso8859[i]);

      if(c <= 0x7F)
         utf8 += static_cast<char>(c);
      else
         {
         utf8 += static_cast<char>((0xC0 | (c >> 6)));
         utf8 += static_cast<char>((0x80 | (c & 0x3F)));
         }
      }
   return utf8;
   }

}

/*
* Perform character set transcoding
*/
std::string transcode(const std::string& str,
                      Character_Set to, Character_Set from)
   {
   if(to == LOCAL_CHARSET)
      to = LATIN1_CHARSET;
   if(from == LOCAL_CHARSET)
      from = LATIN1_CHARSET;

   if(to == from)
      return str;

   if(from == LATIN1_CHARSET && to == UTF8_CHARSET)
      return latin1_to_utf8(str);
   if(from == UTF8_CHARSET && to == LATIN1_CHARSET)
      return utf8_to_latin1(str);
   if(from == UCS2_CHARSET && to == LATIN1_CHARSET)
      return ucs2_to_latin1(str);

   throw Invalid_Argument("Unknown transcoding operation from " +
                          std::to_string(from) + " to " + std::to_string(to));
   }

/*
* Check if a character represents a digit
*/
bool is_digit(char c)
   {
   if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
      c == '5' || c == '6' || c == '7' || c == '8' || c == '9')
      return true;
   return false;
   }

/*
* Check if a character represents whitespace
*/
bool is_space(char c)
   {
   if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
      return true;
   return false;
   }

/*
* Convert a character to a digit
*/
byte char2digit(char c)
   {
   switch(c)
      {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      }

   throw Invalid_Argument("char2digit: Input is not a digit character");
   }

/*
* Convert a digit to a character
*/
char digit2char(byte b)
   {
   switch(b)
      {
      case 0: return '0';
      case 1: return '1';
      case 2: return '2';
      case 3: return '3';
      case 4: return '4';
      case 5: return '5';
      case 6: return '6';
      case 7: return '7';
      case 8: return '8';
      case 9: return '9';
      }

   throw Invalid_Argument("digit2char: Input is not a digit");
   }

/*
* Case-insensitive character comparison
*/
bool caseless_cmp(char a, char b)
   {
   return (std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b)));
   }

}

}
/*
* Runtime CPU detection
* (C) 2009-2010,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

#if defined(BOTAN_TARGET_OS_IS_DARWIN)
  #include <sys/sysctl.h>
#endif

#if defined(BOTAN_TARGET_OS_IS_OPENBSD)
  #include <sys/param.h>
  #include <machine/cpu.h>
#endif

#endif

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)

#include <intrin.h>

#define X86_CPUID(type, out) do { __cpuid((int*)out, type); } while(0)
#define X86_CPUID_SUBLEVEL(type, level, out) do { __cpuidex((int*)out, type, level); } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_INTEL)

#include <ia32intrin.h>

#define X86_CPUID(type, out) do { __cpuid(out, type); } while(0)
#define X86_CPUID_SUBLEVEL(type, level, out) do { __cpuidex((int*)out, type, level); } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_X86_64) && BOTAN_USE_GCC_INLINE_ASM

#define X86_CPUID(type, out)                                                    \
   asm("cpuid\n\t" : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3]) \
       : "0" (type))

#define X86_CPUID_SUBLEVEL(type, level, out)                                    \
   asm("cpuid\n\t" : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3]) \
       : "0" (type), "2" (level))

#elif defined(BOTAN_BUILD_COMPILER_IS_GCC)

#include <cpuid.h>

#define X86_CPUID(type, out) do { __get_cpuid(type, out, out+1, out+2, out+3); } while(0)

#define X86_CPUID_SUBLEVEL(type, level, out) \
   do { __cpuid_count(type, level, out[0], out[1], out[2], out[3]); } while(0)

#else

#warning "No way of calling cpuid for this compiler"

#define X86_CPUID(type, out) do { clear_mem(out, 4); } while(0)
#define X86_CPUID_SUBLEVEL(type, level, out) do { clear_mem(out, 4); } while(0)

#endif

#endif

namespace Botan {

u64bit CPUID::m_x86_processor_flags[2] = { 0, 0 };
size_t CPUID::m_cache_line_size = 0;
bool CPUID::m_altivec_capable = false;

namespace {

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

bool altivec_check_sysctl()
   {
#if defined(BOTAN_TARGET_OS_IS_DARWIN) || defined(BOTAN_TARGET_OS_IS_OPENBSD)

#if defined(BOTAN_TARGET_OS_IS_OPENBSD)
   int sels[2] = { CTL_MACHDEP, CPU_ALTIVEC };
#else
   // From Apple's docs
   int sels[2] = { CTL_HW, HW_VECTORUNIT };
#endif
   int vector_type = 0;
   size_t length = sizeof(vector_type);
   int error = sysctl(sels, 2, &vector_type, &length, NULL, 0);

   if(error == 0 && vector_type > 0)
      return true;
#endif

   return false;
   }

bool altivec_check_pvr_emul()
   {
   bool altivec_capable = false;

#if defined(BOTAN_TARGET_OS_IS_LINUX) || defined(BOTAN_TARGET_OS_IS_NETBSD)

   /*
   On PowerPC, MSR 287 is PVR, the Processor Version Number
   Normally it is only accessible to ring 0, but Linux and NetBSD
   (others, too, maybe?) will trap and emulate it for us.

   PVR identifiers for various AltiVec enabled CPUs. Taken from
   PearPC and Linux sources, mostly.
   */

   const u16bit PVR_G4_7400  = 0x000C;
   const u16bit PVR_G5_970   = 0x0039;
   const u16bit PVR_G5_970FX = 0x003C;
   const u16bit PVR_G5_970MP = 0x0044;
   const u16bit PVR_G5_970GX = 0x0045;
   const u16bit PVR_POWER6   = 0x003E;
   const u16bit PVR_POWER7   = 0x003F;
   const u16bit PVR_CELL_PPU = 0x0070;

   // Motorola produced G4s with PVR 0x800[0123C] (at least)
   const u16bit PVR_G4_74xx_24  = 0x800;

   u32bit pvr = 0;

   asm volatile("mfspr %0, 287" : "=r" (pvr));

   // Top 16 bit suffice to identify model
   pvr >>= 16;

   altivec_capable |= (pvr == PVR_G4_7400);
   altivec_capable |= ((pvr >> 4) == PVR_G4_74xx_24);
   altivec_capable |= (pvr == PVR_G5_970);
   altivec_capable |= (pvr == PVR_G5_970FX);
   altivec_capable |= (pvr == PVR_G5_970MP);
   altivec_capable |= (pvr == PVR_G5_970GX);
   altivec_capable |= (pvr == PVR_POWER6);
   altivec_capable |= (pvr == PVR_POWER7);
   altivec_capable |= (pvr == PVR_CELL_PPU);
#endif

   return altivec_capable;
   }

#endif

}

void CPUID::print(std::ostream& o)
   {
   o << "CPUID flags: ";

#define CPUID_PRINT(flag) do { if(has_##flag()) o << #flag << " "; } while(0)
   CPUID_PRINT(sse2);
   CPUID_PRINT(ssse3);
   CPUID_PRINT(sse41);
   CPUID_PRINT(sse42);
   CPUID_PRINT(avx2);
   CPUID_PRINT(avx512f);
   CPUID_PRINT(altivec);

   CPUID_PRINT(rdtsc);
   CPUID_PRINT(bmi2);
   CPUID_PRINT(clmul);
   CPUID_PRINT(aes_ni);
   CPUID_PRINT(rdrand);
   CPUID_PRINT(rdseed);
   CPUID_PRINT(intel_sha);
   CPUID_PRINT(adx);
#undef CPUID_PRINT
   o << "\n";
   }

void CPUID::initialize()
   {
#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
      if(altivec_check_sysctl() || altivec_check_pvr_emul())
         m_altivec_capable = true;
#endif

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   const u32bit INTEL_CPUID[3] = { 0x756E6547, 0x6C65746E, 0x49656E69 };
   const u32bit AMD_CPUID[3] = { 0x68747541, 0x444D4163, 0x69746E65 };

   u32bit cpuid[4] = { 0 };
   X86_CPUID(0, cpuid);

   const u32bit max_supported_sublevel = cpuid[0];

   if(max_supported_sublevel == 0)
      return;

   const bool is_intel = same_mem(cpuid + 1, INTEL_CPUID, 3);
   const bool is_amd = same_mem(cpuid + 1, AMD_CPUID, 3);

   X86_CPUID(1, cpuid);

   m_x86_processor_flags[0] = (static_cast<u64bit>(cpuid[2]) << 32) | cpuid[3];

   if(is_intel)
      m_cache_line_size = 8 * get_byte(2, cpuid[1]);

   if(max_supported_sublevel >= 7)
      {
      clear_mem(cpuid, 4);
      X86_CPUID_SUBLEVEL(7, 0, cpuid);
      m_x86_processor_flags[1] = (static_cast<u64bit>(cpuid[2]) << 32) | cpuid[1];
      }

   if(is_amd)
      {
      X86_CPUID(0x80000005, cpuid);
      m_cache_line_size = get_byte(3, cpuid[2]);
      }

#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)
   /*
   * If we don't have access to CPUID, we can still safely assume that
   * any x86-64 processor has SSE2 and RDTSC
   */
   if(m_x86_processor_flags[0] == 0)
      m_x86_processor_flags[0] = (1 << CPUID_SSE2_BIT) | (1 << CPUID_RDTSC_BIT);
#endif
   }

}
/*
* Various string utils and parsing functions
* (C) 1999-2007,2013,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

u32bit to_u32bit(const std::string& str)
   {
   return std::stoul(str, nullptr);
   }

/*
* Convert a string into a time duration
*/
u32bit timespec_to_u32bit(const std::string& timespec)
   {
   if(timespec == "")
      return 0;

   const char suffix = timespec[timespec.size()-1];
   std::string value = timespec.substr(0, timespec.size()-1);

   u32bit scale = 1;

   if(Charset::is_digit(suffix))
      value += suffix;
   else if(suffix == 's')
      scale = 1;
   else if(suffix == 'm')
      scale = 60;
   else if(suffix == 'h')
      scale = 60 * 60;
   else if(suffix == 'd')
      scale = 24 * 60 * 60;
   else if(suffix == 'y')
      scale = 365 * 24 * 60 * 60;
   else
      throw Decoding_Error("timespec_to_u32bit: Bad input " + timespec);

   return scale * to_u32bit(value);
   }

/*
* Parse a SCAN-style algorithm name
*/
std::vector<std::string> parse_algorithm_name(const std::string& namex)
   {
   if(namex.find('(') == std::string::npos &&
      namex.find(')') == std::string::npos)
      return std::vector<std::string>(1, namex);

   std::string name = namex, substring;
   std::vector<std::string> elems;
   size_t level = 0;

   elems.push_back(name.substr(0, name.find('(')));
   name = name.substr(name.find('('));

   for(auto i = name.begin(); i != name.end(); ++i)
      {
      char c = *i;

      if(c == '(')
         ++level;
      if(c == ')')
         {
         if(level == 1 && i == name.end() - 1)
            {
            if(elems.size() == 1)
               elems.push_back(substring.substr(1));
            else
               elems.push_back(substring);
            return elems;
            }

         if(level == 0 || (level == 1 && i != name.end() - 1))
            throw Invalid_Algorithm_Name(namex);
         --level;
         }

      if(c == ',' && level == 1)
         {
         if(elems.size() == 1)
            elems.push_back(substring.substr(1));
         else
            elems.push_back(substring);
         substring.clear();
         }
      else
         substring += c;
      }

   if(substring != "")
      throw Invalid_Algorithm_Name(namex);

   return elems;
   }

std::vector<std::string> split_on(const std::string& str, char delim)
   {
   return split_on_pred(str, [delim](char c) { return c == delim; });
   }

std::vector<std::string> split_on_pred(const std::string& str,
                                       std::function<bool (char)> pred)
   {
   std::vector<std::string> elems;
   if(str == "") return elems;

   std::string substr;
   for(auto i = str.begin(); i != str.end(); ++i)
      {
      if(pred(*i))
         {
         if(substr != "")
            elems.push_back(substr);
         substr.clear();
         }
      else
         substr += *i;
      }

   if(substr == "")
      throw Invalid_Argument("Unable to split string: " + str);
   elems.push_back(substr);

   return elems;
   }

/*
* Join a string
*/
std::string string_join(const std::vector<std::string>& strs, char delim)
   {
   std::string out = "";

   for(size_t i = 0; i != strs.size(); ++i)
      {
      if(i != 0)
         out += delim;
      out += strs[i];
      }

   return out;
   }

/*
* Parse an ASN.1 OID string
*/
std::vector<u32bit> parse_asn1_oid(const std::string& oid)
   {
   std::string substring;
   std::vector<u32bit> oid_elems;

   for(auto i = oid.begin(); i != oid.end(); ++i)
      {
      char c = *i;

      if(c == '.')
         {
         if(substring == "")
            throw Invalid_OID(oid);
         oid_elems.push_back(to_u32bit(substring));
         substring.clear();
         }
      else
         substring += c;
      }

   if(substring == "")
      throw Invalid_OID(oid);
   oid_elems.push_back(to_u32bit(substring));

   if(oid_elems.size() < 2)
      throw Invalid_OID(oid);

   return oid_elems;
   }

/*
* X.500 String Comparison
*/
bool x500_name_cmp(const std::string& name1, const std::string& name2)
   {
   auto p1 = name1.begin();
   auto p2 = name2.begin();

   while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
   while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

   while(p1 != name1.end() && p2 != name2.end())
      {
      if(Charset::is_space(*p1))
         {
         if(!Charset::is_space(*p2))
            return false;

         while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
         while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

         if(p1 == name1.end() && p2 == name2.end())
            return true;
         }

      if(!Charset::caseless_cmp(*p1, *p2))
         return false;
      ++p1;
      ++p2;
      }

   while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
   while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

   if((p1 != name1.end()) || (p2 != name2.end()))
      return false;
   return true;
   }

/*
* Convert a decimal-dotted string to binary IP
*/
u32bit string_to_ipv4(const std::string& str)
   {
   std::vector<std::string> parts = split_on(str, '.');

   if(parts.size() != 4)
      throw Decoding_Error("Invalid IP string " + str);

   u32bit ip = 0;

   for(auto part = parts.begin(); part != parts.end(); ++part)
      {
      u32bit octet = to_u32bit(*part);

      if(octet > 255)
         throw Decoding_Error("Invalid IP string " + str);

      ip = (ip << 8) | (octet & 0xFF);
      }

   return ip;
   }

/*
* Convert an IP address to decimal-dotted string
*/
std::string ipv4_to_string(u32bit ip)
   {
   std::string str;

   for(size_t i = 0; i != sizeof(ip); ++i)
      {
      if(i)
         str += ".";
      str += std::to_string(get_byte(i, ip));
      }

   return str;
   }

std::string erase_chars(const std::string& str, const std::set<char>& chars)
   {
   std::string out;

   for(auto c: str)
      if(chars.count(c) == 0)
         out += c;

   return out;
   }

std::string replace_chars(const std::string& str,
                          const std::set<char>& chars,
                          char to_char)
   {
   std::string out = str;

   for(size_t i = 0; i != out.size(); ++i)
      if(chars.count(out[i]))
         out[i] = to_char;

   return out;
   }

std::string replace_char(const std::string& str, char from_char, char to_char)
   {
   std::string out = str;

   for(size_t i = 0; i != out.size(); ++i)
      if(out[i] == from_char)
         out[i] = to_char;

   return out;
   }

}
/*
* Simple config/test file reader
* (C) 2013,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <ctype.h>

namespace Botan {

void lex_cfg(std::istream& is,
             std::function<void (std::string)> cb)
   {
   while(is.good())
      {
      std::string s;

      std::getline(is, s);

      while(is.good() && s.back() == '\\')
         {
         while(s.size() && (s.back() == '\\' || s.back() == '\n'))
            s.resize(s.size()-1);

         std::string x;
         std::getline(is, x);

         size_t i = 0;

         while(i < x.size() && (::isspace(x[i])))
            ++i;

         s += x.substr(i);
         }

      auto comment = s.find('#');
      if(comment)
         s = s.substr(0, comment);

      if(s.empty())
         continue;

      auto parts = split_on_pred(s, [](char c) { return ::isspace(c); });

      for(auto& p : parts)
         {
         if(p.empty())
            continue;

         auto eq = p.find("=");

         if(eq == std::string::npos || p.size() < 2)
            {
            cb(p);
            }
         else if(eq == 0)
            {
            cb("=");
            cb(p.substr(1, std::string::npos));
            }
         else if(eq == p.size() - 1)
            {
            cb(p.substr(0, p.size() - 1));
            cb("=");
            }
         else if(eq != std::string::npos)
            {
            cb(p.substr(0, eq));
            cb("=");
            cb(p.substr(eq + 1, std::string::npos));
            }
         }
      }
   }

void lex_cfg_w_headers(std::istream& is,
                       std::function<void (std::string)> cb,
                       std::function<void (std::string)> hdr_cb)
   {
   auto intercept = [cb,hdr_cb](const std::string& s)
      {
      if(s[0] == '[' && s[s.length()-1] == ']')
         hdr_cb(s.substr(1, s.length()-2));
      else
         cb(s);
      };

   lex_cfg(is, intercept);
   }

std::map<std::string, std::map<std::string, std::string>>
   parse_cfg(std::istream& is)
   {
   std::string header = "default";
   std::map<std::string, std::map<std::string, std::string>> vals;
   std::string key;

   auto header_cb = [&header](const std::string i) { header = i; };
   auto cb = [&header,&key,&vals](const std::string s)
      {
      if(s == "=")
         {
         BOTAN_ASSERT(!key.empty(), "Valid assignment in config");
         }
      else if(key.empty())
         key = s;
      else
         {
         vals[header][key] = s;
         key = "";
         }
      };

   lex_cfg_w_headers(is, cb, header_cb);

   return vals;
   }

}
/*
* Semaphore
* by Pierre Gaston (http://p9as.blogspot.com/2012/06/c11-semaphores.html)
* modified by Joel Low for Botan
*
*/


namespace Botan {

void Semaphore::release(size_t n)
   {
   for(size_t i = 0; i != n; ++i)
      {
      std::lock_guard<std::mutex> lock(m_mutex);

      ++m_value;

      if(m_value <= 0)
         {
         ++m_wakeups;
         m_cond.notify_one();
         }
      }
   }

void Semaphore::acquire()
   {
   std::unique_lock<std::mutex> lock(m_mutex);
   --m_value;
   if(m_value < 0)
      {
      m_cond.wait(lock, [this] { return m_wakeups > 0; });
      --m_wakeups;
      }
   }

}
/*
* Version Information
* (C) 1999-2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

/*
  These are intentionally compiled rather than inlined, so an
  application running against a shared library can test the true
  version they are running against.
*/

/*
* Return the version as a string
*/
std::string version_string()
   {
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

   /*
   It is intentional that this string is a compile-time constant;
   it makes it much easier to find in binaries.
   */

   return "Botan " STR(BOTAN_VERSION_MAJOR) "."
                   STR(BOTAN_VERSION_MINOR) "."
                   STR(BOTAN_VERSION_PATCH) " ("
                   BOTAN_VERSION_RELEASE_TYPE
#if (BOTAN_VERSION_DATESTAMP != 0)
                   ", dated " STR(BOTAN_VERSION_DATESTAMP)
#endif
                   ", revision " BOTAN_VERSION_VC_REVISION
                   ", distribution " BOTAN_DISTRIBUTION_INFO ")";

#undef STR
#undef QUOTE
   }

u32bit version_datestamp() { return BOTAN_VERSION_DATESTAMP; }

/*
* Return parts of the version as integers
*/
u32bit version_major() { return BOTAN_VERSION_MAJOR; }
u32bit version_minor() { return BOTAN_VERSION_MINOR; }
u32bit version_patch() { return BOTAN_VERSION_PATCH; }

}
/*
* Zero Memory
* (C) 2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/


namespace Botan {

void zero_mem(void* ptr, size_t n)
   {
   volatile byte* p = reinterpret_cast<volatile byte*>(ptr);

   for(size_t i = 0; i != n; ++i)
      p[i] = 0;
   }

}
