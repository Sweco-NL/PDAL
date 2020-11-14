#
# HackathonToo plugin CMake configuration
#

# Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Sql Gui REQUIRED)

#
# Hackathon Writer
#

PDAL_ADD_PLUGIN(writer_libname writer hackathontoo
    FILES
        io/cache.cpp
        io/hackathonwriter.cpp
        io/storage.cpp
    LINK_WITH
        Qt5::Core
	Qt5::Sql
	Qt5::Gui
    INCLUDES
)

if (WIN32)
    target_compile_definitions(${writer_libname} PRIVATE -DNONMINMAX)
endif()
