#ifndef MEMINFO_H
#define MEMINFO_H
#include <cstddef>

struct MemInfo {
  long m_memTotal = 0;
  long m_memFree = 0;
  long m_memAvailable = 0;
  long m_buffers = 0;
  long m_cached = 0;
  long m_swapTotal = 0;
  long m_swapFree = 0;
  MemInfo() { ReadMemInfo(); }
  void ReadMemInfo();

  long GetUsedMemory();
  long GetMaxAvailableMemory();

 private:
  void SetByIndex(size_t index, size_t value);
};

#endif  // MEMINFO_H
