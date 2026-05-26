#
# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

set(LIBRARY_NAME shared)

set(SRC_GRP_AUTH
    Auth/BigNumber.cpp
    Auth/BigNumber.h
    Auth/HMACSHA1.cpp
    Auth/HMACSHA1.h
    Auth/SARC4.cpp
    Auth/SARC4.h
    Auth/SRP6.cpp
    Auth/SRP6.h
    Auth/CryptoHash.h
    Auth/base32.cpp
    Auth/base32.h
)

set(SRC_GRP_CONFIG
    Config/Config.cpp
    Config/Config.h
)

set(SRC_GRP_DATABASE
    Database/Database.cpp
    Database/Database.h
    Database/DatabaseEnv.h
    Database/DatabaseImpl.h
    Database/DatabaseMysql.cpp
    Database/DatabaseMysql.h
    Database/DatabasePostgre.cpp
    Database/DatabasePostgre.h
    Database/DatabaseSqlite.cpp
    Database/DatabaseSqlite.h
    Database/Field.cpp
    Database/Field.h
    Database/PGSQLDelayThread.h
    Database/QueryResult.h
    Database/QueryResultMysql.cpp
    Database/QueryResultMysql.h
    Database/QueryResultPostgre.cpp
    Database/QueryResultPostgre.h
    Database/QueryResultSqlite.cpp
    Database/QueryResultSqlite.h
    Database/SqlDelayThread.cpp
    Database/SqlDelayThread.h
    Database/SqlOperations.cpp
    Database/SqlOperations.h
    Database/SqlPreparedStatement.cpp
    Database/SqlPreparedStatement.h
    Database/SQLStorage.cpp
    Database/SQLStorage.h
    Database/SQLStorageImpl.h
)

set(SRC_GRP_DATABASE_DBC
    Database/DBCFileLoader.cpp
    Database/DBCFileLoader.h
    Database/DBCStore.h
)

set(SRC_GRP_LOG
    Log/Log.cpp
    Log/Log.h
)

set(SRC_GRP_MT
    Multithreading/Messager.h
    Multithreading/Messager.cpp
    Multithreading/Threading.cpp
    Multithreading/Threading.h
)

if(BUILD_METRICS)
    set(SRC_GRP_METRIC
        Metric/Measurement.cpp
        Metric/Measurement.h
        Metric/Metric.cpp
        Metric/Metric.h
    )
endif()

set(SRC_GRP_NETWORK
    Network/AsyncSocket.hpp
    Network/AsyncListener.hpp
)

set(SRC_GRP_PLATFORM
    Platform/CompilerDefs.h
    Platform/Define.h
    Platform/Filesystem.h
)

# OS specific files
if(WIN32)
  set(SRC_GRP_PLATFORM
      ${SRC_GRP_PLATFORM}
      Platform/ServiceWin32.cpp
      Platform/ServiceWin32.h
     )
elseif(UNIX)
  set(SRC_GRP_PLATFORM
      ${SRC_GRP_PLATFORM}
      Platform/PosixDaemon.cpp
      Platform/PosixDaemon.h
     )
endif()

set(SRC_GRP_UTIL
    Util/ByteBuffer.cpp
    Util/ByteBuffer.h
    Util/ByteConverter.h
    Util/Errors.h
    Util/ProgressBar.cpp
    Util/ProgressBar.h
    Util/Timer.h
    Util/Util.cpp
    Util/Util.h
    Util/ProducerConsumerQueue.h
    Util/CommonDefines.h
    Util/UniqueTrackablePtr.h
)

set(LIBRARY_SRCS
    ${SRC_GRP_AUTH}
    ${SRC_GRP_CONFIG}
    ${SRC_GRP_DATABASE}
    ${SRC_GRP_DATABASE_DBC}
    ${SRC_GRP_LOG}
    ${SRC_GRP_METRIC}
    ${SRC_GRP_MT}
    ${SRC_GRP_NETWORK}
    ${SRC_GRP_PLATFORM}
    ${SRC_GRP_UTIL}
    Common.cpp
    Common.h
    revision_sql.h
    revision.h
    SystemConfig.h
)

# define header files
set(PCH_FILE_HEADER "pchdef.h")
set(PCH_FILE_SOURCE "pchdef.cpp")

source_group("Auth"
  FILES
    ${SRC_GRP_AUTH}
)

source_group("Config"
  FILES
    ${SRC_GRP_CONFIG}
)

source_group("Database"
  FILES
    ${SRC_GRP_DATABASE}
)

source_group("Database\\DataStores"
  FILES
    ${SRC_GRP_DATABASE_DBC}
)

source_group("Log"
  FILES
    ${SRC_GRP_LOG}
)

if(BUILD_METRICS)
    source_group("Metric"
    FILES
        ${SRC_GRP_METRIC}
    )
endif()

source_group("Network"
  FILES
    ${SRC_GRP_NETWORK}
)

source_group("Platform"
  FILES
    ${SRC_GRP_PLATFORM}
)

source_group("Util"
  FILES
    ${SRC_GRP_UTIL}
)

source_group("Multithreading"
  FILES
    ${SRC_GRP_MT}
)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

