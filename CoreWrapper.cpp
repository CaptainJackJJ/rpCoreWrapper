// This is the main DLL file.

#include "stdafx.h"
#include <windows.h>
#include "CoreWrapper.h"
#include "PlcoreDecls.h"
#include <vcclr.h>
#include <stdlib.h>
#include <assert.h>

using namespace System::Runtime::InteropServices;


namespace CoreWrapper 
{
  class CPlayCallback : public IPlcorePlayerCallback
  {
  public:
    CPlayCallback() {};
    ~CPlayCallback() {};

    void OnPlayBackEnded() {ManagedCaller::m_CoreCallback->OnEnded();};
    void OnPlayBackStarted(){};
    void OnPlayBackPaused() {};
    void OnPlayBackResumed() {};
    void OnPlayBackStopped()  {ManagedCaller::m_CoreCallback->OnStopped();};
    void OnQueueNextItem()  {};
    void OnPlayBackSeek(int iTime, int seekOffset){};
    void OnPlayBackSeekChapter(int iChapter) {};
    void OnPlayBackSpeedChanged(int iSpeed) {};
    void OnPlayBackSeekState(PL_SeekState state)
    {
      switch (state)
      {
      case PL_SEEK_STATE_START:
        ManagedCaller::m_CoreCallback->OnSeekStarted();
        break;
      case PL_SEEK_STATE_CANNOT_SEEK:
      case PL_SEEK_STATE_FAILED:
        ManagedCaller::m_CoreCallback->OnSeekFailed();
        break;
      case PL_SEEK_STATE_DISPLAY_FIRST_PIC:
        ManagedCaller::m_CoreCallback->OnSeekEnded();
        break;
      }
    };
    void OnPlayBackHwDecodeFailed() {ManagedCaller::m_CoreCallback->OnHwDecodeFailed();};
    void OnPlayBackDecodeModeNotify(bool Hw) {ManagedCaller::m_CoreCallback->OnDecodeModeNotify(Hw);};
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

    //get previously selected index
    int GetPreSelectedAudioIdx(){ return m_nPreSelectedAudioIdx; };

    //get previously selected index
    int GetPreSelectedSubtitleIdx(){ return m_nPreSelectedSubtitleIdx; };

    int GetPlaylist(){ return MAINMOVIE_PLAYLIST; };

  public:
    double m_nStartTime;
    double m_nStartPercent;
    std::string m_strState;
    bool m_bVideoOnly;
    int m_nPreSelectedAudioIdx;
    int m_nPreSelectedSubtitleIdx;
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

  // convert native utf8 to .net string
  String^ newString(char* pChar)
  {
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, pChar, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, pChar, -1, wstr, wchars_num);

    int chars_num = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0,NULL,NULL);
    char* str = new char[chars_num];
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, chars_num,NULL,NULL);

    String^ newString = gcnew String(str);

    delete[] str;
    delete[] wstr;
    return newString;
  }

