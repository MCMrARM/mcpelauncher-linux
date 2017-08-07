cmake_minimum_required(VERSION 2.8)
project(mcpelauncher)

enable_language(C ASM)

#SET(CMAKE_CXX_COMPILER "g++-4.9")
#SET(CMAKE_C_COMPILER "gcc-4.9")

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(X11 REQUIRED)

include(FindEGL.cmake)

set(CEF_ENABLED true)
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/libs/cef)
    message(WARNING "CEF not found. Xbox Live login will be disabled.\nTo set up CEF please run setup_cef.sh in the source directory.")
    set(CEF_ENABLED false)
endif()

link_directories(${CMAKE_SOURCE_DIR}/libs/cef/)

include_directories(${PNG_INCLUDE_DIRS})
include_directories(${EGL_INCLUDE_DIRS})
include_directories(eglut)
include_directories(libs/rapidxml)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0")

set(HYBRIS_SOURCES hybris/src/cache.c hybris/src/dlfcn.c hybris/src/hooks.c hybris/src/hooks_shm.c hybris/src/logging.c hybris/src/properties.c hybris/src/strlcpy.c hybris/src/sysconf.c hybris/src/jb/dlfcn.c hybris/src/jb/linker.c hybris/src/jb/linker_environ.c hybris/src/jb/linker_format.c hybris/src/jb/rt.c)
set(EGLUT_SOURCES eglut/eglut.c eglut/eglut_x11.c)

set(SOURCE_FILES src/main.cpp src/common.cpp src/hook.cpp src/amdfix.s src/linux_appplatform.cpp src/minecraft/types.cpp src/minecraft/ImagePickingCallback.h src/minecraft/FilePickerSettings.h src/minecraft/Xbox.h src/minecraft/string.cpp src/minecraft/string.h src/base64.cpp src/base64.h src/msa.cpp src/msa.h src/msa_token.h src/msa_network.cpp src/msa_network.h src/xboxlive.h src/xboxlive.cpp src/cll.cpp src/cll.h src/async_result_util.h)
set(SOURCE_FILES_CEF)
if (CEF_ENABLED)
    set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_FILES_CEF} src/browser.cpp src/browser.h src/xbox_login_browser.h src/xbox_login_browser.cpp)
endif()
add_executable(mcpelauncher ${HYBRIS_SOURCES} ${EGLUT_SOURCES} ${SOURCE_FILES})
target_link_libraries(mcpelauncher ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt libEGL.so libGLESv2.so ${X11_X11_LIB} uuid z curl crypto)
target_compile_definitions(mcpelauncher PRIVATE ANDROID_X86_LINKER _GNU_SOURCE LINKER_TEXT_BASE=0xB0000100 LINKER_AREA_SIZE=0x01000000 LINKER_DEBUG=1)
set_target_properties(mcpelauncher PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

if (CEF_ENABLED)
    include_directories(libs/cef)
    target_link_libraries(mcpelauncher libcef.so libcef_dll_wrapper.a)
else()
    target_compile_definitions(mcpelauncher PRIVATE DISABLE_CEF)
endif()

add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/libs/cef/runtime/ $<TARGET_FILE_DIR:mcpelauncher>)