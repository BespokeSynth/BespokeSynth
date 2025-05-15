#include "Push2-Display.h"

namespace ableton {

Push2Display *Push2Display::create()
{
#if BESPOKE_PUSH2_SUPPORT
  return new Push2Display;
#else
  return nullptr;
#endif
}

NBase::Result Push2Display::Init(DeviceType deviceType)
{
#if BESPOKE_PUSH2_SUPPORT
  return communicator_.Init(dataSource_, deviceType);
#else
  return {""}; // unreachable unless this is nullptr
#endif
}

} // namespace ableton
