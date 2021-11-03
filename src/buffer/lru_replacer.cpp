#include "buffer/lru_replacer.h"

namespace MisakiDB {

bool LRUReplacer::exist(FrameIDType frameID) {
  return m_lruMap.find(frameID) != m_lruMap.end();
}

void LRUReplacer::remove(FrameIDType frameID) {
  /* frame-id-corresponding frame doesn't exist */
  if (!exist(frameID)) {
    return;
  }
  
  auto frameIter = m_lruMap[frameID];
  m_lruCache.erase(frameIter);
  m_lruMap.erase(frameID);
}

bool LRUReplacer::victim(FrameIDType *frameID) {
  std::lock_guard<std::mutex> lck(m_lruLatch);
  /* no page can be victim */
  if (m_lruCache.empty()) {
    return false;
  }
  
  if (frameID != nullptr) {
    *frameID = m_lruCache.back();
  }
  remove(m_lruCache.back());
  return true;
}

void LRUReplacer::pin(FrameIDType frameID) {
  std::lock_guard<std::mutex> lck(m_lruLatch);
  remove(frameID);
}

void LRUReplacer::unpin(FrameIDType frameID) {
  std::lock_guard<std::mutex> lck(m_lruLatch);
  /* repeat unpin have no effect */
  if (exist(frameID)) {
    return;
  }
  
  m_lruCache.push_front(frameID);
  m_lruMap[frameID] = m_lruCache.begin();
}

size_t LRUReplacer::size() {
  std::unique_lock<std::mutex> lck(m_lruLatch);
  return m_lruCache.size();
}
}

