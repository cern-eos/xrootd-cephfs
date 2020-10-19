find_package( XRootD REQUIRED )
find_package( Ceph REQUIRED )

add_library( CephfsOss SHARED
             CephfsOss.cc CephfsOss.hh
             CephfsOssDir.cc CephfsOssDir.hh
             CephfsOssFile.cc CephfsOssFile.hh
)

include_directories( ${XROOTD_INCLUDE_DIR} ${CEPH_INCLUDE_DIR} )

add_definitions( -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 )

target_link_libraries( CephfsOss ${CEPH_LIB} )

if( Linux )
  set_target_properties( CephfsOss PROPERTIES
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    SOVERSION ${VERSION_MAJOR}
    CLEAN_DIRECT_OUTPUT 1
  )
endif( Linux )

install( TARGETS CephfsOss LIBRARY DESTINATION ${LIB_INSTALL_DIR} )
