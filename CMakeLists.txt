cmake_minimum_required(VERSION 3.20)
project(kastrafiles VERSION 0.6.1 LANGUAGES CXX)

# ── Platform detection ─────────────────────────────────────────────
# Android is a first-class target but a reduced one: the storage-browser
# edition. Desktop-only backends (GIO volume monitor, DBus/UDisks2, the
# argon2+OpenSSL vault) are compiled out on Android and replaced by
# interface-compatible stubs so the QML layer is unchanged.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(KASTRAFILES_MOBILE ON)
else()
    set(KASTRAFILES_MOBILE OFF)
endif()

if(KASTRAFILES_MOBILE)
    message(STATUS "Kastrafiles: Android build — storage-browser edition (desktop backends stubbed)")
endif()

if(NOT KASTRAFILES_MOBILE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0 gio-unix-2.0)
    pkg_check_modules(ARGON2 IMPORTED_TARGET libargon2)
    if(NOT ARGON2_FOUND)
        find_path(ARGON2_INCLUDE_DIR NAMES argon2.h)
        find_library(ARGON2_LIBRARY NAMES argon2)
        if(ARGON2_INCLUDE_DIR AND ARGON2_LIBRARY)
            add_library(PkgConfig::ARGON2 UNKNOWN IMPORTED)
            set_target_properties(PkgConfig::ARGON2 PROPERTIES
                IMPORTED_LOCATION "${ARGON2_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ARGON2_INCLUDE_DIR}"
            )
        else()
            message(FATAL_ERROR "libargon2 (or libargon2-dev) is required but was not found.")
        endif()
    endif()
    find_package(OpenSSL REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# QtDBus is unavailable on Android, so only require it on desktop.
set(KASTRAFILES_QT_COMPONENTS Core Gui Qml Quick QuickControls2 Svg Network Concurrent Sql)
if(NOT KASTRAFILES_MOBILE)
    list(APPEND KASTRAFILES_QT_COMPONENTS DBus)
endif()
find_package(Qt6 REQUIRED COMPONENTS ${KASTRAFILES_QT_COMPONENTS})
find_package(KF6WindowSystem QUIET)

add_subdirectory(src)

# Install themes
install(DIRECTORY themes/ DESTINATION share/kastrafiles/themes FILES_MATCHING PATTERN "*.toml")

# Ship the license and attribution notice alongside the application so that
# every package (deb/rpm/pkg.tar.zst/AppImage) carries its GPLv3 text.
install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/NOTICE"
    DESTINATION share/doc/kastrafiles)

# Install QML sources (Quill module + icons needed at runtime)
install(DIRECTORY src/qml/ DESTINATION share/kastrafiles/src/qml)

# Install the generated Kastrafiles QML module: qmldir + qml/ tree + qmltypes.
# The qmldir keeps Qt's `prefer :/Kastrafiles/`, so at runtime the module resolves
# to the qmlcachegen-compiled qrc payload and nothing is recompiled on launch
# (a random AppImage mount path defeats the on-disk .qmlc cache every start).
# The on-disk tree is only the fallback main.cpp uses when the qrc load fails,
# as it did with Qt 6.7.3 NO_CACHEGEN in v0.4.14 ("SettingsPanel is not a
# type"); release builds force KASTRAFILES_ENABLE_QML_CACHEGEN=ON for that reason.
install(DIRECTORY ${CMAKE_BINARY_DIR}/src/Kastrafiles/
    DESTINATION share/kastrafiles/Kastrafiles
    FILES_MATCHING
        PATTERN "*.qml"
        PATTERN "qmldir"
        PATTERN "*.qmltypes")


option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

# ─────────────────────────────────────────────────────────────────────
# Packaging (.deb / .rpm via CPack). The AppImage and the Arch
# .pkg.tar.zst are produced by CI (linuxdeploy + makepkg respectively).
set(CPACK_PACKAGE_NAME                "kastrafiles")
set(CPACK_PACKAGE_VENDOR              "Kastrafiles Project")
set(CPACK_PACKAGE_CONTACT             "tejaskhanna989 <khannatejas989@gmail.com>")
set(CPACK_PACKAGE_HOMEPAGE_URL        "https://github.com/tejaskhanna989/Kastrafiles")
set(CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY   "kastrafiles")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A fast, modern Wayland file manager with Miller columns and a built-in secure vault")
set(CPACK_RESOURCE_FILE_LICENSE       "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGING_INSTALL_PREFIX    "/usr")
set(CPACK_STRIP_FILES                 ON)

# Debian / Ubuntu (.deb)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "tejaskhanna989 <khannatejas989@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION      "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_FILE_NAME            DEB-DEFAULT)
# Auto-compute shared-library deps; the extra Depends cover QML modules that
# shlibdeps cannot see (they are dlopened from import paths at runtime).
# NOTE: shlibdeps cannot derive package names for the non-Debian Qt libs the CI
# builds against (aqt Qt), and it silently contributes nothing for them — so we
# enumerate the complete runtime dependency set explicitly for a dependable install.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS    OFF)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libc6, libstdc++6, libgcc-s1, \
libqt6core6, libqt6gui6, libqt6network6, libqt6dbus6, libqt6concurrent6, \
libqt6sql6, libqt6sql6-sqlite, libqt6svg6, libqt6opengl6, libqt6openglwidgets6, \
libqt6qml6, libqt6quick6, libqt6quickwidgets6, libqt6quickcontrols2-6, libqt6quicktemplates2-6, \
qt6-wayland, libglib2.0-0, libargon2-1, libssl3, libgl1, libegl1, \
libxkbcommon0, libwayland-client0, xdg-utils, gvfs, \
qml6-module-qtqml, qml6-module-qtqml-workerscript, \
qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, \
qml6-module-qtquick-dialogs, qml6-module-qtquick-window, qml6-module-qtquick-templates, \
qml6-module-qt-labs-platform, qml6-module-qt5compat-graphicaleffects")

# Fedora / RHEL / openSUSE (.rpm)
set(CPACK_RPM_PACKAGE_LICENSE         "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP           "Applications/System")
set(CPACK_RPM_FILE_NAME               RPM-DEFAULT)
# rpm's ELF-based autoreq resolves sonames regardless of which Qt built the
# binary, so it is reliable here (unlike dpkg-shlibdeps above).
set(CPACK_RPM_PACKAGE_AUTOREQPROV     ON)
set(CPACK_RPM_PACKAGE_REQUIRES
    "qt6-qtbase, qt6-qtdeclarative, qt6-qtsvg, qt6-qtwayland, libargon2, openssl, gvfs")

set(CPACK_GENERATOR "DEB;RPM")
include(CPack)
