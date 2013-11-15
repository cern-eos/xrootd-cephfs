/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright © 2013 CERN/Switzerland                                    *
 *                                                                      *
 * Author: Joaquim Rocha <joaquim.rocha@cern.ch>                        *
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

#include "CephOss.hh"
#include "CephOssDir.hh"
#include "CephOssFile.hh"

extern XrdSysError OssEroute;

extern "C"
{
  XrdOss*
  XrdOssGetStorageSystem(XrdOss* native_oss,
                         XrdSysLogger* Logger,
                         const char* config_fn,
                         const char* parms)
  {
    OssEroute.SetPrefix("CephOss_");
    OssEroute.logger(Logger);
    CephOss* cephOss = new CephOss();

    return (cephOss->Init(Logger, config_fn) ? 0 : (XrdOss*) cephOss);
  }
}

CephOss::CephOss()
{
  ceph_create (&mCephMount, 0);
}

CephOss::~CephOss()
{
  ceph_shutdown (mCephMount);
}

int
CephOss::Init(XrdSysLogger *logger, const char *configFn)
{
  mConfigFN = configFn;

  const char *cephConfigPath = getCephConfigurationFilePath();
  if (cephConfigPath == 0 || ceph_conf_read_file(mCephMount, cephConfigPath))
    return -1;

  int ret = ceph_mount(mCephMount, "/");

  return ret ;
}

const char *
CephOss::getCephConfigurationFilePath()
{
  XrdOucStream Config;
  int cfgFD;
  char *var, *configPath = 0;

  if ((cfgFD = open(mConfigFN, O_RDONLY, 0)) < 0)
  {
    return 0;
  }

  Config.Attach(cfgFD);
  while ((var = Config.GetMyFirstWord()))
  {
    if (strcmp(var, "cephoss.config") == 0)
    {
      var += 14;
      configPath = Config.GetWord();
      break;
    }
  }

  Config.Close();

  return configPath;
}

int
CephOss::Stat(const char* path,
	      struct stat* buff,
	      int opts,
	      XrdOucEnv* env)
{
  return ceph_stat(mCephMount, path, buff);
}

int
CephOss::Mkdir(const char *path, mode_t mode, int mkpath, XrdOucEnv *envP)
{
  if (!mkpath)
    return ceph_mkdir(mCephMount, path, mode);

  return ceph_mkdirs(mCephMount, path, mode);
}

int
CephOss::Remdir(const char *path, int Opts, XrdOucEnv *eP)
{
  return ceph_rmdir(mCephMount, path);
}

int
CephOss::Rename(const char *from,
		const char *to,
		XrdOucEnv *eP1,
		XrdOucEnv *eP2)
{
  return ceph_rename(mCephMount, from, to);
}

int
CephOss::Unlink(const char *path, int Opts, XrdOucEnv *eP)
{
  return ceph_unlink(mCephMount, path);
}

int
CephOss::Chmod(const char *path, mode_t mode, XrdOucEnv *envP)
{
  return ceph_chmod(mCephMount, path, mode);
}

int
CephOss::Truncate (const char* path,
		   unsigned long long size,
		   XrdOucEnv* envP)
{
  return ceph_truncate(mCephMount, path, size);
}

XrdOssDF *
CephOss::newDir(const char *tident)
{
  return dynamic_cast<XrdOssDF *>(new CephOssDir(mCephMount));
}

XrdOssDF *
CephOss::newFile(const char *tident)
{
  return dynamic_cast<XrdOssDF *>(new CephOssFile(mCephMount));
}

int
CephOss::Create(const char *tident, const char *path, mode_t access_mode,
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
CephOss::StatFS(const char *path, char *buff, int &blen, XrdOucEnv *eP)
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

XrdVERSIONINFO(XrdOssGetStorageSystem, CephOss);
