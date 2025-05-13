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
    sc_in<sc_uint<16>> read_data1_in, read_data2_in, immediate_in;
    sc_out<sc_uint<16>> read_data1_out, read_data2_out, immediate_out;

    void latch() {
        if (!rst_n.read()) {
            read_data1_out.write(0);
            read_data2_out.write(0);
            immediate_out.write(0);
        } else if (clk.posedge()) {
            read_data1_out.write(read_data1_in.read());
            read_data2_out.write(read_data2_in.read());
            immediate_out.write(immediate_in.read());
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
    sc_in<sc_uint<16>> ula_result_in;
    sc_out<sc_uint<16>> ula_result_out;

    void latch() {
        if (!rst_n.read()) {
            ula_result_out.write(0);
        } else if (clk.posedge()) {
            ula_result_out.write(ula_result_in.read());
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
    sc_in<sc_uint<16>> mem_read_in;
    sc_out<sc_uint<16>> mem_read_out;

    void latch() {
        if (!rst_n.read()) {
            mem_read_out.write(0);
        } else if (clk.posedge()) {
            mem_read_out.write(mem_read_in.read());
        }
    }

    SC_CTOR(PipeReg_MEM_WB) {
        SC_METHOD(latch);
        sensitive << clk.pos();
    }
};
