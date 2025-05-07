cmake_minimum_required(VERSION 3.0)

project(Risc16Project)

# Diretório do SystemC
if(NOT DEFINED ENV{SYSTEMC_HOME})
    message(FATAL_ERROR "Por favor defina a variável de ambiente SYSTEMC_HOME")
endif()

# Set the SystemC include and library directories
set(SYSTEMC_INCLUDE_DIR $ENV{SYSTEMC_HOME}/include)
set(SYSTEMC_LIBRARY_DIR $ENV{SYSTEMC_HOME}/lib-linux64)

# Diretórios do projeto
file(GLOB SRC_FILES src/*.cpp)
file(GLOB TB_FILES tests/*.cpp)

# Compilar cada testbench como executável separado
foreach(tb_file ${TB_FILES})
    get_filename_component(tb_name ${tb_file} NAME_WE)
    add_executable(${tb_name} ${SRC_FILES} ${tb_file})
    target_include_directories(${tb_name} PRIVATE ${SYSTEMC_INCLUDE_DIR})
    target_link_libraries(${tb_name} ${SYSTEMC_LIBRARY_DIR}/libsystemc.so)
endforeach()

# # Adiciona os arquivos de origem
# set(SRC_FILES
#     src/cpu.cpp
# )

# # Adiciona os arquivos de teste
# set(TB_FILES
#     tests/cpu_tb.cpp
# )

# # Adiciona o executável para o arquivo de teste
# add_executable(cpu_tb ${SRC_FILES} ${TB_FILES})

# # Link against SystemC
# target_include_directories(cpu_tb PRIVATE ${SYSTEMC_INCLUDE_DIR})
# target_link_libraries(cpu_tb ${SYSTEMC_LIBRARY_DIR}/libsystemc.so)

