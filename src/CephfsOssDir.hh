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

#ifndef __CEPHFS_OSS_DIR_HH__
#define __CEPHFS_OSS_DIR_HH__

#include <xrootd/XrdOss/XrdOss.hh>
 
class CephfsOssDir : public XrdOssDF
{
public:
  CephfsOssDir(struct ceph_mount_info *cmount);
  virtual ~CephfsOssDir();
  virtual int Opendir(const char *, XrdOucEnv &);
  virtual int Readdir(char *buff, int blen);
  virtual int Close(long long *retsz=0);

private:
  struct ceph_mount_info *mCephMount;
  struct ceph_dir_result *mDirRes;
};

#endif /* __CEPHFS_OSS_DIR_HH__ */
