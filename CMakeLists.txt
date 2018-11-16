cmake_minimum_required(VERSION 2.6)
project(opncms)

enable_testing()

# add a target to generate API documentation with Doxygen
find_package(Doxygen)

option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
option(MONGODB_ORIGINAL "Use installed mongodb library" OFF)
option(CPPCMS_OLD "Use CppCMS 1.x earlier then 1.1.0" OFF)
option(USE_STATIC_VIEW "Compile view statically into opnpcms" OFF)

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()

# Main
message(STATUS "System: " ${CMAKE_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION} ", CPU: " ${CMAKE_HOST_SYSTEM_PROCESSOR})

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
# TODO: using BOOST_OS (boost/predef.h) since v1.55.0
	set(OPNCMS_WIN_NATIVE 1)
    if(MSVC)
        message(STATUS "Compiler: MSVC, version: " ${MSVC_VERSION})
    endif(MSVC)
    if(BORLAND)
        message(STATUS "Compiler: BCC")
    endif(BORLAND)
    if(MINGW)
        message(STATUS "Compiler: MinGW")
    endif(MINGW)
else(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    message(STATUS "Only GCC is supported on Linux")
endif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)

message(STATUS "CMake generates " ${CMAKE_GENERATOR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if (CMAKE_COMPILER_IS_GNUCXX)
  #use __SKIP_GNU for the crypt declaration's conflict in unistd.h
  set ( CMAKE_CXX_FLAGS "-Wall -Wno-long-long -ldl -pedantic -pg -D__SKIP_GNU" )

  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_GREATER 6.0 OR GCC_VERSION VERSION_EQUAL 6.0)
    message(STATUS "C++14 activated")
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14" )
    #    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set ( MONGODB_FLAGS "--c++11=on" )
    string (REPLACE "-pedantic" "-Wpedantic" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  elseif (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
    string (REPLACE "-pedantic" "-Wpedantic" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11" )
    set ( MONGODB_FLAGS "--c++11=on" )
  elseif (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
    message(STATUS "C++11 activated")
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11" )
    set ( MONGODB_FLAGS "--c++11=on" )
  elseif(GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
    message(WARNING "C++0x activated")
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x" )
  else()
    message(WARNING "C++98 activated")
  endif()

  if(CPPCMS_OLD)
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OPNCMS_CPPCMS_OLD" )
  endif ()

endif ()

set (MONGODB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/contrib/mongo-cxx-driver)
set (BCRYPT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/contrib/crypt_blowfish-1.2)
set (FMTLIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/contrib/fmt)
set (CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/opncms ${BCRYPT_SRC} ${MONGODB_SRC}/build/install/include ${FMTLIB_SRC})

if (NOT EXISTS ${MONGODB_SRC})
        execute_process(COMMAND git clone -b "legacy-1.1.3" https://github.com/mongodb/mongo-cxx-driver.git ${MONGODB_SRC})
endif ()

if (NOT EXISTS ${FMTLIB_SRC})
        execute_process(COMMAND git clone -b "4.0.0" https://github.com/fmtlib/fmt.git ${FMTLIB_SRC})
endif ()

if(NOT MONGODB_ORIGINAL)
	set(MONGODB_SHARED ${MONGODB_SRC}/build/install/lib/libmongoclient.so)
	set(MONGODB_STATIC ${MONGODB_SRC}/build/install/lib/libmongoclient.a)
	find_path(MONGODB_INC ${MONGODB_SRC}/build/install/include/mongo/client/dbclient.h)
	link_directories(${MONGODB_SRC}/build/install/lib)
	include_directories(${MONGODB_SRC}/build/install/include)
else()
	set(MONGODB_SHARED libmongoclient.so)
	set(MONGODB_STATIC libmongoclient.a)
	find_path(MONGODB_INC mongo/client/dbclient.h)
	include_directories(${MONGODB_INC})
endif()

link_directories(${BCRYPT_INC})
link_directories(${FMTLIB_SRC})

find_library(CPPCMS cppcms)
find_library(BOOSTER booster)
find_library(CPPDB cppdb)
find_library(MONGODB mongoclient)
find_library(FMTLIB libfmt)
find_library(BOOST_SYSTEM boost_system)
find_library(BOOST_FILESYSTEM boost_filesystem)
find_library(ESMTP esmtp)

find_library(BOOST_THREAD boost_thread)
find_library(BOOST_REGEX boost_regex)

find_path(CPPCMS_INC cppcms/application.h)
find_path(BOOSTER_INC booster/shared_ptr.h)
find_path(CPPDB_INC cppdb/frontend.h)
find_path(BCRYPT_INC ow-crypt.h)
find_path(FMTLIB_INC fmt/format.h)

include_directories(${CPPCMS_INC})
include_directories(${BOOSTER_INC})
include_directories(${CPPDB_INC})
include_directories(${BCRYPT_INC})
include_directories(${FMTLIB_INC})
include_directories(${PROJECT_BINARY_DIR})
include_directories(.)

find_program(TMPLCC cppcms_tmpl_cc)
find_program(XGETTEXT xgettext)
find_program(MSGFMT msgfmt)
find_program(MSGMERGE msgmerge)

if(WIN32)
	add_definitions(-DDLL_EXPORT)
endif()


set(TEMPLATES
${CMAKE_CURRENT_SOURCE_DIR}/opncms/base.tmpl
)

set(SRC main.cpp
	src/base64.cpp
	src/esmtp.cpp
	src/tools.cpp
	src/bcrypt.cpp
	src/site.cpp
	module/data_sql.cpp
	module/data_file.cpp
	module/data_mongodb.cpp
	module/data.cpp
	module/auth.cpp
	module/media.cpp
	module/view.cpp
	src/rpc.cpp
)

# a custom target that is always built
add_custom_target(revisiontag ALL)

# creates version.h using cmake script
add_custom_command(TARGET revisiontag COMMAND ${CMAKE_COMMAND}
	-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} 
	-P ${CMAKE_CURRENT_SOURCE_DIR}/getversion.cmake)

#configure_file (
#	"${PROJECT_SOURCE_DIR}/version.h.in"
#	"${PROJECT_BINARY_DIR}/version.h")

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp
	COMMAND ${TMPLCC}
		-d opncms
                -s opncms_view
		-o ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp
		${TEMPLATES}
	DEPENDS ${TEMPLATES})

add_custom_target(
	libbcrypt
	COMMAND make library
	WORKING_DIRECTORY ${BCRYPT_INC}
)

add_custom_target(
	libmongoclient
	COMMAND scons install --prefix=${MONGODB_SRC}/build/install ${MONGODB_FLAGS} #--sharedclient
	WORKING_DIRECTORY ${MONGODB_SRC}
)

add_custom_target(
        libfmt
        COMMAND cmake .
        COMMAND make
        WORKING_DIRECTORY ${FMTLIB_INC}
)

set(BCRYPT_SHARED ${BCRYPT_INC}/libbcrypt.so)
set(BCRYPT_STATIC ${BCRYPT_INC}/libbcrypt.a)

set(FMTLIB_SHARED ${FMTLIB_INC}/fmt/libfmt.so)
set(FMTLIB_STATIC ${FMTLIB_INC}/fmt/libfmt.a)

if(USE_STATIC_VIEW)
	add_executable(opncms ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp)
else()
	add_executable(opncms ${SRC})
	add_library(opncms_view SHARED ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp)
	target_link_libraries(opncms_view ${BOOSTER} ${CPPCMS})
#        target_link_libraries(opncms ${BOOSTER} ${CPPCMS} ${CPPDB} ${BCRYPT_SHARED})
#        add_dependencies(opncms libbcrypt)
endif()

target_link_libraries(opncms ${BOOSTER} ${CPPCMS} ${CPPDB} ${BCRYPT_STATIC} ${MONGODB_STATIC} ${FMTLIB_STATIC} ${ESMTP} ${BOOST_SYSTEM} ${BOOST_FILESYSTEM} ${BOOST_THREAD} ${BOOST_REGEX})
add_dependencies(opncms libbcrypt libfmt libmongoclient revisiontag)

#  Localization
set(LOCALES ru)

set(MO_FILES)
set(UPDATE_PO_LIST)
set(POT_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/po/opncms.pot")

add_custom_command(
	OUTPUT ${POT_TEMPLATE}
	COMMAND
		${XGETTEXT}
		--keyword=translate:1,1t
		--keyword=translate:1,2,3t
		--keyword=_
		--keyword=N_
		${SRC}
		${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp
		--output=${POT_TEMPLATE}
	DEPENDS ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

add_custom_command(
        OUTPUT opncms.xml
        COMMAND
                gccxml
                ${SRC}
                ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp
                --fxml=opncms.xml
        DEPENDS ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/opncms_view.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_custom_target(update-po)

foreach(LOCALE ${LOCALES})
	set(MODIR "${CMAKE_CURRENT_BINARY_DIR}/locale/${LOCALE}/LC_MESSAGES")
	file(MAKE_DIRECTORY "${MODIR}")
	set(MOFILE "${MODIR}/opncms.mo")
	set(POFILE "${CMAKE_CURRENT_SOURCE_DIR}/po/${LOCALE}.po")

	add_custom_command(
		OUTPUT ${MOFILE}
		COMMAND ${MSGFMT} ${POFILE} -o ${MOFILE}
		DEPENDS ${POFILE})

	add_custom_target(update-po-${LOCALE}
		COMMAND ${MSGMERGE} -U ${POFILE} ${CMAKE_CURRENT_SOURCE_DIR}/po/opncms.pot
		DEPENDS ${POT_TEMPLATE}
		)
	add_dependencies(update-po update-po-${LOCALE})

	set(MO_FILES ${MO_FILES} ${MOFILE})
	set(UPDATE_PO_LIST ${UPDATE_PO_LIST} update-po-${LOCALE})
endforeach()

add_custom_target(create-po ALL DEPENDS ${MO_FILES})

foreach(LOCALE ${LOCALES})
        install(FILES ${MOFILE} DESTINATION share/locale/${LOCALE}/LC_MESSAGES/)
endforeach()

# Tests

set(BASIC_TESTS
        tools
        )

set(TESTDIR "${CMAKE_CURRENT_SOURCE_DIR}/test")

foreach(TEST ${BASIC_TESTS})
        add_executable(test_${TEST} ${TESTDIR}/test_${TEST}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/${TEST}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/esmtp.cpp)
        target_link_libraries(test_${TEST} ${BOOSTER} ${CPPCMS} ${BOOST_SYSTEM} ${BOOST_FILESYSTEM} ${FMTLIB_STATIC} ${ESMTP})
        add_test(test_${TEST} test_${TEST})
        #set_tests_properties(test_${TEST} PROPERTIES PASS_REGULAR_EXPRESSION "Passed;All ok;Ok")
endforeach()

add_executable(test_base64 ${TESTDIR}/test_base64.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/base64.cpp)
add_test(test_base64 test_base64)

add_executable(test_esmtp ${TESTDIR}/test_esmtp.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/esmtp.cpp)
target_link_libraries(test_esmtp ${ESMTP} ${BOOSTER})
add_test(test_esmtp test_esmtp test test@localhost)

add_executable(test_bcrypt ${TESTDIR}/test_bcrypt.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/bcrypt.cpp)
target_link_libraries(test_bcrypt ${BOOSTER} ${BCRYPT_STATIC})
add_test(test_bcrypt test_bcrypt)

add_executable(test_data ${TESTDIR}/test_data.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/tools.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/esmtp.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data_sql.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data_file.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data_mongodb.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data.cpp)
target_link_libraries(test_data ${BOOSTER} ${BOOST_SYSTEM} ${BOOST_FILESYSTEM} ${CPPCMS} ${CPPDB} ${ESMTP} ${MONGODB_STATIC} ${FMTLIB_STATIC} ${BOOST_THREAD} ${BOOST_REGEX})
add_test(test_data test_data)

add_executable(test_auth ${TESTDIR}/test_auth.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/tools.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/esmtp.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/bcrypt.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data_sql.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data_file.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data_mongodb.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/data.cpp ${CMAKE_CURRENT_SOURCE_DIR}/module/auth.cpp)
target_link_libraries(test_auth ${BOOSTER} ${BOOST_SYSTEM} ${BOOST_FILESYSTEM} ${CPPCMS} ${CPPDB} ${BCRYPT_STATIC} ${FMTLIB_STATIC} ${ESMTP} ${MONGODB_STATIC} ${BOOST_THREAD} ${BOOST_REGEX})
add_test(test_auth test_auth)
