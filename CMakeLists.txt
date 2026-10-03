cmake_minimum_required(VERSION 3.20)
project(mingw64_dart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    foreach(_cand "$ENV{MINGW_PREFIX}" "D:/MSYS64/mingw64" "C:/msys64/mingw64" "D:/msys64/mingw64" "C:/MSYS64/mingw64")
        if(_cand AND EXISTS "${_cand}/include/curl/curl.h")
            list(PREPEND CMAKE_PREFIX_PATH "${_cand}")
            break()
        endif()
    endforeach()
endif()

find_path(CURL_INCLUDE_DIR curl/curl.h REQUIRED)
find_library(CURL_LIBRARY NAMES curl libcurl REQUIRED)

add_library(core STATIC
    src/API/google_storage.cpp
    src/API/release.cpp
    src/Utils/files.cpp
    src/Utils/json.cpp
    src/Utils/strings.cpp
)

target_include_directories(core PUBLIC include ${CURL_INCLUDE_DIR})
target_link_libraries(core PUBLIC ${CURL_LIBRARY})

add_executable(release_client src/release_client.cpp)
target_link_libraries(release_client PRIVATE core)
