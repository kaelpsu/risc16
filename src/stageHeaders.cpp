#ifndef STAGES_H
#define STAGES_H

#include <systemc.h>
#include "memory.cpp"
#include "ula.cpp"
#include "signext.cpp"
#include "reg_file.cpp"
#include "control.cpp"

// IF Stage
SC_MODULE(IFStage) {
    sc_in<bool> clk, rst_n;
    sc_in<sc_uint<16>> pc_next;
    sc_out<sc_uint<16>> pc_out, instruction;

    memory *instruction_memory;

    void pc_process() {
        if (!rst_n.read())
            pc_out.write(0);
        else if (clk.posedge())
            pc_out.write(pc_next.read());
    }

    SC_CTOR(IFStage) {
        instruction_memory = new memory("instruction_memory");
        instruction_memory->load_memory("../test_imemory.bin");

        SC_METHOD(pc_process);
        sensitive << clk.pos();

        instruction_memory->clk(clk);
        instruction_memory->rst_n(rst_n);
        instruction_memory->address(pc_out);

        sc_signal<sc_uint<16>> null_data;
        sc_signal<bool> write_enable;
        write_enable.write(false);
        instruction_memory->write_enable(write_enable);
        instruction_memory->write_data(null_data);
        instruction_memory->read_data(instruction);
    }
};

// ID Stage
SC_MODULE(IDStage) {
    sc_in<bool> clk;
    sc_in<sc_uint<16>> instruction;

    sc_out<sc_uint<4>> op;
    sc_out<sc_uint<3>> source_reg1, source_reg2, dest_reg;
    sc_out<sc_uint<16>> read_data1, read_data2, immediate;
    sc_out<bool> imm_source;

    regfile *regfile_inst;
    signext *sign_extender;

    void decode_instr() {
        auto instr = instruction.read();
        op.write(instr.range(15, 12));
        dest_reg.write(instr.range(11, 9));
        source_reg1.write(instr.range(8, 6));
        source_reg2.write(instr.range(5, 3));
    }

    SC_CTOR(IDStage) {
        regfile_inst = new regfile("regfile");
        sign_extender = new signext("signext");

        SC_METHOD(decode_instr);
        sensitive << instruction;
    }
};

// EX Stage
SC_MODULE(EXStage) {
    sc_in<sc_uint<3>> ula_op;
    sc_in<sc_uint<16>> read_data1, read_data2, immediate;
    sc_in<bool> ula_src;

    sc_out<sc_uint<16>> ula_result;
    sc_out<bool> zero, negative;

    sc_signal<sc_uint<16>> src2_signal;

    ula *ula_inst;

    void select_src2() {
        src2_signal.write(ula_src.read() ? immediate.read() : read_data2.read());
    }

    SC_CTOR(EXStage) {
        ula_inst = new ula("ula_inst");

        // Connect inputs
        ula_inst->ula_op(ula_op);
        ula_inst->src1(read_data1);
        ula_inst->src2(src2_signal);

        // Connect outputs
        ula_inst->ula_result(ula_result);
        ula_inst->zero(zero);
        ula_inst->negative(negative);

        // Multiplexador para src2
        SC_METHOD(select_src2);
        sensitive << read_data1 << read_data2 << immediate << ula_src;
    }
};


// MEM Stage
SC_MODULE(MEMStage) {
    sc_in<bool> mem_write;
    sc_in<sc_uint<16>> ula_result, write_data;

    sc_out<sc_uint<16>> mem_read;

    memory *data_memory;

    SC_CTOR(MEMStage) {
        data_memory = new memory("data_memory");
        data_memory->load_memory("../test_dmemory.bin");

        data_memory->write_enable(mem_write);
        data_memory->address(ula_result);
        data_memory->write_data(write_data);
        data_memory->read_data(mem_read);
    }
};

// WB Stage
SC_MODULE(WBStage) {
    sc_in<bool> mem_to_reg;
    sc_in<sc_uint<16>> ula_result, mem_read;

    sc_out<sc_uint<16>> write_back_data;

    void wb_mux() {
        write_back_data.write(mem_to_reg.read() ? mem_read.read() : ula_result.read());
    }

    SC_CTOR(WBStage) {
        SC_METHOD(wb_mux);
        sensitive << mem_to_reg << ula_result << mem_read;
    }
};

#endif // STAGES_H
