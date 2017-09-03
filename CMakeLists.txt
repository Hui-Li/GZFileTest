cmake_minimum_required(VERSION 2.8)
project(GZFileTest)

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    link_libraries(${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

find_package(Boost COMPONENTS system iostreams)

if(NOT ${Boost_FOUND})
    SET(BOOST_ROOT ~/local) # default
    SET(Boost_NO_SYSTEM_PATHS ON) # force to use own build
    find_package(Boost COMPONENTS system iostreams REQUIRED)
endif(NOT ${Boost_FOUND})

if(Boost_FOUND)
    message(STATUS "Boost found")
    include_directories(${Boost_INCLUDE_DIR})
    link_libraries(${Boost_LIBRARIES})
endif(Boost_FOUND)

set(SOURCE_FILES main.cpp)
add_executable(GZFileTest ${SOURCE_FILES})
target_link_libraries(GZFileTest ${Boost_LIBRARIES})
target_link_libraries(GZFileTest ${ZLIB_LIBRARIES} )