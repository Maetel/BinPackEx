######################################################
## Bin packing example                              ##
##  * Author : Wonjun Hwang                         ##
##  * E-mail : iamjam4944@gmail.com                 ##
######################################################

cmake_minimum_required(VERSION 3.0)
set(PROJECT_TITLE "BinPackEx" CACHE STRING "Project Title")
project(${PROJECT_TITLE})
set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${CUR_DIR}/src CACHE STRING "Source directory")

######################################################
## 3rd party
set(3RD_PARTY "${CUR_DIR}/3rdparty" CACHE STRING "3rd party dir")
set(CUSTOM_MODULE_PATH "${3RD_PARTY}/OpenCV")
set(CMAKE_MODULE_PATH "${CUSTOM_MODULE_PATH};${CMAKE_MODULE_PATH}")

######################################################
## RectangleBinPack
## https://github.com/juj/RectangleBinPack.git

macro(link_RectangleBinPack DST_PROJ)
    set(RBP_PROJECT "RectangleBinPack" CACHE STRING "RectangleBinPack_Project_Name")
    project(${RBP_PROJECT})
    set(RBP_DIR ${3RD_PARTY}/RectangleBinPack)
    set(RBP_BASE_SRC "${RBP_DIR}/Rect.cpp")
    set(RBP_BASE_HEADER "${RBP_DIR}/Rect.h")
    set(RBP_ALG_TYPE "GuillotineBinPack" CACHE STRING "RBP_ALG_TYPE")
    set(RBP_SRCS "${RBP_BASE_SRC};${RBP_DIR}/${RBP_ALG_TYPE}.cpp")
    set(RBP_HEADERS "${RBP_BASE_HEADER};${RBP_DIR}/${RBP_ALG_TYPE}.h")
    add_library(${RBP_PROJECT} STATIC "${RBP_SRCS};${RBP_HEADERS}")
    target_link_libraries(${DST_PROJ} ${RBP_PROJECT})
    target_include_directories(${DST_PROJ} PUBLIC ${RBP_DIR})
    message("${DST_PROJ} - RectangleBinPack (${RBP_ALG_TYPE}) linked")
endmacro()
## !RectangleBinPack
######################################################

######################################################
## OpenCV
macro(link_opencv DST_PROJ)
    ##find package does not work here, so designate the paths manually
    #find_package(OpenCV REQUIRED)
    #target_link_libraries(${DST_PROJ} ${OpenCV_LIBS} )
    
    set(OPENCV_DIR "C:/work/srcs/opencv")
    set(OPENCV_VERSION "4.1.2")
    set(OPENCV_BUILD "${OPENCV_DIR}/opencv-${OPENCV_VERSION}/build")
    set(OPENCV_INCLUDE "${OPENCV_BUILD}/include")
    set(OPENCV_LIB_DIR "${OPENCV_BUILD}/x64/vc15/lib")
    set(OPENCV_LIB_OPTIMIZED "${OPENCV_LIB_DIR}/opencv_world412.lib")
    set(OPENCV_LIB_DEBUG "${OPENCV_LIB_DIR}/opencv_world412d.lib")
    
    include_directories(${DST_PROJ} ${OPENCV_INCLUDE})
    target_link_libraries(${DST_PROJ}
        optimized ${OPENCV_LIB_OPTIMIZED}
        debug ${OPENCV_LIB_DEBUG}
        )
    message("${DST_PROJ} - OpenCV world Linked")
endmacro()
## OpenCV
######################################################
## !3rd party
######################################################

######################################################
## main project
FILE(GLOB MAIN_SRCS ${SRC_DIR}/*.cpp ${SRC_DIR}/*.h)
add_executable(${PROJECT_TITLE} ${MAIN_SRCS})
link_RectangleBinPack(${PROJECT_TITLE})
link_opencv(${PROJECT_TITLE})
## !main project
######################################################
