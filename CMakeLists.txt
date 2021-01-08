cmake_minimum_required(VERSION 3.17)
project( bubba-admin )

# defaults
option(WITH_GUI "Enable the web based GUI" YES)


set(BUBBA_ADMIN_PREFIX /opt/bubba)
set(BUBBA_ADMIN_GUI_PREFIX ${BUBBA_ADMIN_PREFIX}/web-admin)



# compile the individual components as External Projects
include(ExternalProject)

# all External Projects temporary files to folder `_tmp`
set(EXT_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/_tmp)
# all External Projects deployment files in folder `_build`
set(EXT_TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/_build)

ExternalProject_Add(libeutils
  SOURCE_DIR        static-libs/libeutils-0.7.39
  CMAKE_ARGS        -DBUILD_STATIC_LIBRARIES=ON -DBUILD_SHARED_LIBRARIES=OFF -DCMAKE_INSTALL_PREFIX=${EXT_BUILD_DIR}
  PREFIX            ${EXT_BUILD_DIR}
)

ExternalProject_Add(sigc++
  SOURCE_DIR        static-libs/libsigc++-2.4.1
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-static --disable-shared --disable-documentation --prefix=${EXT_BUILD_DIR}
  PREFIX            ${EXT_BUILD_DIR}
)

# sigc++ install paths are weird, tidy up for the remainder of our project
add_custom_target(post-sigc++
  COMMAND           ln -snf /${EXT_BUILD_DIR}/include/sigc++-2.0/sigc++ /${EXT_BUILD_DIR}/include/sigc++ && ln -snf /${EXT_BUILD_DIR}/lib/sigc++-2.0/include/sigc++config.h /${EXT_BUILD_DIR}/include/sigc++config.h
  DEPENDS           sigc++
)


# tcl-expect does not supply a pkg-config so we need to locate the library manually
file(GLOB LIBEXPECT /usr/lib*/expect*/libexpect*.so)
string(REGEX REPLACE "/libexpect.*$" "" LIBEXPECTPATH ${LIBEXPECT})
string(REGEX MATCH "expect.*$" LIBEXPECT ${LIBEXPECTPATH})
message("Found lib${LIBEXPECT}.so at ${LIBEXPECTPATH}")

# define sed replace strings for our patch command
set(REPLACE1 [=["s/ libeutils/ \$\(LIBDIR\)\/libeutils/"]=])
set(REPLACE2 [=["s/ libsigc/ \$\(LIBDIR\)\/libsigc/"]=])
set(REPLACE3 [=["s/\-lexpect/-L\$\(EXPECTDIR\) -Wl,-rpath,\$\(EXPECTDIR\) -l\$\(LIBEXPECT\)/"]=])

ExternalProject_Add(diskmanager
  SOURCE_DIR        bubba-diskmanager
  PATCH_COMMAND     eval sed -e "${REPLACE1}" -e "${REPLACE2}" -e "${REPLACE3}" -i <SOURCE_DIR>/Makefile
  BUILD_IN_SOURCE   OFF
  CONFIGURE_COMMAND mkdir -p ${EXT_TARGET_DIR}${BUBBA_ADMIN_PREFIX}/sbin
  BUILD_COMMAND     make DEBV=0 INCLUDEDIR=${EXT_BUILD_DIR}/include LIBDIR=${EXT_BUILD_DIR}/lib EXPECTDIR=${LIBEXPECTPATH} LIBEXPECT=${LIBEXPECT} -C <SOURCE_DIR> 
  INSTALL_COMMAND   mv <SOURCE_DIR>/diskmanager ${EXT_TARGET_DIR}${BUBBA_ADMIN_PREFIX}/sbin/
  PREFIX ${EXT_BUILD_DIR}
  DEPENDS post-sigc++ libeutils
)

