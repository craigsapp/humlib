cmake_minimum_required(VERSION 2.8)

project(humlib C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckIncludeFiles)

include_directories(include)


##############################
##
## Operating-system specific settings:
##

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    option(STATIC_CRT "Use static CRT libraries" OFF)

    # Rewrite command line flags to use /MT if necessary
    if(STATIC_CRT)
        foreach(flag_var
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
        endforeach(flag_var)
    endif()
endif()


##############################
##
## Library:
##

set(SRCS
	src/Convert-harmony.cpp
	src/Convert-kern.cpp
	src/Convert-math.cpp
	src/Convert-pitch.cpp
	src/Convert-rhythm.cpp
	src/Convert-string.cpp
	src/GridMeasure.cpp
	src/GridPart.cpp
	src/GridSide.cpp
	src/GridSlice.cpp
	src/GridStaff.cpp
	src/GridVoice.cpp
	src/HumAddress.cpp
	src/HumGrid.cpp
	src/HumHash.cpp
	src/HumInstrument.cpp
	src/HumNum.cpp
	src/HumParamSet.cpp
	src/HumRegex.cpp
	src/HumTool.cpp
	src/HumdrumFile.cpp
	src/HumdrumFileBase-net.cpp
	src/HumdrumFileBase.cpp
	src/HumdrumFileContent-accidental.cpp
	src/HumdrumFileContent-metlev.cpp
	src/HumdrumFileContent-slur.cpp
	src/HumdrumFileContent-tie.cpp
	src/HumdrumFileContent-timesig.cpp
	src/HumdrumFileContent.cpp
	src/HumdrumFileStream.cpp
	src/HumdrumFileStructure.cpp
	src/HumdrumLine.cpp
	src/HumdrumToken.cpp
	src/MxmlEvent.cpp
	src/MxmlMeasure.cpp
	src/MxmlPart.cpp
	src/NoteCell.cpp
	src/NoteGrid.cpp
	src/Options.cpp
	src/tool-autobeam.cpp
	src/tool-autostem.cpp
	src/tool-binroll.cpp
	src/tool-chord.cpp
	src/tool-cint.cpp
	src/tool-deg.cpp
	src/tool-dissonant.cpp
	src/tool-esac2hum.cpp
	src/tool-extract.cpp
	src/tool-fb.cpp
	src/tool-filter.cpp
	src/tool-hproof.cpp
	src/tool-imitation.cpp
	src/tool-mei2hum.cpp
	src/tool-metlev.cpp
	src/tool-msearch.cpp
	src/tool-musicxml2hum.cpp
	src/tool-myank.cpp
    src/tool-notemark.cpp
	src/tool-recip.cpp
	src/tool-restfill.cpp
	src/tool-ruthfix.cpp
	src/tool-satb2gs.cpp
	src/tool-tassoize.cpp
	src/tool-transpose.cpp
	src/tool-trillspell.cpp
	src/pugixml/pugixml.cpp
)

set(HDRS
	include/Convert.h
	include/GridCommon.h
	include/GridMeasure.h
	include/GridPart.h
	include/GridSide.h
	include/GridSlice.h
	include/GridStaff.h
	include/GridVoice.h
	include/HumAddress.h
	include/HumGrid.h
	include/HumHash.h
	include/HumInstrument.h
	include/HumNum.h
	include/HumParamSet.h
	include/HumRegex.h
	include/HumTool.h
	include/HumdrumFile.h
	include/HumdrumFileBase.h
	include/HumdrumFileContent.h
	include/HumdrumFileStream.h
	include/HumdrumFileStructure.h
	include/HumdrumLine.h
	include/HumdrumToken.h
	include/MxmlEvent.h
	include/MxmlMeasure.h
	include/MxmlPart.h
	include/NoteCell.h
	include/NoteGrid.h
	include/Options.h
	include/humlib.h
	include/pugixml/pugiconfig.hpp
	include/pugixml/pugixml.hpp
	include/tool-autobeam.h
	include/tool-autostem.h
	include/tool-binroll.h
	include/tool-chord.h
	include/tool-cint.h
	include/tool-deg.h
	include/tool-dissonant.h
	include/tool-esac2hum.h
	include/tool-extract.h
	include/tool-fb.h
	include/tool-hproof.h
	include/tool-imitation.h
	include/tool-mei2hum.h
	include/tool-metlev.h
	include/tool-msearch.h
	include/tool-musicxml2hum.h
	include/tool-myank.h
	include/tool-notemark.h
	include/tool-recip.h
	include/tool-restfill.h
	include/tool-ruthfix.h
	include/tool-satb2gs.h
	include/tool-tassoize.h
	include/tool-transpose.h
	include/tool-trillspell.h
	include/tool-filter.h
)

add_library(humlib STATIC ${SRCS} ${HDRS})

##############################
##
## Programs:
##

#add_executable(80off src-programs/80off.cpp)

# target_link_libraries(80off humlib)

