// Learn with Examples, 2020, MIT license
#include <systemc.h> // include the systemC header file
#include "../src/reg_file.cpp" // módulo de memória
using namespace sc_core; // use namespace
using namespace sc_dt;

SC_MODULE(regfile_tb) {
    sc_clock clk;
    sc_signal<bool> rst_n;
    sc_signal<sc_uint<3>> address1, address2, address3;
    sc_signal<sc_uint<16>> read_data1, read_data2;
    sc_signal<bool> write_enable;
    sc_signal<sc_uint<16>> write_data;

    regfile* uut;

    void test() {
        // Inicializa sinais
        rst_n.write(false);
        write_enable.write(false);
        address1.write(0);
        address2.write(0);
        address3.write(0);
        write_data.write(0);
        wait(2, SC_NS);

        // Tira o reset
        rst_n.write(true);
        wait(2, SC_NS);

        // Escreve 0xAAAA no registrador 1
        write_enable.write(true);
        address3.write(1);
        write_data.write(0xAAAA);
        wait(clk.posedge_event()); // espera borda de clock

        // Escreve 0x1234 no registrador 2
        address3.write(2);
        write_data.write(0x1234);
        wait(clk.posedge_event());

        // Desabilita escrita
        write_enable.write(false);
        wait(clk.posedge_event());

        // Lê dos registradores 1 e 2
        address1.write(1);
        address2.write(2);
        wait(1, SC_NS); // tempo pra propagação

        std::cout << "Leitura R1: 0x" << std::hex << read_data1.read()
                  << ", R2: 0x" << read_data2.read() << std::endl;

        // Testa leitura de registrador 0 (só deve ter 0)
        address1.write(0);
        address2.write(0);
        wait(1, SC_NS);

        std::cout << "Leitura R0: 0x" << std::hex << read_data1.read()
                  << ", R0: 0x" << read_data2.read() << std::endl;

        sc_stop(); // Encerra a simulação
    }

    SC_CTOR(regfile_tb) : clk("clk", 1, SC_NS) {
        uut = new regfile("regfile");
        uut->clk(clk);
        uut->rst_n(rst_n);
        uut->address1(address1);
        uut->address2(address2);
        uut->read_data1(read_data1);
        uut->read_data2(read_data2);
        uut->write_enable(write_enable);
        uut->write_data(write_data);
        uut->address3(address3);

        SC_THREAD(test);
        sensitive << clk;
    }

    ~regfile_tb() {
        delete uut;
    }
};

// Função principal
int sc_main(int argc, char* argv[]) {
    regfile_tb tb("tb");
    sc_start();
    return 0;
}
