// This is the main DLL file.

#include "stdafx.h"

#include "RpCoreWrapper.h"

#include <stddef.h>
#include "IPlcore.h"

namespace RpCoreWrapper 
{
  bool RpCore::Init()
  {
    IPlcore* p = Libplayercore_CreateIPlcore();
    if (!p)
      return false;
    return true;
  }
}