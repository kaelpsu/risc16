#ifndef ULA_H
#define ULA_H

#include <systemc.h>
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(ula) {
    // Entradas
    sc_in<sc_uint<3>> ula_op;         // Código da operação
    sc_in<sc_uint<16>> src1;          // Operando 1
    sc_in<sc_uint<16>> src2;          // Operando 2

    // Saídas
    sc_out<sc_uint<16>> ula_result;   // Resultado
    sc_out<bool> zero;                // Flag zero
    sc_out<bool> negative;            // Flag negativo

    // Lógica combinacional da ULA
    void process() {
        sc_uint<16> result = 0;
        sc_uint<3> op = ula_op.read();

        switch (op) {
            case 0b000: // ADD
                result = src1.read() + src2.read();
                break;

            case 0b001: // SUB
                result = src1.read() - src2.read();
                break;

            case 0b010: // AND
                result = src1.read() & src2.read();
                break;

            case 0b011: // OR
                result = src1.read() | src2.read();
                break;

            case 0b100: // XOR
                result = src1.read() ^ src2.read();
                break;

            case 0b101: // NOT (unário, só usa src1)
                result = ~src1.read();
                break;

            case 0b110: // CMP
                result = src1.read() == src2.read();
                break;

            default:
                result = 0;
                break;
        }

        // NOTA: Instrução CMP não altera ula_result, mas aqui ainda o usamos para gerar flags
        ula_result.write(result);
        zero.write(result == 0);
        negative.write(result[15]); // MSB como sinal (bit de sinal)
    }

    SC_CTOR(ula) {
        SC_METHOD(process);
        sensitive << ula_op << src1 << src2;
    }
};

#endif // ULA_H
