#!/usr/bin/env bash
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
if [[ "${TARGET_CPU}" == "x86" ]]; then
	sudo dpkg --add-architecture i386
fi

sudo apt-get update -qq

sudo apt-get install -qq g++-9 gcovr
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90

if [[ "${TARGET_CPU}" == "x86" ]]; then
	sudo apt-get install -y g++-9-multilib
fi

CMAKE_VERSION=3.13.4
CMAKE_VERSION_DIR=v3.13

CMAKE_OS=Linux-x86_64
CMAKE_TAR=cmake-${CMAKE_VERSION}-${CMAKE_OS}.tar.gz
CMAKE_URL=http://www.cmake.org/files/${CMAKE_VERSION_DIR}/${CMAKE_TAR}
CMAKE_DIR=$(pwd)/cmake-${CMAKE_VERSION}

wget ${CMAKE_URL}
mkdir -p ${CMAKE_DIR}
tar --strip-components=1 -xzf ${CMAKE_TAR} -C ${CMAKE_DIR}
export PATH=${CMAKE_DIR}/bin:${PATH}
cmake --version

