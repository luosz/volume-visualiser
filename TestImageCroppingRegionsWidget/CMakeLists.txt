cmake_minimum_required(VERSION 2.8.8)
set(VTK_DIR "D:/VTK/VTK-build" CACHE PATH "VTK directory")

project(TestImageCroppingRegionsWidget)
find_package(VTK 6.1 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})
add_executable(${PROJECT_NAME} TestImageCroppingRegionsWidget.cxx)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
	)
endif()
