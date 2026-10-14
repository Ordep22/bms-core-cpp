---

### Arquivo `CMakeLists.txt` (Raiz)

```cmake
cmake_minimum_required(VERSION 3.16)
project(bms_core_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Avisos de compilação rigorosos
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Diretórios de include
include(FetchContent)
include_directories(include)



FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)


# Código-fonte
set(SOURCES
    src/main.cpp
    src/CoreController.cpp
    src/States.cpp
    src/SafetyMonitor.cpp
    src/EstimatorModels.cpp
    src/DataInjector.cpp
    src/AuditLogger.cpp
)


# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_executable(bms_app ${SOURCES})
