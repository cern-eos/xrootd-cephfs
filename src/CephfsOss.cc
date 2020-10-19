/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright © 2020 CERN/Switzerland                                    *
 *                                                                      *
 * Author: Joaquim Rocha <joaquim.rocha@cern.ch>                        *
 *         Andreas Joachim Peters <andreas.joachim.peters@cern.ch>      *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 ************************************************************************/

#include <cephfs/libcephfs.h>
#include <fcntl.h>
#include <XrdSys/XrdSysError.hh>
#include <XrdOuc/XrdOucString.hh>
#include <XrdOuc/XrdOucStream.hh>
#include <xrootd/XrdVersion.hh>

#include "CephfsOss.hh"
#include "CephfsOssDir.hh"
#include "CephfsOssFile.hh"

extern XrdSysError OssEroute;
CephfsOss* CephfsOss::sInstance = 0;

extern "C"
{
  XrdOss*
  XrdOssGetStorageSystem(XrdOss* native_oss,
                         XrdSysLogger* Logger,
                         const char* config_fn,
                         const char* parms)
  {
    OssEroute.SetPrefix("CephfsOss_");
    OssEroute.logger(Logger);
    return (CephfsOss::Instance()->Init(Logger, config_fn) ? 0 : (XrdOss*) CephfsOss::Instance());
  }
}

CephfsOss::CephfsOss()
{
  mCephMount = 0;
}

CephfsOss::~CephfsOss()
{
  sInstance = 0;
  Shutdown();
}

void
CephfsOss::Shutdown() 
{
  if (mCephMount) {
    fprintf(stderr,"------ running shutdown ...\n");
    ceph_shutdown (mCephMount);
    fprintf(stderr,"------ shutdown completed\n");
    mCephMount = 0;
  }
}

void
CephfsOss::sShutdown(int sig)
{
  (void) signal(SIGINT, SIG_IGN);
  (void) signal(SIGTERM, SIG_IGN);
  (void) signal(SIGQUIT, SIG_IGN);

  fprintf(stderr,"------ received signal %d\n", sig);
  if (sInstance) {
    sInstance->Shutdown();
  }
  std::quick_exit(0);
}

int
CephfsOss::Init(XrdSysLogger *logger, const char *configFn)
{
  mConfigFN = configFn;

  bool cephConfigurationOk = getCephConfiguration();

  if ( !cephConfigurationOk ) {
    return -1;
  }

  ceph_create (&mCephMount, mCephConfig["id"].c_str());

  if ( ceph_conf_read_file(mCephMount, mCephConfig["config"].c_str())) {
    fprintf(stderr,"error: failed to read config file %s\n", mCephConfig["config"].c_str());
    return -1;
  }


  std::string volume = mCephConfig["volume"];
  int ret = ceph_mount(mCephMount, volume.c_str());

  if (ret) {
    fprintf(stderr,"error: ceph mount retc=%d\n", ret);
  }  else {
    signal(SIGINT, CephfsOss::sShutdown);
    signal(SIGTERM, CephfsOss::sShutdown);
    signal(SIGQUIT, CephfsOss::sShutdown);
  }
  return ret ;
}

bool 
CephfsOss::getCephConfiguration()
{
  XrdOucStream Config;
  int cfgFD;
  char *var;

  if ((cfgFD = open(mConfigFN, O_RDONLY, 0)) < 0) {
    return 0;
  }

  mCephConfig["volume"] = "/";
  mCephConfig["id"] = "admin";
  mCephConfig["config"] = "/etc/ceph/ceph.conf";

  Config.Attach(cfgFD);
  while ((var = Config.GetMyFirstWord())) {
    if (strcmp(var, "cephfs.config") == 0) {
      var += 14; 
     mCephConfig["config"] = Config.GetWord();
      continue;
    }
    
    if (strcmp(var, "cephfs.id") == 0) {
      var += 9;
      mCephConfig["id"] = Config.GetWord();
      continue;
    }

    if (strcmp(var, "cephfs.volume") == 0) {
      var += 13;
      mCephConfig["volume"] = Config.GetWord();
      continue;
    }

    if (strncmp(var, "cephfs.", 7) == 0) {
      fprintf(stderr,"error: unknown cephfs configuration '%s'\n", var);
      return false;
    }
  }
  
  for ( auto item : mCephConfig ) {
    fprintf(stderr,"       cephfs.%-6s %s\n", item.first.c_str(),  item.second.c_str());
  }

  Config.Close();

  return true;
}

