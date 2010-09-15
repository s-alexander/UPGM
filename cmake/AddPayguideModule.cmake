FUNCTION(ADD_PAYGUIDE_MODULE MODNAME classname)
MESSAGE("${CMAKE_CURRENT_SOURCE_DIR}")

SET(PG_MODULE_CLASS ${classname})
SET(SKELETON "PayguideModuleSkeleton.cpp")
SET(MODPATH "${CMAKE_CURRENT_SOURCE_DIR}/${MODNAME}")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${SKELETON}.stub" "${MODPATH}/${SKELETON}")
ADD_SUBDIRECTORY(webmoney)

SET(PG_TARGET "pg_${MODNAME}")
INCLUDE("${MODPATH}/include")
ADD_LIBRARY (${PG_TARGET} SHARED 
	${MODPATH}/${SKELETON}
	)

TARGET_LINK_LIBRARIES(${PG_TARGET}
	${MODNAME}
	upgm
	pthread
	payguide_legacy
	)


INSTALL(TARGETS ${PG_TARGET} DESTINATION ${PAYGUIDE_SRC}/modules/paysys)
INSTALL(FILES ${MODPATH}/config/lib${PG_TARGET}.so.cfg.example DESTINATION ${PAYGUIDE_SRC}/modules/paysys)

INSTALL(FILES
	${MODPATH}/config/config.cfg.example
	${MODPATH}/config/scheme.cfg.example
	${MODPATH}/config/codes.cfg.example
       	DESTINATION /etc/upgm/${MODNAME})

INSTALL(FILES
	${MODPATH}/config/${MODNAME}.op.example
       	DESTINATION ${PAYGUIDE_SRC}/operators)

ENDFUNCTION(ADD_PAYGUIDE_MODULE)
