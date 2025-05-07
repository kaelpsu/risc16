#include <systemc.h>
using namespace sc_core;
using namespace sc_dt;


SC_MODULE(control) {
    // Entradas
    sc_in<sc_uint<4>> op;
    sc_in<bool> ula_zero;
    sc_in<bool> ula_negative;

    // Saídas
    sc_out<sc_uint<3>> ula_op;
    sc_out<bool> imm_source;
    sc_out<bool> mem_write;
    sc_out<bool> reg_write;
    sc_out<bool> ula_src;
    sc_out<bool> mem_to_reg;
    sc_out<bool> pc_src;

    void decode() {
        // Defaults
        ula_op.write(0b000);
        imm_source.write(false);
        mem_write.write(false);
        reg_write.write(false);
        ula_src.write(false);
        mem_to_reg.write(false);
        pc_src.write(false);

        switch (op.read()) {
            case 0b0000: // ADD
                reg_write.write(true);
                ula_op.write(0b000);
                break;

            case 0b0001: // SUB
                reg_write.write(true);
                ula_op.write(0b001);
                break;

            case 0b0010: // AND
                reg_write.write(true);
                ula_op.write(0b010);
                break;

            case 0b0011: // OR
                reg_write.write(true);
                ula_op.write(0b011);
                break;

            case 0b0100: // XOR
                reg_write.write(true);
                ula_op.write(0b100);
                break;

            case 0b0101: // NOT
                reg_write.write(true);
                ula_op.write(0b101);
                break;

            case 0b0110: // CMP
                reg_write.write(false);
                ula_op.write(0b110);
                break;

            case 0b0111: // LD
                reg_write.write(true);
                ula_op.write(0b000);
                ula_src.write(true);
                mem_to_reg.write(true);
                imm_source.write(false);
                break;

            case 0b1000: // ST
                reg_write.write(false);
                mem_write.write(true);
                ula_op.write(0b000);
                ula_src.write(true);
                imm_source.write(false);
                break;

            case 0b1001: // J
                reg_write.write(false);
                pc_src.write(true);
                imm_source.write(true);
                break;

            case 0b1010: // JN
                reg_write.write(false);
                pc_src.write(ula_negative.read());
                imm_source.write(true);
                break;

            case 0b1011: // JZ
                reg_write.write(false);
                pc_src.write(ula_zero.read());
                imm_source.write(true);
                break;

            default: // NOP ou desconhecida
                break;
        }
    }

    SC_CTOR(control) {
        SC_METHOD(decode);
        sensitive << op << ula_zero << ula_negative;
    }
};
