#pragma once

#include "globals.h"

namespace MisakiDB {
/**
 * Replacer is an abstract class that tracks page usage.
 */
class Replacer {
public:
  Replacer() = default;
  virtual ~Replacer() = default;
  
  /**
   * Remove the victim frame as defined by the replacement policy.
   * @param[out] frameID id of frame that was removed, nullptr if no victim was found
   * @return true if a victim frame was found, false otherwise
   */
  virtual bool victim(FrameIDType *frameID) = 0;
  
  /**
   * Pins a frame, indicating that it should not be victimized until it is unpinned.
   * @param frameID the id of the frame to pin
   */
  virtual void pin(FrameIDType frameID) = 0;
  
  /**
   * Unpins a frame, indicating that it can now be victimized.
   * @param frameID the id of the frame to unpin
   */
  virtual void unpin(FrameIDType frameID) = 0;
  
  /** @return the number of elements in the replacer that can be victimized */
  virtual size_t size() = 0;
};
}