set(MAKE_NETWORKMANAGER "make DESTDIR=${EXT_TARGET_DIR} INCLUDEDIR=${EXT_BUILD_DIR}/include LIBDIR=${EXT_BUILD_DIR}/lib EXPECTDIR=${LIBEXPECTPATH} LIBEXPECT=${LIBEXPECT}")
ExternalProject_Add(networkmanager
  SOURCE_DIR        bubba-networkmanager
  PATCH_COMMAND     eval sed -e "${REPLACE1}" -e "${REPLACE2}" -e "${REPLACE3}" -e [=["s/..APP.$/bubba/"]=] -e [=["s/..APP.\//bubba\//"]=] -i <SOURCE_DIR>/Makefile
  BUILD_IN_SOURCE   OFF
  CONFIGURE_COMMAND mkdir -p ${EXT_TARGET_DIR}${BUBBA_ADMIN_PREFIX}/sbin -p ${EXT_TARGET_DIR}${BUBBA_ADMIN_PREFIX}/bin -p ${EXT_TARGET_DIR}/etc/bubba
  BUILD_COMMAND     eval ${MAKE_NETWORKMANAGER} -C <SOURCE_DIR> 
  INSTALL_COMMAND   eval ${MAKE_NETWORKMANAGER} -C <SOURCE_DIR> prefix=${BUBBA_ADMIN_PREFIX} datadir=/var/lib sysconfigdir=etc/bubba install 
  PREFIX ${EXT_BUILD_DIR}
  DEPENDS post-sigc++ libeutils
)

ExternalProject_Add(admin-backend
  SOURCE_DIR        bubba-backend
  BUILD_IN_SOURCE   OFF
  CONFIGURE_COMMAND eval "sh -c \"cd <SOURCE_DIR> && perl Makefile.PL NAME=\\\"Bubba\\\"\""
  BUILD_COMMAND     make DESTDIR=${EXT_TARGET_DIR} -C <SOURCE_DIR> 
  INSTALL_COMMAND   make DESTDIR=${EXT_TARGET_DIR} -C <SOURCE_DIR>  install 
  PREFIX ${EXT_BUILD_DIR}
)

if(WITH_GUI)
  # There is an issue with the msgfmt check format routine always comparing the translations to msgid_plural
  set(PO2JSON_PREBUILD_COMMAND [=[sed -e "s/\([^\%]\)\%d /\1@NUMBER@ /g" -i `grep -a -r -m1 "\%d " po/ | cut -d: -f1`]=])
  set(PO2JSON_POSTBUILD_COMMAND [=[sed -e "s/@NUMBER@/\%d/g" -i `grep -a -r -m1 "@NUMBER@" po/ | cut -d: -f1`]=])

  ExternalProject_Add(admin-frontend
    SOURCE_DIR        bubba-frontend
    BUILD_IN_SOURCE   OFF
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     cd <SOURCE_DIR> && eval "${PO2JSON_PREBUILD_COMMAND}" && make DESTDIR=${EXT_TARGET_DIR} -C <SOURCE_DIR> && eval "${PO2JSON_POSTBUILD_COMMAND}"
    INSTALL_COMMAND   make DESTDIR=${EXT_TARGET_DIR} -C <SOURCE_DIR>  install 
    PREFIX ${EXT_BUILD_DIR}
  )
endif()


install(DIRECTORY ${EXT_TARGET_DIR}/opt ${EXT_TARGET_DIR}/usr ${EXT_TARGET_DIR}/var ${EXT_TARGET_DIR}/etc
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        PATTERN "*bin/*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
       )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/contrib/fake_dpkg/dpkg-query DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUBBA_ADMIN_PREFIX}/bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
       )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/contrib/identify_box_b3/identify_box DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUBBA_ADMIN_PREFIX}/bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
       )
if(WITH_GUI)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bubba-frontend/admin DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUBBA_ADMIN_GUI_PREFIX})
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/contrib/jquery/jquery-1.4.2.js ${CMAKE_CURRENT_SOURCE_DIR}/contrib/jquery/jquery-ui.js
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${BUBBA_ADMIN_GUI_PREFIX}/admin/views/default/_js
         )
endif()



