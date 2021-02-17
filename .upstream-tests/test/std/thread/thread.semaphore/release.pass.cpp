//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: pre-sm-70

// <cuda/std/semaphore>

#include <cuda/std/semaphore>

#include "test_macros.h"
#include "concurrent_agents.h"
#include "cuda_space_selector.h"

template<typename Semaphore,
    template<typename, typename> typename Selector,
    typename Initializer = constructor_initializer>
__host__ __device__
void test()
{
  Selector<Semaphore, Initializer> sel;
  SHARED Semaphore * s;
  s = sel.construct(2);

  NV_DISPATCH_TARGET(
      NV_IS_DEVICE, (
        if (threadIdx.x == 0) {
          s->release();
          s->acquire();
        }

        auto acquirer = LAMBDA (){
          s->acquire();
        };
        auto releaser = LAMBDA (){
          s->release(2);
        };

        concurrent_agents_launch(acquirer, releaser);

        if (threadIdx.x == 0) {
          s->acquire();
        }

      ),
      NV_IS_HOST, (
        s->release();
        s->acquire();

        auto acquirer = LAMBDA (){
          s->acquire();
        };
        auto releaser = LAMBDA (){
          s->release(2);
        };

        concurrent_agents_launch(acquirer, releaser);

        s->acquire();
      )
  )
}

int main(int, char**)
{
  NV_DISPATCH_TARGET(
    NV_IS_HOST, (
        cuda_thread_count = 2;

        test<cuda::std::counting_semaphore<>, local_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_block>, local_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_device>, local_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_system>, local_memory_selector>();
    NV_IS_DEVICE, (
        test<cuda::std::counting_semaphore<>, shared_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_block>, shared_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_device>, shared_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_system>, shared_memory_selector>();

        test<cuda::std::counting_semaphore<>, global_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_block>, global_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_device>, global_memory_selector>();
        test<cuda::counting_semaphore<cuda::thread_scope_system>, global_memory_selector>();
    )
  )

  return 0;
}
