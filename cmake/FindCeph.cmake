FIND_PATH(CEPH_INCLUDE_DIR libcephfs.h
  HINTS
  $ENV{XROOTD_DIR}
  /usr
  /opt/cephfs/
  PATH_SUFFIXES include/cephfs
  PATHS /opt/cephfs
)

FIND_LIBRARY(CEPH_LIB libcephfs cephfs
  HINTS
  /usr
  /opt/cephfs/
  PATH_SUFFIXES lib
  lib64
  lib/cephfs/
  lib64/cephfs/
)

# GET_FILENAME_COMPONENT( XROOTD_LIB_DIR ${XROOTD_UTILS} PATH )

INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Ceph DEFAULT_MSG
                                        CEPH_LIB
                                        CEPH_INCLUDE_DIR )
