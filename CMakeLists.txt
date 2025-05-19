cmake_minimum_required(VERSION 3.16)

project(TmcSdkCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(TmcSdkCore SHARED
  TmcSdkCore_global.h
  tmc_application.cpp
  tmc_application.h
)

target_link_directories(TmcSdkCore PUBLIC $ENV{TMC_SDK_PATH}/current/lib/${CMAKE_HOST_SYSTEM_PROCESSOR})

target_include_directories(TmcSdkCore PUBLIC $ENV{TMC_SDK_PATH}/current/helpers/include)

target_link_libraries(TmcSdkCore PUBLIC -lTmcHelpers)

target_compile_definitions(TmcSdkCore PRIVATE TMCSDKCORE_LIBRARY)

if (WIN32)
  target_compile_definitions(TmcSdkCore PUBLIC ITS_WINDOWS_FROM_MICROSOFT)
endif()
if (APPLE)
  target_compile_definitions(TmcSdkCore PUBLIC OUR_HARD_WORK_BY_THESE_WORDS)
endif()
if(LINUX)
  target_compile_definitions(TmcSdkCore PUBLIC ID_JUST_LIKE_TO_INTERJECT)
endif()
