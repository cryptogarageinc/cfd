if(${ENABLE_DEBUG})
if(MSVC)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /Od /Zi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od /Zi")
else()
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")
endif()
else()
if(MSVC)
else()
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
endif()
endif()

if(${USE_EMSCRIPTEN})
# feature: -fwasm-exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
endif()

if(${STD_CPP_VERSION})
set(CMAKE_CXX_STANDARD  ${STD_CPP_VERSION})
message(STATUS "[STD_CPP_VERSION] ${STD_CPP_VERSION}")
if(${STD_CPP_VERSION} STREQUAL "11")
set(ENABLE_LOGGING FALSE)
else()
set(ENABLE_LOGGING TRUE)
endif()
else()
set(CMAKE_CXX_STANDARD 14)
set(ENABLE_LOGGING FALSE)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
