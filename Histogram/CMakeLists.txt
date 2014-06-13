cmake_minimum_required(VERSION 2.8.8)
set(VTK_DIR "D:/VTK/VTK-build" CACHE PATH "VTK directory")

project(HistogramBarChart)
find_package(VTK 6.1 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})
add_executable(${PROJECT_NAME} HistogramBarChart.cpp)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
	)
endif()

project(HistogramXYPlot)
find_package(VTK 6.1 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})
add_executable(${PROJECT_NAME} HistogramXYPlot.cpp)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
	)
endif()

project(ImageAccumulateGreyscale)
find_package(VTK 6.1 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})
add_executable(${PROJECT_NAME} ImageAccumulateGreyscale.cpp)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
	)
endif()

project(volume_histogram)
find_package(VTK 6.1 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})
add_executable(${PROJECT_NAME} volume_histogram.cpp)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
	)
endif()

project(histogram)
find_package(VTK 6.1 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})
add_executable(${PROJECT_NAME} histogram.cpp)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

if(WIN32) # Check if we are on Windows
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy /d /f /y "\"${VTK_DIR}/bin/$<CONFIGURATION>/*.dll\"" "\"$<TARGET_FILE_DIR:${PROJECT_NAME}>\""
	)
endif()
