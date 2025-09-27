cmake_minimum_required(VERSION 3.16)

project(Inscribe VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(INSCRIBE_WITH_QTTEST "Build QtTest integration tests" OFF)
option(INSCRIBE_WITH_QSCINTILLA_SYSTEM "Use system QScintilla if available" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# Qt PDF (Qt6 only)
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Pdf PdfWidgets)
endif()

# QScintilla (optional but recommended)
set(QSCINTILLA_FOUND FALSE)
set(QSCINTILLA_TARGET "")
if(INSCRIBE_WITH_QSCINTILLA_SYSTEM)
    # Try common package names
    find_package(QScintilla CONFIG QUIET)
    if(QScintilla_FOUND)
        set(QSCINTILLA_FOUND TRUE)
        set(QSCINTILLA_TARGET QScintilla::Qsci)
    else()
        find_package(QScintillaQt6 CONFIG QUIET)
        if(QScintillaQt6_FOUND)
            set(QSCINTILLA_FOUND TRUE)
            # Arch Linux often exposes this target
            if(TARGET QScintillaQt6::Qsci)
                set(QSCINTILLA_TARGET QScintillaQt6::Qsci)
            endif()
        endif()
    endif()

    if(QSCINTILLA_FOUND AND QSCINTILLA_TARGET)
        message(STATUS "QScintilla enabled with target: ${QSCINTILLA_TARGET}")
        add_compile_definitions(HAVE_QSCINTILLA)
    else()
        message(WARNING "QScintilla not found; building without code editor highlighting. Install qscintilla-qt6 and ensure CMake config is visible.")
    endif()
endif()

set(TS_FILES
    Inscribe_ru_RU.ts
)

# ------------- Core library -------------
add_library(inscribe_core STATIC
    core/build/BuildManager.cpp
    core/build/BuildManager.h
    core/compiler/ICompiler.h
    core/compiler/TectonicCompiler.cpp
    core/compiler/TectonicCompiler.h
    core/logs/LogParser.cpp
    core/logs/LogParser.h
    core/project/ProjectModel.cpp
    core/project/ProjectModel.h
    core/settings/SettingsService.cpp
    core/settings/SettingsService.h
)

target_include_directories(inscribe_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(inscribe_core
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
)

# ------------- UI library -------------
add_library(inscribe_ui STATIC
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    ui/StartDialog.cpp
    ui/StartDialog.h
    ui/EditorWidget.cpp
    ui/EditorWidget.h
    ui/PdfPreviewWidget.cpp
    ui/PdfPreviewWidget.h
    ui/LogPane.cpp
    ui/LogPane.h
)

target_link_libraries(inscribe_ui
    PUBLIC
        inscribe_core
        Qt${QT_VERSION_MAJOR}::Widgets
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(inscribe_ui PUBLIC Qt6::Pdf Qt6::PdfWidgets)
endif()

if(QSCINTILLA_FOUND AND QSCINTILLA_TARGET)
    target_link_libraries(inscribe_ui PUBLIC ${QSCINTILLA_TARGET})
else()
    # Fallback: manual discovery (common on Arch where CMake config may be missing)
    find_path(QSCINTILLA_INCLUDE_DIR
        NAMES Qsci/qsciscintilla.h
        PATHS /usr/include/qt6 /usr/include/qt /usr/include
    )
    find_library(QSCINTILLA_LIBRARY
        NAMES qscintilla2_qt6 qscintilla2-qt6 qscintilla2
        PATHS /usr/lib /usr/lib64
    )
    if(QSCINTILLA_INCLUDE_DIR AND QSCINTILLA_LIBRARY)
        message(STATUS "QScintilla fallback enabled: ${QSCINTILLA_LIBRARY}")
        target_include_directories(inscribe_ui PUBLIC ${QSCINTILLA_INCLUDE_DIR})
        target_link_libraries(inscribe_ui PUBLIC ${QSCINTILLA_LIBRARY})
        add_compile_definitions(HAVE_QSCINTILLA)
    endif()
endif()

# ------------- Application executable -------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Inscribe
        MANUAL_FINALIZATION
        app/main.cpp
        ${TS_FILES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    add_executable(Inscribe
        app/main.cpp
        ${TS_FILES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

set_target_properties(Inscribe PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(Inscribe
    PRIVATE
        inscribe_ui
        Qt${QT_VERSION_MAJOR}::Widgets
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Inscribe)
endif()

include(GNUInstallDirs)
install(TARGETS Inscribe
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
