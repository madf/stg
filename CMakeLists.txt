cmake_minimum_required (VERSION 3.12)

project ( Stargazer )

set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

option ( BUILD_STG "Build Stargazer server." OFF )
option ( BUILD_SGCONF "Build SGConf client." OFF )
option ( BUILD_RSCRIPTD "Build RScriptD server." OFF )
option ( BUILD_SGAUTH "Build SGAuth client." OFF )

# General modules
option ( BUILD_MOD_AO "Build AlwaysOnline STG module." ON )
option ( BUILD_MOD_RADIUS "Build Radius STG module." ON )
option ( BUILD_MOD_IA "Buils InetAccess STG module." ON )
option ( BUILD_MOD_SGCONFIG "Build SGConfig STG module." ON )
option ( BUILD_MOD_PING "Build Ping STG module." ON )
option ( BUILD_MOD_RSCRYPT "Build RScrypt STG module." ON )
option ( BUILD_MOD_SMUX "Buils SMUX STG module." ON )
option ( BUILD_MOD_STORE_FILES "Build file store STG module." ON )
option ( BUILD_MOD_CAP_NF "Build NetFlow capture STG module." ON )

# OS-specific modules
option ( BUILD_MOD_CAP_ETHER_LINUX "Build Linux Ethernet capture STG module." OFF )
option ( BUILD_MOD_CAP_ETHER_FREEBSD "Build FreeBSD Ethernet capture STG module." OFF )
option ( BUILD_MOD_CAP_DIVERT "Build FreeBSD divert-socket capture STG MODULE." OFF )

# Lib-specific modules
option ( BUILD_MOD_STORE_FIREBIRD "Build Firebird store STG module." OFF )
option ( BUILD_MOD_STORE_POSTGRESQL "Build PostgreSQL store STG module." OFF )
option ( BUILD_MOD_STORE_MYSQL "Build MySQL store STG module." OFF )
option ( BUILD_MOD_RPCCONFIG "Build XML-RPC configuretion STG module." OFF )
option ( BUILD_MOD_CAP_PCAP "Build PCap capture STG module." OFF )
option ( BUILD_MOD_CAP_NFQUEUE "Build NFQueue capture STG module." OFF )

# Firebird is getting deprecated, negation option
option ( BUILD_NO_MOD_STORE_FIREBIRD "Do not build Firebird store STG module." OFF )

# Grouping
option ( BUILD_ALL_MODS "Build all modules." OFF )
option ( BUILD_ALL_LIBS "Build all libraries." OFF )
option ( BUILD_TESTS "Build tests." OFF )
option ( BUILD_ALL "Build binaries, libraries and tests." OFF )
option ( ENABLE_COVERAGE "Enable code coverage analysis." OFF )

# CPPCheck-related
option ( CPPCHECK_FOREIGN "Run cppcheck over foreign lib code" OFF )

if ( BUILD_ALL )
    set ( BUILD_ALL_MODS ON )
    set ( BUILD_ALL_LIBS ON )
    set ( BUILD_TESTS ON )
    set ( BUILD_SGAUTH ON )
    set ( BUILD_RSCRIPTD ON )
    set ( BUILD_SGCONF ON )
    set ( BUILD_STG ON )
endif ( BUILD_ALL )

set ( BUILD_LIB_COMMON ON )

if ( BUILD_SGAUTH OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_CONFFILES ON )
    set ( BUILD_LIB_CRYPTO ON )
    set ( BUILD_LIB_IA ON )
endif ( BUILD_SGAUTH OR BUILD_ALL_LIBS )

if ( BUILD_SGCONF OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_CRYPTO ON )
    set ( BUILD_LIB_SRVCONF ON )
endif ( BUILD_SGCONF OR BUILD_ALL_LIBS )

if ( BUILD_RSCRIPTD OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_CONFFILES ON )
    set ( BUILD_LIB_CRYPTO ON )
    set ( BUILD_LIB_LOGGER ON )
    set ( BUILD_LIB_SCRIPTEXECUTER ON )
endif ( BUILD_RSCRIPTD OR BUILD_ALL_LIBS )

if ( BUILD_ALL_LIBS )
    set ( BUILD_LIB_CRYPTO ON )
endif ( BUILD_ALL_LIBS )

if ( BUILD_STG OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_CONFFILES ON )
    set ( BUILD_LIB_CRYPTO ON )
    set ( BUILD_LIB_DOTCONFPP ON )
    set ( BUILD_LIB_LOGGER ON )
    set ( BUILD_LIB_SCRIPTEXECUTER ON )
