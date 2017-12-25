pkgname=qtchan
pkgver=0.8
pkgrel=1
pkgdesc="qtchan is a 4chan browser written with qt5"
arch=('any')
url="https://github.com/siavash119/qtchan"
license=('MIT')
depends=('qt5-base')
makedepends=('qt5-base')
source=("$pkgname::https://github.com/siavash119/${pkgname}/archive/v${pkgver}.zip")
md5sums=('6a58656718fa5fccfa6f313bfc3d7dc5')

build() {
  cd ${srcdir}/${pkgname}-${pkgver}
  mkdir -p "build"
  cd "build"
  qmake ..
  make
}

package() {
  cd ${srcdir}/${pkgname}-${pkgver}/build
  install -Dm755 qtchan "$pkgdir/usr/bin/qtchan"
}

# vim:set ts=2 sw=2 et:
