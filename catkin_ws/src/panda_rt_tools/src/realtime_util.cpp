// SPDX-License-Identifier: Apache-2.0
#include "panda_rt/realtime_util.h"

#include <pthread.h>
#include <sys/mman.h>

#include <cstring>
#include <cstdio>

namespace panda_rt {

bool setRealtime(int priority) {
  struct sched_param param;
  std::memset(&param, 0, sizeof(param));
  param.sched_priority = priority;
  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
    std::perror("pthread_setschedparam");
    return false;
  }
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    std::perror("mlockall");
    return false;
  }
  return true;
}

}  // namespace panda_rt
