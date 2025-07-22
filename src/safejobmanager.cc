
#include "safejobmanager.h"
#include <algorithm>
//#include <iostream>

const SafeJobManager::Clock::duration SafeJobManager::m_TimeOut =
    std::chrono::seconds(2);
const SafeJobManager::Clock::duration SafeJobManager::m_safeModeTimeOut =
    std::chrono::seconds(5);
namespace {

long GetMemoryPerJob(const MemInfo& memory, int maxParalelism,
                     int minParalelism) {
  return (memory.m_memAvailable + memory.m_memTotal) /
         (maxParalelism + minParalelism + 1);
}
constexpr int ParalelismToSaveMode = 2;
}

long SafeJobManager::GetJobAmount(long currenMemoryPerFlow) {
  auto UsedMemory = m_memInfo.GetUsedMemory();
  if (UsedMemory + m_memoryPerFlow >= m_maxUsedMemory ||
      m_memInfo.m_memFree <= currenMemoryPerFlow) {
    return 0;
  }

  auto jobsCount =
      std::min(m_maxUsedMemory - UsedMemory, m_memInfo.m_memAvailable) /
      m_memoryPerFlow;
  return jobsCount;
}

SafeJobManager::SafeJobManager(int safeParallelism, int maxParallelism)
    : m_safeParallelism{ safeParallelism }, m_maxParallelism{ maxParallelism },
      m_startClock{ Clock::now() }, m_currentParalelism{ m_safeParallelism },
      m_memoryPerFlow{ GetMemoryPerJob(m_memInfo, m_maxParallelism,
                                       m_safeParallelism) },
      m_maxUsedMemory{ m_memInfo.GetMaxAvailableMemory() },
      m_startUsedMemory{ m_memInfo.GetUsedMemory() } {
  // std::cout << "\nm_safeParallelism: " << m_safeParallelism
  //           << "\nm_maxParallelism: " << m_maxParallelism;
}

int SafeJobManager::GetExtraParalelism(int currentParallelism) {
  updateSafeStatus(currentParallelism);
  m_memInfo.ReadMemInfo();

  auto currentMemoryPerFlow = m_memoryPerFlow;
  if (currentParallelism) {
    auto usedMemoryByNinja = m_memInfo.GetUsedMemory() - m_startUsedMemory;
    if (usedMemoryByNinja > 0) {
      currentMemoryPerFlow = usedMemoryByNinja / currentParallelism;
    }
  }
  currentMemoryPerFlow = std::max(currentMemoryPerFlow, m_memoryPerFlow);

  auto extraParalelism = GetJobAmount(currentMemoryPerFlow);
  auto suggestedParalelism = extraParalelism;

  suggestUpdateJobCount(currentParallelism, extraParalelism);
  if (extraParalelism + currentParallelism > m_currentParalelism) {
    extraParalelism = m_currentParalelism - currentParallelism;
  }
  // std::cout << "\nMax Memory: " << m_maxUsedMemory
  //           << " usedMemory: " << m_memInfo.GetUsedMemory()
  //           << " FreeMemory: " << m_memInfo.m_memFree
  //           << " MemoryPerFlow: " << currentMemoryPerFlow
  //           << " suggestedParalelism: " << suggestedParalelism
  //           << " MaxJobs: " << m_currentParalelism
  //           << " extra paralelism: " << extraParalelism << "\n";
  // if (!m_safeMode && extraParalelism > 2) {
  //   std::cout << "\nExtra paralelism!!! > 2\n";
  // }
  return extraParalelism;
}

void SafeJobManager::updateSafeStatus(int currentParalelism) {
  if (currentParalelism < std::min(ParalelismToSaveMode, m_safeParallelism) && !m_safeMode) {
    m_currentParalelism = m_safeParallelism;
    m_safeMode = true;
    m_startClock = Clock::now();
    // std::cout << "\n Safe mode: ON";
  }
}

void SafeJobManager::suggestUpdateJobCount(int currentJobs, long count) {
  auto now = Clock::now();
  if( count < 2 )
  {
    m_startClock = now;
  }
  if (m_safeMode) {
    if (now - m_startClock > m_safeModeTimeOut) {
      m_safeMode = false;
      m_startClock = now;
      // std::cout << "\n Safe mode: OFF";
    }
    return;
  }
  if (!count && currentJobs < m_currentParalelism) {
    m_currentParalelism = currentJobs;
    // std::cout << "\n DOWNGRADE m_currentParalelism: " << m_currentParalelism;
    return;
  }

  if (count == 1) {
    m_safeMode = true;
    return;
  }
  if (count > 0 && now - m_startClock > m_TimeOut &&
      m_currentParalelism < m_maxParallelism) {
    ++m_currentParalelism;
    // std::cout << "\n UPGRADE m_currentParalelism: " << m_currentParalelism;
    m_startClock = now;
  }
}
