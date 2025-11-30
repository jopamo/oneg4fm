# Private libfm library for pcmanfm-qt

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.50.0)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.50.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0>=2.50.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0>=2.50.0)
pkg_check_modules(GTHREAD REQUIRED gthread-2.0>=2.50.0)
pkg_check_modules(MENU_CACHE REQUIRED libmenu-cache>=1.1.0)
pkg_check_modules(EXIF REQUIRED libexif>=0.6.0)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base
    ${CMAKE_CURRENT_SOURCE_DIR}/src/job
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extra
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gtk
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${GIO_UNIX_INCLUDE_DIRS}
    ${MENU_CACHE_INCLUDE_DIRS}
)

# Generate marshal files
find_program(GLIB_GENMARSHAL glib-genmarshal)
if(GLIB_GENMARSHAL)
    # Generate fm-marshal.h and fm-marshal.c
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.c
        COMMAND ${GLIB_GENMARSHAL}
            --prefix=fm_marshal
            --header
            ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.list
            > ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.h
        COMMAND ${GLIB_GENMARSHAL}
            --prefix=fm_marshal
            --body
            ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.list
            > ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.list
        COMMENT "Generating fm-marshal files"
    )
endif()

# Compiler definitions for private use
add_definitions(
    -DPACKAGE_DATA_DIR="${CMAKE_SOURCE_DIR}/libfm/data"
    -DPACKAGE_LOCALE_DIR="${CMAKE_INSTALL_PREFIX}/share/locale"
    -DPACKAGE_MODULES_DIR="${CMAKE_INSTALL_PREFIX}/lib/libfm/modules"
    -DPACKAGE_VERSION="1.3.2"
    -DGETTEXT_PACKAGE="libfm"
)

# Base library sources
set(base_SOURCES
    src/base/fm-action.c
    src/base/fm-app-info.c
    src/base/fm-archiver.c
    src/base/fm-bookmarks.c
    src/base/fm-config.c
    src/base/fm-dummy-monitor.c
    src/base/fm-file.c
    src/base/fm-file-info.c
    src/base/fm-file-launcher.c
    src/base/fm-folder.c
    src/base/fm-folder-config.c
    src/base/fm-icon.c
    src/base/fm-list.c
    src/base/fm-marshal.c  # This will be generated
    src/base/fm-mime-type.c
    src/base/fm-module.c
    src/base/fm-monitor.c
    src/base/fm-nav-history.c
    src/base/fm-path.c
    src/base/fm-templates.c
    src/base/fm-terminal.c
    src/base/fm-thumbnail-loader.c
    src/base/fm-thumbnailer.c
    src/base/fm-utils.c
)

# Job sources
set(job_SOURCES
    src/job/fm-deep-count-job.c
    src/job/fm-dir-list-job.c
    src/job/fm-file-info-job.c
    src/job/fm-file-ops-job.c
    src/job/fm-file-ops-job-change-attr.c
    src/job/fm-file-ops-job-delete.c
    src/job/fm-file-ops-job-xfer.c
    src/job/fm-job.c
    src/job/fm-simple-job.c
)

# Common sources
set(glib_compat_SOURCES
    src/glib-compat.c
)

# UDisks sources (conditional)
if(UDISKS_FOUND)
    set(udisks_SOURCES
        src/udisks/fm-udisks.c
        src/udisks/g-udisks-volume-monitor.c
        src/udisks/g-udisks-device.c
        src/udisks/g-udisks-volume.c
        src/udisks/g-udisks-drive.c
        src/udisks/g-udisks-mount.c
        src/udisks/dbus-utils.c
    )
    add_definitions(-DENABLE_UDISKS)
endif()

# Main libfm library
set(libfm_SOURCES
    src/fm.c
    ${glib_compat_SOURCES}
    ${base_SOURCES}
    ${job_SOURCES}
    ${udisks_SOURCES}
)

# Extra library
set(libfm_extra_SOURCES
    src/extra/fm-xml-file.c
)

# Create libfm libraries as shared libraries (not installed)
if(GLIB_GENMARSHAL)
    add_library(fm SHARED ${libfm_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.h ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fm-marshal.c)
else()
    add_library(fm SHARED ${libfm_SOURCES})
endif()
add_library(fm-extra SHARED ${libfm_extra_SOURCES})

# Set library version
set_target_properties(fm PROPERTIES
    VERSION "1.3.2"
    SOVERSION "1"
)
set_target_properties(fm-extra PROPERTIES
    VERSION "1.3.2"
    SOVERSION "1"
)

# Export all symbols from libfm libraries (needed for shared library builds)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(fm PRIVATE "-fvisibility=default")
    target_compile_options(fm-extra PRIVATE "-fvisibility=default")
endif()

# Link dependencies to the object libraries
target_link_libraries(fm
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${GTHREAD_LIBRARIES}
    ${MENU_CACHE_LIBRARIES}
    ${EXIF_LIBRARIES}
)

target_link_libraries(fm-extra
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
)

# Include directories for targets
target_include_directories(fm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLIB_INCLUDE_DIRS}
)

target_include_directories(fm-extra PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLIB_INCLUDE_DIRS}
)

# Install libfm shared libraries
install(TARGETS fm fm-extra
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

# Install libfm public headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libfm"
    FILES_MATCHING
        PATTERN "*.h"
)