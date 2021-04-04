
cmake_minimum_required(VERSION 3.10.2)


project("live")

add_subdirectory(librtmp)
file(GLOB cpp_source *.cpp)
include_directories(${CMAKE_SOURCE_DIR}/x264/${ANDROID_ABI}/include)
include_directories(${CMAKE_SOURCE_DIR}/faac/${ANDROID_ABI}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/x264/${ANDROID_ABI}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/faac/${ANDROID_ABI}/lib")


add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${cpp_source})


target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        log
        rtmp
        x264
        faac)