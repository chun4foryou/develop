#cmake 최소 버전
cmake_minimum_required(VERSION 3.18.1)
 
#프로젝트 이름
project(dltest)
  
#CMake 옵션 처리
set(CMAKE_VERBOSE_MAKEFILE ON)
   
#컴파일 옵션 추가
add_compile_options(-std=c++11 -ggdb3 -O3 -Wall -fPIC)
#-Werror)
    
#define 추가
add_definitions(-D_REENTRANT)
     
#header include 경로
include_directories("./")
      
#라이브러리 링크 디렉토리 경로
link_directories()
       
enable_testing()
        
#cmake 변수 처리
file(GLOB main_src "./dl_main.cpp")
         
#라이브러리 생성
#set(OUTPUT_STATIC_LIB "hello")
#add_library(${OUTPUT_STATIC_LIB} STATIC ${all_srcs})

#실행 파일 생성
add_executable(${PROJECT_NAME} ${main_src})
#add_executable(test_smart ${main_src})

add_library(test_func SHARED test_func.cpp)

#특정 실행 파일에 적용될 라이브러리 링크추가
target_link_libraries(${PROJECT_NAME}
#    -Wl,--disable-new-dtags
    -Wl,--no-as-needed
    -Wl,--rpath,./
    dl
    pthread
)

set(CMAKE_INSTALL_PREFIX ./)

install(TARGETS ${PROJECT_NAME}
    COMPONENT full
    )

install(TARGETS ${PROJECT_NAME}
    COMPONENT full
    )

install(TARGETS ${PROJECT_NAME}
    COMPONENT test
    )


install(DIRECTORY "./lib"
    COMPONENT full
    DESTINATION lib
    )


#install(TARGETS test_smart_pointer
#    COMPONENT test 
#    RUNTIME DESTINATION bin
#)


set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME smart_pointer)
set(CPACK_PACKAGE_NAME smart_pointer)

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
include(CPack)
