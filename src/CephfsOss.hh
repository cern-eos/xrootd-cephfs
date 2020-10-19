/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright © 2020 CERN/Switzerland                                    *
 *                                                                      *
 * Author: Joaquim Rocha <joaquim.rocha@cern.ch>                        *
 *         Andreas-Joachim Peters <andreas.joachim.peters@cern.ch>      *
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

#ifndef __CEPHFS_OSS_HH__
#define __CEPHFS_OSS_HH__

#include <xrootd/XrdOss/XrdOss.hh>
#include <stdio.h>
#include <map>

class CephfsOss : public XrdOss
{
public:
  virtual XrdOssDF *newDir(const char *tident);
  virtual XrdOssDF *newFile(const char *tident);

  virtual int     Chmod(const char *, mode_t mode, XrdOucEnv *eP=0);
  virtual int     Create(const char *, const char *, mode_t, XrdOucEnv &, 
			 int opts=0);
  virtual int     Init(XrdSysLogger *, const char *);
  virtual int     Mkdir(const char *, mode_t mode, int mkpath=0,
			XrdOucEnv *eP=0);
  virtual int     Remdir(const char *, int Opts=0, XrdOucEnv *eP=0);
  virtual int     Rename(const char *, const char *,
			 XrdOucEnv *eP1=0, XrdOucEnv *eP2=0);
  virtual int     Stat(const char *, struct stat *, int opts=0, XrdOucEnv *eP=0);
  virtual int     StatFS(const char *path, char *buff, int &blen, XrdOucEnv *eP=0);
  virtual int     Truncate(const char *, unsigned long long, XrdOucEnv *eP=0);
  virtual int     Unlink(const char *path, int Opts=0, XrdOucEnv *eP=0);
  void            Shutdown();

  CephfsOss();
  virtual ~CephfsOss();

  // static members
  static void     sShutdown(int sig);
  static CephfsOss* sInstance;

  static CephfsOss* Instance() {
    if (sInstance) {
      return sInstance;
    } else {
      sInstance = new CephfsOss();
      return sInstance;
    }
  }

private:
  bool getCephConfiguration(void);

  std::map<std::string, std::string> mCephConfig;
  struct ceph_mount_info *mCephMount;
  const char *mConfigFN;
};

#endif /* __CEPHFS_OSS_HH__ */
