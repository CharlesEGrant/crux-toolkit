cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(cometsearch)

# MSToolkit is built from the copy in the Comet source tree. The expat and
# zlib archives in MSToolkit/extern are unpacked by the PATCH_COMMAND in
# ext/CMakeLists.txt, which also copies in Crux's expat_config.h and zutil.h.
set(MST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/MSToolkit)
set(EXPAT_PATH ${MST_PATH}/extern/expat-2.2.9/lib)
set(ZLIB_PATH ${MST_PATH}/extern/zlib-1.2.11)
set(
  MSTOOLKIT_INCLUDE_DIRS
  ${MST_PATH}/include
  ${EXPAT_PATH}
  ${ZLIB_PATH}
)

# INCLUDE HEADER FILES IN SRC DIRECTORY
include_directories(
  ${BASE_INCLUDE_PATH}
  ${MSTOOLKIT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/CometSearch
  ${CMAKE_CURRENT_BINARY_DIR}/AScorePro/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")

#############################################################################
# Build MSToolkit static library (mstoolkitlite) from Comet's MSToolkit
###############################################################################

set(
  EXPAT
  ${EXPAT_PATH}/xmlparse.c
  ${EXPAT_PATH}/xmlrole.c
  ${EXPAT_PATH}/xmltok.c
)

set(
  ZLIB
  ${ZLIB_PATH}/adler32.c
  ${ZLIB_PATH}/compress.c
  ${ZLIB_PATH}/crc32.c
  ${ZLIB_PATH}/deflate.c
  ${ZLIB_PATH}/inffast.c
  ${ZLIB_PATH}/inflate.c
  ${ZLIB_PATH}/infback.c
  ${ZLIB_PATH}/inftrees.c
  ${ZLIB_PATH}/trees.c
  ${ZLIB_PATH}/uncompr.c
  ${ZLIB_PATH}/zutil.c
)

# RAWReader.cpp is C++/CLI and is built separately on Windows (see below)
set(
  MSTOOLKIT
  ${MST_PATH}/src/MSToolkit/MSObject.cpp
  ${MST_PATH}/src/MSToolkit/MSReader.cpp
  ${MST_PATH}/src/MSToolkit/Spectrum.cpp
)

# mzMLReader.cpp is a stand-alone example program, so it is left out
set(
  MZPARSER
  ${MST_PATH}/src/mzParser/BasicChromatogram.cpp
  ${MST_PATH}/src/mzParser/BasicSpectrum.cpp
  ${MST_PATH}/src/mzParser/Czran.cpp
  ${MST_PATH}/src/mzParser/MSNumpress.cpp
  ${MST_PATH}/src/mzParser/mz5handler.cpp
  ${MST_PATH}/src/mzParser/mzParser.cpp
  ${MST_PATH}/src/mzParser/mzpMz5Config.cpp
  ${MST_PATH}/src/mzParser/mzpMz5Structs.cpp
  ${MST_PATH}/src/mzParser/mzp_base64.cpp
  ${MST_PATH}/src/mzParser/PWIZface.cpp
  ${MST_PATH}/src/mzParser/RAMPface.cpp
  ${MST_PATH}/src/mzParser/saxhandler.cpp
  ${MST_PATH}/src/mzParser/saxmzmlhandler.cpp
  ${MST_PATH}/src/mzParser/saxmzxmlhandler.cpp
)

add_library(
  mstoolkitlite
  STATIC
  ${ZLIB}
  ${EXPAT}
  ${MZPARSER}
  ${MSTOOLKIT}
)

target_include_directories(mstoolkitlite PUBLIC ${MSTOOLKIT_INCLUDE_DIRS})

if (UNIX)
  target_compile_options(mstoolkitlite PRIVATE -O3)
  target_compile_definitions(
    mstoolkitlite
    PRIVATE
    _LARGEFILE_SOURCE
    _FILE_OFFSET_BITS=64
    GCC
    HAVE_EXPAT_CONFIG_H
  )
elseif(MSVC)
  target_compile_definitions(
    mstoolkitlite
    PRIVATE
    _CRT_SECURE_NO_WARNINGS
    XML_STATIC
  )
endif(UNIX)

#############################################################################
# Build Thermo RAW file reader (Windows only)
#
# Comet's RAWReader.cpp reads .raw files through Thermo's RawFileReader .NET
# assemblies, so it has to be compiled as C++/CLI (/clr). It is kept in its
# own library so the rest of MSToolkit stays plain native code. The crux
# executable must link both mstoolkitlite and mstoolkitraw.
###############################################################################

if (MSVC)
  if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    message(
      FATAL_ERROR
      "Comet's Thermo RAW reader is C++/CLI and requires a Visual Studio generator."
    )
  endif()

  # Compile against the same RawFileReader assemblies that are shipped with
  # crux, since the CLR binds to the exact assembly version used at compile
  # time. ProteoWizard installs them in lib/ when vendor libraries are
  # included; otherwise use the copies from Comet's local NuGet packages.
  set(THERMO_DLL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
  if (NOT EXISTS ${THERMO_DLL_DIR}/ThermoFisher.CommonCore.RawFileReader.dll)
    set(THERMO_VERSION 5.0.0.93)
    set(THERMO_NUGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/RealtimeSearch/ThermoNuGet)
    set(THERMO_EXTRACT_DIR ${CMAKE_CURRENT_BINARY_DIR}/ThermoNuGet)
    foreach(thermo_pkg Data RawFileReader)
      file(MAKE_DIRECTORY ${THERMO_EXTRACT_DIR}/${thermo_pkg})
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf
          ${THERMO_NUGET_DIR}/ThermoFisher.CommonCore.${thermo_pkg}.${THERMO_VERSION}.nupkg
        WORKING_DIRECTORY ${THERMO_EXTRACT_DIR}/${thermo_pkg}
      )
      install(
        FILES ${THERMO_EXTRACT_DIR}/${thermo_pkg}/lib/netstandard2.0/ThermoFisher.CommonCore.${thermo_pkg}.dll
        DESTINATION lib
      )
    endforeach()
    set(THERMO_DATA_DLL ${THERMO_EXTRACT_DIR}/Data/lib/netstandard2.0/ThermoFisher.CommonCore.Data.dll)
    set(THERMO_RAW_DLL ${THERMO_EXTRACT_DIR}/RawFileReader/lib/netstandard2.0/ThermoFisher.CommonCore.RawFileReader.dll)
  else()
    set(THERMO_DATA_DLL ${THERMO_DLL_DIR}/ThermoFisher.CommonCore.Data.dll)
    set(THERMO_RAW_DLL ${THERMO_DLL_DIR}/ThermoFisher.CommonCore.RawFileReader.dll)
  endif()

  # /RTC run-time checks are incompatible with /clr
  string(REGEX REPLACE "/RTC[1csu]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

  add_library(
    mstoolkitraw
    STATIC
    ${MST_PATH}/src/MSToolkit/RAWReader.cpp
  )
  target_include_directories(mstoolkitraw PRIVATE ${MSTOOLKIT_INCLUDE_DIRS})
  target_compile_definitions(
    mstoolkitraw
    PRIVATE
    _CRT_SECURE_NO_WARNINGS
    XML_STATIC
  )
  # Comet's own project found that the assembly references alone did not
  # always produce /FU switches, so they are forced explicitly as well.
  target_compile_options(
    mstoolkitraw
    PRIVATE
    /EHa
    "/FU${THERMO_DATA_DLL}"
    "/FU${THERMO_RAW_DLL}"
  )
  set_target_properties(
    mstoolkitraw
    PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
    DOTNET_TARGET_FRAMEWORK_VERSION v4.7.2
    VS_DOTNET_REFERENCE_ThermoFisher.CommonCore.Data ${THERMO_DATA_DLL}
    VS_DOTNET_REFERENCE_ThermoFisher.CommonCore.RawFileReader ${THERMO_RAW_DLL}
  )
endif(MSVC)

#############################################################################
# Build AScorePro static library (headers in AScorePro/include, sources in AScorePro/)
###############################################################################

# collect AScorePro sources; replace GLOB with explicit list if preferred
file(GLOB ASCOREPRO_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/*.cc
)

if (ASCOREPRO_SRCS)
  add_library(
    ascorepro
    STATIC
    ${ASCOREPRO_SRCS}
  )

  # Force output base name to 'ascorepro' so on Unix we get libascorepro.a
#  set_target_properties(ascorepro PROPERTIES OUTPUT_NAME ascorepro)

  # Make headers available to dependents while building
  target_include_directories(
    ascorepro
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/include
      ${CMAKE_CURRENT_BINARY_DIR}/AScorePro/include
  )

  if (UNIX)
    if (CMAKE_BUILD_TYPE MATCHES "Release")
      target_compile_options(
        ascorepro
        PUBLIC
        -O3 -fpermissive -Wall -Wextra -Wno-write-strings -static -std=c++17 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DGCC -DCRUX -I.
      )
    else()
      target_compile_options(
        ascorepro
        PUBLIC
        -g -O0 -fpermissive -Wall -Wextra -Wno-write-strings -static -std=c++17 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DGCC -DCRUX -I.
      )
    endif()
  elseif(WIN32 AND NOT CYGWIN)
    target_compile_definitions(
      ascorepro
      PUBLIC
      _CRT_SECURE_NO_WARNINGS
      CRUX
    )
  endif()

  # Install AScorePro artifacts (headers and library) into the usual locations
  # ARCHIVE -> static libraries (.a) on Unix
  install(TARGETS ascorepro
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

  # Copy the contents of AScorePro/include into include/AScorePro (no extra 'include' level)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/include/ DESTINATION include/AScorePro FILES_MATCHING PATTERN "*.h")
endif()

#############################################################################
# COMPILE COMETSEARCH
###############################################################################

set(
  COMETSEARCH
  CometSearch/CombinatoricsUtils.cpp
  CometSearch/CometAlignment.cpp
  CometSearch/CometFragmentIndex.cpp
  CometSearch/CometInterfaces.cpp
  CometSearch/CometMassSpecUtils.cpp
  CometSearch/CometModificationsPermuter.cpp
  CometSearch/CometPeptideIndex.cpp
  CometSearch/CometPostAnalysis.cpp
  CometSearch/CometPreprocess.cpp
  CometSearch/CometSearch.cpp
  CometSearch/CometSearchManager.cpp
  CometSearch/CometSpecLib.cpp
  CometSearch/CometWriteMzIdentML.cpp
  CometSearch/CometWritePepXML.cpp
  CometSearch/CometWritePercolator.cpp
  CometSearch/CometWriteSqt.cpp
  CometSearch/CometWriteTxt.cpp
  CometSearch/Threading.cpp
  CometSearch/threading/SearchMemoryPool.cpp
  CometSearch/search/FastaStrategy.cpp
  CometSearch/search/FiStrategy.cpp
  CometSearch/search/PiStrategy.cpp
  CometSearch/search/Pipeline.cpp
  CometSearch/search/SearchUtils.cpp
)

add_library(
  cometsearch
  STATIC
  ${COMETSEARCH}
)

# Comet and Kojak both define a class named Threading; rename Comet's so
# both libraries can be linked into crux. src/CMakeLists.txt applies the
# same rename to the Crux sources that include Comet headers.
target_compile_definitions(cometsearch PUBLIC Threading=CometThreading)

# Make sure cometsearch can find AScorePro include files and MSToolkit headers
target_include_directories(
  cometsearch
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/CometSearch
    ${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/include
    ${CMAKE_CURRENT_BINARY_DIR}/AScorePro/include
    ${MSTOOLKIT_INCLUDE_DIRS}
)

if (UNIX)
  if (CMAKE_BUILD_TYPE MATCHES "Release")
    target_compile_options(
      cometsearch
      PUBLIC
      -O3 -static -std=c++20 -fpermissive -Wall -Wextra -Wno-write-strings -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DGCC -D_NOSQLITE -D__int64=off64_t -I. -I${MST_PATH}/include -I${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/include
    )
  else()
    target_compile_options(
      cometsearch
      PUBLIC
      -O0 -static -std=c++20 -g -fpermissive -Wall -Wextra -Wno-write-strings -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DGCC -D_NOSQLITE -D__int64=off64_t -I. -I${MST_PATH}/include -I${CMAKE_CURRENT_SOURCE_DIR}/AScorePro/include
    )
  endif ()
elseif(MSVC)
  target_compile_features(cometsearch PUBLIC cxx_std_17)
  target_compile_definitions(
    cometsearch
    PUBLIC
    _HAS_STD_BYTE=0
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    CRUX
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif(UNIX)

# Link cometsearch against AScorePro if that lib was created
if (TARGET ascorepro)
  target_link_libraries(cometsearch PUBLIC ascorepro)
endif()

target_link_libraries(cometsearch PUBLIC mstoolkitlite)
if (TARGET mstoolkitraw)
  target_link_libraries(cometsearch PUBLIC mstoolkitraw)
endif()

################################################################################

# INSTALL cometsearch library and headers
install(TARGETS cometsearch
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# INSTALL MSToolkit libraries and headers. Crux, Kojak, Hardklor and the other
# tools expect the MSToolkit, expat and zlib headers in include/MSToolkit.
install(TARGETS mstoolkitlite
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
if (TARGET mstoolkitraw)
  install(TARGETS mstoolkitraw
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
endif()
install(DIRECTORY ${MST_PATH}/include/ DESTINATION include/MSToolkit FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "extern" EXCLUDE)
install(
  FILES
    ${EXPAT_PATH}/expat.h
    ${EXPAT_PATH}/expat_external.h
    ${ZLIB_PATH}/zlib.h
    ${ZLIB_PATH}/zconf.h
  DESTINATION include/MSToolkit
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/CometSearch/ DESTINATION include/CometSearch FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# Note: AScorePro headers and static library are installed above with ascorepro target.
# With the OUTPUT_NAME set to 'ascorepro', the installed static library on Unix will be libascorepro.a
# and it will be placed in the same lib/ directory as libcometsearch.a
