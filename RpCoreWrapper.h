// RpCoreWrapper.h

#pragma once

using namespace System;

namespace RpCoreWrapper 
{
	public ref class IRpCallback
	{
	public:
		virtual void OnEnded() {}
		virtual void OnStopped()  {}
		virtual void OnSeekStarted(){}
		virtual void OnSeekEnded(){}
		virtual void OnHwDecodeFailed() {}
		virtual void OnDecodeModeNotify(bool Hw) {}
	};

	private ref class ManagedCaller
	{
	public:
		static IRpCallback^ m_rpCallback;
	};

	public ref class RpCore
	{
  public:
    static bool LoadLib(String^ strRuntimesPath,String^ strTempPath, IRpCallback^ callback);
    static void UnLoadLib();
    static bool InitPlayer(int wndHandle, int wndWidth, int wndHeight);
    static bool UninitPlayer();
    static bool Play(String^ url,double nStartTime);
    static void Stop();
    static void Pause();
    static double GetTotalTime();
    static double GetCurTime();
    static void Seek(double time, bool bAccurate);
    static void SetVolume(float volume);
    static int  GetAudioCount();
    static int  GetCurrentAudio();
    static void SwitchAudio(int iStream);
    static int  GetSubtitleCount();
    static int  GetCurrentSubtitle();
    static void SwitchSubtitle(int iStream);
    static void ToFFRW(float iSpeed);
    static int  GetChapterCount();
    static int  GetCurrentChapter();
    static String^ GetChapterName();
    static int  SwitchChapter(int iChapter);
    static bool IsCaching();
    static int GetCachePercent();
    static int AddSubtitle(String^ strSubPath);
    static void PlayWndResized(int width,int height);
		static bool IsPlaying();
		static bool IsPaused();
	};
}
