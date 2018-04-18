# Script generated with Bloom
pkgdesc="ROS - This package contains generic definitions of geometric shapes and bodies."
url='http://ros.org/wiki/geometric_shapes'

pkgname='ros-melodic-geometric-shapes'
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
'ros-melodic-catkin>=0.5.68'
'ros-melodic-eigen-stl-containers'
'ros-melodic-octomap'
'ros-melodic-random-numbers'
'ros-melodic-resource-retriever'
'ros-melodic-rosunit'
'ros-melodic-shape-msgs'
'ros-melodic-visualization-msgs'
)

depends=('assimp'
'boost'
'console-bridge'
'eigen3'
'qhull'
'ros-melodic-eigen-stl-containers'
'ros-melodic-octomap'
'ros-melodic-random-numbers'
'ros-melodic-resource-retriever'
'ros-melodic-shape-msgs'
'ros-melodic-visualization-msgs'
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
  [ -f /opt/ros/melodic/setup.bash ] && source /opt/ros/melodic/setup.bash

  # Create build directory
  [ -d ${srcdir}/build ] || mkdir ${srcdir}/build
  cd ${srcdir}/build

  # Fix Python2/Python3 conflicts
  /usr/share/ros-build-tools/fix-python-scripts.sh -v 2 ${srcdir}/${_dir}

  # Build project
  cmake ${srcdir}/${_dir} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCATKIN_BUILD_BINARY_PACKAGE=ON \
        -DCMAKE_INSTALL_PREFIX=/opt/ros/melodic \
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

