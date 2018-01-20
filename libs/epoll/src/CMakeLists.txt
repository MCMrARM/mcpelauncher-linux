set(INCLUDES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")
add_library(epoll STATIC epoll.c signalfd.c common.c)
target_include_directories(epoll PUBLIC "${INCLUDES_DIR}")
