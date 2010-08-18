CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT("upgm" CXX C)

# Debug build by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)


SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT -lpthread -Wall")

INCLUDE_DIRECTORIES(AFTER
	${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY(xml)

ADD_LIBRARY (upgm
	SHARED
	src/upgm.cpp
	src/config.cpp
	src/data_tree.cpp
	src/path.cpp
	src/transport.cpp
	src/parser.cpp
	src/payment.cpp
	src/xml_parser.cpp
	src/request_template.cpp
	src/http_transport.cpp
	src/payguide_curl.cpp
	)

TARGET_LINK_LIBRARIES(upgm tinyxml curl)

ADD_SUBDIRECTORY(modules)

ADD_EXECUTABLE (test
	test/test_transport.cpp
	test/test.cpp
	)
TARGET_LINK_LIBRARIES(test webmoney)
