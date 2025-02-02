# Definir a versão mínima do CMake e o nome do projeto
cmake_minimum_required(VERSION 3.10)
project(BlockchainAPI)

# Definir a versão do C++ que você está usando
set(CMAKE_CXX_STANDARD 11)

# Localização dos arquivos fonte
set(SOURCE_FILES
    main.cpp
    blockchain.cpp
)

# Procurar as dependências usando find_package
find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(nlohmann_json REQUIRED)

# Incluir diretórios
include_directories(${CMAKE_SOURCE_DIR})

# Adicionar o executável
add_executable(BlockchainAPI ${SOURCE_FILES})

# Linkar as bibliotecas necessárias
target_link_libraries(BlockchainAPI
    cpprest
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    pthread  # Necessário para o C++ REST SDK
)

