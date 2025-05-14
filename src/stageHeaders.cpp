#ifndef STAGES_H
#define STAGES_H

#include <systemc.h>
#include "memory.cpp"
#include "ula.cpp"
#include "signext.cpp"
#include "reg_file.cpp"

// IF Stage
SC_MODULE(IFStage) {
    sc_in<bool> clk, rst_n;
    // talvez pc e pc_next sejam signals
    sc_in<sc_uint<16>> pc, pc_next, pc_src, pc_branch;
    sc_out<sc_uint<16>> pc_out, instruction;

    memory *instruction_memory;

    void pc_process() {
        if (!rst_n.read())
            pc.write(0);
        else if (clk.posedge())
            pc.write(pc_next.read());
    }

    void pc_next_logic() {
        switch (pc_src)
        {
        case 0b0: // PC + 2
            pc_next.write(pc_out.read());
            break;
        case 0b1: // PC + 2 + (imediato << 1)
            pc_next.write(pc_branch.read());
            break;
        
        default:
            break;
        }
    }

    void somador() {
        pc_out.write(pc.read() + 2);
    }

    SC_CTOR(IFStage) {
        instruction_memory = new memory("instruction_memory");
        instruction_memory->load_memory("../test_imemory.bin");

        SC_METHOD(pc_process);
        sensitive << clk.pos();
        SC_METHOD(pc_next_logic);
        sensitive << clk.pos();
        SC_METHOD(somador);
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
    sc_in<sc_uint<16>> instruction, reg_write, write_back_data, dest_reg_back;

    // talvez sejam signals
    sc_in<sc_uint<4>> op;
    sc_in<sc_uint<3>> source_reg1, source_reg2, dest_reg, raw_imm, imm_source;

    sc_out<sc_uint<16>> read_data1, read_data2, immediate;
    sc_out<bool> imm_source;

    regfile *regfile_inst;
    signext *sign_extender;

    void decode_instr() {
        auto instr = instruction.read();
        op.write(instr.range(15, 12));
        dest_reg.write(instr.range(11, 9));
        source_reg1.write(instr.range(8, 6));
        // mudar depois pra ser responsabilidade do controle
        source_reg2.write( (op.read() == 0b1000) ? instr.range(11, 9) : instr.range(5, 3));
        immediate.write(instr.range(11, 0));
    }

    SC_CTOR(IDStage) {
        // Módulo: Banco de Registradores
        regfile_inst = new regfile("regfile");
        regfile_inst->clk(clk);
        regfile_inst->rst_n(rst_n);
        regfile_inst->address1(source_reg1);
        regfile_inst->address2(source_reg2);
        regfile_inst->address3(dest_reg_back);
        regfile_inst->write_enable(reg_write);
        regfile_inst->write_data(write_back_data);
        regfile_inst->read_data1(read_data1);
        regfile_inst->read_data2(read_data2);

        // Módulo: Sign Extender
        sign_extender = new signext("signext");
        sign_extender->raw_src(raw_imm);
        sign_extender->imm_source(imm_source);
        sign_extender->immediate(immediate);

        SC_METHOD(decode_instr);
        sensitive << instruction;
    }
};

// EX Stage
SC_MODULE(EXStage) {
    sc_in<sc_uint<3>> ula_op;
    sc_in<sc_uint<16>> read_data1, read_data2, immediate, pc_in;
    sc_in<bool> ula_src;

    sc_out<sc_uint<16>> ula_result;
    sc_out<bool> zero, negative;
    sc_out<sc_uint<16>> pc_branch;

    sc_signal<sc_uint<16>> src2_signal;

    ula *ula_inst;

    void select_src2() {
        src2_signal.write(ula_src.read() ? immediate.read() : read_data2.read());
    }

    void pc_calc() {
        pc_branch.write(pc_in.read() + 2 + (immediate.read() << 1));
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
        SC_METHOD(pc_calc);
        sensitive << clk.pos();
    }
};


// MEM Stage
SC_MODULE(MEMStage) {
    sc_in<bool> clk;
    sc_in<bool> rst_mem;
    sc_in<bool> mem_write;
    sc_in<sc_uint<16>> ula_result, write_data;

    sc_out<sc_uint<16>> mem_read;

    memory *data_memory;

    SC_CTOR(MEMStage) {
        // colocar no modulo de cpu
        data_memory = new memory("data_memory");
        data_memory->load_memory("../test_dmemory.bin");

        data_memory->clk(clk);
        data_memory->rst_n(rst_mem);
        data_memory->write_enable(mem_write);
        data_memory->address(ula_result);
        data_memory->write_data(write_data);
        data_memory->read_data(mem_read);
    }
};

// WB Stage
SC_MODULE(WBStage) {
    sc_in<bool> mem_to_reg;
    sc_in<sc_uint<16>> ula_result, mem_read, write_back_reg_in;

    sc_out<sc_uint<16>> write_back_data, write_back_reg_out;

    void wb_mux() {
        write_back_data.write(mem_to_reg.read() ? mem_read.read() : ula_result.read());
    }

    void write_back() {
        write_back_reg_out.write(write_back_reg_in.read());
    }

    SC_CTOR(WBStage) {
        SC_METHOD(wb_mux);
        sensitive << mem_to_reg << ula_result << mem_read;
        SC_METHOD(write_back);
        sensitive << clk.pos();
    }
};

#endif // STAGES_H
