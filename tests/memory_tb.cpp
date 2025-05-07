#include <systemc.h>
#include "../src/memory.cpp" // Seu módulo de memória
#include <iostream>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(memory_tb) {
    // Sinais
    sc_signal<bool> clk;
    sc_signal<bool> rst_n;
    sc_signal<bool> write_enable;
    sc_signal<sc_uint<16>> address;
    sc_signal<sc_uint<16>> write_data;
    sc_signal<sc_uint<16>> read_data;

    // Instância da memória
    memory* mem_inst;

    // Clock Generator
    void clock_gen() {
        while (true) {
            clk.write(false);
            wait(5, SC_NS);
            clk.write(true);
            wait(5, SC_NS);
        }
    }

    // Estímulos
    void test_proc() {
        // Reset
        rst_n.write(false);
        write_enable.write(false);
        wait(10, SC_NS);
        rst_n.write(true);
        wait(10, SC_NS);

        // (Opcional) Carregar a memória inicializada a partir de arquivo
        mem_inst->load_memory("../test_dmemory.bin"); // Adicione esse arquivo no seu diretório de simulação

        // Escrita válida (endereço alinhado, dentro do limite)
        address.write(0x0004);        // Byte address 4 → word index 2
        write_data.write(0xABCD);
        write_enable.write(true);
        wait(10, SC_NS);

        // Escrita inválida (desalinhada)
        address.write(0x0005);        // desalinhado → deve ser ignorado
        write_data.write(0x1234);
        wait(10, SC_NS);

        // Escrita inválida (fora do limite)
        address.write(0x0090);        // 0x90 >> 1 = 0x48 = 72 (> 31)
        write_data.write(0xDEAD);
        wait(10, SC_NS);

        write_enable.write(false);
        wait(10, SC_NS);

        // Leitura das posições
        address.write(0x0004);        // Espera-se 0xABCD
        wait(1, SC_NS);
        cout << "Leitura [0x0004]: " << std::hex << read_data.read() << endl;

        address.write(0x0005);        // desalinhado → leitura de posição anterior
        wait(1, SC_NS);
        cout << "Leitura [0x0005]: " << std::hex << read_data.read() << endl;

        address.write(0x0090);        // fora da memória → deve ser 0
        wait(1, SC_NS);
        cout << "Leitura [0x0090]: " << std::hex << read_data.read() << endl;
        
        mem_inst->save_memory("../test_dmemory.bin"); // Salva a memória após os testes

        sc_stop(); // Finaliza simulação
    }

    SC_CTOR(memory_tb) {
        // Instancia o módulo
        mem_inst = new memory("mem_inst");

        // Conecta os sinais
        mem_inst->clk(clk);
        mem_inst->rst_n(rst_n);
        mem_inst->write_enable(write_enable);
        mem_inst->address(address);
        mem_inst->write_data(write_data);
        mem_inst->read_data(read_data);

        // Processos
        SC_THREAD(clock_gen);
        SC_THREAD(test_proc);
    }

    ~memory_tb() {
        delete mem_inst;
    }
};

int sc_main(int argc, char* argv[]) {
    memory_tb tb("tb");
    sc_start(); // roda a simulação
    return 0;
}