AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/RaidResetTicket.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_raid_reset_ticket" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/mod_raid_reset_ticket.conf.dist")
