cmake_minimum_required(VERSION 2.8.9)
message( STATUS "CMAKE_SOURCE_DIR.............: " ${CMAKE_SOURCE_DIR})
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build/cmake")
project (KNXTOOLS)

#
# Find Libxml2
#
FIND_PACKAGE(LibXml2)
if ( LIBXML2_FOUND)
	message( STATUS "libxml include directory.....: " ${LIBXML2_INCLUDE_DIR})
	message( STATUS "libxml libraries.............: " ${LIBXML2_LIBRARIES})
  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
  LIST(APPEND ADDITIONAL_LIBS ${LIBXML2_LIBRARIES})
  SET(HAVE_LIBXML2 1)
  # libxml2's include files use iconv.h
  # We need a directory path of iconv.h so that it won't fail to check
  # "libxml/xmlreader.h".
  FIND_PATH(ICONV_INCLUDE_DIR iconv.h)
  INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
  SET(CMAKE_REQUIRED_INCLUDES ${ICONV_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
#  CHECK_INCLUDE_FILES("libxml/xmlreader.h" HAVE_LIBXML_XMLREADER_H)
  SET(CMAKE_REQUIRED_INCLUDES "")
ELSE(LIBXML2_FOUND)
  #
  # Find Expat
  #
  FIND_PACKAGE(EXPAT)
  IF(EXPAT_FOUND)
    INCLUDE_DIRECTORIES(${EXPAT_INCLUDE_DIR})
    LIST(APPEND ADDITIONAL_LIBS ${EXPAT_LIBRARIES})
    SET(HAVE_LIBEXPAT 1)
    LA_CHECK_INCLUDE_FILE("expat.h" HAVE_EXPAT_H)
  ENDIF(EXPAT_FOUND)
ENDIF(LIBXML2_FOUND)

find_package( mysql)
if ( MYSQL_FOUND)
	message( STATUS "MySQL include dir............: " ${MYSQL_INCLUDE_DIR})
	message( STATUS "MySQL libraries..............: " ${MYSQL_LIBRARIES})
	message( "found MySQL ...")
	include_directories( ${MYSQL_INCLUDE_DIRS})
endif( MYSQL_FOUND)

add_subdirectory (apps)
add_subdirectory (libs)

install( DIRECTORY knx.d DESTINATION /etc)

message( "cmake configuration finished")
