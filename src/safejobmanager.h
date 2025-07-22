#ifndef SAVEJOBMANAGER_H
#define SAVEJOBMANAGER_H

#include <chrono>
#include "meminfo.h"

class SafeJobManager {
 public:
   using Clock = std::chrono::steady_clock;
 private:
  const int m_safeParallelism = 1;
  const int m_maxParallelism = 1;
  Clock::time_point m_startClock;
  int m_currentParalelism = 1;
  bool m_safeMode = true;
  MemInfo m_memInfo;
  const long m_memoryPerFlow = 1024 * 1024 * 1024;
  const long m_maxUsedMemory = 8 * m_memoryPerFlow;
  const long m_startUsedMemory = 4 * m_memoryPerFlow;

  static const Clock::duration m_TimeOut;
  static const Clock::duration m_safeModeTimeOut;

 public:
  SafeJobManager(int safeParallelism, int maxParallelism);
  int GetExtraParalelism(int currentParallelism);
private:
 long GetJobAmount(long currenMemoryPerFlow);
 void updateSafeStatus(int currentParalelism);
 void suggestUpdateJobCount(int currentJobs, long count);
};


#endif  // SAVEJOBMANAGER_H
