AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/SoloArena.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_solo_arena"
  "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_CONFIG_FILE(
  "${CMAKE_CURRENT_LIST_DIR}/conf/mod_solo_arena.conf.dist")
