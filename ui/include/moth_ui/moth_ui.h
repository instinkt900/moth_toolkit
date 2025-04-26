#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(MOTH_UI_BUILD_SHARED)
    #if defined(MOTH_UI_BUILD)
      #define MOTH_UI_API __declspec(dllexport)
    #else
      #define MOTH_UI_API __declspec(dllimport)
    #endif
  #else
    #define MOTH_UI_API
  #endif
#else
  #if defined(MOTH_UI_BUILD_SHARED)
    #if defined(MOTH_UI_BUILD)
      #define MOTH_UI_API __attribute__((visibility("default")))
    #else
      #define MOTH_UI_API
    #endif
  #else
    #define MOTH_UI_API
  #endif
#endif
