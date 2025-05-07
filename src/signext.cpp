#include <systemc.h>
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(signext) {
    // Entradas
    sc_in<sc_uint<12>> raw_src;
    sc_in<bool> imm_source;

    // Saída
    sc_out<sc_uint<16>> immediate;

    void process() {
        sc_uint<6> gathered_imm; // precisa ser signed para sign-extension

        if (imm_source.read() == 0)
            gathered_imm = raw_src.read().range(5, 0);  // bits 5:0
        else
            gathered_imm = raw_src.read().range(11, 6); // bits 11:6

        // Sign-extend de 6 para 16 bits
        sc_uint<16> extended = gathered_imm;

        immediate.write(extended);
    }

    SC_CTOR(signext) {
        SC_METHOD(process);
        sensitive << raw_src << imm_source;
    }
};