int
CephfsOss::Stat(const char* path,
	      struct stat* buff,
	      int opts,
	      XrdOucEnv* env)
{
  fprintf(stderr,"stat:%s\n", path);
  return ceph_stat(mCephMount, path, buff);
}

int
CephfsOss::Mkdir(const char *path, mode_t mode, int mkpath, XrdOucEnv *envP)
{
  if (!mkpath)
    return ceph_mkdir(mCephMount, path, mode);

  return ceph_mkdirs(mCephMount, path, mode);
}

int
CephfsOss::Remdir(const char *path, int Opts, XrdOucEnv *eP)
{
  return ceph_rmdir(mCephMount, path);
}

int
CephfsOss::Rename(const char *from,
		const char *to,
		XrdOucEnv *eP1,
		XrdOucEnv *eP2)
{
  return ceph_rename(mCephMount, from, to);
}

int
CephfsOss::Unlink(const char *path, int Opts, XrdOucEnv *eP)
{
  return ceph_unlink(mCephMount, path);
}

int
CephfsOss::Chmod(const char *path, mode_t mode, XrdOucEnv *envP)
{
  return ceph_chmod(mCephMount, path, mode);
}

int
CephfsOss::Truncate (const char* path,
		   unsigned long long size,
		   XrdOucEnv* envP)
{
  return ceph_truncate(mCephMount, path, size);
}

XrdOssDF *
CephfsOss::newDir(const char *tident)
{
  return dynamic_cast<XrdOssDF *>(new CephfsOssDir(mCephMount));
}

XrdOssDF *
CephfsOss::newFile(const char *tident)
{
  return dynamic_cast<XrdOssDF *>(new CephfsOssFile(mCephMount));
}

int
CephfsOss::Create(const char *tident, const char *path, mode_t access_mode,
                XrdOucEnv &env, int Opts)
{
  struct stat stbuf;
  int ret = 0;
  bool dirAlreadyExisted = true;

  if (Opts & XRDOSS_mkpath)
  {
    int lastSlash = XrdOucString(path).rfind('/');
    if (lastSlash > 0)
    {
      XrdOucString dirPath(path, lastSlash);

      dirAlreadyExisted = ceph_stat(mCephMount, dirPath.c_str(), &stbuf) == 0;
      if (!dirAlreadyExisted)
        ret = ceph_mkdirs(mCephMount, dirPath.c_str(), access_mode);
    }
  }

  if (ret != 0)
    return ret;

  if (dirAlreadyExisted)
  {
    ret = ceph_stat(mCephMount, path, &stbuf);

    if (ret == 0)
    {
      if (S_ISDIR(stbuf.st_mode))
        return -EISDIR;
      if (Opts & XRDOSS_new)
        return -EEXIST;
    }
  }

  ret = ceph_open(mCephMount, path, O_CREAT, access_mode);
  if (ret > 0)
    ret = ceph_close(mCephMount, ret);

  return ret;
}

int
CephfsOss::StatFS(const char *path, char *buff, int &blen, XrdOucEnv *eP)
{
  struct statvfs statBuf;
  long long fSpace = 0, fSize = 0;
  int ret, valid, usedSpace = 0;

  ret = ceph_statfs(mCephMount, path, &statBuf);
  valid = ret == 0;

  if (valid && statBuf.f_frsize > 0)
  {
    fSize = statBuf.f_blocks * statBuf.f_frsize;
    fSpace = statBuf.f_bavail * statBuf.f_frsize;
    usedSpace = (fSize - fSpace) / (float) fSize * 100LL;
  }

  blen = snprintf(buff, blen, "%d %lld %d %d %lld %d",
		  valid, (valid ? fSpace : 0LL), (valid ? usedSpace : 0),
		  valid, (valid ? fSpace : 0LL), (valid ? usedSpace : 0));

  return XrdOssOK;
}

XrdVERSIONINFO(XrdOssGetStorageSystem, CephfsOss);
