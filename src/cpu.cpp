#include <systemc.h>
#include "stageHeaders.cpp"
#include "pipeRegs.cpp"

SC_MODULE(cpu) {
    // Entradas
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // IF
    sc_signal<sc_uint<16>> pc_if;
    sc_signal<sc_uint<16>> instruction_if;

    // ID
    sc_signal<sc_uint<16>> pc_id;
    sc_signal<sc_uint<16>> instruction_id;
    sc_signal<sc_uint<16>> read_data1, read_data2;
    sc_signal<sc_uint<3>> dest_reg_id;
    sc_signal<sc_uint<16>> immediate_id;

    // EX
    sc_signal<sc_uint<16>> pc_ex;
    sc_signal<sc_uint<16>> ula_src1, ula_src2;
    sc_signal<sc_uint<16>> immediate_ex;
    sc_signal<sc_uint<3>> dest_reg_ex;
    sc_signal<sc_uint<16>> ula_result;
    sc_signal<bool> ula_zero, ula_negative;

    // MEM
    sc_signal<sc_uint<16>> address;
    sc_signal<sc_uint<16>> write_data;
    sc_signal<sc_uint<16>> read_data_mem;
    sc_signal<sc_uint<3>> dest_reg_mem;

    // WB
    sc_signal<sc_uint<16>> read_data_wb;
    sc_signal<sc_uint<16>> ula_result_wb;
    sc_signal<sc_uint<16>> write_back_data;
    sc_signal<sc_uint<3>> dest_reg_wb;

    // PC Branch   
    sc_signal<sc_uint<16>> pc_branch;

    // HAZARD DETECTION
    sc_signal<sc_uint<3>> rs_id, rt_id;
    sc_signal<sc_uint<3>> rd_ex, rd_mem, rd_wb;
    sc_signal<bool> reg_write_idex, reg_write_exmem, reg_write_memwb;
    sc_signal<bool> stall;

    // CONTROLE
    sc_signal<sc_uint<4>> op;
    sc_signal<bool> ula_zero_ex, ula_zero_mem;
    sc_signal<bool> ula_negative_ex, ula_negative_mem;

    sc_signal<bool> mem_write_id, mem_write_ex, mem_write_mem;

    sc_signal<bool> mem_to_reg_id, mem_to_reg_ex, mem_to_reg_mem, mem_to_reg_wb;

    sc_signal<sc_uint<3>> ula_op_id, ula_op_ex;
    sc_signal<bool> ula_src_id, ula_src_ex;

    sc_signal<bool> reg_write_id, reg_write_ex, reg_write_mem, reg_write_wb;
    sc_signal<bool> pc_src_id, pc_src_ex;
    sc_signal<bool> imm_source;
    
    void debug_signals() {
        std::cout << "===============================================CICLO================================================" << std::endl;
    }

    // INSTÂNCIAS DE MÓDULOS
    PipeReg_IF_ID* if_id;
    PipeReg_ID_EX* id_ex;
    PipeReg_EX_MEM* ex_mem;
    PipeReg_MEM_WB* mem_wb;

    IFStage* if_stage;
    IDStage* id_stage;
    EXStage* ex_stage;
    MEMStage* mem_stage;
    WBStage* wb_stage;

    control* control_unit;


    SC_CTOR(cpu) {
        SC_METHOD(debug_signals);
        sensitive << clk.pos();

        // IF Stage
        if_stage = new IFStage("if_stage");
        // entradas
        if_stage->clk(clk);
        if_stage->rst_n(rst_n);
        if_stage->pc_src(pc_src_ex);
        if_stage->pc_branch(pc_branch);
        // saídas
        if_stage->pc_out(pc_if);
        if_stage->instruction(instruction_if);

        // IF ID Pipe Register
        if_id = new PipeReg_IF_ID("if_id");
        // entradas
        if_id->clk(clk);
        if_id->rst_n(rst_n);
        if_id->pc_in(pc_if);
        if_id->instruction_in(instruction_if);
        // saídas
        if_id->pc_out(pc_id);
        if_id->instruction_out(instruction_id);

        // ID Stage
        id_stage = new IDStage("id_stage");
        // entradas
        id_stage->clk(clk);
        id_stage->rst_n(rst_n);
        id_stage->instruction(instruction_id);
        id_stage->reg_write_in(reg_write_wb);
        id_stage->write_back_data(write_back_data);
        id_stage->dest_reg_back(dest_reg_wb);
        id_stage->ula_zero(ula_zero_ex);
        id_stage->ula_negative(ula_negative_ex);
        // saídas
        id_stage->read_data1(read_data1);
        id_stage->read_data2(read_data2);
        id_stage->immediate(immediate_id);
        id_stage->dest_reg_out(dest_reg_id);      
        id_stage->ula_op(ula_op_id);
        id_stage->mem_write(mem_write_id);
        id_stage->reg_write_out(reg_write_id);
        id_stage->ula_src(ula_src_id);
        id_stage->mem_to_reg(mem_to_reg_id);
        id_stage->pc_src(pc_src_id);

        // ID EX Pipe Register
        id_ex = new PipeReg_ID_EX("id_ex");
        // entradas
        id_ex->clk(clk);
        id_ex->rst_n(rst_n);
        id_ex->pc_in(pc_id);
        id_ex->read_data1_in(read_data1);
        id_ex->read_data2_in(read_data2);
        id_ex->immediate_in(immediate_id);
        id_ex->dest_reg_in(dest_reg_id);

        id_ex->ula_op_in(ula_op_id);
        id_ex->ula_src_in(ula_src_id);
        id_ex->mem_write_in(mem_write_id);
        id_ex->mem_to_reg_in(mem_to_reg_id);
        id_ex->reg_write_in(reg_write_id);
        id_ex->pc_src_in(pc_src_id);
        
        // saídas
        id_ex->pc_out(pc_ex);
        id_ex->read_data1_out(ula_src1);
        id_ex->read_data2_out(ula_src2);
        id_ex->immediate_out(immediate_ex);
        id_ex->dest_reg_out(dest_reg_ex);

        id_ex->ula_op_out(ula_op_ex);
        id_ex->ula_src_out(ula_src_ex);
        id_ex->mem_write_out(mem_write_ex);
        id_ex->mem_to_reg_out(mem_to_reg_ex);
        id_ex->reg_write_out(reg_write_ex);
        id_ex->pc_src_out(pc_src_ex);
        

        // EX Stage
        ex_stage = new EXStage("ex_stage");
        // entradas
        ex_stage->clk(clk);
        ex_stage->rst_n(rst_n);
        ex_stage->pc_in(pc_ex);
        ex_stage->ula_op(ula_op_ex);
        ex_stage->ula_src(ula_src_ex);
        ex_stage->read_data1(ula_src1);
        ex_stage->read_data2(ula_src2);
        ex_stage->immediate(immediate_ex);
        // saídas
        ex_stage->ula_result(ula_result);
        ex_stage->zero(ula_zero_ex);
        ex_stage->negative(ula_negative_ex);
        ex_stage->pc_branch(pc_branch);

        // EX MEM Pipe Register
        ex_mem = new PipeReg_EX_MEM("ex_mem");
        // entradas
        ex_mem->clk(clk);
        ex_mem->rst_n(rst_n);
        ex_mem->ula_result_in(ula_result);
        ex_mem->write_data_in(ula_src2);
        ex_mem->dest_reg_in(dest_reg_ex);

        ex_mem->mem_write_in(mem_write_ex);
        ex_mem->mem_to_reg_in(mem_to_reg_ex);
        ex_mem->reg_write_in(reg_write_ex);

        // saídas
        ex_mem->ula_result_out(address);
        ex_mem->write_data_out(write_data);
        ex_mem->dest_reg_out(dest_reg_mem);

        ex_mem->mem_write_out(mem_write_mem);
        ex_mem->mem_to_reg_out(mem_to_reg_mem);
        ex_mem->reg_write_out(reg_write_mem);

        // MEM Stage
        mem_stage = new MEMStage("mem_stage");
        // entradas
        mem_stage->clk(clk);
        mem_stage->mem_write(mem_write_ex);
        mem_stage->ula_result(address);
        mem_stage->write_data(write_data);
        // saídas
        mem_stage->mem_read(read_data_mem);

        // MEM WB Pipe Register
        mem_wb = new PipeReg_MEM_WB("mem_wb");
        // entradas
        mem_wb->clk(clk);
        mem_wb->rst_n(rst_n);
        mem_wb->mem_read_in(read_data_mem);
        mem_wb->ula_result_in(address);
        mem_wb->dest_reg_in(dest_reg_mem);

        mem_wb->mem_to_reg_in(mem_to_reg_mem);
        mem_wb->reg_write_in(reg_write_mem);

        // saídas
        mem_wb->mem_read_out(read_data_wb);
        mem_wb->ula_result_out(ula_result_wb);
        mem_wb->dest_reg_out(dest_reg_wb);

        mem_wb->mem_to_reg_out(mem_to_reg_wb);
        mem_wb->reg_write_out(reg_write_wb);

        // WB Stage
        wb_stage = new WBStage("wb_stage");
        // entradas
        wb_stage->clk(clk);
        wb_stage->mem_to_reg(mem_to_reg_wb);
        wb_stage->ula_result(ula_result_wb);
        wb_stage->mem_read(read_data_wb);
        // saídas
        wb_stage->write_back_data(write_back_data);
    }
};
