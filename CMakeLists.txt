
cmake_minimum_required (VERSION 2.6)
project (xrootd-prefs-plugins)

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

find_package( Xrootd REQUIRED )

if( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )
endif()

if( CMAKE_COMPILER_IS_GNUCC )
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror" )
endif()

include_directories ("${PROJECT_SOURCE_DIR}")
add_subdirectory(src)
add_subdirectory(pygeoip-database)
