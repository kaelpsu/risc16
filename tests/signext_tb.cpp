#include <systemc> // include the systemC header file
#include "../src/signext.cpp"
using namespace sc_core;
using namespace std;

SC_MODULE(signext_tb) {
    // Sinais
    sc_signal<sc_uint<12>> raw_src; // 16 - 4 do opcode 
    sc_signal<bool> imm_source; //(imediato varia se for ld/st ou j)
    sc_signal<sc_uint<16>> immediate;

    // DUT
    signext* uut;

    void test() {
        // Teste 1: imm_source = 0, raw_src = 0b000001111111 (positivo)
        imm_source.write(0);
        raw_src.write(0x0FF); // 0000 1111 1111
        wait(1, SC_NS);
        cout << "Test 1: imm_source=0, raw_src=0b000001111111 → immediate = " << immediate.read().to_string(SC_BIN) << endl; 
      	// Espera-se 111111

        // Teste 2: imm_source = 0, raw_src = 0b000010000000
        raw_src.write(0x080);
        wait(1, SC_NS);
        cout << "Test 2: imm_source=0, raw_src=0b000010000000 → immediate = " << immediate.read().to_string(SC_BIN) << endl;
        // Espera-se 000000

        // Teste 3: imm_source = 1, raw_src = 0b111111000000
        imm_source.write(1);
        raw_src.write(0xFC0); // bits 11:6 = 111111
        wait(1, SC_NS);
        cout << "Test 3: imm_source=1, raw_src=0b111111000000 → immediate = " << immediate.read().to_string(SC_BIN) << endl;
      	// Espera-se 111111

        // Teste 4: imm_source = 1, raw_src = 0b000000000001
        raw_src.write(0x001);
        wait(1, SC_NS);
        cout << "Test 4: imm_source=1, raw_src=0b000000000001 → immediate = " << immediate.read().to_string(SC_BIN) << endl;
      	// Espera-se 000000

        sc_stop();
    }

    SC_CTOR(signext_tb) {
        uut = new signext("signext");
        uut->raw_src(raw_src);
        uut->imm_source(imm_source);
        uut->immediate(immediate);

        SC_THREAD(test);
    }

    ~signext_tb() {
        delete uut;
    }
};

int sc_main(int argc, char* argv[]) {
    signext_tb tb("tb");
    sc_start(); // Inicia a simulação
    return 0;
}