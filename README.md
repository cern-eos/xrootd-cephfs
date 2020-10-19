XRootD Cephfs Open Storage System Plugin
========================================

This project provides a s.c. OSS plugin for the **XRootD** server to bridge IO to/from a Cephfs filesystem without using a kernel or fuse client mount.

Build
-----

```
  mkdir build
  cd build
  # . /opt/rh/devtoolset-8/enable                   # use a modern GCC with C++11 support
  cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/
  make
  make install
```

Setup
-----

The XRootD configuration file directive to load the OSS plug-in is:

  ofs.osslib /path/to/libCephfsOss.so


Configuration
-------------

The Cephfs monitor host name(s) have to be stored in a ceph configuration file. The configured default ceph configuration file is:

```
  cephfs.config /etc/ceph/ceph.conf
```

The Cephfs volume to mount is by default:

```
  cephfs.volume /
```

If you use e.g. an OpenStack manila volume you replace the volume parameter with the volume path provided by OpenStack.

The Cephfs (cephx) client id is by default:

```
  cephfs.id admin
```

You can replace this with the appropriate client id. In the case of OpenStack the client id is shown for each manila share.  Ceph stores the corresponding keyring for a given id under ```/etc/ceph/ceph.client.<id>.conf``` e.g. the keyring for id admin would be ```/etc/ceph/ceph.client.admin.conf```.

File Layout Configuration
-------------------------

By default Cephfs places files with the given Cephfs extended attributes in a given directory and the default data pool.

You can modify this behaviour on per-file basis appending CGI parameters to your XRootD URL. The supported parameters are:

```   
  cephfs.stripe_unit=<bytes>
  cephfs.stripe_count=<count>
  cephfs.object_size=<bytes>
  cephfs.pool=<data-pool>
```

A description of these can be found here: https://docs.ceph.com/en/latest/cephfs/file-layouts/


File Ownership
--------------

By default all files will be accessed as the effective user of the XRootD process. If you want

Asynchronous vs. synchronous IO
-------------------------------

By default an XRootD server uses synchronous (blocking) IO. If you are interested in high-performance streaming you can configure large asynchronous IO using the following two configuration parameters.

```
xrootd.async nosf segsize 16777216
xrd.buffers maxbsz 16777216
```

In this example we use asynchronous IO with 16M block size. Be aware that large IO segment sizes increase the memory requirements for open files.

