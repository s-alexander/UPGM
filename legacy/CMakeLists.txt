CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE ( PerlLibs )
MESSAGE(STATUS "Perl located in ${PERL_INCLUDE_PATH}")
INCLUDE_DIRECTORIES(AFTER
	${PAYGUIDE_SRC}
	${PAYGUIDE_SRC}/lib/libbonbon/
	${PERL_INCLUDE_PATH}
	)
FIND_LIBRARY(PCRE_LIBRARY pcre)
SET(BONBON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/payguide)
#SET(BONBON_SRC ${PAYGUIDE_SRC})

ADD_LIBRARY(payguide_legacy
	SHARED
	${BONBON_SRC}/lib/libbonbon/bonbon.cc
	${BONBON_SRC}/lib/libbonbon/cjobmanager.cc
	${BONBON_SRC}/lib/libbonbon/cthread.cc
	${BONBON_SRC}/lib/libbonbon/clock.cc
	${BONBON_SRC}/lib/libbonbon/cmodule.cc
	${BONBON_SRC}/lib/libbonbon/cbreak.cc
	${BONBON_SRC}/lib/libbonbon/csemaphore.cc
	${BONBON_SRC}/lib/libbonbon/cthreadjobs.cc
	${BONBON_SRC}/lib/libbonbon/cmutex.cc
	${BONBON_SRC}/lib/libbonbon/cjob.cc
	${BONBON_SRC}/lib/libbonbon/ctparam.cc



		${PAYGUIDE_SRC}/src/thrdmngr.cpp
		${PAYGUIDE_SRC}/src/log.cpp
		${PAYGUIDE_SRC}/src/perlm.cpp
		${PAYGUIDE_SRC}/src/db.cpp
		${PAYGUIDE_SRC}/src/core.cpp
		${PAYGUIDE_SRC}/src/paycheck.cpp
		${PAYGUIDE_SRC}/src/format.cpp
		${PAYGUIDE_SRC}/src/ctl.cpp
		${PAYGUIDE_SRC}/src/operator.cpp
		${PAYGUIDE_SRC}/src/statistic.cpp
		${PAYGUIDE_SRC}/src/paysys.cpp
		${PAYGUIDE_SRC}/src/initsys.cpp
		${PAYGUIDE_SRC}/src/sem.cpp
		${PAYGUIDE_SRC}/src/parser.cpp
		#		${PAYGUIDE_SRC}/src/main.cpp
		${PAYGUIDE_SRC}/src/worker.cpp
		${PAYGUIDE_SRC}/src/uremote_socket.cpp

		${PAYGUIDE_SRC}/src/xmlpaycheck/cpaycheckcore.cc
		${PAYGUIDE_SRC}/src/xmlpaycheck/cconfig.cc
		${PAYGUIDE_SRC}/src/xmlpaycheck/cserver.cc
		${PAYGUIDE_SRC}/src/xmlpaycheck/cpayguiderequest.cc
		${PAYGUIDE_SRC}/src/xmlpaycheck/csocket.cc
		${PAYGUIDE_SRC}/src/xmlpaycheck/cpaycheckthreadparam.cc
	)

TARGET_LINK_LIBRARIES(payguide_legacy pthread ${PERL_LIBRARY} ${PCRE_LIBRARY})
INSTALL(TARGETS payguide_legacy DESTINATION /usr/local/lib)
