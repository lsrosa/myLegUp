# Source (with modifications) from: BALL is a C++ framework for molecular modeling and structural bioinformatics.
# Copyright (C) Andreas Hildebrandt, Oliver Kohlbacher, Hans-Peter Lenhof, and others 1996-2010
## Detect lpsolve
INCLUDE(CheckCXXSourceCompiles)

## first, search the headers
OPTION(LPSOLVE_REQUIRED "Abort if lpsolve cannot be found" OFF)


SET(LPSOLVE_INCLUDE_DIR "" CACHE STRING "Full path to the lpsolve headers")
MARK_AS_ADVANCED(LPSOLVE_INCLUDE_DIR)

SET(LPSOLVE_LIBRARIES "" CACHE STRING "Full path to the lpsolve55 library (including the library)")
MARK_AS_ADVANCED(LPSOLVE_LIBRARIES)

SET(LPSOLVE_INCLUDE_TRIAL_PATH
        /usr/include/lpsolve/
        /usr/include
        /usr/local/include
        /opt/include
        /sw/include
)

FIND_PATH(LPSOLVE_INCLUDE_PATH lp_lib.h ${LPSOLVE_INCLUDE_TRIAL_PATH})

IF (LPSOLVE_INCLUDE_PATH)
    #STRING(REGEX REPLACE "lpsolve/*$" "" LPSOLVE_INCLUDE_PATH ${LPSOLVE_INCLUDE_PATH})
        SET(LPSOLVE_INCLUDE_DIR ${LPSOLVE_INCLUDE_PATH} CACHE STRING "Full path to the lpsolve headers" FORCE)
        INCLUDE_DIRECTORIES(${LPSOLVE_INCLUDE_DIR})

        GET_FILENAME_COMPONENT(LPSOLVE_INSTALL_BASE_PATH ${LPSOLVE_INCLUDE_DIR} PATH)
       
        SET(LPSOLVE_LIB_TRIALPATH
                ${LPSOLVE_INSTALL_BASE_PATH}/lib
                /usr/lib/lp_solve/
                /usr/lib/
                /usr/local/lib
                /opt/lib
        )

        FIND_LIBRARY(TMP_LPSOLVE_LIBRARIES
                #NAMES lpsolve55_pic
                NAMES lpsolve55
                PATHS ${LPSOLVE_LIBRARIES} ${LPSOLVE_LIB_TRIALPATH}
                PATH_SUFFIXES lp_solve
                )
        SET(LPSOLVE_LIBRARIES ${TMP_LPSOLVE_LIBRARIES} CACHE STRING "Full path to the lpsolve55 library (including the library)" FORCE)
        IF (LPSOLVE_LIBRARIES)
                SET(LPSOLVE_FOUND TRUE)

                ## Try to find out if lpsolve can link standalone
                SET(LPSOLVE_TRY_CODE "#include <lp_lib.h>
                        int main(int /*argc*/, char** /*argv*/)
                        {
                                int major, minor, release, build;
                                lp_solve_version(&major, &minor, &release, &build);
                                lprec *lp = make_lp(0, 1);

                                return 0;
                        }")

                SET(CMAKE_REQUIRED_LIBRARIES ${LPSOLVE_LIBRARIES})
                SET(CMAKE_REQUIRED_INCLUDES ${LPSOLVE_INCLUDE_DIR})
                CHECK_CXX_SOURCE_COMPILES("${LPSOLVE_TRY_CODE}" LPSOLVE_LINKS_ALONE)
                SET(CMAKE_REQUIRED_LIBRARIES "")
              
                ## Try to find out if lpsolve can link with some extra libs
                IF (NOT LPSOLVE_LINKS_ALONE)
                        message(STATUS "Can't link lp solve library alone")
                        FIND_LIBRARY(LPSOLVE_LIB_DL "dl")
                        FIND_LIBRARY(LPSOLVE_LIB_COLAMD "colamd")

                        LIST(APPEND LPSOLVE_LIBRARIES "${LPSOLVE_LIB_DL}" "${LPSOLVE_LIB_COLAMD}")

                        SET(CMAKE_REQUIRED_LIBRARIES ${LPSOLVE_LIBRARIES})
                        CHECK_CXX_SOURCE_COMPILES("${LPSOLVE_TRY_CODE}" LPSOLVE_LINKS_WITH_EXTRA_LIBS)
                        SET(CMAKE_REQUIRED_LIBRARIES "")

                        IF (NOT LPSOLVE_LINKS_WITH_EXTRA_LIBS)
                                MESSAGE(STATUS "Could not link against lpsolve55!")
                        ENDIF()
                ENDIF()
        ENDIF()
else()
  message(FATAL_ERROR "Can't find lp_lib.h from lp solve library")
ENDIF()

IF (LPSOLVE_LINKS_ALONE OR LPSOLVE_LINKS_WITH_EXTRA_LIBS)
        SET(LPSOLVE_LINKS ${LPSOLVE_LIBRARIES})
else()
  message(FATAL_ERROR "Can't link lp solve library")
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LPSolve DEFAULT_MSG
        LPSOLVE_LIBRARIES
        LPSOLVE_INCLUDE_PATH
        LPSOLVE_LINKS
)

