cmake_minimum_required(VERSION 3.16)
project(bms_core_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Avisos de compilação rigorosos
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ====================================================================
# CONFIGURAÇÃO DO GOOGLE TEST LOCAL (Substitui o FetchContent)
# ====================================================================
# Se você usa Apple Silicon (M1/M2/M3) com Homebrew, descomente a linha abaixo:
# list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# Se você usa Mac Intel com Homebrew, descomente a linha abaixo:
list(APPEND CMAKE_PREFIX_PATH "/Users/pedropereira/Documents/Work/googletest")
# TODO (entrega): esse caminho só existe no seu Mac. Antes de entregar,
# trocar pela condicional: find_package primeiro, FetchContent como reserva.

# Procura o Google Test instalado no sistema operacional
find_package(GTest REQUIRED)
# ====================================================================

# ====================================================================
# CÓDIGO-FONTE
# ====================================================================
# Regra: o executável (bms_app) só recebe o main.cpp.
# Tudo que é lógica fica na biblioteca (bms_core), para que a aplicação
# E os testes usem o MESMO código, compilado uma vez só.
# (bms_core e bms_app são NOMES DE ALVOS, não arquivos. O CMake gera
#  build/libbms_core.a e build/bms_app a partir deles.)
set(SOURCES
        src/main.cpp
        include/bms/MovingAverageFilter.hpp
)

# Só os .cpp. Headers (.hpp) não entram aqui: o compilador os acha pelo
# include/ (ver [2]). Templates (MovingAverageFilter, RingBufferEventSink)
# e interfaces (IClock, IDataSource...) ficam só em .hpp.
#
# [5] Cada arquivo abaixo precisa ser do core. Pergunta de controle:
#     ele depende de arquivo, terminal ou relógio do sistema operacional?
#     Se sim, é plataforma (platform_desktop), não core.
#       - AuditLogger.cpp:    gravar em arquivo é plataforma (FileEventWriter).
#                             O core só deposita eventos no buffer.
#       - EstimatorModels.cpp: ProfileEstimateModel lê arquivo (load(path)).
#                             Core ou plataforma?
#       - DataInjector.cpp:   AnomalyInjector está no diagrama do núcleo.
#                             Parece certo.
#     Se o core incluir <fstream> ou <iostream>, viola o Req. 7.
set(LIB
        src/CoreController.cpp
        src/States.cpp
        src/SafetyMonitor.cpp
        src/EstimatorModels.cpp   # TODO: core ou plataforma?
        src/DataInjector.cpp
        src/AuditLogger.cpp       # TODO: core ou plataforma?
)

add_executable(bms_app ${SOURCES})

# [3] STATIC declarado de forma explícita. Sem a palavra, o tipo depende
#     da variável global BUILD_SHARED_LIBS e pode mudar sem aviso.
#     STATIC = os .cpp viram um arquivo .a que é "colado" em quem usa.
add_library(bms_core STATIC ${LIB})


# [2] O include/ pertence à BIBLIOTECA, em modo PUBLIC:
#       PRIVATE   = só para compilar a própria biblioteca
#       INTERFACE = só para quem usa a biblioteca
#       PUBLIC    = os dois (é o nosso caso: a biblioteca compila com os
#                   headers E quem a usa também precisa deles)
#     Quem ligar com bms_core herda esse include automaticamente. Por isso
#     a linha antiga target_include_directories(bms_app ...) foi removida.
target_include_directories(bms_core PUBLIC include)


# [4] Flags de tempo real (Req. 8 ii) só no core, e PRIVATE de propósito:
#     PRIVATE = valem só para compilar o bms_core. Se fossem PUBLIC, o
#     test_bms herdaria -fno-exceptions e o GoogleTest (cujas macros usam
#     try/catch) deixaria de compilar.
#     O if(NOT MSVC) existe porque essas flags são de GCC/Clang; o MSVC
#     usa outras (/EHs-c- e /GR-).
if(NOT MSVC)
    target_compile_options(bms_core PRIVATE -fno-exceptions -fno-rtti)
endif()

# [1] O bms_app precisa conhecer a biblioteca, senão o link falha com
#     "undefined reference". É o mesmo comando que você já usou com o
#     gtest_main. PRIVATE porque ninguém depende do bms_app.
target_link_libraries(bms_app PRIVATE bms_core)
add_subdirectory(test_bms)
