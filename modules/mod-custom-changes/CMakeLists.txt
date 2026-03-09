# modules/mod-custom-changes/CMakeLists.txt

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/CustomChanges.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/CustomChangesLoader.cpp")

AC_ADD_SCRIPT_LOADER("mod_custom_changes" "${CMAKE_CURRENT_LIST_DIR}/src/CustomChangesLoader.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/mod_custom_changes.conf.dist")
