// RpCoreWrapper.h

#pragma once

using namespace System;

namespace RpCoreWrapper 
{
	public ref class RpCore
	{
  public:
    static bool LoadLib(String^ strRuntimesPath,String^ strTempPath);
    static bool InitPlayer(int wndHandle, int wndWidth, int wndHeight);
    static bool Play(String^ url);
	};
}
