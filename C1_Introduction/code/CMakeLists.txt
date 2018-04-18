project(time_tcp)
include_directories("../../Code")
set(LIB_PATH "../../Code")
set(BASE_C ${LIB_PATH}/unp_base.c)

add_executable(time_server server.c ${BASE_C})
add_executable(time_client client.c ${BASE_C})
