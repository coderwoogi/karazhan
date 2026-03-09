AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/ItemKarazhan.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/npc_item_karazhan.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/ItemKarazhan_WorldScript.cpp")  # ★ 추가
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_item_karazhan" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")
