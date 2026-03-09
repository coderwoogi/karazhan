# modules/mod-blackmarket/CMakeLists.txt

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/BlackMarketSystem.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/BlackMarketNPC.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/BlackMarketLoader.cpp")

AC_ADD_SCRIPT_LOADER("mod_blackmarket" "${CMAKE_CURRENT_LIST_DIR}/src/BlackMarketLoader.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/mod_blackmarket.conf.dist")
