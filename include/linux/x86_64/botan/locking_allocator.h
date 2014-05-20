/*
* Mlock Allocator
* (C) 2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_MLOCK_ALLOCATOR_H__
#define BOTAN_MLOCK_ALLOCATOR_H__

#include <botan/types.h>
#include <vector>
#include <mutex>

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
