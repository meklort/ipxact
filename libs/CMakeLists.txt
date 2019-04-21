################################################################################
###
### @file       libs/CMakeLists.txt 
###
### @project    ipxact
###
### @brief      Library specific CMake configuration.
###
################################################################################
###
################################################################################
###
### @copyright Copyright (c) 2019, Evan Lojewski
### @cond
###
### All rights reserved.
###
### Redistribution and use in source and binary forms, with or without
### modification, are permitted provided that the following conditions are met:
### 1. Redistributions of source code must retain the above copyright notice,
### this list of conditions and the following disclaimer.
### 2. Redistributions in binary form must reproduce the above copyright notice,
### this list of conditions and the following disclaimer in the documentation
### and/or other materials provided with the distribution.
### 3. Neither the name of the <organization> nor the
### names of its contributors may be used to endorse or promote products
### derived from this software without specific prior written permission.
###
################################################################################
###
### THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
### AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
### IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
### ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
### LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
### CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
### SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
### INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
### CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
### ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
### POSSIBILITY OF SUCH DAMAGE.
### @endcond
################################################################################

add_subdirectory(pugixml)
add_subdirectory(OptParse)

include(ExternalProject)

ExternalProject_Add(
    pcre2-ext
    URL ${CMAKE_CURRENT_SOURCE_DIR}/pcre2/pcre2-10.20.tar.bz2
    URL_HASH MD5=dcd027c57ecfdc8a6c3af9d0acf5e3f7
    PREFIX ${CMAKE_BINARY_DIR}/libs/pcre2
    CONFIGURE_COMMAND ${CMAKE_COMMAND}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DPCRE2_BUILD_PCRE2GREP=OFF
        -DPCRE2_BUILD_TESTS=OFF <SOURCE_DIR>
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/pcre2/pcre2_policy.patch
        <SOURCE_DIR>/CMakeLists.txt
    BUILD_BYPRODUCTS
        <INSTALL_DIR>/lib/libpcre2-8.a
)

add_library(pcre2 STATIC IMPORTED GLOBAL)

add_dependencies(pcre2 pcre2-ext)

ExternalProject_Get_Property(pcre2-ext INSTALL_DIR)
set_target_properties(pcre2      PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libpcre2-8.a")
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_DIR}/include/")
target_include_directories(pcre2 INTERFACE "${INSTALL_DIR}/include/")
MESSAGE("pcre2 IMPORTED_LOCATIONL: ${INSTALL_DIR}/lib/libpcre2-8.a")
MESSAGE("pcre2 INTERFACE: ${INSTALL_DIR}/include/")

