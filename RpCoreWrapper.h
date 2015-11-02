// RpCoreWrapper.h

#pragma once

using namespace System;

namespace RpCoreWrapper 
{
	public ref class VideoStreamInfo
	{
	public:
		int bitrate;
		float videoAspectRatio;
		int height;
		int width;

		String^ language;
		String^ name;
		String^ videoCodecName;
		String^ stereoMode;
	};

	public ref class AudioStreamInfo
	{
	public:
		int bitrate;
		int channels;
		int samplerate;
		int bitspersample;
		int nPhysicalId;
		String^ language;
		String^ name;
		String^ audioCodecName;
	};

	public ref class SubtitleStreamInfo
	{
	public:
		String^ language;
		String^ name;
		bool bExternalSub;
		int nPhysicalId;
		String^ filename; 
	};

  public ref class MediaInfo
  {
  public:    
    double nDuration;
  };

	public ref class IRpCallback
	{
	public:
		virtual void OnEnded() {}
		virtual void OnStopped()  {}
		virtual void OnSeekStarted(){}
		virtual void OnSeekFailed(){}
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
    enum class ELogType : char
    {
      debug,
      notice,
      error
    };

  public:
    static bool LoadLib(String^ strRuntimesPath,String^ strTempPath, IRpCallback^ callback);
    static void UnLoadLib();
    static bool InitPlayer(int wndHandle, int wndWidth, int wndHeight);
    static bool UninitPlayer();
    static bool Play(String^ url,double nStartTime,int nPreSelectedAudioIdx,int nPreSelectedSubtitleIdx);
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
		static void SetMute(bool bMute);
		static VideoStreamInfo^ GetVideoStreamInfo();
		static AudioStreamInfo^ GetAudioStreamInfo(int nStream);
		static SubtitleStreamInfo^ GetSubtitleStreamInfo(int nStream);
		static void SetSubtitleVisible(bool bVisible);
		static bool GetSubtitleVisible();
    static MediaInfo^ GetMediaInfo(String^ strFileUrl);
		static void CatchSnapshot(String^ strSaveUrl);
		static void SetSubtitleColor(int color);
		static void SetSubtitleBorderColor(int color);
		static void SetSubtitleSize(int size);
		static void SetSubtitlePos(int yPos);
		static void SetSubtitleBold(bool b);
		static void SetSubtitleItalic(bool b);
		static void SetOverAssOrig(bool b);
    static void WriteLog(ELogType type,String^ strLog);
	};
}
