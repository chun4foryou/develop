#cmake 최소 버전
cmake_minimum_required(VERSION 3.18.1)

project(test_cmake)
 
#CMake 옵션 처리
set(CMAKE_VERBOSE_MAKEFILE ON)
   
#컴파일 옵션 추가
add_compile_options(-std=c++11 -ggdb3 -O3 -Wall -fPIC -Werror)
    
#define 추가
add_definitions(-D_REENTRANT)
     
#header include 경로
include_directories("./include")
      
enable_testing()
        
#cmake 변수 처리
file(GLOB src "src/*")
         
#라이브러리 생성
set(OUTPUT_STATIC_LIB "hello-static")
add_library(${OUTPUT_STATIC_LIB} STATIC ${src})

set(OUTPUT_SHARED_LIB "hello-shared")
add_library(${OUTPUT_SHARED_LIB} SHARED ${src})


#라이브러리 설치 경로
set(CMAKE_INSTALL_PREFIX ./)

# install(TARGETS <target1> <target2> ...
#        RUNTIME DESTINATION <executable-dir>
#        LIBRARY DESTINATION <shared-lib-dir>
#        ARCHIVE DESTINATION <static-lib-dir>
#        PUBLIC_HEADER DESTINATION <header-dir>
#        )

#install(TARGETS ${OUTPUT_STATIC_LIB} ${OUTPUT_SHARED_LIB}
#        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/libs/lib 
#        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/libs/lib 
#        )

