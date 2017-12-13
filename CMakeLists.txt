project(ps_bench C)
cmake_minimum_required(VERSION 3.7)

if(NOT MARIADB)
  set(MARIADB mariadb_config)
endif()
exec_program(${MARIADB} 
             ARGS "--include"
             OUTPUT_VARIABLE MARIADB_INCLUDE_DIR)
add_definitions(${MARIADB_INCLUDE_DIR})
exec_program(${MARIADB} 
             ARGS "--libs"
             OUTPUT_VARIABLE MARIADB_LIBRARY_DIR)
# since we use the static library we need the directory part only
set(DEFAULT_LIBS  ${MARIADB_LIBRARY_DIR})
string(REPLACE "-lmariadb " "" DEFAULT_LIBS ${DEFAULT_LIBS})
string(SUBSTRING ${MARIADB_LIBRARY_DIR} 2 -1 MARIADB_LIBRARY_DIR)
string(FIND ${MARIADB_LIBRARY_DIR} " -l" MY_LENGTH)
string(SUBSTRING ${MARIADB_LIBRARY_DIR} 0 ${MY_LENGTH} MARIADB_LIBRARY_DIR)

add_executable(ps_bench ps_bench.c)
target_link_libraries(ps_bench ${MARIADB_LIBRARY_DIR}/libmariadbclient.a ${DEFAULT_LIBS} -lz)
