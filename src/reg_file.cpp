#include <systemc.h> // include the systemC header file
using namespace sc_core; // use namespace
using namespace sc_dt; // use namespace
using namespace std;

SC_MODULE(regfile) {
    // Sinais básicos
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // Leitura
    sc_in<sc_uint<3>> address1;
    sc_in<sc_uint<3>> address2;
    sc_out<sc_uint<16>> read_data1;
    sc_out<sc_uint<16>> read_data2;

    // Escrita
    sc_in<bool> write_enable;
    sc_in<sc_uint<16>> write_data;
    sc_in<sc_uint<3>> address3;

    // Registradores internos (8 x 16 bits)
    sc_uint<16> registers[8];

    // Escrita síncrona
    void write_process() {
        if (!rst_n.read()) {
            for (int i = 0; i < 8; i++) {
                registers[i] = 0;
            }
        } else if (write_enable.read() && address3.read() != 0) {
            registers[address3.read()] = write_data.read();
        }
    }

    // Leitura combinacional
    void read_process() {
        read_data1.write(registers[address1.read()]);
        read_data2.write(registers[address2.read()]);
    }

    SC_CTOR(regfile) {
        SC_METHOD(write_process);
        sensitive << clk.pos();

        SC_METHOD(read_process);
        sensitive << address1 << address2 << clk.pos();
    }
};
