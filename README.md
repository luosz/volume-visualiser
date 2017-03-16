# README #

Volume Visualiser (https://bitbucket.org/lsz/volume-visualiser) is an enhanced version of Volume Renderer (https://bitbucket.org/lsz/volume-renderer), which is a volume rendering program based on VTK and Qt.
Volume Visualiser can handle multiple volume data sets as well as time-varying volume data sets.
It uses the vtkSlicerGPURayCastVolumeMapper from Slicer (https://www.slicer.org/) to render volume data and widgets from CTK (www.commontk.org) to display the properties of volume data.

** 2017-03-16 Update: The project has been updated and successfully built with CMake 3.8, VTK 7.1, Qt 5.8 and Visual Studio 2015. **

** 2015-09-14 Update: The CTK widgets in this project are only a small subset of the CTK library downloaded on 14 September 2015 from https://github.com/commontk/CTK **

** CMake settings for this project **  
set(VTK_DIR "D:/VTK/VTK-7.1.0.rc2/build" CACHE PATH "VTK directory")  
set(QT_QMAKE_EXECUTABLE "D:/Qt/Qt5.7.0/5.7/msvc2015/bin/qmake.exe" CACHE PATH "Qt qmake.exe")  
set(CMAKE_PREFIX_PATH "D:/Qt/Qt5.7.0/5.7/msvc2015" CACHE PATH "Qt directory")  

** CMake settings for building VTK **  
set(VTK_DIR "D:/VTK/VTK-7.1.0.rc2/build" CACHE PATH "VTK directory")  
set(QT_QMAKE_EXECUTABLE "D:/Qt/Qt5.7.0/5.7/msvc2015/bin/qmake.exe" CACHE PATH "Qt qmake.exe")  
set(CMAKE_PREFIX_PATH "D:/Qt/Qt5.7.0/5.7/msvc2015" CACHE PATH "Qt directory")  
set(VTK_QT_VERSION 5 CACHE STRING "Qt version")  
set(VTK_NO_LIBRARY_VERSION ON CACHE BOOL "VTK_NO_LIBRARY_VERSION")  
set(VTK_Group_Qt ON CACHE BOOL "VTK_Group_Qt")  
set(Module_vtkGUISupportQt ON CACHE BOOL "Module_vtkGUISupportQt")  
set(Module_vtkGUISupportQtOpenGL ON CACHE BOOL "Module_vtkGUISupportQtOpenGL")  
set(Module_vtkRenderingQt ON CACHE BOOL "Module_vtkRenderingQt")  
set(BUILD_TESTING OFF CACHE BOOL "BUILD_TESTING")  
set(VTK_RENDERING_BACKEND_DEFAULT "OpenGL")  

### Screenshot ###
* ![volume_visualizer.png](https://bitbucket.org/repo/d5Xrn6/images/2362565755-volume_visualizer.png)