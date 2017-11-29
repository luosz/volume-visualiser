Run volume-visualiser\VolumeRenderer\build\Release\VolumeVisualiser.exe to start the Volume Visualiser program.
Paths and filenames for static data sets are stored in the path.xml of the exe file's directory. Paths and filenames for time-varying data sets and transfer functions are stored in NaiveVortex.xml, StaticVortex.xml and DynamicVortex.xml.

2017-11-28 Update:
The source code has been compiled with Qt 5.9.3, VTK 8.0.1, Visual Studio 2017 and CMake 3.10.0.

The following CMake settings are used for compiling VTK:
set(VTK_DIR "C:/VTK/VTK-8.0.1/build" CACHE PATH "VTK directory")
set(QT_QMAKE_EXECUTABLE "C:/Qt/Qt5.9.3/5.9.3/msvc2017_64/bin/qmake.exe" CACHE PATH "Qt qmake.exe")
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.9.3/5.9.3/msvc2017_64" CACHE PATH "Qt directory")
set(VTK_QT_VERSION 5 CACHE STRING "Qt version")
set(VTK_NO_LIBRARY_VERSION ON CACHE BOOL "VTK_NO_LIBRARY_VERSION")
set(VTK_Group_Qt ON CACHE BOOL "VTK_Group_Qt")
set(Module_vtkGUISupportQt ON CACHE BOOL "Module_vtkGUISupportQt")
set(Module_vtkGUISupportQtOpenGL ON CACHE BOOL "Module_vtkGUISupportQtOpenGL")
set(Module_vtkRenderingQt ON CACHE BOOL "Module_vtkRenderingQt")
set(BUILD_TESTING OFF CACHE BOOL "BUILD_TESTING")
set(VTK_RENDERING_BACKEND_DEFAULT "OpenGL")

The following CMake settings are used for compiling the project:
set(VTK_DIR "C:/VTK/VTK-8.0.1/build" CACHE PATH "VTK directory")
set(QT_QMAKE_EXECUTABLE "C:/Qt/Qt5.9.3/5.9.3/msvc2017_64/bin/qmake.exe" CACHE PATH "Qt qmake.exe")
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.9.3/5.9.3/msvc2017_64" CACHE PATH "Qt directory")
