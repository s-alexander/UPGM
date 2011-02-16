FUNCTION(ADD_PAYGUIDE_MODULE MODNAME classname)
MESSAGE("${CMAKE_CURRENT_SOURCE_DIR}")

SET(PG_MODULE_CLASS ${classname})
SET(SKELETON "PayguideModuleSkeleton.cpp")
SET(MODPATH "${CMAKE_CURRENT_SOURCE_DIR}/${MODNAME}")
SET(INCLUDE_MODFILES
		"#include \"${MODPATH}/include/${MODNAME}.hpp\"")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${SKELETON}.stub" "${MODPATH}/${SKELETON}")
ADD_SUBDIRECTORY(${MODNAME})

SET(PG_TARGET "pg_${MODNAME}")
INCLUDE("${MODPATH}/include")
ADD_LIBRARY (${PG_TARGET} SHARED 
	${MODPATH}/${SKELETON}
	)

TARGET_LINK_LIBRARIES(${PG_TARGET}
	${MODNAME}
	upgm
	pthread
	)


INSTALL(TARGETS ${PG_TARGET} DESTINATION ${MODULES_INSTALL_PATH})
INSTALL(FILES ${MODPATH}/config/lib${PG_TARGET}.so.cfg.example DESTINATION ${MODULES_INSTALL_PATH})

INSTALL(FILES
	${MODPATH}/config/config.cfg.example
	${MODPATH}/config/scheme.cfg.example
	${MODPATH}/config/codes.cfg.example
       	DESTINATION /etc/upgm/${MODNAME})

INSTALL(FILES
	${MODPATH}/config/${MODNAME}.sql
       	DESTINATION ${PAYGUIDE_INSTALL}/sql_tables)

INSTALL(FILES
	${MODPATH}/config/${MODNAME}.op.example
       	DESTINATION ${PAYGUIDE_INSTALL}/operators)

ENDFUNCTION(ADD_PAYGUIDE_MODULE)

FUNCTION(ADD_SIMPLE_PAYGUIDE_MODULE MODNAME)
MESSAGE("${CMAKE_CURRENT_SOURCE_DIR}")

SET(PG_MODULE_CLASS "PG::UPGM")
SET(SKELETON "PayguideModuleSkeleton.cpp")
SET(MODPATH "${CMAKE_CURRENT_SOURCE_DIR}/${MODNAME}")

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${SKELETON}.stub" "${MODPATH}/${SKELETON}")

SET(PG_TARGET "pg_${MODNAME}")
ADD_LIBRARY (${PG_TARGET} SHARED 
	${MODPATH}/${SKELETON}
	)

TARGET_LINK_LIBRARIES(${PG_TARGET}
	upgm
	pthread
	)

INSTALL(TARGETS ${PG_TARGET} DESTINATION ${MODULES_INSTALL_PATH})
INSTALL(FILES ${MODPATH}/config/lib${PG_TARGET}.so.cfg.example DESTINATION ${MODULES_INSTALL_PATH})

INSTALL(FILES
	${MODPATH}/config/config.cfg.example
	${MODPATH}/config/scheme.cfg.example
	${MODPATH}/config/codes.cfg.example
       	DESTINATION /etc/upgm/${MODNAME})

INSTALL(FILES
	${MODPATH}/config/${MODNAME}.sql
       	DESTINATION ${PAYGUIDE_INSTALL}/sql_tables)

INSTALL(FILES
	${MODPATH}/config/${MODNAME}.op.example
       	DESTINATION ${PAYGUIDE_INSTALL}/operators)

ENDFUNCTION(ADD_SIMPLE_PAYGUIDE_MODULE)
