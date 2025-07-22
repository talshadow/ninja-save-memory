
#include "meminfo.h"
#include <algorithm>
#include <string>
#include <array>
#include <fstream>

namespace {
constexpr auto MemoryParameterCount = sizeof(MemInfo) / sizeof(size_t);

size_t GetSupportedMemInfoIndex(const std::string& fieldName) {
  static std::array<size_t, MemoryParameterCount> HashesMap = {
    std::hash<std::string>{}("MemTotal"),
    std::hash<std::string>{}("MemFree"),
    std::hash<std::string>{}("MemAvailable"),
    std::hash<std::string>{}("Buffers"),
    std::hash<std::string>{}("Cached"),
    std::hash<std::string>{}("SwapTotal"),
    std::hash<std::string>{}("SwapFree")
  };
  auto it = std::find(HashesMap.begin(), HashesMap.end(),
                      std::hash<std::string>{}(fieldName));
  if (it != HashesMap.end())
    return std::distance(HashesMap.begin(), it);
  return MemoryParameterCount;
}

long GetValue(const std::string& data, const std::string& fieldName) {
  auto start_number = data.find_first_not_of(' ', fieldName.size() + 2);
  auto end_number = data.find_first_of(' ', start_number);
  char* endPoint = const_cast<char*>(data.data()) + end_number;
  return std::strtoul(data.data() + start_number, &endPoint, 10);
}

std::string GetFieldName(const std::string& data) {
  auto pos = data.find(':');
  return data.substr(0, pos);
}

}  // namespace

void MemInfo::ReadMemInfo() {
  std::ifstream fileMemInfo("/proc/meminfo");
  auto readCount = 0;
  if (fileMemInfo) {
    std::string line;
    while (std::getline(fileMemInfo, line) &&
           readCount < MemoryParameterCount) {
      auto fieldName = GetFieldName(line);
      auto index = GetSupportedMemInfoIndex(fieldName);
      if (index < MemoryParameterCount) {
        auto value = GetValue(line, fieldName);
        SetByIndex(index, value);
        ++readCount;
      }
    }
  }
}

void MemInfo::SetByIndex(size_t index, size_t value) {
  auto element = reinterpret_cast<long*>(this) + index;
  *element = value;
}

long MemInfo::GetUsedMemory() {
  return m_swapTotal - m_swapFree + m_memTotal - m_memAvailable;
}

long MemInfo::GetMaxAvailableMemory() {
  auto usedMemory = GetUsedMemory();
  auto maxMemory = usedMemory + m_memAvailable;
  // usedMemory + memory.m_memTotal - memory.m_cached - memory.m_buffers;
  if (maxMemory > (m_memTotal + m_swapTotal)) {
    maxMemory = m_memTotal + m_swapTotal - usedMemory;
  }
  return maxMemory;
}
