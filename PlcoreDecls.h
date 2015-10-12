#pragma once

#include <string>
#include "IPlcore.h"
#include "IPlcore_playtool.h"
#include "IPlcore_mediatool.h"


class CPlcoreConfig : public IPlcoreConfig
{
public:
  CPlcoreConfig()
  {
    m_nAccountType = ACCOUNT_TYPE_CHINA;
    m_strSpecifyName = m_strClientAppName = m_strClientPackageName = m_strClientVersion 
      = m_strUserName = m_strUserPassword = m_strUserEmail = "Unknow";
  };
  virtual ~CPlcoreConfig()
  {
  };

  virtual const char* GetSpecifyName(){m_strSpecifyName = "RpCore";return m_strSpecifyName.c_str();};
  virtual const char* GetClientAppName(){m_strClientAppName = "Rp";return m_strClientAppName.c_str();};
  virtual const char* GetClientPackageName(){m_strClientPackageName = "Rp";return m_strClientPackageName.c_str();};
  virtual const char* GetClientVersion(){m_strClientVersion = "1.0.0";return m_strClientVersion.c_str();};

  virtual int GetAccountType(){return m_nAccountType;};
  virtual const char* GetUsername(){return m_strUserName.c_str();};
  virtual const char* GetUserPassword(){return m_strUserPassword.c_str();};
  virtual const char* GetUserEmail(){return m_strUserEmail.c_str();};

  virtual const char* GetPath(PL_PathType type)
  {
    switch (type)
    {
    case PL_PATH_TYPE_FONT_FILE:
      {
        return m_strFontPath.c_str();
      }
    case PL_PATH_TYPE_CORE_HOME:
      {
        return m_strPlcoreHomePath.c_str();
      }
    case PL_PATH_TYPE_LOG:
      {
        return m_strLogPath.c_str();
      }
    case PL_PATH_TYPE_TEMPFOLDER:
      {
        return m_strLogPath.c_str();
      }
    case PL_PATH_TYPE_MASTER_PROFILE_FOLDER:
      {
        return m_strPlcoreHomePath.c_str();
      }
    case PL_PATH_TYPE_HOME:
      {
        return m_strPlcoreHomePath.c_str();
      }
    default:
      return "";
    }
  }
  virtual const char* GetFileName(PL_FileType type)
  {
    switch (type)
    {
    case PL_FILE_TYPE_FONT_FILE:
      {
        return m_strFontName.c_str();
      }
    default:
      return "";
    }
  }

  std::string m_strPlcoreHomePath;
  std::string m_strLogPath;
  std::string m_strFontPath;
  std::string m_strFontName;

private:
  std::string m_strSpecifyName;
  std::string m_strClientAppName;
  std::string m_strClientPackageName;
  std::string m_strClientVersion;
  int m_nAccountType;
  std::string m_strUserName;
  std::string m_strUserPassword;
  std::string m_strUserEmail;
};

class CPlayToolConfig : public IPlayToolConfig
{
public:

  CPlayToolConfig(){m_hander = NULL; m_nWidth = m_nHeight = -1;};
  virtual ~CPlayToolConfig(){};
  virtual void** GetRenderWindowHandle() const {return (void**)&m_hander;};
  virtual int GetRenderWindowInitWidth() const {return m_nWidth;};
  virtual int GetRenderWindowInitHeight() const{return m_nHeight;};

  void* m_hander;
  int m_nWidth;
  int m_nHeight;
};

class CPlaytoolCallback : public IPlcorePlaytoolCallback
{
public:
  CPlaytoolCallback() {};
  virtual ~CPlaytoolCallback() {};
};

class CMediatoolConfig: public IMediatoolConfig
{
public:
  CMediatoolConfig() {}
  virtual ~CMediatoolConfig() {}

  const char* GetSourcePath() {return m_strSourcePath.c_str();}

  int GetSourceType() {return 0;}

  bool IsCancel() {return false;}

  const char* GetThumbnailPath(int playlist = -1) {return "";}

  const char* GetMovieImagePath() {return "";}

  const char* GetMultiThumbnailPath(int nSeekTo, int& nIntervalTime, int playlist = -1) {return "";}

  int GetThumbnailPercentTime() {return 20;}

  void GetThumbnailSize(int& iWidth, int& iHeight) {iWidth = 150;iHeight = 100;}

  int GetMinTimeLengthOfPlaylist() {return 10 * 60 * 1000;}

  std::string m_strSourcePath;
};