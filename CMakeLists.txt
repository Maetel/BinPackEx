######################################################
## Bin packing example                              ##
##  * Author : Wonjun Hwang                         ##
##  * E-mail : iamjam4944@gmail.com                 ##
######################################################

#prerequisites
cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_TITLE "BinPackEx" CACHE STRING "Project Title")
project(${PROJECT_TITLE} LANGUAGES CXX)
set(CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${CUR_DIR}/src CACHE STRING "Source directory")

######################################################
## 3rd party
set(3RD_PARTY "${CUR_DIR}/3rdparty" CACHE STRING "3rd party dir")
set(CUSTOM_MODULE_PATH)
set(CMAKE_MODULE_PATH "${CUSTOM_MODULE_PATH};${CMAKE_MODULE_PATH}")

######################################################
## RectangleBinPack
## https://github.com/juj/RectangleBinPack.git

macro(link_RectangleBinPack DST_PROJ)
    set(RBP_PROJECT "RectangleBinPack" CACHE STRING "RectangleBinPack_Project_Name")
    #project(${RBP_PROJECT})
    set(RBP_DIR ${3RD_PARTY}/RectangleBinPack)
    set(RBP_BASE_SRC "${RBP_DIR}/Rect.cpp")
    set(RBP_BASE_HEADER "${RBP_DIR}/Rect.h")
    set(RBP_ALG_TYPE "GuillotineBinPack" CACHE STRING "RBP_ALG_TYPE")
    set(RBP_SRCS "${RBP_BASE_SRC};${RBP_DIR}/${RBP_ALG_TYPE}.cpp")
    set(RBP_HEADERS "${RBP_BASE_HEADER};${RBP_DIR}/${RBP_ALG_TYPE}.h")
    add_library(${RBP_PROJECT} STATIC "${RBP_SRCS};${RBP_HEADERS}")
    message("RBP_DIR ${RBP_DIR}")
    target_link_libraries(${DST_PROJ} PUBLIC
        debug ${RBP_PROJECT}
        optimized ${RBP_PROJECT}
        )
    target_include_directories(${DST_PROJ} PUBLIC ${RBP_DIR})
    message("${DST_PROJ} - RectangleBinPack (${RBP_ALG_TYPE}) linked")
endmacro()
## !RectangleBinPack
######################################################

## !3rd party
######################################################

######################################################
## main project type

macro(simple_main)
    FILE(GLOB MAIN_SRCS ${SRC_DIR}/*.cpp ${SRC_DIR}/*.h)
    add_executable(${PROJECT_TITLE} ${MAIN_SRCS})
    link_RectangleBinPack(${PROJECT_TITLE})
    link_opencv(${PROJECT_TITLE})
endmacro(simple_main)

macro(qt_main QT_MODULE_STRING)

    set(QT5_VERSION "5.12.5")
    set(QT5_CMAKE_PATH C:/Qt/${QT5_VERSION}/msvc2017_64/lib/cmake)
    set(QT5_MODUL_PATH ${QT5_CMAKE_PATH})
    set(Qt5_DIR "${QT5_MODUL_PATH}/Qt5")

    FILE(GLOB project_headers "${SRC_DIR}/*.h" "${SRC_DIR}/*.hpp")
    FILE(GLOB project_srcs "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")

    string(REPLACE " " ";" QT_MODULE_LIST ${QT_MODULE_STRING})
    find_package(Qt5 REQUIRED COMPONENTS ${QT_MODULE_LIST})

    qt5_wrap_cpp(project_sources_moc ${project_headers})

    add_executable(${PROJECT_TITLE} 
        ${project_headers}
        ${project_srcs}
        ${project_sources_moc}
        )

    set(MODS "")
    foreach(MOD ${QT_MODULE_LIST})
        set(MODS "${MODS};Qt5::${MOD}")
    endforeach()
    message("MODS ${MODS}")
    target_link_libraries(${PROJECT_TITLE} PUBLIC ${MODS})    

    message("Qt5 Project : ${PROJECT_TITLE} added")

    #link_RectangleBinPack(${PROJECT_TITLE})
    #link_opencv(${PROJECT_TITLE})

endmacro(qt_main)

macro(link_qt DST_PROJ MODULE_STRING)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    #set basic infos
    set(QT5_VERSION "5.12.5")
    set(QT5_CMAKE_PATH C:/Qt/${QT5_VERSION}/msvc2017_64/lib/cmake)
    set(QT5_MODUL_PATH ${QT5_CMAKE_PATH})
    set(Qt5_DIR "${QT5_MODUL_PATH}/Qt5")

    string(REPLACE " " ";" MODULE_LIST ${MODULE_STRING})
    find_package(Qt5 REQUIRED COMPONENTS ${MODULE_LIST})
    
    set(MODS "")
    foreach(MOD ${MODULE_LIST})
        set(MODS "${MODS};Qt5::${MOD}")
    endforeach()
    message("MODS ${MODS}")
    target_link_libraries(${DST_PROJ} PUBLIC ${MODS})

endmacro()

macro(link_eigen)
    #Eigen
    set(EIGEN_VERSION "3.3.7")
    set(EIGEN_SRC "${3RD_PARTY}/eigen/${EIGEN_VERSION}")
    include_directories(${PROJECT_TITLE} "${EIGEN_SRC}")
endmacro()


## !main project
######################################################

#simple_main()
#link_qt(${PROJECT_TITLE} "Core Widgets Gui")

qt_main("Core Widgets Gui")
link_eigen()
link_RectangleBinPack(${PROJECT_TITLE})