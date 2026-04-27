#include "Move-Display.h"

namespace abletonmove {

MoveDisplay *MoveDisplay::create()
{
  return new MoveDisplay;
}

NBase::Result MoveDisplay::Init()
{
  return communicator_.Init(dataSource_);
}

} // namespace abletonmove
