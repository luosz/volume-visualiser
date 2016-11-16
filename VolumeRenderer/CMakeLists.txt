cmake_minimum_required(VERSION 2.8.8)

set(VTK_DIR "C:/VTK/VTK-6.3.0/build" CACHE PATH "VTK directory")
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.5.1/5.5/msvc2013" CACHE PATH "CMAKE_PREFIX_PATH")
set(QT_QMAKE_EXECUTABLE "C:/Qt/Qt5.5.1/5.5/msvc2013/bin/qmake.exe" CACHE PATH "QT_QMAKE_EXECUTABLE")
set(VTK_QT_VERSION 5 CACHE PATH "VTK_QT_VERSION")

# CTK paths
set(CTK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CTK" CACHE PATH "CTK directory")
set(CTK_Libs_Core_DIR "${CTK_DIR}/Libs/Core")
set(CTK_Libs_Widgets_DIR "${CTK_DIR}/Libs/Widgets")
set(CTK_Libs_Visualization_VTK_Core_DIR "${CTK_DIR}/Libs/Visualization/VTK/Core")
set(CTK_Libs_Visualization_VTK_Widgets_DIR "${CTK_DIR}/Libs/Visualization/VTK/Widgets")
file(GLOB CTK_header_files
    ${CTK_Libs_Core_DIR}/*.h
    ${CTK_Libs_Widgets_DIR}/*.h
    ${CTK_Libs_Visualization_VTK_Core_DIR}/*.h
    ${CTK_Libs_Visualization_VTK_Widgets_DIR}/*.h
)
file(GLOB CTK_source_files
    ${CTK_Libs_Core_DIR}/*.h
    ${CTK_Libs_Core_DIR}/*.cpp
    ${CTK_Libs_Widgets_DIR}/*.h
    ${CTK_Libs_Widgets_DIR}/*.cpp
    ${CTK_Libs_Visualization_VTK_Core_DIR}/*.h
    ${CTK_Libs_Visualization_VTK_Core_DIR}/*.cpp
    ${CTK_Libs_Visualization_VTK_Widgets_DIR}/*.h
    ${CTK_Libs_Visualization_VTK_Widgets_DIR}/*.cpp
)
file(GLOB CTK_qrc_files
    ${CTK_Libs_Widgets_DIR}/Resources/*.qrc
    ${CTK_Libs_Visualization_VTK_Widgets_DIR}/Resources/*.qrc
)
file(GLOB CTK_ui_files
    ${CTK_Libs_Widgets_DIR}/Resources/UI/*.ui
    ${CTK_Libs_Visualization_VTK_Widgets_DIR}/Resources/UI/*.ui
)

project(VolumeVisualizer)

# VTK 6.x is required
find_package(VTK 6.0 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
## Instruct CMake to run moc automatically when needed.
#set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5OpenGL)

# Add the include directories for the Qt 5 Widgets module to the compile lines.
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# Use the compile definitions defined in the Qt 5 Widgets module
add_definitions(${Qt5Widgets_DEFINITIONS})

# Add compiler flags for building executables (-fPIE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# CTK
source_group(CTK FILES ${CTK_header_files} ${CTK_source_files})
message(STATUS "CTK header files: " ${CTK_header_files})
message(STATUS "CTK source files: " ${CTK_source_files})
QT5_WRAP_CPP(CTK_moc_source_files ${CTK_header_files})
message(STATUS "CTK moc source files: " ${CTK_moc_source_files})
source_group(CTK_moc FILES ${CTK_moc_source_files})
QT5_WRAP_UI(CTK_ui_header_files ${CTK_ui_files})
message(STATUS "CTK ui header files: " ${CTK_ui_header_files})
QT5_ADD_RESOURCES(CTK_qrc_complied ${CTK_qrc_files})
source_group(CTK_ui FILES ${CTK_ui_header_files} ${CTK_ui_files} ${CTK_qrc_files} ${CTK_qrc_complied})

# Slicer
set(Slicer_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Slicer/Modules/Loadable/VolumeRendering/VolumeRenderingReplacements)
file(GLOB Slicer_files
    ${Slicer_DIR}/*.h
	${Slicer_DIR}/*.cxx
)
source_group(Slicer FILES ${Slicer_files})

# TinyXML-2
set(tinyxml2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
set(tinyxml2_files ${tinyxml2_DIR}/tinyxml2.h ${tinyxml2_DIR}/tinyxml2.cpp)
source_group(tinyxml2 FILES ${tinyxml2_files})

# nvMath
set(nvMath_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nvMath)
file(GLOB nvMath_files
    ${nvMath_DIR}/*.h
)
source_group(nvMath FILES ${nvMath_files})

# cereal
set(cereal_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cereal)
file(GLOB cereal_files
    ${cereal_DIR}/cereal/*.hpp
    ${cereal_DIR}/cereal/archives/*.hpp
    ${cereal_DIR}/cereal/details/*.hpp
    ${cereal_DIR}/cereal/types/*.hpp
    ${cereal_DIR}/cereal/external/*.hpp
    ${cereal_DIR}/cereal/external/rapidxml/*.hpp
    ${cereal_DIR}/cereal/external/rapidjson/*.hpp
    ${cereal_DIR}/cereal/external/rapidjson/internal/*.hpp
 )
source_group(cereal FILES ${cereal_files})

# volume
set(volume_files voxel_utility.h lh_histogram.h K_Means_PP_Generic.h volume_utility.h)
source_group(volume FILES ${volume_files})

# Qt and project source code
#set(Project_source_files main.cpp mainwindow.cpp)
#set(Project_header_files mainwindow.h transfer_function_xml.h)
file(GLOB Project_source_files
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
file(GLOB Project_header_files
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
QT5_WRAP_CPP(Project_moc_source_files ${Project_header_files})
#set(Project_ui_files mainwindow.ui)
file(GLOB Project_ui_files
	${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)
QT5_WRAP_UI(Project_ui_header_files ${Project_ui_files})
source_group(ui FILES ${Project_ui_files} ${Project_ui_header_files})
source_group(moc FILES ${Project_moc_source_files})
#SET(Project_qrc_files YourProject.qrc)
#QT5_ADD_RESOURCES(Project_qrc_compiled ${Project_qrc_files})

message(STATUS "Project_source_files: " ${Project_source_files})
message(STATUS "Project_header_files: " ${Project_header_files})
message(STATUS "Project_moc_source_files: " ${Project_moc_source_files}) 
message(STATUS "Project_ui_files: " ${Project_ui_files}) 
message(STATUS "Project_ui_header_files: " ${Project_ui_header_files}) 

include_directories(${Project_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${CTK_Libs_Core_DIR} ${CTK_Libs_Widgets_DIR} ${CTK_Libs_Visualization_VTK_Core_DIR} ${CTK_Libs_Visualization_VTK_Widgets_DIR}
	${tinyxml2_DIR}
	${nvMath_DIR}
	${Slicer_DIR}
	${cereal_DIR}
)

add_executable(${PROJECT_NAME} ${Project_source_files} ${Project_ui_header_files}
	${Project_header_files} ${Project_moc_source_files}
	#${Project_qrc_compiled}
	${CTK_header_files} ${CTK_source_files} ${CTK_moc_source_files} ${CTK_ui_header_files} ${CTK_qrc_complied}
	${tinyxml2_files}
	${nvMath_files}
	${volume_files}
	${Slicer_files}
	${cereal_files}
)

qt5_use_modules(${PROJECT_NAME} Widgets OpenGL)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND xcopy /d /f /y "\"${Qt5Widgets_DIR}/../../../bin/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
    )
endif()
