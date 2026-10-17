# Maintainer: Your Name <your@email.com>
pkgname=kastrafiles-git
pkgver=r198.g53be041
pkgrel=1
pkgdesc="A fast, modern Wayland file manager with Miller columns and a built-in secure vault"
arch=('x86_64' 'aarch64')
url="https://github.com/tejaskhanna989/Kastrafiles"
license=('GPL-3.0-or-later')
depends=(
    'glib2'
    'gvfs'
    'kwindowsystem'
    'qt6-base'
    'qt6-declarative'
    'qt6-svg'
    'qt6-wayland'
    'xdg-utils'
    'openssl'
    'argon2'
)
makedepends=(
    'cmake'
    'ninja'
    'git'
    'kwindowsystem'
    'qt6-base'
    'qt6-declarative'
    'qt6-svg'
)
optdepends=(
    'wl-clipboard: clipboard support via wl-copy and wl-paste'
    'fd: fast recursive search (falls back to a built-in walker)'
    'bat: syntax-highlighted text previews'
    'gvfs-smb: SMB/CIFS remote browsing support'
    'gvfs-mtp: Android phones (MTP) in the sidebar'
    'ffmpeg: video thumbnails and audio/video metadata (via ffprobe)'
    'poppler: PDF thumbnails, previews, and metadata (via pdftoppm/pdfinfo)'
    'perl-image-exiftool: EXIF metadata for images (via exiftool)'
    'udisks2: mount/unmount devices from sidebar'
)
provides=('kastrafiles')
conflicts=('kastrafiles')
source=(
    "${pkgname}::git+https://github.com/tejaskhanna989/Kastrafiles.git"
    "quill-icons::git+https://github.com/soyeb-jim285/quill-icons.git"
    "quill::git+https://github.com/soyeb-jim285/quill.git"
)
sha256sums=('SKIP' 'SKIP' 'SKIP')

pkgver() {
    cd "${pkgname}"
    printf "r%s.g%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
    cd "${pkgname}"
    git submodule init
    git config submodule.src/qml/icons.url "${srcdir}/quill-icons"
    git config submodule.src/qml/Quill.url "${srcdir}/quill"
    git -c protocol.file.allow=always submodule update
}

build() {
    cmake -B build -S "${pkgname}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DBUILD_TESTS=OFF \
        -DKASTRAFILES_DATA_DIR=/usr/share/kastrafiles
    cmake --build build --parallel
}

package() {
    # Install the compiled binary
    install -Dm755 "build/src/kastrafiles" "${pkgdir}/usr/bin/kastrafiles"

    # Install themes — loaded via applicationDirPath()/../themes → /usr/share/kastrafiles/themes
    install -dm755 "${pkgdir}/usr/share/kastrafiles/themes"
    install -Dm644 "${pkgname}/themes/"*.toml \
        -t "${pkgdir}/usr/share/kastrafiles/themes/"

    # Install QML module metadata (needed for loadFromModule to find Kastrafiles)
    install -Dm644 "build/src/Kastrafiles/qmldir" \
        "${pkgdir}/usr/share/kastrafiles/Kastrafiles/qmldir"
    install -Dm644 "build/src/Kastrafiles/kastrafiles.qmltypes" \
        "${pkgdir}/usr/share/kastrafiles/Kastrafiles/kastrafiles.qmltypes" 2>/dev/null || true

    # Install QML sources for Quill module
    install -dm755 "${pkgdir}/usr/share/kastrafiles/src"
    cp -r "${pkgname}/src/qml" "${pkgdir}/usr/share/kastrafiles/src/qml"

    # Install desktop entry, icon and AppStream metainfo
    install -Dm644 "${pkgname}/dist/io.github.tejaskhanna989.Kastrafiles.desktop" \
        "${pkgdir}/usr/share/applications/io.github.tejaskhanna989.Kastrafiles.desktop"
    install -Dm644 "${pkgname}/dist/io.github.tejaskhanna989.Kastrafiles.svg" \
        "${pkgdir}/usr/share/icons/hicolor/scalable/apps/io.github.tejaskhanna989.Kastrafiles.svg"
    install -Dm644 "${pkgname}/dist/io.github.tejaskhanna989.Kastrafiles.metainfo.xml" \
        "${pkgdir}/usr/share/metainfo/io.github.tejaskhanna989.Kastrafiles.metainfo.xml"

    # Install license
    install -Dm644 "${pkgname}/LICENSE" \
        "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
