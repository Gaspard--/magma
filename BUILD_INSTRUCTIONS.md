Linux:

CMake 3.9 is required.

- a valid installation of gtest is required on your computer:

```bash
## Debug / Release
mkdir build
cd build
cmake -DMAGMA_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE="Debug" ..
make -j

##! installation
sudo make install -j

##! unit tests
cd build
ctest --no-compress-output --output-on-failure -T Test || exit 1
```