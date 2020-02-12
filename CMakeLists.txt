################################################################################
###
### @file       CMakeLists.txt
###
### @project    ipxact
###
### @brief      Top Level CMake configuration.
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
cmake_minimum_required (VERSION 2.6)

project (ipxact)

SET(CXX_STANDARD_REQUIRED True)
SET(CXX_STANDARD 11)

set( CMAKE_CXX_FLAGS "-Wall -Werror -O3" )

add_subdirectory(libs)
add_subdirectory(resources)

include_directories(includes)
include_directories(${CMAKE_BINARY_DIR}/resources/includes)

ADD_RESOURCES(RESOURCES
    resources/SimulatorOutput.cpp
    resources/SimulatorOutput_mmap.cpp
    resources/SimulatorOutput_ape.cpp
    resources/HeaderWriter.h
    resources/ASMHeader.s
    resources/ASMSymbols.s
)

set(${PROJECT_NAME}_SRCS
    main.cpp

    Number.cpp
    Register.cpp

    reader/ReaderFactory.cpp
    reader/IPXACTReader.cpp
    reader/XHTMLReader.cpp

    writer/ASMWriter.cpp
    writer/ASMSymbols.cpp
    writer/HeaderWriter.cpp
    writer/SimulatorWriter.cpp
    writer/APESimulatorWriter.cpp
    writer/IPXACTWriter.cpp
    writer/LaTeXWriter.cpp
    writer/WriterFactory.cpp

    ${RESOURCES}
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})
target_link_libraries(${PROJECT_NAME} pugixml OptParse)

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
