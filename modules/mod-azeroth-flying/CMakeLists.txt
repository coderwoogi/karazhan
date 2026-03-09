AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/AzerothFlying.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_azeroth_flying" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

message(STATUS ">> mod-azeroth-flying: Loaded")