endif ( BUILD_STG OR BUILD_ALL_LIBS )

if ( BUILD_ALL_MODS )
    set ( BUILD_MOD_STORE_FIREBIRD ON )
    set ( BUILD_MOD_STORE_MYSQL ON )
    set ( BUILD_MOD_STORE_POSTGRESQL ON )
    set ( BUILD_MOD_RPCCONFIG ON )
    set ( BUILD_MOD_CAP_PCAP ON )
endif ( BUILD_ALL_MODS )

if ( BUILD_NO_MOD_STORE_FIREBIRD )
    set ( BUILD_MOD_STORE_FIREBIRD OFF )
endif ( BUILD_NO_MOD_STORE_FIREBIRD )

if ( BUILD_MOD_STORE_FIREBIRD OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_IBPP ON )
endif ( BUILD_MOD_STORE_FIREBIRD OR BUILD_ALL_LIBS )

if ( BUILD_MOD_PING OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_PINGER ON )
endif ( BUILD_MOD_PING OR BUILD_ALL_LIBS )

if ( BUILD_MOD_SMUX OR BUILD_ALL_LIBS )
    set ( BUILD_LIB_SMUX ON )
endif ( BUILD_MOD_SMUX OR BUILD_ALL_LIBS )

if ( TIDY )
    find_program( CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable" )
    if ( NOT CLANG_TIDY_EXE )
        message( STATUS "clang-tidy not found." )
    else ()
        message( STATUS "clang-tidy found: ${CLANG_TIDY_EXE}" )
        set( DO_CLANG_TIDY "${CLANG_TIDY_EXE}"
             "-checks=-*,\
clang-analyzer-*,\
misc-*,\
modernize-*,\
performance-*,\
portability-*,\
readability-*,\
bugprone-*,\
-modernize-avoid-bind,\
-modernize-use-trailing-return-type,\
-readability-braces-around-statements, \
-readability-magic-numbers, \
-readability-identifier-length, \
-bugprone-easily-swappable-parameters" )
    endif ()
endif ()

if ( IWYU )
    find_program( INCLUDE_WHAT_YOU_USE_EXE NAMES "include-what-you-use" DOC "Path to include-what-you-use executable" )
    if ( NOT INCLUDE_WHAT_YOU_USE_EXE )
        message( STATUS "include-what-you-use not found." )
    else ()
        message( STATUS "include-what-you-use found: ${INCLUDE_WHAT_YOU_USE_EXE}" )
        set( DO_INCLUDE_WHAT_YOU_USE "${INCLUDE_WHAT_YOU_USE_EXE}" )
    endif ()
endif ()

enable_language (CXX)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wshadow -Wno-long-long -Wold-style-cast -Wstrict-aliasing -pedantic")

if ( CMAKE_COMPILER_IS_GNUCC )
    # GCC-specific
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op -Wuseless-cast")
endif ( CMAKE_COMPILER_IS_GNUCC )

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    add_definitions ( -DLINUX )
    set ( BUILD_MOD_CAP_ETHER_LINUX ON )
    if ( BUILD_LIB_IBPP )
        add_definitions ( -DIBPP_LINUX )
    endif ( BUILD_LIB_IBPP )
    if ( BUILD_ALL_MODS )
        set ( BUILD_MOD_CAP_NFQUEUE ON )
    endif ( BUILD_ALL_MODS )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" )
    add_definitions ( -DFREE_BSD )
    set ( BUILD_MOD_CAP_ETHER_FREEBSD ON )
    set ( BUILD_MOD_CAP_DIVERT ON )
    if ( BUILD_LIB_IBPP )
        add_definitions ( -DIBPP_LINUX )
        message ( WARNING "IBPP does not support '${CMAKE_SYSTEM_NAME}'" )
    endif ( BUILD_LIB_IBPP )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    add_definitions ( -DDARWIN )
    if ( BUILD_LIB_IBPP )
        add_definitions ( -DIBPP_DARWIN )
    endif ( BUILD_LIB_IBPP )
else ()
    message ( WARNING "Unsupported system: '${CMAKE_SYSTEM_NAME}'" )
endif ()

include_directories ( include )
include_directories ( 3rdparty/jthread/source )

add_subdirectory ( libs )
add_subdirectory ( projects )

if ( BUILD_TESTS )
    enable_testing ()
    add_subdirectory ( tests )
endif ( BUILD_TESTS )
