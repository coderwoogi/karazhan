# modules/mod-random-quest/CMakeLists.txt

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/RandomQuestSystem.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/npc_random_quest_giver.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/RandomQuest_WorldScript.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER("RandomQuest" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

message(STATUS ">> mod-random-quest: Loaded")
