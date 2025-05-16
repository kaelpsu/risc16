#include <systemc.h>

// PipeReg IF/ID
SC_MODULE(PipeReg_IF_ID) {
    sc_in<bool> clk, rst_n;
    sc_in<sc_uint<16>> pc_in, instruction_in;
    sc_out<sc_uint<16>> pc_out, instruction_out;

    void latch() {
        if (!rst_n.read()) {
            pc_out.write(0);
            instruction_out.write(0);
        } else if (clk.posedge()) {
            pc_out.write(pc_in.read());
            instruction_out.write(instruction_in.read());
        }
    }

    SC_CTOR(PipeReg_IF_ID) {
        SC_METHOD(latch);
        sensitive << clk.pos();
    }
};

// PipeReg ID/EX
SC_MODULE(PipeReg_ID_EX) {
    sc_in<bool> clk, rst_n;
    sc_in<sc_uint<16>> read_data1_in, read_data2_in, immediate_in, pc_in;
    sc_in<sc_uint<3>> dest_reg_in;

    sc_out<sc_uint<16>> pc_out, read_data1_out, read_data2_out, immediate_out;
    sc_out<sc_uint<3>> dest_reg_out;

    // control inputs
    sc_in<bool> mem_write_in;
    sc_in<bool> mem_to_reg_in;
    sc_in<sc_uint<3>> ula_op_in;
    sc_in<bool> ula_src_in;
    sc_in<bool> reg_write_in;
    sc_in<bool> pc_src_in;

    // control outputs
    sc_out<bool> mem_write_out;
    sc_out<bool> mem_to_reg_out;
    sc_out<sc_uint<3>> ula_op_out;
    sc_out<bool> ula_src_out;
    sc_out<bool> reg_write_out;
    sc_out<bool> pc_src_out;



    void latch() {
        if (!rst_n.read()) {
            pc_out.write(0);
            read_data1_out.write(0);
            read_data2_out.write(0);
            immediate_out.write(0);
            dest_reg_out.write(0);

            mem_write_out.write(false);
            mem_to_reg_out.write(false);
            ula_op_out.write(0);
            ula_src_out.write(false);
            reg_write_out.write(false);
            pc_src_out.write(false);

        } else if (clk.posedge()) {
            pc_out.write(pc_in.read());
            read_data1_out.write(read_data1_in.read());
            read_data2_out.write(read_data2_in.read());
            immediate_out.write(immediate_in.read());
            dest_reg_out.write(dest_reg_in.read());

            mem_write_out.write(mem_write_in.read());
            mem_to_reg_out.write(mem_to_reg_in.read());
            ula_op_out.write(ula_op_in.read());
            ula_src_out.write(ula_src_in.read());
            reg_write_out.write(reg_write_in.read());
            pc_src_out.write(pc_src_in.read());
        }
    }

    SC_CTOR(PipeReg_ID_EX) {
        SC_METHOD(latch);
        sensitive << clk.pos();
    }
};

// PipeReg EX/MEM
SC_MODULE(PipeReg_EX_MEM) {
    sc_in<bool> clk, rst_n;
    sc_in<sc_uint<16>> ula_result_in, write_data_in;
    sc_in<sc_uint<3>> dest_reg_in;
    sc_out<sc_uint<16>> ula_result_out, write_data_out;
    sc_out<sc_uint<3>> dest_reg_out;


    // control inputs
    sc_in<bool> mem_write_in;
    sc_in<bool> mem_to_reg_in;
    sc_in<bool> reg_write_in;
    // control outputs
    sc_out<bool> mem_write_out;
    sc_out<bool> mem_to_reg_out;
    sc_out<bool> reg_write_out;



    void latch() {
        if (!rst_n.read()) {
            ula_result_out.write(0);
            write_data_out.write(0);
            dest_reg_out.write(0);

            mem_write_out.write(false);
            mem_to_reg_out.write(false);
            reg_write_out.write(false);
        } else if (clk.posedge()) {
            ula_result_out.write(ula_result_in.read());
            write_data_out.write(write_data_in.read());
            dest_reg_out.write(dest_reg_in.read());

            mem_write_out.write(mem_write_in.read());
            mem_to_reg_out.write(mem_to_reg_in.read());
            reg_write_out.write(reg_write_in.read());
        }
    }

    SC_CTOR(PipeReg_EX_MEM) {
        SC_METHOD(latch);
        sensitive << clk.pos();
    }
};

// PipeReg MEM/WB
SC_MODULE(PipeReg_MEM_WB) {
    sc_in<bool> clk, rst_n;
    sc_in<sc_uint<16>> mem_read_in, ula_result_in;
    sc_in<sc_uint<3>> dest_reg_in;
    sc_out<sc_uint<16>> mem_read_out, ula_result_out;
    sc_out<sc_uint<3>> dest_reg_out;


    // control inputs
    sc_in<bool> mem_to_reg_in;
    sc_in<bool> reg_write_in;
    
    // control outputs
    sc_out<bool> mem_to_reg_out;
    sc_out<bool> reg_write_out;

    void latch() {
        if (!rst_n.read()) {
            mem_read_out.write(0);
            ula_result_out.write(0);
            dest_reg_out.write(0);

            mem_to_reg_out.write(false);
            reg_write_out.write(false);
        } else if (clk.posedge()) {
            mem_read_out.write(mem_read_in.read());
            ula_result_out.write(ula_result_in.read());
            dest_reg_out.write(dest_reg_in.read());

            mem_to_reg_out.write(mem_to_reg_in.read());
            reg_write_out.write(reg_write_in.read());
        }
    }

    SC_CTOR(PipeReg_MEM_WB) {
        SC_METHOD(latch);
        sensitive << clk.pos();
    }
};
