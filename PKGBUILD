# Script generated with Bloom
pkgdesc="ROS - This package contains generic definitions of geometric shapes and bodies."
url='http://ros.org/wiki/geometric_shapes'

pkgname='ros-lunar-geometric-shapes'
pkgver='0.5.4_1'
pkgrel=1
arch=('any')
license=('BSD'
)

makedepends=('assimp'
'boost'
'console-bridge'
'eigen3'
'pkg-config'
'qhull'
'ros-lunar-catkin>=0.5.68'
'ros-lunar-eigen-stl-containers'
'ros-lunar-octomap'
'ros-lunar-random-numbers'
'ros-lunar-resource-retriever'
'ros-lunar-rosunit'
'ros-lunar-shape-msgs'
'ros-lunar-visualization-msgs'
)

depends=('assimp'
'boost'
'console-bridge'
'eigen3'
'qhull'
'ros-lunar-eigen-stl-containers'
'ros-lunar-octomap'
'ros-lunar-random-numbers'
'ros-lunar-resource-retriever'
'ros-lunar-shape-msgs'
'ros-lunar-visualization-msgs'
)

conflicts=()
replaces=()

_dir=geometric_shapes
source=()
md5sums=()

prepare() {
    cp -R $startdir/geometric_shapes $srcdir/geometric_shapes
}

build() {
  # Use ROS environment variables
  source /usr/share/ros-build-tools/clear-ros-env.sh
  [ -f /opt/ros/lunar/setup.bash ] && source /opt/ros/lunar/setup.bash

  # Create build directory
  [ -d ${srcdir}/build ] || mkdir ${srcdir}/build
  cd ${srcdir}/build

  # Fix Python2/Python3 conflicts
  /usr/share/ros-build-tools/fix-python-scripts.sh -v 2 ${srcdir}/${_dir}

  # Build project
  cmake ${srcdir}/${_dir} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCATKIN_BUILD_BINARY_PACKAGE=ON \
        -DCMAKE_INSTALL_PREFIX=/opt/ros/lunar \
        -DPYTHON_EXECUTABLE=/usr/bin/python2 \
        -DPYTHON_INCLUDE_DIR=/usr/include/python2.7 \
        -DPYTHON_LIBRARY=/usr/lib/libpython2.7.so \
        -DPYTHON_BASENAME=-python2.7 \
        -DSETUPTOOLS_DEB_LAYOUT=OFF
  make
}

package() {
  cd "${srcdir}/build"
  make DESTDIR="${pkgdir}/" install
}

