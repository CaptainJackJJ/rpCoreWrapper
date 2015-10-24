// This is the main DLL file.

#include "stdafx.h"
#include <windows.h>
#include "RpCoreWrapper.h"
#include "PlcoreDecls.h"
#include <vcclr.h>
#include <stdlib.h>
#include <assert.h>

using namespace System::Runtime::InteropServices;


namespace RpCoreWrapper 
{
  class CPlayCallback : public IPlcorePlayerCallback
  {
  public:
    CPlayCallback() {};
    ~CPlayCallback() {};

    void OnPlayBackEnded() {ManagedCaller::m_rpCallback->OnEnded();};
    void OnPlayBackStarted(){};
    void OnPlayBackPaused() {};
    void OnPlayBackResumed() {};
		void OnPlayBackStopped()  {ManagedCaller::m_rpCallback->OnStopped();};
    void OnQueueNextItem()  {};
    void OnPlayBackSeek(int iTime, int seekOffset){};
    void OnPlayBackSeekChapter(int iChapter) {};
    void OnPlayBackSpeedChanged(int iSpeed) {};
    void OnPlayBackSeekState(PL_SeekState state)
		{
			switch (state)
			{
			case PL_SEEK_STATE_START:
				ManagedCaller::m_rpCallback->OnSeekStarted();
				break;
			case PL_SEEK_STATE_CANNOT_SEEK:
			case PL_SEEK_STATE_FAILED:
				ManagedCaller::m_rpCallback->OnSeekFailed();
				break;
			case PL_SEEK_STATE_DISPLAY_FIRST_PIC:
				ManagedCaller::m_rpCallback->OnSeekEnded();
				break;
			}
		};
    void OnPlayBackHwDecodeFailed() {ManagedCaller::m_rpCallback->OnHwDecodeFailed();};
    void OnPlayBackDecodeModeNotify(bool Hw) {ManagedCaller::m_rpCallback->OnDecodeModeNotify(Hw);};
    void OnOpenAC3() {};
  };

  class CFileItem : public IPlcoreFileItem
  {
  public:
    CFileItem() {};
    virtual ~CFileItem() {};

    const char* GetPath() 
    { 
      return m_path.c_str();
    };
    const char* GetMimeType() 
    {
      return m_strMimeType.c_str();
    }; 
    const PL_PropertyNode* GetProperties() { return 0; };

    std::string m_path;
    std::string m_strMimeType;
  };

  class CPlayOptions : public IPlcorePlayerOptions
  {
  public:
    CPlayOptions()
    {
      m_nStartTime = m_nStartPercent = 0;
      m_bVideoOnly = false;
    };
    virtual ~CPlayOptions() {};

    double GetStartTime() { return m_nStartTime; };

    double GetStartPercent() { return m_nStartPercent; };

    const char* GetState() { return m_strState.c_str(); };

    bool IsVideoOnly() { return m_bVideoOnly; };

  public:
    double m_nStartTime;
    double m_nStartPercent;
    std::string m_strState;
    bool m_bVideoOnly;
  };

#pragma region utils

  char* newChar(String^ str)
  {
	  wchar_t* wstr = (wchar_t*)Marshal::StringToHGlobalUni(str).ToPointer();

	  int chars_num = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0,NULL,NULL);
	  char* chstr = new char[chars_num];
	  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, chstr, chars_num,NULL,NULL);

		Marshal::FreeHGlobal((IntPtr)wstr);

    return chstr;
  }

#pragma endregion utils

  IPlcore* g_pPlcore = NULL;
  IPlayTool* g_pPlayTool = NULL;
  ICorePlayer* g_pCorePlayer = NULL;
  CPlcoreConfig* g_pPlcoreConfig = NULL;
  CPlayToolConfig* g_pPlaytoolConfig = NULL;
  CPlaytoolCallback* g_pPlaytoolCallback = NULL;
  CPlayCallback* g_pPlayCallback = NULL; 


  bool RpCore::LoadLib(String^ strRuntimesPath,String^ strTempPath, IRpCallback^ callback)
  {
		ManagedCaller::m_rpCallback = callback;
    g_pPlcore = Libplayercore_CreateIPlcore();
    if (!g_pPlcore) return false;

    g_pPlcoreConfig = new CPlcoreConfig();
    g_pPlcoreConfig->m_strFontName = "ARIALUNI.TTF";
    g_pPlcoreConfig->m_strFontPath = "C:/Windows/Fonts/";

    char* temp = newChar(strRuntimesPath);
    g_pPlcoreConfig->m_strPlcoreHomePath = temp;
    delete[] temp;

    temp = newChar(strTempPath);
    g_pPlcoreConfig->m_strLogPath = temp;
		delete[] temp;

    bool bInit = g_pPlcore->Initialize(g_pPlcoreConfig);
    assert(bInit);
    if (!bInit) return false;
    return true;
  }

  void RpCore::UnLoadLib()
  {
    Libplayercore_ReleaseIPlcore(g_pPlcore);g_pPlcore = NULL;
    delete g_pPlcoreConfig;g_pPlcoreConfig = NULL; 
  }

  bool RpCore::InitPlayer(int wndHandle, int wndWidth, int wndHeight)
  {
    g_pPlaytoolConfig = new CPlayToolConfig();
    g_pPlaytoolConfig->m_hander = (void*)wndHandle;
    g_pPlaytoolConfig->m_nWidth =wndWidth;
    g_pPlaytoolConfig->m_nHeight = wndHeight;
    g_pPlaytoolCallback = new CPlaytoolCallback();
    g_pPlayTool = g_pPlcore->CreatePlayTool(g_pPlaytoolConfig, g_pPlaytoolCallback);
    assert(g_pPlayTool);
    if (!g_pPlayTool) return false;
    if (!g_pPlayTool->Init()) 
    {
      assert(0);
      return false;
    }
    g_pPlayTool->SetCSettings(PL_CSETTING_ID_SUBTITLES_CHARSET,"GBK");

    g_pPlayCallback = new CPlayCallback();

    g_pCorePlayer = g_pPlayTool->GetPlayer(g_pPlayCallback);

    return true;
  }

  bool RpCore::UninitPlayer()
  {
    g_pPlayTool->ReleasePlayer(g_pCorePlayer);g_pPlcoreConfig = NULL;
    delete g_pPlayCallback; g_pPlayCallback = NULL;
    bool b = g_pPlayTool->UnInit();
    g_pPlcore->ReleasePlayTool(g_pPlayTool);g_pPlayTool = NULL;
    delete g_pPlaytoolCallback; g_pPlaytoolCallback = NULL;
    delete g_pPlaytoolConfig; g_pPlaytoolConfig = NULL;
    return b;
  }

  bool RpCore::Play(String^ url,double nStartTime)
  {
    CFileItem item;
    CPlayOptions options;

    char* temp = newChar(url);
    item.m_path = temp;
		delete[] temp;

    options.m_nStartTime = nStartTime;

    return g_pCorePlayer->OpenFile(&item, &options);
  }

  void RpCore::Stop()
  {
    g_pCorePlayer->CloseFile();
  }

  void RpCore::Pause()
  {
    g_pCorePlayer->Pause();
  }

  double RpCore::GetTotalTime()
  {
     return g_pCorePlayer->GetTotalTime();
  }

  double RpCore::GetCurTime()
  {
    return g_pCorePlayer->GetTime();
  }

  void RpCore::Seek(double time, bool bAccurate)
  {
    g_pCorePlayer->SeekTime(time,bAccurate);
  }

  void RpCore::SetVolume(float volume)
  {
    g_pPlayTool->SetVolume(volume);
  }

  int RpCore::GetAudioCount()
  {
    return g_pCorePlayer->GetAudioStreamCount();
  }
  
  int RpCore::GetCurrentAudio()
  {
    return g_pCorePlayer->GetAudioStream();
  }

  void RpCore::SwitchAudio(int iStream)
  {
    g_pCorePlayer->SetAudioStream(iStream);
  }

  int RpCore::GetSubtitleCount()
  {
    return g_pCorePlayer->GetSubtitleCount();
  }

  int RpCore::GetCurrentSubtitle()
  {
    return g_pCorePlayer->GetSubtitle();
  }

  void RpCore::SwitchSubtitle(int iStream)
  {
    g_pCorePlayer->SetSubtitle(iStream);
  }

  void RpCore::ToFFRW(float iSpeed)
  {
    g_pCorePlayer->ToFFRW(iSpeed);
  }

  int RpCore::GetChapterCount()
  {
    return g_pCorePlayer->GetChapterCount();
  }

  int RpCore::GetCurrentChapter()
  {
    return g_pCorePlayer->GetChapter();
  }

  String^ RpCore::GetChapterName()
  {    
    char* strName = g_pCorePlayer->GetChapterName();
    String^ clistr = gcnew String(strName);
    g_pCorePlayer->ReleaseChapterName(strName);
    return clistr;
  }

  int RpCore::SwitchChapter(int iChapter)
  {
    return g_pCorePlayer->SeekChapter(iChapter);
  }

  bool RpCore::IsCaching()
  {
    return g_pCorePlayer->IsCaching();
  }

  int RpCore::GetCachePercent()
  {
    return g_pCorePlayer->GetCacheLevel();
  }

  int RpCore::AddSubtitle(String^ strSubPath)
  {
    char* temp = newChar(strSubPath);
		int index = g_pCorePlayer->AddSubtitle(temp);
		delete[] temp;
		return index;
  }

  void RpCore::PlayWndResized(int width,int height)
  {
    g_pPlayTool->RenderWndResized(width,height);
  }

	bool RpCore::IsPlaying()
	{
		return g_pCorePlayer->IsPlaying();
	}

	bool RpCore::IsPaused()
	{
		return g_pCorePlayer->IsPaused();
	}

	void RpCore::SetMute(bool bMute)
	{
		g_pPlayTool->SetMute(bMute);
	}

	VideoStreamInfo^ RpCore::GetVideoStreamInfo()
	{
		VideoStreamInfo^ info = gcnew VideoStreamInfo();
		PL_PlayerVideoStreamInfo* plInfo = g_pCorePlayer->GetVideoStreamInfo();
		info->bitrate = plInfo->bitrate;
		info->videoAspectRatio = plInfo->videoAspectRatio;
		info->height = plInfo->height;
		info->width = plInfo->width;
		info->language = gcnew String(plInfo->language);
		info->name = gcnew String(plInfo->name);
		info->videoCodecName = gcnew String(plInfo->videoCodecName);
		info->stereoMode = gcnew String(plInfo->stereoMode);
		g_pCorePlayer->ReleaseVideoStreamInfo(plInfo);
		return info;
	}

	AudioStreamInfo^ RpCore::GetAudioStreamInfo(int nStream)
	{
		AudioStreamInfo^ info = gcnew AudioStreamInfo();
		PL_PlayerAudioStreamInfo* plInfo = g_pCorePlayer->GetAudioStreamInfo(nStream);
		info->bitrate = plInfo->bitrate;
		info->channels = plInfo->channels;
		info->samplerate = plInfo->samplerate;
		info->bitspersample = plInfo->bitspersample;
		info->nPhysicalId = plInfo->nPhysicalId;
		info->language = gcnew String(plInfo->language);
		info->name = gcnew String(plInfo->name);
		info->audioCodecName = gcnew String(plInfo->audioCodecName);
		g_pCorePlayer->ReleaseAudioStreamInfo(plInfo);
		return info;
	}

	SubtitleStreamInfo^ RpCore::GetSubtitleStreamInfo(int nStream)
	{
		SubtitleStreamInfo^ info = gcnew SubtitleStreamInfo();
		PL_PlayerSubtitleStreamInfo* plInfo = g_pCorePlayer->GetSubtitleStreamInfo(nStream);
		info->language = gcnew String(plInfo->language);
		info->name = gcnew String(plInfo->name);
		info->bExternalSub = plInfo->bExternalSub;
		info->nPhysicalId = plInfo->nPhysicalId;
		info->filename = gcnew String(plInfo->filename);
		g_pCorePlayer->ReleaseSubtitleStreamInfo(plInfo);
		return info;
	}

	void RpCore::SetSubtitleVisible(bool bVisible)
	{
		g_pPlayTool->SetSubtitleOn(bVisible);
	}

	bool RpCore::GetSubtitleVisible()
	{
		return g_pPlayTool->GetSubtitleOn();
	}

  MediaInfo^ RpCore::GetMediaInfo(String^ strFileUrl)
  {
    MediaInfo^ info = gcnew MediaInfo();

    CMediatoolConfig* pConfig = new CMediatoolConfig();
    char* temp = newChar(strFileUrl);
    pConfig->m_strSourcePath = temp;
    delete[] temp;

    IMediatool* pMediatool = g_pPlcore->CreateMediatool(pConfig);
    bool b = pMediatool->Parser();
    IMediaPlaylist* pPlaylist = pMediatool->GetMediaPlaylistByPlaylist();
    info->nDuration = (double)(pPlaylist->GetDurationMs() / 1000);

    g_pPlcore->ReleaseMediatool(pMediatool);
    delete pConfig;

    return info;
  }

	void RpCore::CatchSnapshot(String^ strSaveUrl)
	{
		char* temp = newChar(strSaveUrl);
		g_pPlayTool->CaptureRenderImage(temp,0);
		delete[] temp;
	}
}