#
# Find the MySQL client includes and library
#

# This module defines
# MYSQL_INCLUDE_DIR, where to find mysql.h
# MYSQL_LIBRARIES, the libraries to link against to connect to MySQL
# MYSQL_FOUND, if false, you cannot build anything that requires MySQL.

# also defined, but not for general use are
# MYSQL_LIBRARY, where to find the MySQL library.
# MYSQL_EXTRA_LIBRARIES, transitive dependencies of the client library.

set( MYSQL_FOUND 0 )

if( UNIX )
  set(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
    "preferred path to MySQL (mysql_config)"
  )

  find_program(MYSQL_CONFIG mysql_config
    ${MYSQL_CONFIG_PREFER_PATH}
    /usr/local/mysql/bin/
    /usr/local/bin/
    /usr/bin/
    /usr/local/opt/mysql/bin/
    /opt/homebrew/opt/mysql-client/bin
    /opt/homebrew/opt/mysql-client@8.4/bin
    /opt/mysql/mysql/bin/
  )

  if( MYSQL_CONFIG )
    message(STATUS "Using mysql-config: ${MYSQL_CONFIG}")
    # set INCLUDE_DIR
	execute_process(
	  COMMAND ${MYSQL_CONFIG} --include
      OUTPUT_VARIABLE MY_TMP
	  OUTPUT_STRIP_TRAILING_WHITESPACE
	)

    string(REGEX REPLACE "-I([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")
    set(MYSQL_ADD_INCLUDE_PATH ${MY_TMP} CACHE FILEPATH INTERNAL)
    #message("[DEBUG] MYSQL ADD_INCLUDE_PATH : ${MYSQL_ADD_INCLUDE_PATH}")
    # set LIBRARY_DIR
	execute_process(
      COMMAND ${MYSQL_CONFIG} --libs_r
      OUTPUT_VARIABLE MY_TMP
	  OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(MYSQL_ADD_LIBRARIES "")
    string(REGEX MATCHALL "-l[^ ]*" MYSQL_LIB_LIST "${MY_TMP}")
    foreach(LIB ${MYSQL_LIB_LIST})
      string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
      list(APPEND MYSQL_ADD_LIBRARIES "${LIB}")
      #message("[DEBUG] MYSQL ADD_LIBRARIES : ${MYSQL_ADD_LIBRARIES}")
    endforeach(LIB ${MYSQL_LIB_LIST})

    set(MYSQL_ADD_LIBRARIES_PATH "")
    string(REGEX MATCHALL "-L[^ ]*" MYSQL_LIBDIR_LIST "${MY_TMP}")
    foreach(LIB ${MYSQL_LIBDIR_LIST})
      string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
      list(APPEND MYSQL_ADD_LIBRARIES_PATH "${LIB}")
      #message("[DEBUG] MYSQL ADD_LIBRARIES_PATH : ${MYSQL_ADD_LIBRARIES_PATH}")
    endforeach(LIB ${MYSQL_LIBS})

  else( MYSQL_CONFIG )
    set(MYSQL_ADD_LIBRARIES "mysqlclient" "z")
  endif( MYSQL_CONFIG )
endif( UNIX )

find_path(MYSQL_INCLUDE_DIR
  NAMES
    mysql.h
  PATHS
    ${MYSQL_ADD_INCLUDE_PATH}
    /usr/include
    /usr/include/mysql
    /usr/local/include
    /usr/local/include/mysql
    /usr/local/mysql/include
    /usr/local/mysql/include/mysql
    /usr/local/opt/mysql/include
    /usr/local/opt/mysql-client/include
    /usr/local/opt/mysql-client/include/mysql
    /opt/homebrew/opt/mysql-client
    /opt/homebrew/opt/mysql-client/include
    /opt/homebrew/opt/mysql-client@8.4
    /opt/homebrew/opt/mysql-client@8.4/include
    /opt/mysql/mysql/include
    /opt/mysql/mysql/include/mysql
    "C:/Program Files/MySQL/include"
    "C:/Program Files/MySQL/MySQL Server 5.0/include"
    "C:/Program Files/MySQL/MySQL Server 5.1/include"
    "C:/MySQL/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server 5.0;Location]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server 5.1;Location]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server 5.0;Location]/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server 5.1;Location]/include"
    "c:/msys/local/include"
  DOC
    "Specify the directory containing mysql.h."
)

if( UNIX )
  link_directories(${MYSQL_ADD_LIBRARIES_PATH} /usr/local/lib)

  set(MYSQL_CLIENT_NAMES
    mysql libmysql
    mysqlclient mysqlclient_r
    mariadb mariadbclient
    perconaserverclient perconaserverclient_r
  )

  set(MYSQL_EXTRA_LIBRARIES "")
  foreach(LIB ${MYSQL_ADD_LIBRARIES})
    list(FIND MYSQL_CLIENT_NAMES "${LIB}" _is_client)
    if(NOT _is_client EQUAL -1)
      find_library( MYSQL_LIBRARY
        NAMES
          mysql libmysql ${LIB}
        PATHS
          ${MYSQL_ADD_LIBRARIES_PATH}
          /usr/lib
          /usr/lib/mysql
          /usr/local/lib
          /usr/local/lib/mysql
          /usr/local/mysql/lib
          /usr/local/mysql/lib/mysql
          /usr/local/opt/mysql/lib
          /usr/local/opt/mysql-client/lib
          /opt/mysql/mysql/lib
          /opt/mysql/mysql/lib/mysql
        DOC "Specify the location of the mysql library here."
      )
    else()
      list(APPEND MYSQL_EXTRA_LIBRARIES "${LIB}")
    endif()
  endforeach()
endif( UNIX )

if( WIN32 )
  find_library( MYSQL_LIBRARY
    NAMES
      mysql libmysql ${LIB}
    PATHS
      ${MYSQL_ADD_LIBRARIES_PATH}
      "C:/Program Files/MySQL/lib"
      "C:/Program Files/MySQL/MySQL Server 5.0/lib/opt"
      "C:/Program Files/MySQL/MySQL Server 5.1/lib/opt"
      "C:/MySQL/lib/debug"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server 5.0;Location]/lib/opt"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server 5.1;Location]/lib/opt"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server 5.0;Location]/lib/opt"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server 5.1;Location]/lib/opt"
      "C:/msys/local/include"
    DOC "Specify the location of the mysql library here."
  )
endif( WIN32 )

if( WIN32 )
  set( MYSQL_EXTRA_LIBRARIES "" )
endif( WIN32 )

if( MYSQL_LIBRARY )
  if( MYSQL_INCLUDE_DIR )
    set( MYSQL_FOUND 1 )
    set( MYSQL_LIBRARIES ${MYSQL_LIBRARY} ${MYSQL_EXTRA_LIBRARIES} )
    message(STATUS "Found MySQL library: ${MYSQL_LIBRARY}")
    message(STATUS "Found MySQL headers: ${MYSQL_INCLUDE_DIR}")
  else( MYSQL_INCLUDE_DIR )
      message(FATAL_ERROR "Could not find MySQL headers! Please install the development-libraries and headers.")
  endif( MYSQL_INCLUDE_DIR )
  mark_as_advanced( MYSQL_FOUND MYSQL_LIBRARY MYSQL_LIBRARIES MYSQL_EXTRA_LIBRARIES MYSQL_INCLUDE_DIR )
else( MYSQL_LIBRARY )
  message(FATAL_ERROR "Could not find the MySQL libraries! Please install the development-libraries and headers.")
endif( MYSQL_LIBRARY )
