CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT("upgm" CXX C)

# Debug build by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)


SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT -lpthread -Wall -fPIC -pthread")

ADD_SUBDIRECTORY(xml)


SET(PAYGUIDE_SRC     "${CMAKE_CURRENT_SOURCE_DIR}/legacy/payguide" CACHE PATH "Legacy payguide source")

IF (NOT EXISTS ${PAYGUIDE_SRC}/src/pay.h)
	MESSAGE(FATAL_ERROR "Looks like payguide path is wrong (Can't read ${PAYGUIDE_SRC}/src/pay.h) Check PAYGUIDE_SRC option value")
ENDIF(NOT EXISTS ${PAYGUIDE_SRC}/src/pay.h)

IF(NOT PAYGUIDE_INSTALL)
	MESSAGE("PAYGUIDE_INSTALL not set. Install will be done into /var/lib/upgm. Make sure that you will use them with proper payguide version! (Compiled with legacy source ${PAYGUIDE_SRC})")
	SET(PAYGUIDE_INSTALL "/var/lib/upgm")
	SET(MODULES_INSTALL_PATH "${PAYGUIDE_INSTALL}/modules")
ELSE(NOT PAYGUIDE_INSTALL)
	MESSAGE("Payguide installation path located at ${PAYGUIDE_INSTALL}.")
	EXECUTE_PROCESS(COMMAND diff --brief ${PAYGUIDE_INSTALL}/src/pay.h  ${PAYGUIDE_SRC}/src/pay.h OUTPUT_QUIET RESULT_VARIABLE DIFF_FAILED)
	IF(DIFF_FAILED)
		MESSAGE(FATAL_ERROR "${PAYGUIDE_INSTALL}/src/pay.h\nand\n${PAYGUIDE_SRC}/src/pay.h\ndiffer. Basically this means that you are trying to compile UPGM with payguide version X (${PAYGUIDE_SRC}) and make it work with another payguide version Y (${PAYGUIDE_INSTALL})")
	ENDIF(DIFF_FAILED)

	SET(MODULES_INSTALL_PATH "${PAYGUIDE_INSTALL}/modules/paysys/")
ENDIF(NOT PAYGUIDE_INSTALL)

MESSAGE("Payguide legacy source located at ${PAYGUIDE_SRC}")

INCLUDE_DIRECTORIES(AFTER
	${CMAKE_CURRENT_SOURCE_DIR}
	${PAYGUIDE_SRC}/src
	"/usr/include/mysql")

ADD_SUBDIRECTORY(legacy)

FIND_LIBRARY(MYSQLPP_LIBRARY mysqlpp)
FIND_LIBRARY(CURL_LIBRARY curl)

ADD_LIBRARY (upgm
	SHARED
	src/upgm.cpp
	src/common_hooks.cpp
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
	src/db.cpp
	src/db_mysql.cpp
	src/shared_mysql_connection.cpp
	)

TARGET_LINK_LIBRARIES(upgm tinyxml ${CURL_LIBRARY} ${MYSQLPP_LIBRARY} payguide_legacy pthread)
INSTALL(TARGETS upgm DESTINATION /usr/local/lib)

ADD_SUBDIRECTORY(modules)

ADD_EXECUTABLE (test
	test/test_transport.cpp
	test/test.cpp
	)
TARGET_LINK_LIBRARIES(test webmoney payguide_legacy pthread)

