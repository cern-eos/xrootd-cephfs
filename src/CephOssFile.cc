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
#include <private/XrdOss/XrdOssError.hh>
#include <XrdOuc/XrdOucEnv.hh>

#include "CephOssFile.hh"

#define CEPH_ENV_PREFIX  "ceph."

CephOssFile::CephOssFile(struct ceph_mount_info *cmount)
  : mCephMount(cmount)
{
  fd = 0;
}

CephOssFile::~CephOssFile()
{
  Close();
}

int
CephOssFile::Close(long long *retsz)
{
  return ceph_close(mCephMount, fd);
}

int
CephOssFile::Open(const char *path, int flags, mode_t mode, XrdOucEnv &env)
{
  int stripe_unit = (int) env.GetInt(CEPH_ENV_PREFIX "stripe_unit");
  int stripe_count = (int) env.GetInt(CEPH_ENV_PREFIX "stripe_count");
  int object_size = (int) env.GetInt(CEPH_ENV_PREFIX  "object_size");
  char *data_pool = env.Get(CEPH_ENV_PREFIX "data_pool");

  if (stripe_unit < 0)
    stripe_count = 0;
  if (stripe_count < 0)
    stripe_count = 0;
  if (object_size < 0)
    object_size = 0;

  fd = ceph_open_layout(mCephMount, path, flags, mode, stripe_unit,
			stripe_count, object_size, data_pool);

  return (fd < 0 ? fd : XrdOssOK);
}

ssize_t
CephOssFile::Read(off_t offset, size_t blen)
{
  if (fd < 0)
    return (ssize_t)-XRDOSS_E8004;

  return 0;
}

ssize_t
CephOssFile::Read(void *buff, off_t offset, size_t blen)
{
  return ceph_read(mCephMount, fd, (char *) buff, blen, offset);
}

int
CephOssFile::Fstat(struct stat *buff)
{
  return ceph_fstat(mCephMount, fd, buff);
}

ssize_t
CephOssFile::Write(const void *buff, off_t offset, size_t blen)
{
  return ceph_write(mCephMount, fd, (const char *) buff, blen, offset);
}

int
CephOssFile::Fsync()
{
  return ceph_fsync(mCephMount, fd, 1);
}
