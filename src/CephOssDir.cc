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
#include <assert.h>
#include <XrdSys/XrdSysPlatform.hh>

#include "CephOssDir.hh"

CephOssDir::CephOssDir(struct ceph_mount_info *cmount)
  : mCephMount(cmount),
    mDirRes(0)
{
}

CephOssDir::~CephOssDir()
{
  Close();
}

int
CephOssDir::Opendir(const char *path, XrdOucEnv &env)
{
  assert(mDirRes == 0);
  int ret = ceph_opendir(mCephMount, path, &mDirRes);
  return ret;
}

int
CephOssDir::Close(long long *retsz)
{
  if (mDirRes != 0)
    ceph_closedir(mCephMount, mDirRes);

  mDirRes = 0;

  return XrdOssOK;
}

int
CephOssDir::Readdir(char *buff, int blen)
{
  assert(mDirRes != 0);
  struct dirent *dirp = ceph_readdir(mCephMount, mDirRes);

  if (dirp)
    strlcpy(buff, dirp->d_name, blen);
  else
    *buff = '\0';

  return XrdOssOK;
}
