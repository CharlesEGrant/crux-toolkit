#include_directories(${PERCOLATOR_SOURCE_DIR}/src)
#link_directories(${PERCOLATOR_SOURCE_DIR}/src)
#link_directories(${PERCOLATOR_BINARY_DIR}/src)

#file(GLOB FIDO_SOURCES Set.cpp Vector.cpp Numerical.cpp Random.cpp BasicBigraph.cpp BasicGroupBigraph.cpp GroupPowerBigraph.cpp)

#add_executable(Fido ${FIDO_SOURCES})

#target_link_libraries(Fido perclibrary)

#install(TARGETS Fido EXPORT PERCOLATOR DESTINATION ./bin) # Important to use relative path here (used by CPack)!

#add_library(fido STATIC ${FIDO_SOURCES})

if(CRUX)
  ADD_DEFINITIONS(-DCRUX)
  include_directories(${PERCOLATOR_SOURCE_DIR}/src ${CRUX} ${CRUX}/src ${EXT_BINARY_DIR})
else(CRUX)
include_directories(${PERCOLATOR_SOURCE_DIR}/src)
endif(CRUX)
link_directories(${PERCOLATOR_SOURCE_DIR}/src)

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif(UNIX)
file(GLOB FIDO_SOURCES Set.cpp Vector.cpp Numerical.cpp Random.cpp BasicBigraph.cpp BasicGroupBigraph.cpp GroupPowerBigraph.cpp)
#add_library(fido ${FIDO_SOURCES})
add_library(fido STATIC ${FIDO_SOURCES})
