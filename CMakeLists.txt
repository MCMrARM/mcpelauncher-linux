cmake_minimum_required(VERSION 2.8)
project(mcpelauncher)

enable_language(C ASM)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(APPLE)
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-apple-darwin")
else()
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
endif(APPLE)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG)
find_package(CURL)
find_package(OpenSSL)

if(APPLE)
    find_package(OpenGL REQUIRED)
    find_package(Cocoa REQUIRED)
    find_package(CoreVideo REQUIRED)
    find_package(IOKit REQUIRED)
else()
    find_package(X11)
endif(APPLE)

include(ExternalProject)

set(BUILD_CLIENT true)
set(BUILD_SERVER true)
set(GLFW_ENABLED false)
set(CEF_ENABLED true)
set(PLAYAPI_ENABLED true)
set(LIBZIP_FROM_SOURCES false)
set(GAMEPAD_SUPPORT_ENABLED true)

if(APPLE)
    set(GLFW_ENABLED true)
    set(CEF_ENABLED false)
    set(PLAYAPI_ENABLED false)
    set(LIBZIP_FROM_SOURCES true)
    set(GAMEPAD_SUPPORT_ENABLED false)
endif(APPLE)

if (NOT CURL_FOUND OR NOT OPENSSL_FOUND OR ((NOT PNG_FOUND OR NOT X11_FOUND) AND NOT APPLE))
    message(WARNING "One of the required dependencies for building the client were not found.\nOnly the server will be built as a result. To build the client please install the missing dependencies, and run cmake again.")
    set(BUILD_CLIENT false)
    set(CEF_ENABLED false)
    set(PLAYAPI_ENABLED false)
endif()

if (GLFW_ENABLED)
    ExternalProject_Add(
            glfw
            URL "https://github.com/glfw/glfw/archive/3.2.1.tar.gz"
            URL_HASH SHA1=bee9cd9a8b4925184070ade5bc287c5635830eb0
            INSTALL_DIR ${CMAKE_BINARY_DIR}/ext/glfw
            CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/glfw" "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32" "-DCMAKE_LIBRARY_ARCHITECTURE=${CMAKE_LIBRARY_ARCHITECTURE}"
    )
    set(GLFW_LIBRARIES ${CMAKE_BINARY_DIR}/ext/glfw/lib/libglfw3.a)
    set(GLFW_INCLUDE_DIR ${CMAKE_BINARY_DIR}/ext/glfw/include/)
endif()
set(LIBZIP_LIBRARIES zip)
set(LIBZIP_INCLUDE_DIR )
if (LIBZIP_FROM_SOURCES)
    ExternalProject_Add(
            libzip
            URL "https://github.com/nih-at/libzip/archive/rel-1-4-0.tar.gz"
            URL_HASH SHA1=7b38ace94d806f200bd34d67e40c9924291b6745
            INSTALL_DIR ${CMAKE_BINARY_DIR}/ext/libzip
            CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/libzip" "-DBUILD_SHARED_LIBS=OFF" "CMAKE_CXX_FLAGS=-m32" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32" "-DCMAKE_LIBRARY_ARCHITECTURE=${CMAKE_LIBRARY_ARCHITECTURE}"
    )
    set(LIBZIP_LIBRARIES ${CMAKE_BINARY_DIR}/ext/libzip/lib/libzip.a)
    set(LIBZIP_INCLUDE_DIR ${CMAKE_BINARY_DIR}/ext/libzip/include/)

    find_package(BZip2)
    if (BZIP2_FOUND)
        set(LIBZIP_LIBRARIES ${LIBZIP_LIBRARIES} ${BZIP2_LIBRARY})
    endif()
endif()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/libs/cef)
    if(NOT APPLE)
        message(WARNING "CEF not found. Xbox Live login and APK download features will be disabled.\nTo set up CEF please run setup_cef.sh in the source directory.")
    endif(NOT APPLE)
    set(CEF_ENABLED false)
    set(PLAYAPI_ENABLED false)
endif()
if (PLAYAPI_ENABLED)
    find_package(Protobuf)
    if (NOT Protobuf_FOUND)
        message(WARNING "Protobuf not found. Downloading the apk with a Google account will not be possible.")
        set(PLAYAPI_ENABLED false)
    endif()
endif()

