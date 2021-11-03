#pragma once

#include "replacer.h"
#include "globals.h"

namespace MisakiDB {
/**
 * LRUReplacer implements the lru replacement policy, which approximates the Least Recently Used policy.
 */
class LRUReplacer : public Replacer {
public:
  LRUReplacer() = default;
  ~LRUReplacer() override = default;
  
  bool victim(FrameIDType *frameID) override;
  void pin(FrameIDType frameID) override;
  void unpin(FrameIDType frameID) override;
  size_t size() override;

private:
  bool exist(FrameIDType frameID);
  void remove(FrameIDType frameID);
  
  std::mutex m_lruLatch;
  std::list<FrameIDType> m_lruCache;
  std::map<FrameIDType, std::list<FrameIDType>::iterator> m_lruMap;
};
}
