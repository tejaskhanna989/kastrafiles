cmake_minimum_required(VERSION 3.20)
project(kastrafiles VERSION 0.6.1 LANGUAGES CXX)

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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 DBus Svg Network Concurrent Sql)
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
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS    ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6sql6-sqlite, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-dialogs, qml6-module-qtquick-window, qml6-module-qtquick-templates, qml6-module-qtqml-workerscript, qml6-module-qt-labs-platform, qml6-module-qt5compat-graphicaleffects")

# Fedora / RHEL / openSUSE (.rpm)
set(CPACK_RPM_PACKAGE_LICENSE         "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP           "Applications/System")
set(CPACK_RPM_FILE_NAME               RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_AUTOREQPROV     ON)
set(CPACK_RPM_PACKAGE_REQUIRES
    "qt6-qtbase, qt6-qtdeclarative, qt6-qtsvg, libargon2, openssl")

set(CPACK_GENERATOR "DEB;RPM")
include(CPack)
