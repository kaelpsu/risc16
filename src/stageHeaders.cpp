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
    sc_in<bool> clk, rst_n, clear;
    sc_in<sc_uint<16>>pc_branch;
    sc_in<bool> pc_src;
    sc_in<bool> stall;
    sc_signal<sc_uint<16>> pc, pc_next;
    sc_out<sc_uint<16>> pc_out, instruction;

    sc_signal<bool> rst_mem;
    sc_signal<bool> write_enable;
    sc_signal<sc_uint<16>> null_data;

    memory *instruction_memory;

    void pc_process() {
        if (!rst_n.read())
            pc.write(0);
        else if (!stall.read())
            pc.write(pc_next.read());
    }

    void pc_mux() {
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
    
    void debug_signals() {
        std::cout << "[IF]----------------------------------------------------------------------------" << std::endl;
        if (stall.read()) { std::cout << "(stall)" << std::endl; }
        std::cout << "PC: " << pc.read().to_string(SC_BIN) << std::endl;
        std::cout << "PC Branch: " << pc_branch.read().to_string(SC_BIN) << std::endl;
        std::cout << "PC + 2: " << pc_out.read().to_string(SC_BIN) << std::endl;
        std::cout << "PC Src: " << pc_src.read() << std::endl;
        std::cout << "PC Next: " << pc_next.read().to_string(SC_BIN) << std::endl;
        std::cout << "Instruction: " << instruction.read().to_string(SC_BIN) << std::endl;
        std::cout << "Clear: " << clear.read() << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }

    SC_CTOR(IFStage) {
        std::cout << "Carregando memória de instruções..." << std::endl;
        instruction_memory = new memory("instruction_memory");
        instruction_memory->load_memory("../test_imemory.bin");

        SC_METHOD(pc_process);
        sensitive << clk.pos();
        SC_METHOD(somador);
        sensitive << pc;
        SC_METHOD(pc_mux);
        sensitive << pc_src << pc_branch << pc_out;
        
        rst_mem.write(true); // desativa reset da memoria
        write_enable.write(false); // desativa escrita na memoria de instruções
        null_data.write(0); // dado nulo para escrita na memória de instruções
        
        instruction_memory->clk(clk);
        instruction_memory->rst_n(rst_mem);
        instruction_memory->address(pc);
        instruction_memory->write_enable(write_enable);
        instruction_memory->write_data(null_data);
        instruction_memory->read_data(instruction);

        SC_METHOD(debug_signals);
        sensitive << clk.pos();
    }
};

// ID Stage
SC_MODULE(IDStage) {
    sc_in<bool> clk, rst_n, clear;
    sc_in<sc_uint<16>> instruction, write_back_data;
    sc_in<sc_uint<3>> dest_reg_back;
    sc_in<bool> reg_write_in;

    sc_signal<bool> imm_source;
    sc_signal<sc_uint<4>> op;
    sc_signal<sc_uint<3>> source_reg1, source_reg2;
    sc_signal<sc_uint<12>> raw_imm;

    sc_out<sc_uint<16>> read_data1, read_data2, immediate;
    sc_out<sc_uint<3>> dest_reg_out;

    // para hazard detection
    sc_out<sc_uint<3>> rs_id, rt_id;

    // CONTROL
    // Entradas
    sc_in<bool> ula_zero;
    sc_in<bool> ula_negative;
    // Saídas
    sc_out<sc_uint<3>> ula_op;
    sc_out<bool> mem_write;
    sc_out<bool> reg_write_out;
    sc_out<bool> ula_src;
    sc_out<bool> mem_to_reg;
    sc_out<bool> pc_src;

    control *control_unit;
    regfile *regfile_inst;
    signext *sign_extender;

    void decode_instr() {
        sc_uint<16> instr;
        if (!rst_n.read() || !clear.read()) {
            instr = 0;
        } else {
            instr = instruction.read();
        }
        op.write(instr.range(15, 12));
        dest_reg_out.write(instr.range(11, 9));
        source_reg1.write(instr.range(8, 6));
        // mudar depois pra ser responsabilidade do controle
        source_reg2.write( (instr.range(15, 12) == 0b1000) ? instr.range(11, 9) : instr.range(5, 3));
        raw_imm.write(instr.range(11, 0));
        rs_id.write(instr.range(8, 6));
        rt_id.write((instr.range(15, 12) == 0b1000) ? instr.range(11, 9) : instr.range(5, 3));
    }

    void debug_signals() {
        std::cout << "[ID]----------------------------------------------------------------------------" << std::endl;
        std::cout << "Instruction: " << instruction.read().to_string(SC_BIN) << std::endl;
        std::cout << "Op: " << op.read().to_string(SC_BIN) << std::endl;
        std::cout << "Source Reg 1: " << source_reg1.read().to_string(SC_BIN) << std::endl;
        std::cout << "Source Reg 2: " << source_reg2.read().to_string(SC_BIN) << std::endl;
        std::cout << "Dest Reg Out: " << dest_reg_out.read().to_string(SC_BIN) << std::endl;
        std::cout << "Immediate: " << immediate.read().to_string(SC_BIN) << std::endl;
        std::cout << "Read Data 1: " << read_data1.read().to_string(SC_BIN) << std::endl;
        std::cout << "Read Data 2: " << read_data2.read().to_string(SC_BIN) << std::endl;
        std::cout << "Dest Reg Back: " << dest_reg_back.read().to_string(SC_BIN) << std::endl;
        std::cout << "Write Back Data: " << write_back_data.read().to_string(SC_BIN) << std::endl;
        std::cout << "Clear: " << clear.read() << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }

    SC_CTOR(IDStage) {
        SC_METHOD(decode_instr);
        sensitive << instruction << clk.pos();
        
        // Unidade de Controle
        control_unit = new control("control_unit");
        // entrada
        control_unit->op(op);
        control_unit->ula_zero(ula_zero);
        control_unit->ula_negative(ula_negative);
        // saídas
        control_unit->ula_op(ula_op);
        control_unit->imm_source(imm_source);
        control_unit->mem_write(mem_write);
        control_unit->reg_write(reg_write_out);
        control_unit->ula_src(ula_src);
        control_unit->mem_to_reg(mem_to_reg);
        control_unit->pc_src(pc_src);   
        
        // Módulo: Banco de Registradores
        regfile_inst = new regfile("regfile");
        regfile_inst->clk(clk);
        regfile_inst->rst_n(rst_n);
        regfile_inst->address1(source_reg1);
        regfile_inst->address2(source_reg2);
        regfile_inst->address3(dest_reg_back);
        regfile_inst->write_enable(reg_write_in);
        regfile_inst->write_data(write_back_data);
        regfile_inst->read_data1(read_data1);
        regfile_inst->read_data2(read_data2);
        
        // Módulo: Sign Extender
        sign_extender = new signext("signext");
        sign_extender->raw_src(raw_imm);
        sign_extender->imm_source(imm_source);
        sign_extender->immediate(immediate);


        SC_METHOD(debug_signals);
        sensitive << clk.pos();
    }
};

