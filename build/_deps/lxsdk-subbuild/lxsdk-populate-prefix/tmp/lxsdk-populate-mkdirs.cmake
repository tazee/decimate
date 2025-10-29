# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-src")
  file(MAKE_DIRECTORY "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-src")
endif()
file(MAKE_DIRECTORY
  "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-build"
  "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix"
  "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix/tmp"
  "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix/src/lxsdk-populate-stamp"
  "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix/src"
  "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix/src/lxsdk-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix/src/lxsdk-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/taz/Desktop/dev/decimate/build/_deps/lxsdk-subbuild/lxsdk-populate-prefix/src/lxsdk-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
