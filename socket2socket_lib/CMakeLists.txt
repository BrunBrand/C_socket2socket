project(socket2socket)

set(HEADER_FILES socket2socket_lib.h)
set(SOURCE_FILES socket2socket_lib.c)

add_library(socket2socket_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})


