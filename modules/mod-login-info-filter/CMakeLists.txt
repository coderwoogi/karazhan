AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/LoginInfoFilter.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_login_info_filter"
  "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_CONFIG_FILE(
  "${CMAKE_CURRENT_LIST_DIR}/conf/mod_login_info_filter.conf.dist")
