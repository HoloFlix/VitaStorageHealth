cmake_minimum_required(VERSION 3.10)
project(VitaStorageHealth C)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "Toolchain file")
  else()
    message(FATAL_ERROR "Please set VITASDK environment variable or pass -DCMAKE_TOOLCHAIN_FILE")
  endif()
endif()

include("${VITASDK}/share/vita.toolchain.cmake" OPTIONAL)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3")

add_executable(VitaStorageHealth
    src/main.c
    src/storage.c
    src/benchmark.c
    src/integrity.c
    src/tools.c
    src/plugins.c
    src/ui.c
)

target_link_libraries(VitaStorageHealth
    vita2d
    png
    z
    ScePgf_stub
    SceDisplay_stub
    SceGxm_stub
    SceSysmodule_stub
    SceCtrl_stub
    SceTouch_stub
    SceAppMgr_stub
    ScePower_stub
    SceRtc_stub
    SceCommonDialog_stub
    SceAppUtil_stub
    m
    c
)

vita_create_self(eboot.bin VitaStorageHealth)
vita_create_vpk(VitaStorageHealth.vpk "VSHL00001" eboot.bin
    NAME "Vita Storage Health"
    VERSION "01.00"
    IMAGE icon0.png sce_sys/icon0.png
    IMAGE pic0.png sce_sys/pic0.png
    FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    FILE sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
    FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE art/sd2vita.png art/sd2vita.png
    FILE art/memorycard.png art/memorycard.png
    FILE art/internal.png art/internal.png
    FILE art/bg0.png art/bg0.png
)
