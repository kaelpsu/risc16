#include <systemc.h> // include the systemC header file
#include "../src/ula.cpp" // Include the ULA module header
using namespace sc_core;
using namespace std;

SC_MODULE(ula_tb) {
    // Signals
    sc_signal<sc_uint<3>> ula_op;
    sc_signal<sc_uint<16>> src1, src2;
    sc_signal<sc_uint<16>> ula_result;
    sc_signal<bool> zero, negative;

    // DUT
    ula* uut;

    void test() {
        cout << "Starting ULA test...\n";

        // Test 1: Addition 5 + 10
        ula_op.write(0);        // ula_op = 000 → addition
        src1.write(5);
        src2.write(10);
        wait(1, SC_NS);

        cout << "Add 5 + 10 = " << ula_result.read()
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;             

        ula_op.write(1);        // ula_op = 001 → subtraction
        src1.write(10);
        src2.write(5);
        wait(1, SC_NS);

        cout << "Subtract 10 - 5 = " << ula_result.read()
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;

        ula_op.write(2);        // ula_op = 010 → AND
        src1.write(0b1100); 
        src2.write(0b1010);
        wait(1, SC_NS);

        cout << "AND 1100 & 1010 = " << ula_result.read().to_string(SC_BIN)
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;

        ula_op.write(3);        // ula_op = 011 → OR
        src1.write(0b1100); 
        src2.write(0b1010);
        wait(1, SC_NS);

        cout << "OR 1100 | 1010 = " << ula_result.read().to_string(SC_BIN)
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;

        ula_op.write(4);        // ula_op = 100 → XOR
        src1.write(0b1100); 
        src2.write(0b1010);
        wait(1, SC_NS);

        cout << "XOR 1100 ^ 1010 = " << ula_result.read().to_string(SC_BIN)
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;


        ula_op.write(5);
        src1.write(0b1100); 
        wait(1, SC_NS);
        cout << "NOT ~1100 = " << ula_result.read().to_string(SC_BIN)
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;             


        ula_op.write(6);        // ula_op = 110 → CMP
        src1.write(5);
        src2.write(5);
        wait(1, SC_NS);
        cout << "Compare 5 == 5: " << ula_result.read()
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;

        // Test 2: Compare 5 != 10
        src1.write(5);
        src2.write(10);
        wait(1, SC_NS);
        cout << "Compare 5 == 10: " << ula_result.read()
             << ", zero: " << zero.read()
             << ", negative: " << negative.read() << endl;

        sc_stop();
    }

    SC_CTOR(ula_tb) {
        uut = new ula("ula");
        uut->ula_op(ula_op);
        uut->src1(src1);
        uut->src2(src2);
        uut->ula_result(ula_result);
        uut->zero(zero);
        uut->negative(negative);

        SC_THREAD(test);
    }

    ~ula_tb() {
        delete uut;
    }
};

int sc_main(int argc, char* argv[]) {
    ula_tb tb("tb");
    sc_start(); // roda a simulação
    return 0;
}