target_include_directories(${LIBRARY_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${Boost_INCLUDE_DIRS}
  PUBLIC ${OPENSSL_INCLUDE_DIR}
  PUBLIC ${CMAKE_BINARY_DIR}/src/shared # For the generated revision.h.
  PUBLIC "${CMAKE_SOURCE_DIR}/src/game"
)

target_link_libraries(${LIBRARY_NAME} PUBLIC cmangos-compile-option-interface)
target_link_libraries(${LIBRARY_NAME} PRIVATE g3dlite)

if(UNIX)
  target_link_libraries(${LIBRARY_NAME}
    PUBLIC framework
    PUBLIC utf8cpp
    PUBLIC dl
    PUBLIC ${Boost_LIBRARIES}
    PUBLIC ${OPENSSL_LIBRARIES}
  )
else()
  target_link_libraries(${LIBRARY_NAME}
    PUBLIC framework
    PUBLIC ${Boost_LIBRARIES}
    PUBLIC ${OPENSSL_LIBRARIES}
    PUBLIC utf8cpp
  )
endif()

# Some Boost installations (especially Homebrew on macOS) pull in ICU
# transitively via Boost::regex. Boost's interface dependencies frequently
# emit bare "-licudata -licui18n -licuuc" without any -L path.
# We must therefore:
#   1. Add the ICU library directory (propagates to realmd/mangosd).
#   2. Link the ICU libraries (preferably via imported targets, which produce
#      better/fully-qualified output).
#   3. Add the include dirs for good measure.
if(ICU_FOUND)
  if(ICU_LIBRARY_DIRS)
    target_link_directories(${LIBRARY_NAME} PUBLIC ${ICU_LIBRARY_DIRS})
  elseif(APPLE)
    # Extra safety net for the most common Homebrew layout
    target_link_directories(${LIBRARY_NAME} PUBLIC
      "/opt/homebrew/opt/icu4c/lib"
      "/usr/local/opt/icu4c/lib"
    )
  endif()

  # Prefer imported targets when available (CMake 3.7+ / FindICU)
  if(TARGET ICU::data AND TARGET ICU::i18n AND TARGET ICU::uc)
    target_link_libraries(${LIBRARY_NAME} PUBLIC ICU::data ICU::i18n ICU::uc)
  elseif(ICU_LIBRARIES)
    target_link_libraries(${LIBRARY_NAME} PUBLIC ${ICU_LIBRARIES})
  endif()

  if(ICU_INCLUDE_DIRS)
    target_include_directories(${LIBRARY_NAME} PUBLIC ${ICU_INCLUDE_DIRS})
  endif()
endif()

# zstd (and potentially other libs) can be pulled in transitively by the
# MySQL client library on Homebrew/macOS and some Linux setups.
# MYSQL_EXTRA_LIBRARIES often contains bare "zstd", so we need the search path.
if(ZSTD_LIBRARY_DIR)
  target_link_directories(${LIBRARY_NAME} PUBLIC "${ZSTD_LIBRARY_DIR}")
elseif(APPLE)
  # Common Homebrew fallback
  target_link_directories(${LIBRARY_NAME} PUBLIC
    "/opt/homebrew/opt/zstd/lib"
    "/usr/local/opt/zstd/lib"
  )
endif()
if(ZSTD_LIBRARIES)
  target_link_libraries(${LIBRARY_NAME} PUBLIC ${ZSTD_LIBRARIES})
endif()

if(POSTGRESQL AND POSTGRESQL_FOUND)
  target_include_directories(${LIBRARY_NAME} PUBLIC ${PostgreSQL_INCLUDE_DIRS})
  target_link_libraries(${LIBRARY_NAME} PUBLIC ${PostgreSQL_LIBRARIES})
elseif(SQLITE AND SQLite3_FOUND)
  target_include_directories(${LIBRARY_NAME} PUBLIC ${SQLite3_INCLUDE_DIRS})
  target_link_libraries(${LIBRARY_NAME} PUBLIC ${SQLite3_LIBRARIES})
else()
  target_include_directories(${LIBRARY_NAME} PUBLIC ${MYSQL_INCLUDE_DIR})
  target_link_libraries(${LIBRARY_NAME} PUBLIC ${MYSQL_LIBRARY} ${MYSQL_EXTRA_LIBRARIES})
  if (WIN32)
    # Add provided dependency lib to dev folder
    string(REGEX REPLACE "/" "\\\\" LibFolder ${DEV_PROVIDED_LIBS_FOLDER})
    add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
      COMMAND xcopy /Y \"${LibFolder}\\*.dll\" \"${DEV_BIN_DIR}\\*.dll\")
  endif()
endif()

# Specific definition for an issue with boost/stacktrace when building on macOS
if(APPLE OR "${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
  add_compile_definitions(_GNU_SOURCE)
endif()

# Generate precompiled header
if(PCH)
  target_precompile_headers(${LIBRARY_NAME} PRIVATE "${PCH_FILE_HEADER}")
endif()

# Mark the revision header as being generated.
set_source_files_properties(${CMAKE_BINARY_DIR}/src/shared/revision.h
  PROPERTIES GENERATED TRUE
             HEADER_FILE_ONLY TRUE)
if(MSVC)
  set_target_properties(${LIBRARY_NAME} PROPERTIES PROJECT_LABEL "Shared")
endif()
