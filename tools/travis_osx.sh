#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

wget https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -O miniconda.sh;
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge --force
conda_pkgs="cmake>=3.3 obake-devel boost-cpp python=$OBAKE_PY_VERSION pybind11 clang clangdev"
conda create -q -p $deps_dir -y $conda_pkgs
source activate $deps_dir

export CXX=clang++
export CC=clang

cmake ../ -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=$OBAKE_BUILD_TYPE -DBoost_NO_BOOST_CMAKE=ON
make -j2 VERBOSE=1

python -c "import obake; obake.test.run_test_suite()"

set +e
set +x