if (PLAYAPI_ENABLED)
    add_subdirectory(gplay_api)
    set_target_properties(gplaydl PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

if(APPLE)
    add_subdirectory(libs/epoll)
    include_directories(${Cocoa_INCLUDE_DIRS} ${CoreVideo_INCLUDE_DIRS} ${IOKit_INCLUDE_DIRS})
    include_directories(libs/elf)
endif(APPLE)

include_directories(${ZLIB_INCLUDE_DIRS} ${LIBZIP_INCLUDE_DIRS})
include_directories(libs/rapidxml)
include_directories(libs/hybris/include)

set(HYBRIS_SOURCES libs/hybris/src/cache.c libs/hybris/src/dlfcn.c libs/hybris/src/hooks.c libs/hybris/src/hooks_shm.c libs/hybris/src/logging.c libs/hybris/src/properties.c libs/hybris/src/ctype.c libs/hybris/src/sysconf.c libs/hybris/src/jb/dlfcn.c libs/hybris/src/jb/linker.c libs/hybris/src/jb/linker_environ.c libs/hybris/src/jb/linker_format.c libs/hybris/src/jb/rt.c libs/hybris/src/hooks_pthread.c libs/hybris/src/hooks_dirent.c libs/hybris/src/hooks_io.c libs/hybris/src/hooks_net.c)
if(APPLE)
    set(HYBRIS_SOURCES ${HYBRIS_SOURCES} libs/hybris/src/hooks_net_darwin.c libs/hybris/src/hooks_darwin_pthread_once.cpp)
else()
    set(HYBRIS_SOURCES ${HYBRIS_SOURCES} libs/hybris/src/strlcpy.c)
endif(APPLE)

set(EGLUT_SOURCES libs/eglut/eglut.c libs/eglut/eglut_x11.c)

set(COMMON_SOURCE_FILES src/common/common.cpp src/common/hook.cpp src/client/appplatform.cpp src/minecraft/ImagePickingCallback.h src/minecraft/FilePickerSettings.h src/minecraft/Common.h src/minecraft/Xbox.h src/minecraft/string.h src/common/base64.cpp src/common/base64.h src/common/async_result_util.h src/common/extract.cpp src/common/extract.h src/common/path_helper.cpp src/common/path_helper.h src/common/log.cpp src/common/log.h src/minecraft/symbols.cpp src/minecraft/symbols.h src/common/modloader.cpp src/common/modloader.h)
set(CLIENT_SOURCE_FILES src/client/main.cpp src/client/appplatform.cpp src/xbox/msa.cpp src/xbox/msa.h src/xbox/msa_token.h src/xbox/msa_network.cpp src/xbox/msa_network.h src/xbox/xboxlive.h src/xbox/xboxlive.cpp src/common/cll.cpp src/common/cll.h src/common/openssl_multithread.cpp src/common/openssl_multithread.h)
set(CLIENT_SOURCE_FILES_GAMEPAD src/client/gamepad.cpp src/client/gamepad.h)
set(CLIENT_SOURCE_FILES_CEF src/common/browser.cpp src/common/browser.h src/xbox/xbox_login_browser.h src/xbox/xbox_login_browser.cpp src/client/initial_setup_browser.cpp src/client/initial_setup_browser.h src/client/window.h)
set(CLIENT_SOURCE_FILES_CEF_GAMEPAD src/client/gamepad_mapper_browser.cpp src/client/gamepad_mapper_browser.h)
set(CLIENT_SOURCE_FILES_PLAYAPI gplay_api/src/config.h gplay_api/src/config.cpp src/client/google_login_browser.h src/client/google_login_browser.cpp src/client/google_play_helper.cpp src/client/google_play_helper.h)
set(CLIENT_SOURCE_FILES_EGLUT src/client/window_eglut.cpp src/client/window_eglut.h)
set(CLIENT_SOURCE_FILES_GLFW src/client/window_glfw.cpp src/client/window_glfw.h)
set(SERVER_SOURCE_FILES src/server/server.cpp src/server/server_minecraft_app.h src/server/server_properties.cpp src/server/server_properties.h)

if(APPLE)
    set(COMMON_SOURCE_FILES ${COMMON_SOURCE_FILES} src/minecraft/string_darwin.cpp src/minecraft/function.h)
else()
    set(COMMON_SOURCE_FILES ${COMMON_SOURCE_FILES} src/minecraft/string_linux.cpp)
    set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} src/client/amdfix.s)
endif(APPLE)


set(HYBRIS_DEFINES PRIVATE ANDROID_X86_LINKER _GNU_SOURCE LINKER_TEXT_BASE=0xB0000100 LINKER_AREA_SIZE=0x01000000 LINKER_DEBUG=1)

if (CEF_ENABLED)
    set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} ${CLIENT_SOURCE_FILES_CEF})
    link_directories(${CMAKE_SOURCE_DIR}/libs/cef/lib/)
endif()
if (PLAYAPI_ENABLED)
    set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} ${CLIENT_SOURCE_FILES_PLAYAPI})
endif()
if (GLFW_ENABLED)
    include_directories(${GLFW_INCLUDE_DIR})
    set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} ${CLIENT_SOURCE_FILES_GLFW})
else()
    include_directories(libs/eglut)
    set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} ${CLIENT_SOURCE_FILES_EGLUT} ${EGLUT_SOURCES})
endif()
if (GAMEPAD_SUPPORT_ENABLED)
    set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} ${CLIENT_SOURCE_FILES_GAMEPAD})
    if (CEF_ENABLED)
        set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} ${CLIENT_SOURCE_FILES_CEF_GAMEPAD})
    endif()
endif()

