AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/InstanceBonusMission.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT_LOADER(
  "mod_instance_bonus_mission"
  "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp"
)

AC_ADD_CONFIG_FILE(
  "${CMAKE_CURRENT_LIST_DIR}/conf/mod_instance_bonus_mission.conf.dist"
)
