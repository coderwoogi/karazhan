AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/AutoLevelup.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_auto_levelup"
  "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_CONFIG_FILE(
  "${CMAKE_CURRENT_LIST_DIR}/conf/mod_auto_levelup.conf.dist")
