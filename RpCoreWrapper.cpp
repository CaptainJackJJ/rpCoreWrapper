// This is the main DLL file.

#include "stdafx.h"

#include "RpCoreWrapper.h"
#include "PlcoreDecls.h"
#include <vcclr.h>
#include <stdlib.h>
#include <assert.h>


namespace RpCoreWrapper 
{
  class CPlayCallback : public IPlcorePlayerCallback
  {
  public:
    CPlayCallback() {};
    ~CPlayCallback() {};

    void OnPlayBackEnded() {};
    void OnPlayBackStarted(){};
    void OnPlayBackPaused() {};
    void OnPlayBackResumed() {};
    void OnPlayBackStopped()  {};
    void OnQueueNextItem()  {};
    void OnPlayBackSeek(int iTime, int seekOffset) {};
    void OnPlayBackSeekChapter(int iChapter) {};
    void OnPlayBackSpeedChanged(int iSpeed) {};
    void OnPlayBackSeekState(PL_SeekState state){};
    void OnPlayBackHwDecodeFailed() {};
    void OnPlayBackDecodeModeNotify(bool Hw) {};
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
    pin_ptr<const wchar_t> cpwc = PtrToStringChars(str);

    size_t convertedChars = 0;
    size_t  sizeInBytes = ((str->Length + 1) * 2);
    errno_t err = 0;
    char *ch = new char[sizeInBytes];
    err = wcstombs_s(&convertedChars, ch, sizeInBytes, cpwc, sizeInBytes);
    return ch;
  }

#pragma endregion utils

  IPlcore* g_pPlcore = NULL;
  IPlayTool* g_pPlayTool = NULL;
  ICorePlayer* pCorePlayer = NULL;
  CPlcoreConfig* g_pPlcoreConfig = NULL;
  CPlayToolConfig* g_pPlaytoolConfig = NULL;
  CPlaytoolCallback* g_pPlaytoolCallback = NULL;
  CPlayCallback* g_pPlayCallback = NULL; 


  bool RpCore::LoadLib(String^ strRuntimesPath,String^ strTempPath)
  {
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

    pCorePlayer = g_pPlayTool->GetPlayer(g_pPlayCallback);

    return true;
  }

  bool RpCore::Play(String^ url)
  {
    CFileItem item;
    CPlayOptions options;

    char* temp = newChar(url);
    item.m_path = temp;
    delete[] temp;

    return pCorePlayer->OpenFile(&item, &options);
  }
}