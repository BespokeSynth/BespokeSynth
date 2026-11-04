#include "Move-Display.h"

namespace abletonmove {

MoveDisplay *MoveDisplay::create()
{
#if BESPOKE_ABLETONMOVE_SUPPORT
  return new MoveDisplay;
#else
  return nullptr;
#endif
}

NBase::Result MoveDisplay::Init()
{
#if BESPOKE_ABLETONMOVE_SUPPORT
  return communicator_.Init(dataSource_);
#else
  return {""}; // unreachable unless this is nullptr
#endif
}

} // namespace abletonmove
