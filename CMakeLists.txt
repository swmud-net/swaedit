cmake_minimum_required(VERSION 3.16)
project(swaedit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Xml)
find_package(OpenGL REQUIRED)
find_package(LibXml2 REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/XmlIO.cpp
    src/core/Renumberer.cpp
    src/gui/MainWindow.cpp
    src/gui/FlagsWidget.cpp
    src/gui/ExtraDescWidget.cpp
    src/gui/ProgramsWidget.cpp
    src/gui/ProgramsHighlighter.cpp
    src/gui/ValueFlagsWidget.cpp
    src/gui/ResetFlagsWidget.cpp
    src/gui/ResetVnumEventFilter.cpp
    src/gui/NewExitWidget.cpp
    src/gui/NewResetWidget.cpp
    src/gui/NewShopWidget.cpp
    src/gui/NewAreaWidget.cpp
    src/gui/MobileSpecialsWidget.cpp
    src/gui/RenumberWidget.cpp
    src/gui/RenumberWarningsWidget.cpp
    src/gui/VnumQuestionWidget.cpp
    src/gui/ExitEventFilter.cpp
    src/gui/WelcomeScreen.cpp
    src/gui/BalloonWidget.cpp
    src/gui/ToolTipEventFilter.cpp
    src/map/RoomCoords.cpp
    src/map/ExitWrapper.cpp
    src/map/MapRoom.cpp
    src/map/Mapper.cpp
    src/map/MapWidget.cpp
)

set(HEADERS
    src/model/Area.h
    src/model/ConfigData.h
    src/core/XmlIO.h
    src/core/Renumberer.h
    src/gui/MainWindow.h
    src/gui/FlagsWidget.h
    src/gui/ExtraDescWidget.h
    src/gui/ProgramsWidget.h
    src/gui/ProgramsHighlighter.h
    src/gui/ValueFlagsWidget.h
    src/gui/ResetFlagsWidget.h
    src/gui/ResetVnumEventFilter.h
    src/gui/NewExitWidget.h
    src/gui/NewResetWidget.h
    src/gui/NewShopWidget.h
    src/gui/NewAreaWidget.h
    src/gui/MobileSpecialsWidget.h
    src/gui/RenumberWidget.h
    src/gui/RenumberWarningsWidget.h
    src/gui/VnumQuestionWidget.h
    src/gui/ExitEventFilter.h
    src/gui/WelcomeScreen.h
    src/gui/BalloonWidget.h
    src/gui/ToolTipEventFilter.h
    src/map/RoomCoords.h
    src/map/ExitWrapper.h
    src/map/MapRoom.h
    src/map/RoomSpread.h
    src/map/Mapper.h
    src/map/MapWidget.h
)

qt_add_executable(swaedit ${SOURCES} ${HEADERS})

# Remove stale log files from the source tree at the start of every build
add_custom_target(clean_logs
    COMMAND ${CMAKE_COMMAND} -E rm -f
        ${CMAKE_SOURCE_DIR}/swaedit_out.log
        ${CMAKE_SOURCE_DIR}/swaedit_err.log
    COMMENT "Cleaning log files"
)
add_dependencies(swaedit clean_logs)

target_include_directories(swaedit PRIVATE src ${LIBXML2_INCLUDE_DIR})

target_compile_definitions(swaedit PRIVATE GL_SILENCE_DEPRECATION)

target_link_libraries(swaedit PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Xml
    OpenGL::GL
    LibXml2::LibXml2
)

# Copy data/ and schemas/ into the build directory so the app can find them at runtime
add_custom_command(TARGET swaedit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:swaedit>/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/schemas $<TARGET_FILE_DIR:swaedit>/schemas
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/images $<TARGET_FILE_DIR:swaedit>/images
)
