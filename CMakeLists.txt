# Generated Cmake Pico project file
#
# Este arquivo CMakeLists.txt configura o projeto para compilar no Raspberry Pi Pico
# Utiliza o Pico SDK para gerenciar a compilação e as dependências de hardware.

cmake_minimum_required(VERSION 3.13)

# Define os padrões para C e C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Gera o arquivo compile_commands.json, útil para ferramentas de análise de código (como IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
# Estas linhas são frequentemente usadas por extensões do VS Code para encontrar o SDK
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1) # Verifique se a versão do SDK corresponde à sua instalação
set(toolchainVersion 14_2_Rel1) # Verifique se a versão do toolchain corresponde à sua instalação
set(picotoolVersion 2.1.1) # Verifique se a versão do picotool corresponde à sua instalação
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Define o tipo da placa. Use 'pico' para Raspberry Pi Pico padrão.
# Se estiver usando a Pico W, mantenha 'pico_w'.
set(PICO_BOARD pico CACHE STRING "Board type")

# Inclui o script do Raspberry Pi Pico SDK (deve vir antes da definição do projeto)
include(pico_sdk_import.cmake)

# Define o projeto com os idiomas C, C++ e ASM
project(ATIVCap3TratamentoDeInterrupcoes C CXX ASM)

# Inicializa o Raspberry Pi Pico SDK
pico_sdk_init()

# Adiciona o executável principal do projeto
# Inclui o arquivo C principal (main.c)
# E inclui o cabeçalho PIO gerado (ws2812.pio.h), indicando que o executável depende dele
add_executable(${PROJECT_NAME}
    main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ws2812.pio.h # Inclua o cabeçalho PIO gerado aqui
)

# Gera o cabeçalho C a partir do arquivo fonte PIO (.pio)
# Isso compila o programa PIO e o torna acessível no código C via ws2812.pio.h
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/ws2812.pio)

# Define nome e versão para o programa (usado em arquivos .uf2, etc.)
pico_set_program_name(${PROJECT_NAME} "ATIVCap3TratamentoDeInterrupcoes")
pico_set_program_version(${PROJECT_NAME} "0.1")

# Habilita a saída Standard I/O (stdio)
# 0 para desabilitar, 1 para habilitar
# Habilita stdio via USB para ver a saída no monitor serial do computador
pico_enable_stdio_uart(${PROJECT_NAME} 0) # Desabilitado via UART
pico_enable_stdio_usb(${PROJECT_NAME} 1) # Habilitado via USB

# Linka as bibliotecas necessárias para o projeto
# pico_stdlib: Biblioteca padrão do Pico
# hardware_adc: Para usar o Conversor Analógico-Digital
# hardware_timer: Para usar os timers de hardware (timer periódico)
# hardware_pio: Para usar o periférico PIO (controle dos Neopixels)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_adc      # Adicionado: necessário para ler o microfone
    hardware_timer    # Adicionado: necessário para o timer periódico
    hardware_pio      # Adicionado: necessário para controlar os Neopixels via PIO
)

# Adiciona diretórios de inclusão privados
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Adiciona saídas extras úteis (como .uf2 para fácil upload)
pico_add_extra_outputs(${PROJECT_NAME})

# Fim do arquivo CMakeLists.txt