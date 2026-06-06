# modules/mod-karazhan-commands/CMakeLists.txt

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/KarazhanCommands.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/KarazhanCommandsLoader.cpp")

AC_ADD_SCRIPT_LOADER("mod_karazhan_commands" "${CMAKE_CURRENT_LIST_DIR}/src/KarazhanCommandsLoader.cpp")
