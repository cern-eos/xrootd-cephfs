/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright © 2020 CERN/Switzerland                                    *
 *                                                                      *
 * Author: Joaquim Rocha <joaquim.rocha@cern.ch>                        *
 *         Andreas-Joachim.Peters <andreas.joachim.peters@cern.ch>      *
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

#include <thread>
#include <future>
#include <cephfs/libcephfs.h>
#include <private/XrdOss/XrdOssError.hh>
#include <XrdOuc/XrdOucEnv.hh>
#include <XrdSfs/XrdSfsAio.hh>
#include "CephfsOssFile.hh"

#define CEPHFS_ENV_PREFIX  "cephfs."

CephfsOssFile::CephfsOssFile(struct ceph_mount_info *cmount)
  : mCephMount(cmount)
{
  fd = 0;
}

CephfsOssFile::~CephfsOssFile()
{
  Close();
}

int
CephfsOssFile::Close(long long *retsz)
{
  return ceph_close(mCephMount, fd);
}

int
CephfsOssFile::Open(const char *path, int flags, mode_t mode, XrdOucEnv &env)
{
  int stripe_unit = (int) env.GetInt(CEPHFS_ENV_PREFIX "stripe_unit");
  int stripe_count = (int) env.GetInt(CEPHFS_ENV_PREFIX "stripe_count");
  int object_size = (int) env.GetInt(CEPHFS_ENV_PREFIX  "object_size");
  char *data_pool = env.Get(CEPHFS_ENV_PREFIX "pool");

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
CephfsOssFile::Read(off_t offset, size_t blen)
{
  if (fd < 0)
    return (ssize_t)-XRDOSS_E8004;

  return 0;
}

ssize_t
CephfsOssFile::Read(void *buff, off_t offset, size_t blen)
{
  return ceph_read(mCephMount, fd, (char *) buff, blen, offset);
}

int
CephfsOssFile::ReadAsync(XrdSfsAio *aiop) {
  aiop->Result = Read((void*)aiop->sfsAio.aio_buf, aiop->sfsAio.aio_offset, aiop->sfsAio.aio_nbytes);
  aiop->doneRead();
  return 0;
}


int
CephfsOssFile::Write(XrdSfsAio *aiop) {
  aiop->Result = Write((void*)aiop->sfsAio.aio_buf, aiop->sfsAio.aio_offset, aiop->sfsAio.aio_nbytes);
  aiop->doneWrite();
  return 0;
}

int 
CephfsOssFile::Read(XrdSfsAio *aiop)
{
  int rc;
  std::async(std::launch::async, [this,aiop] { this->ReadAsync(aiop); });
  return 0;
}

ssize_t
CephfsOssFile::ReadRaw(void *buff, off_t offset, size_t blen)
{
  return Read(buff, offset, blen);
}

int
CephfsOssFile::Fstat(struct stat *buff)
{
  return ceph_fstat(mCephMount, fd, buff);
}

ssize_t
CephfsOssFile::Write(const void *buff, off_t offset, size_t blen)
{
  return ceph_write(mCephMount, fd, (const char *) buff, blen, offset);
}

int
CephfsOssFile::Fsync()
{
  return ceph_fsync(mCephMount, fd, 1);
}
