AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/RewardPlayedTime.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/PlaytimeRewardLog.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/RPT_loader.cpp")

AC_ADD_SCRIPT_LOADER("mod_reward_played_time_improved" "${CMAKE_CURRENT_LIST_DIR}/src/RPT_loader.cpp")