if (BUILD_CLIENT)
    include_directories(${PNG_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
    add_executable(mcpelauncher ${HYBRIS_SOURCES} ${COMMON_SOURCE_FILES} ${CLIENT_SOURCE_FILES})

    if (CEF_ENABLED)
        include_directories(libs/cef)
        target_link_libraries(mcpelauncher libcef.so libcef_dll_wrapper.a)
    else()
        target_compile_definitions(mcpelauncher PRIVATE DISABLE_CEF)
    endif()
    if (PLAYAPI_ENABLED)
        target_link_libraries(mcpelauncher gplayapi)
    else()
        target_compile_definitions(mcpelauncher PRIVATE DISABLE_PLAYAPI)
    endif()
    if (GLFW_ENABLED)
        add_dependencies(mcpelauncher glfw)
        target_link_libraries(mcpelauncher ${GLFW_LIBRARIES} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB} ${X11_Xxf86vm_LIB} ${X11_Xinerama_LIB})
        target_compile_definitions(mcpelauncher PRIVATE USE_GLFW)
    else()
        target_compile_definitions(mcpelauncher PRIVATE USE_EGLUT)
    endif()
    if (GAMEPAD_SUPPORT_ENABLED)
        target_link_libraries(mcpelauncher evdev udev)
        target_compile_definitions(mcpelauncher PRIVATE GAMEPAD_SUPPORT)
    endif()
    if (LIBZIP_FROM_SOURCES)
        add_dependencies(mcpelauncher libzip)
    endif()

    target_link_libraries(mcpelauncher ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${LIBZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})

    if(APPLE)
        add_dependencies(mcpelauncher epoll)
        target_link_libraries(mcpelauncher ${Cocoa_LIBRARIES} ${CoreVideo_LIBRARIES} ${IOKit_LIBRARIES} ${OPENGL_LIBRARIES} epoll)
    else()
        target_link_libraries(mcpelauncher ${X11_X11_LIB} ${PNG_LIBRARIES} rt EGL GLESv2)
    endif(APPLE)

    target_compile_definitions(mcpelauncher ${HYBRIS_DEFINES})
endif()

if (BUILD_SERVER)
    add_executable(server ${HYBRIS_SOURCES} ${COMMON_SOURCE_FILES} ${SERVER_SOURCE_FILES})

    if(LIBZIP_FROM_SOURCES)
        add_dependencies(server libzip)
    endif()

    target_link_libraries(server ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${LIBZIP_LIBRARIES})

    if(APPLE)
        add_dependencies(server epoll)
        target_link_libraries(server epoll)
    else()
        target_link_libraries(server rt)
    endif(APPLE)


    target_compile_definitions(server ${HYBRIS_DEFINES} SERVER)
endif()

function (add_non_source_dir_custom_command command...)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND $<$<NOT:$<STREQUAL:$<TARGET_FILE_DIR:mcpelauncher>,${CMAKE_SOURCE_DIR}>>:${command}>)
endfunction()

if (BUILD_CLIENT)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:mcpelauncher>/src)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/libs/cef/lib/libcef.so $<TARGET_FILE_DIR:mcpelauncher>/libcef.so)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/libs/cef/bin/icudtl.dat $<TARGET_FILE_DIR:mcpelauncher>/icudtl.dat)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/libs/cef/bin/snapshot_blob.bin $<TARGET_FILE_DIR:mcpelauncher>/snapshot_blob.bin)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/libs/cef/bin/natives_blob.bin $<TARGET_FILE_DIR:mcpelauncher>/natives_blob.bin)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/libs/cef/bin/icudtl.dat $<TARGET_FILE_DIR:mcpelauncher>/icudtl.dat)
    add_non_source_dir_custom_command(${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/src/client/initial_setup_resources $<TARGET_FILE_DIR:mcpelauncher>/src/client/initial_setup_resources)
    add_non_source_dir_custom_command(${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/libs $<TARGET_FILE_DIR:mcpelauncher>/libs)
    add_non_source_dir_custom_command(${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/mcpelauncher-icon.png $<TARGET_FILE_DIR:mcpelauncher>/mcpelauncher-icon.png)

    install(TARGETS mcpelauncher
            RUNTIME DESTINATION share/mcpelauncher/bin/)
    install(FILES libs/cef/lib/libcef.so libs/cef/bin/snapshot_blob.bin libs/cef/bin/natives_blob.bin libs/cef/bin/icudtl.dat start_mcpelauncher.sh
            DESTINATION share/mcpelauncher/bin
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
    install(FILES mcpelauncher-icon.png
            DESTINATION share/mcpelauncher
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
    install(DIRECTORY libs/cef/res
            DESTINATION share/mcpelauncher/libs/cef
            PATTERN "*"
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
    install(DIRECTORY src/client/initial_setup_resources
            DESTINATION share/mcpelauncher/src/client
            PATTERN "*"
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
    install(DIRECTORY libs/native
            DESTINATION share/mcpelauncher/libs
            PATTERN "*"
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
    install(DIRECTORY DESTINATION bin/)
    install(CODE "
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_INSTALL_PREFIX}/share/mcpelauncher/bin/start_mcpelauncher.sh ${CMAKE_INSTALL_PREFIX}/bin/mcpelauncher
    )")
    install(FILES mcpelauncher.desktop DESTINATION share/applications/)
    install(FILES mcpelauncher-icon.png DESTINATION share/pixmaps/)
endif()
