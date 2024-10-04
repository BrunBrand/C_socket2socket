project(quick_socket)

set(
        HEADER_FILES portable_sockets.h
        socket_client.h socket_server.h
        socket_util.h
)


set(
        SOURCE_FILES socket_client.c
        socket_server.c socket_util.c
)

add_library(quick_socket SHARED ${HEADER_FILES} ${SOURCE_FILES})

if (WIN32)
    target_link_libraries(quick_socket ws2_32.lib)
endif()