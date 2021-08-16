#cmake 최소 버전
cmake_minimum_required(VERSION 3.18.1)
 
#프로젝트 이름
project(test_cmake)
  
#CMake 옵션 처리
set(CMAKE_VERBOSE_MAKEFILE ON)
   
#컴파일 옵션 추가
add_compile_options(-std=c++11 -ggdb3 -O3 -Wall -fPIC -Werror)
    
#define 추가
add_definitions(-D_REENTRANT)

#라이브러리 추가 
add_subdirectory(libs)

set(CMAKE_MODULE_PATH ./cmake)

find_package(hello REQUIRED COMPONENT full)

#header include 경로
include_directories(
        "./include"
        ${hello_INCLUDE_DIRS}
        )
      
#라이브러리 링크 디렉토리 경로
link_directories(
        "./"
        ${hello_LIB_DIRS}
        )
       
enable_testing()
        
#cmake 변수 처리
file(GLOB main_src "src/main.cpp")

#실행 파일 생성
add_executable(${PROJECT_NAME} ${main_src})
#add_library(test_func SHARED test_func.cpp)

#특정 실행 파일에 적용될 라이브러리 링크추가
target_link_libraries(${PROJECT_NAME}
#    -Wl,--no-as-needed
    -Wl,--rpath,$ORIGIN/../lib
    dl
    pthread
    ${hello_LIBRARIES} 
)

set(CMAKE_INSTALL_PREFIX ./)

install(TARGETS ${PROJECT_NAME}
    COMPONENT full
    )

install(TARGETS ${PROJECT_NAME}
    COMPONENT test
    )

# install(TARGETS <target1> <target2> ...
#        RUNTIME DESTINATION <executable-dir>
#        LIBRARY DESTINATION <shared-lib-dir>
#        ARCHIVE DESTINATION <static-lib-dir>
#        PUBLIC_HEADER DESTINATION <header-dir>
#        )

install(FILES ${CMAKE_SOURCE_DIR}/libs/hello/libhello-static.a ${CMAKE_SOURCE_DIR}/libs/hello/libhello-shared.so
    COMPONENT full
    DESTINATION lib
    DESTINATION lib
    )

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
include(CPack)