#pragma endregion utils

  IPlcore* g_pPlcore = NULL;
  IPlayTool* g_pPlayTool = NULL;
  ICorePlayer* g_pCorePlayer = NULL;
  CPlcoreConfig* g_pPlcoreConfig = NULL;
  CPlcoreCallback* g_pPlcoreCallback = NULL;
  CPlayToolConfig* g_pPlaytoolConfig = NULL;
  CPlaytoolCallback* g_pPlaytoolCallback = NULL;
  CPlayCallback* g_pPlayCallback = NULL; 


  bool Core::LoadLib(String^ strRuntimesPath,String^ strLogPath,String^ strTempPath, ICoreCallback^ callback)
  {
    ManagedCaller::m_CoreCallback = callback;
    g_pPlcore = Libplayercore_CreateIPlcore();
    if (!g_pPlcore) return false;

    g_pPlcoreConfig = new CPlcoreConfig();
    g_pPlcoreConfig->m_strFontName = "ARIALUNI.TTF";
    g_pPlcoreConfig->m_strFontPath = "C:/Windows/Fonts/";

    char* temp = newChar(strRuntimesPath);
    g_pPlcoreConfig->m_strPlcoreRuntimePath = temp;
    delete[] temp;

    temp = newChar(strLogPath);
    g_pPlcoreConfig->m_strLogPath = temp;
    delete[] temp;

    temp = newChar(strTempPath);
    g_pPlcoreConfig->m_strTempPath = temp;
    delete[] temp;

    g_pPlcoreCallback = new CPlcoreCallback();

    bool bInit = g_pPlcore->Initialize(g_pPlcoreConfig,g_pPlcoreCallback);
    assert(bInit);
    if (!bInit) return false;
    return true;
  }

  void Core::UnLoadLib()
  {
    if (!g_pPlcore)
      return;
    Libplayercore_ReleaseIPlcore(g_pPlcore);g_pPlcore = NULL;
    delete g_pPlcoreConfig;g_pPlcoreConfig = NULL; 
    delete g_pPlcoreCallback, g_pPlcoreCallback = NULL;
  }

  bool Core::InitPlayer(int wndHandle, int wndWidth, int wndHeight)
  {
    if (!g_pPlcore)
      return false;
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

    g_pPlayTool->SetQuickSwitchAudio(true);
    g_pPlayTool->SetQuickSwitchSubtitle(true);

    return true;
  }

  bool Core::UninitPlayer()
  {
    if (!g_pPlayTool)
      return true;
    g_pPlayTool->ReleasePlayer();g_pCorePlayer = NULL;
    delete g_pPlayCallback; g_pPlayCallback = NULL;
    bool b = g_pPlayTool->UnInit();
    g_pPlcore->ReleasePlayTool();g_pPlayTool = NULL;
    delete g_pPlaytoolCallback; g_pPlaytoolCallback = NULL;
    delete g_pPlaytoolConfig; g_pPlaytoolConfig = NULL;
    return b;
  }

  bool Core::Play(String^ url,double nStartTime, int nPreSelectedAudioIdx,int nPreSelectedSubtitleIdx )
  {
    if (!g_pCorePlayer)
      return false;

    CFileItem item;
    CPlayOptions options;

    char* temp = newChar(url);
    item.m_path = temp;
    delete[] temp;

    options.m_nStartTime = nStartTime;
    options.m_nPreSelectedAudioIdx = nPreSelectedAudioIdx;
    options.m_nPreSelectedSubtitleIdx = nPreSelectedSubtitleIdx;

    return g_pCorePlayer->OpenFile(&item, &options);
  }

  void Core::Stop()
  {
    if (!g_pCorePlayer)
      return;
    g_pCorePlayer->CloseFile();
  }

  void Core::Pause()
  {
    if (!g_pCorePlayer)
      return;
    g_pCorePlayer->Pause();
  }

  double Core::GetTotalTime()
  {
    if (!g_pCorePlayer)
      return 0;
    return g_pCorePlayer->GetTotalTime();
  }

  double Core::GetCurTime()
  {
    if (!g_pCorePlayer)
      return 0;
    return g_pCorePlayer->GetTime();
  }

  void Core::Seek(double time, bool bAccurate)
  {
    if (!g_pCorePlayer)
      return;
    g_pCorePlayer->SeekTime(time,bAccurate);
  }

  void Core::SetVolume(float volume)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetVolume(volume);
  }

  int Core::GetAudioCount()
  {
    if (!g_pCorePlayer)
      return 0;
    return g_pCorePlayer->GetAudioStreamCount();
  }

  int Core::GetCurrentAudio()
  {
    if (!g_pCorePlayer)
      return -1;
    return g_pCorePlayer->GetAudioStream();
  }

  void Core::SwitchAudio(int iStream)
  {
    if (!g_pCorePlayer)
      return;
    g_pCorePlayer->SetAudioStream(iStream);
  }

  int Core::GetSubtitleCount()
  {
    if (!g_pCorePlayer)
      return 0;
    return g_pCorePlayer->GetSubtitleCount();
  }

  int Core::GetCurrentSubtitle()
  {
    if (!g_pCorePlayer)
      return -1;
    return g_pCorePlayer->GetSubtitle();
  }

  void Core::SwitchSubtitle(int iStream)
  {
    if (!g_pCorePlayer)
      return ;
    g_pCorePlayer->SetSubtitle(iStream);
  }

  void Core::ToFFRW(float iSpeed)
  {
    if (!g_pCorePlayer)
      return ;
    g_pCorePlayer->ToFFRW(iSpeed);
  }

  int Core::GetChapterCount()
  {
    if (!g_pCorePlayer)
      return 0;
    return g_pCorePlayer->GetChapterCount();
  }

  int Core::GetCurrentChapter()
  {
    if (!g_pCorePlayer)
      return -1;
    return g_pCorePlayer->GetChapter();
  }

  String^ Core::GetChapterName()
  {    
    if (!g_pCorePlayer)
      return "";
    char* strName = g_pCorePlayer->GetChapterName();
    String^ clistr = newString(strName);
    g_pCorePlayer->ReleaseChapterName(strName);
    return clistr;
  }

  int Core::SwitchChapter(int iChapter)
  {
    if (!g_pCorePlayer)
      return -1;
    return g_pCorePlayer->SeekChapter(iChapter);
  }

  bool Core::IsCaching()
  {
    if (!g_pCorePlayer)
      return false;
    return g_pCorePlayer->IsCaching();
  }

  int Core::GetCachePercent()
  {
    if (!g_pCorePlayer)
      return 0;
    return g_pCorePlayer->GetCacheLevel();
  }

  int Core::AddSubtitle(String^ strSubPath)
  {
    if (!g_pCorePlayer)
      return -1;
    char* temp = newChar(strSubPath);
    int index = g_pCorePlayer->AddSubtitle(temp);
    delete[] temp;
    return index;
  }

  void Core::PlayWndResized(int width,int height)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->RenderWndResized(width,height);
  }

  bool Core::IsPlaying()
  {
    if (!g_pPlayTool || !g_pCorePlayer)
      return false;
    return g_pCorePlayer->IsPlaying();
  }

  bool Core::IsPaused()
  {
    if (!g_pPlayTool || !g_pCorePlayer)
      return false;
    return g_pCorePlayer->IsPaused();
  }

  void Core::SetMute(bool bMute)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetMute(bMute);
  }

  VideoStreamInfo^ Core::GetVideoStreamInfo()
  {
    VideoStreamInfo^ info = gcnew VideoStreamInfo();
    if (!g_pCorePlayer)
      return info;
    PL_PlayerVideoStreamInfo* plInfo = g_pCorePlayer->GetVideoStreamInfo();
    info->bitrate = plInfo->bitrate;
    info->videoAspectRatio = plInfo->videoAspectRatio;
    info->height = plInfo->height;
    info->width = plInfo->width;
    info->language = newString(plInfo->language);
    info->name = newString(plInfo->name);
    info->videoCodecName = newString(plInfo->videoCodecName);
    info->stereoMode = newString(plInfo->stereoMode);
    g_pCorePlayer->ReleaseVideoStreamInfo(plInfo);
    return info;
  }

  AudioStreamInfo^ Core::GetAudioStreamInfo(int nStream)
  {
    AudioStreamInfo^ info = gcnew AudioStreamInfo();
    if (!g_pCorePlayer)
      return info;
    PL_PlayerAudioStreamInfo* plInfo = g_pCorePlayer->GetAudioStreamInfo(nStream);
    info->bitrate = plInfo->bitrate;
    info->channels = plInfo->channels;
    info->samplerate = plInfo->samplerate;
    info->bitspersample = plInfo->bitspersample;
    info->nPhysicalId = plInfo->nPhysicalId;
    info->language = newString(plInfo->language);
    info->name = newString(plInfo->name);
    info->audioCodecName = newString(plInfo->audioCodecName);
    g_pCorePlayer->ReleaseAudioStreamInfo(plInfo);
    return info;
  }

  SubtitleStreamInfo^ Core::GetSubtitleStreamInfo(int nStream)
  {
    SubtitleStreamInfo^ info = gcnew SubtitleStreamInfo();
    if (!g_pCorePlayer)
      return info;
    PL_PlayerSubtitleStreamInfo* plInfo = g_pCorePlayer->GetSubtitleStreamInfo(nStream);
    info->language = newString(plInfo->language);
    info->name = newString(plInfo->name);
    info->bExternalSub = plInfo->bExternalSub;
    info->nPhysicalId = plInfo->nPhysicalId;
    info->filename = newString(plInfo->filename);
    g_pCorePlayer->ReleaseSubtitleStreamInfo(plInfo);
    return info;
  }

  void Core::SetSubtitleVisible(bool bVisible)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetSubtitleOn(bVisible);
  }

  bool Core::GetSubtitleVisible()
  {
    if (!g_pPlayTool)
      return false;
    return g_pPlayTool->GetSubtitleOn();
  }

  MediaInfo^ Core::GetMediaInfo(String^ strFileUrl,String^ strThumbUrl,int nThumbPercent,int nThumbWidth)
  {
    MediaInfo^ info = gcnew MediaInfo();
    if (!g_pPlcore)
      return info;

    CMediatoolConfig* pConfig = new CMediatoolConfig();
    char* temp = newChar(strFileUrl);
    pConfig->m_strSourcePath = temp;    
    delete[] temp;
    temp = newChar(strThumbUrl);
    pConfig->m_strThumbPath = temp;
    delete[] temp;
    pConfig->m_nThumbPercent = nThumbPercent;
    pConfig->m_nThumbWidth = nThumbWidth;
    pConfig->m_nThumbHeight = nThumbWidth / 1.77;

    IMediatool* pMediatool = g_pPlcore->CreateMediatool(pConfig);
    if(pMediatool->Parser())
    {
      IMediaPlaylist* pPlaylist = pMediatool->GetMediaPlaylistByPlaylist();
      if(pPlaylist)
      {
        info->nDuration = (double)(pPlaylist->GetDurationMs() / 1000);
        if (strThumbUrl != "")
        {
          IMediaInfo* pMInfo = pPlaylist->GetMediaInfo(STREAM_TYPE_VIDEO,0);
          pConfig->m_nThumbHeight = nThumbWidth * pMInfo->GetHeignt() / pMInfo->GetWidth();// Remain orig aspect
          pMediatool->GetThumbnail();
        }        
      }      
    }

    g_pPlcore->ReleaseMediatool(pMediatool);
    delete pConfig;

    return info;
  }

  bool Core::CatchSnapshot(String^ strSaveUrl)
  {
    if (!g_pPlayTool)
      return false;
    char* temp = newChar(strSaveUrl);
    bool r = g_pPlayTool->CaptureRenderImage(temp,0);
    delete[] temp;
    return r;
  }

  void  Core::SetSubtitleColor(int color)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetSubColor(color);
  }
  void  Core::SetSubtitleBorderColor(int color)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetSubtitleBorderColor(color);
  }
  void  Core::SetSubtitleSize(int size)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetSubtitleSize(size);
  }
  void  Core::SetSubtitlePos(int yPos)
  {
    if (!g_pPlayTool)
      return;
    float fYpos = (float)(yPos * 0.01);
    g_pPlayTool->SetSubtitlePos(fYpos);
  }
  void  Core::SetSubtitleBold(bool b)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetSubtitleBold(b);
  }
  void  Core::SetSubtitleItalic(bool b)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetSubtitleItalic(b);
  }
  void  Core::SetOverAssOrig(bool b)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetWhetherOverAssOrigSettings(b);
  }

  void  Core::EnableHW(bool hw)
  {
    if (!g_pPlayTool)
      return;
    g_pPlayTool->SetCSettings("videoplayer.useintelgpu", hw);
    g_pPlayTool->SetCSettings("videoplayer.usedxva2", hw);
  }

  void Core::WriteLog(ELogType type,String^ strLog)
  {
    if (!g_pPlcore)
      return;
    char* temp = newChar(strLog);
    int nLogLevel = 0;
    switch (type)
    {
    case ELogType::debug:
      nLogLevel = 0;
      break;
    case ELogType::notice:
      nLogLevel = 2;
      break;
    case ELogType::error:
      nLogLevel = 4;
      break;
    }
    g_pPlcore->OutputToLogFile(nLogLevel,temp);
    delete[] temp;
  }
}