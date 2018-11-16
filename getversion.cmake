include(FindSubversion)
include(FindGit)

set (opncms_VERSION_MAJOR 0)
set (opncms_VERSION_MINOR 1)
set (opncms_VERSION_PATCH 1)

function(opncms_noscm)
	file(WRITE version.h.in "#define opncms_VERSION_MAJOR ${opncms_VERSION_MAJOR}\n"
							"#define opncms_VERSION_MINOR ${opncms_VERSION_MINOR}\n"
							"#define opncms_VERSION_PATCH ${opncms_VERSION_PATCH}\n"
							"#define opncms_REVISION -1\n")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different version.h.in version.h)
endfunction()

#define own function in case of absence git_get_revision in FindGit.cmake
function(git_get_revision VARNAME)
    if (NOT GIT_FOUND)
        message(FATAL_ERROR "Cannot use git_get_revision: Git was not 
found.")
    endif()
    cmake_parse_arguments(GIT "SHORT" "WORKING_DIRECTORY;GIT_OPTIONS" "" ${ARGN})
    if(NOT GIT_WORKING_DIRECTORY OR "${GIT_WORKING_DIRECTORY}" 
STREQUAL "")
        set(GIT_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    if(GIT_SHORT)
	set (GIT_SHORT_REV "--short")
    endif()
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse ${GIT_OPTIONS} ${GIT_SHORT_REV} --verify -q HEAD
        OUTPUT_VARIABLE RES
        ERROR_VARIABLE ERR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${GIT_WORKING_DIRECTORY})
    set(${VARNAME} ${RES} PARENT_SCOPE)
    if (ERR)
        message(WARNING "Issuing Git command '${GIT_EXECUTABLE} rev-parse --verify -q HEAD' failed: ${ERR}")
    endif()
endfunction()

if(EXISTS "${CMAKE_SOURCE_DIR}/../.svn" AND Subversion_FOUND)
	Subversion_WC_INFO(${SOURCE_DIR} opncms)
	message(STATUS "Found Subversion revision: " ${opncms_WC_REVISION})
	file(WRITE version.h.in "#define opncms_VERSION_MAJOR ${opncms_VERSION_MAJOR}\n"
		"#define opncms_VERSION_MINOR ${opncms_VERSION_MINOR}\n"
		"#define opncms_VERSION_PATCH ${opncms_VERSION_PATCH}\n"
		"#define opncms_REVISION ${opncms_WC_REVISION}\n")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different version.h.in version.h)
		
ELSEIF(EXISTS "${CMAKE_SOURCE_DIR}/../.git" AND GIT_FOUND)
	git_get_revision(GIT_REVISION SHORT)
	message(STATUS "Found Git revision: " ${GIT_REVISION})
	file(WRITE version.h.in "#define opncms_VERSION_MAJOR ${opncms_VERSION_MAJOR}\n"
		"#define opncms_VERSION_MINOR ${opncms_VERSION_MINOR}\n"
		"#define opncms_VERSION_PATCH ${opncms_VERSION_PATCH}\n"
		"#define opncms_REVISION \"${GIT_REVISION}\"\n")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different version.h.in version.h)

ELSE()
	message(STATUS "Can't find any SCM")
	opncms_noscm()
ENDIF()