// EX Stage
SC_MODULE(EXStage) {
    sc_in<bool> clk, rst_n;
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

    void debug_signals(){
        std::cout << "[EX]----------------------------------------------------------------------------" << std::endl;
        std::cout << "PC: " << pc_in.read().to_string(SC_BIN) << std::endl;
        std::cout << "ULA Op: " << ula_op.read().to_string(SC_BIN) << std::endl;
        std::cout << "Read Data 1: " << read_data1.read().to_string(SC_BIN) << std::endl;
        std::cout << "Read Data 2: " << read_data2.read().to_string(SC_BIN) << std::endl;
        std::cout << "Immediate: " << immediate.read().to_string(SC_BIN) << std::endl;
        std::cout << "Src 2 Signal: " << src2_signal.read().to_string(SC_BIN) << std::endl;
        std::cout << "ULA Result: " << ula_result.read().to_string(SC_BIN) << std::endl;
        std::cout << "Zero: " << zero.read() << std::endl;
        std::cout << "Negative: " << negative.read() << std::endl;
        std::cout << "PC Branch: " << pc_branch.read().to_string(SC_BIN) << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }

    SC_CTOR(EXStage) {
        
        // Multiplexador para src2
        SC_METHOD(select_src2);
        sensitive << read_data1 << read_data2 << immediate << ula_src;
        SC_METHOD(pc_calc);
        sensitive << pc_in << immediate;
        
        ula_inst = new ula("ula_inst");
        // Connect inputs
        ula_inst->ula_op(ula_op);
        ula_inst->src1(read_data1);
        ula_inst->src2(src2_signal);
        
        // Connect outputs
        ula_inst->ula_result(ula_result);
        ula_inst->zero(zero);
        ula_inst->negative(negative);
        
        SC_METHOD(debug_signals);
        sensitive << clk.pos();
    }
};


// MEM Stage
SC_MODULE(MEMStage) {
    sc_in<bool> clk;

    sc_signal<bool> rst_mem;

    sc_in<bool> mem_write;
    sc_in<sc_uint<16>> ula_result, write_data;

    sc_out<sc_uint<16>> mem_read;

    memory *data_memory;

    void debug_signals() {
        std::cout << "[MEM]---------------------------------------------------------------------------" << std::endl;
        std::cout << "ULA Result: " << ula_result.read().to_string(SC_BIN) << std::endl;
        std::cout << "Write Data: " << write_data.read().to_string(SC_BIN) << std::endl;
        std::cout << "Mem Read: " << mem_read.read().to_string(SC_BIN) << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }

    SC_CTOR(MEMStage) {
        
        // Reset da memória de dados
        rst_mem.write(true); // desativa reset da memoria

        std::cout << "Carregando memória de dados..." << std::endl;
        data_memory = new memory("data_memory");
        data_memory->load_memory("../test_dmemory.bin");
        
        data_memory->clk(clk);
        data_memory->rst_n(rst_mem);
        data_memory->write_enable(mem_write);
        data_memory->address(ula_result);
        data_memory->write_data(write_data);
        data_memory->read_data(mem_read);

        SC_METHOD(debug_signals);
        sensitive << clk.pos();
    }
};

// WB Stage
SC_MODULE(WBStage) {
    sc_in<bool> clk;
    sc_in<bool> mem_to_reg;
    sc_in<sc_uint<16>> ula_result, mem_read;

    sc_out<sc_uint<16>> write_back_data;

    void wb_mux() {
        write_back_data.write(mem_to_reg.read() ? mem_read.read() : ula_result.read());
    }

    void debug_signals(){
        std::cout << "[WB]----------------------------------------------------------------------------" << std::endl;
        std::cout << "Mem To Reg: " << mem_to_reg.read() << std::endl;
        std::cout << "ULA Result: " << ula_result.read().to_string(SC_BIN) << std::endl;
        std::cout << "Mem Read: " << mem_read.read().to_string(SC_BIN) << std::endl;
        std::cout << "Write Back Data: " << write_back_data.read().to_string(SC_BIN) << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }

    SC_CTOR(WBStage) {
        SC_METHOD(wb_mux);
        sensitive << mem_to_reg << ula_result << mem_read;
        SC_METHOD(debug_signals);
        sensitive << clk.pos();
    }
};

#endif // STAGES_H
