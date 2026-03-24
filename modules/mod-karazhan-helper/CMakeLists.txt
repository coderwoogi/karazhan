AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/KarazhanHelper.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER(
  "mod_karazhan_helper"
  "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")
