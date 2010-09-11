CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT("webmoney" CXX C)

# Debug build by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)


SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT -lpthread -Wall")

INCLUDE_DIRECTORIES(AFTER
	${CMAKE_CURRENT_SOURCE_DIR}/include)

#ADD_EXECUTABLE (wmsigner
#	src/wmsigner/base64.cpp
#	src/wmsigner/cmdbase.cpp
#	#	src/wmsigner/code64.cpp
#	src/wmsigner/crypto.cpp
#	src/wmsigner/md4.cpp
#	src/wmsigner/rsalib1.cpp
#	src/wmsigner/signer.cpp
#	src/wmsigner/stdafx.cpp
#	src/wmsigner/wmsigner.cpp
#	)

ADD_EXECUTABLE (code64
	src/wmsigner/base64.cpp
	src/wmsigner/cmdbase.cpp
	src/wmsigner/code64.cpp
	src/wmsigner/crypto.cpp
	src/wmsigner/md4.cpp
	src/wmsigner/rsalib1.cpp
	src/wmsigner/signer.cpp
	src/wmsigner/stdafx.cpp
	)

ADD_LIBRARY (wmsigner
	STATIC
	src/wmsigner/base64.cpp
	src/wmsigner/cmdbase.cpp
	src/wmsigner/crypto.cpp
	src/wmsigner/md4.cpp
	src/wmsigner/rsalib1.cpp
	src/wmsigner/signer.cpp
	src/wmsigner/stdafx.cpp
	)

ADD_LIBRARY(webmoney
	STATIC
	src/webmoney.cpp)

TARGET_LINK_LIBRARIES(webmoney upgm wmsigner)
