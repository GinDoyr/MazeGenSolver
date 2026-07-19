cmake_minimum_required(VERSION 3.16)

project(MazeGenSolver VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        maze.cpp
        maze.h
        maze_widget.cpp
        maze_widget.h
        maze_generator_worker.cpp
        maze_generator_worker.h
        generator.cpp
        generator.h
        solver.h
        generators/dfs_generator.cpp
        generators/dfs_generator.h
        generators/kruskal_generator.cpp
        generators/kruskal_generator.h
        generators/prim_generator.cpp
        generators/prim_generator.h
        solvers/bfs_solver.cpp
        solvers/bfs_solver.h
        solvers/astar_solver.cpp
        solvers/astar_solver.h
        solvers/dijkstra_solver.cpp
        solvers/dijkstra_solver.h
        resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MazeGenSolver
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        refwindow.h refwindow.cpp
        refwindow.h refwindow.cpp refwindow.ui
        infowindow.h infowindow.cpp infowindow.ui
        stats_entry.h

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MazeGenSolver APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MazeGenSolver SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MazeGenSolver
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(MazeGenSolver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(MazeGenSolver PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MazeGenSolver)
endif()
set_target_properties(MazeGenSolver PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MazeGenSolver
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MazeGenSolver)
endif()
