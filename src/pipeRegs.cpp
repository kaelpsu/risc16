#include <systemc.h>

// PipeReg IF/ID
SC_MODULE(PipeReg_IF_ID) {
    sc_in<bool> clk, rst_n, stall, clear;
    sc_in<sc_uint<16>> pc_in, instruction_in;
    sc_out<sc_uint<16>> pc_out, instruction_out;

    // Armazenamento interno do estágio
    sc_signal<sc_uint<16>> pc_reg, instruction_reg;

    // Primeiro escreve as saídas com base no estado anterior
    void write_outputs() {
        pc_out.write(pc_reg.read());
        instruction_out.write(instruction_reg.read());
    }

    // Depois captura entradas na borda de clock
    void latch_inputs() {
        if (!rst_n.read() || !clear.read()) {
            pc_reg.write(0);
            instruction_reg.write(0);
        } else {
            if (!stall.read()) {
                pc_reg.write(pc_in.read());
                instruction_reg.write(instruction_in.read());
            } else {
                std::cout << "(stall)" << std::endl;
            }
        }
    }

    SC_CTOR(PipeReg_IF_ID) {
        // Saídas atualizadas a todo instante (com valores estáveis)
        SC_METHOD(write_outputs);
        sensitive << pc_reg << instruction_reg;

        // Entradas capturadas na borda de clock
        SC_METHOD(latch_inputs);
        sensitive << clk.pos();
    }
};


// PipeReg ID/EX
SC_MODULE(PipeReg_ID_EX) {
    sc_in<bool> clk, rst_n;

    // Entradas
    sc_in<sc_uint<16>> read_data1_in, read_data2_in, immediate_in, pc_in;
    sc_in<sc_uint<3>> dest_reg_in;
    sc_in<bool> mem_write_in;
    sc_in<bool> mem_to_reg_in;
    sc_in<sc_uint<3>> ula_op_in;
    sc_in<bool> ula_src_in;
    sc_in<bool> reg_write_in;
    sc_in<bool> pc_src_in;

    // Saídas
    sc_out<sc_uint<16>> pc_out, read_data1_out, read_data2_out, immediate_out;
    sc_out<sc_uint<3>> dest_reg_out;
    sc_out<bool> mem_write_out;
    sc_out<bool> mem_to_reg_out;
    sc_out<sc_uint<3>> ula_op_out;
    sc_out<bool> ula_src_out;
    sc_out<bool> reg_write_out;
    sc_out<bool> pc_src_out;
    sc_out<sc_uint<3>> dest_idex;
    sc_out<bool>       reg_write_idex;  
    sc_out<bool> clear;

    // Registradores internos
    sc_signal<sc_uint<16>> pc_reg, read_data1_reg, read_data2_reg, immediate_reg;
    sc_signal<sc_uint<3>> dest_reg_reg;
    sc_signal<bool> mem_write_reg, mem_to_reg_reg, ula_src_reg, reg_write_reg, pc_src_reg;
    sc_signal<sc_uint<3>> ula_op_reg;

    // Primeiro escreve nas saídas
    void write_outputs() {
            pc_out.write(pc_reg.read());
            read_data1_out.write(read_data1_reg.read());
            read_data2_out.write(read_data2_reg.read());
            immediate_out.write(immediate_reg.read());
            dest_reg_out.write(dest_reg_reg.read());

            mem_write_out.write(mem_write_reg.read());
            mem_to_reg_out.write(mem_to_reg_reg.read());
            ula_op_out.write(ula_op_reg.read());
            ula_src_out.write(ula_src_reg.read());
            reg_write_out.write(reg_write_reg.read());
            pc_src_out.write(pc_src_reg.read());
            clear.write(!pc_src_out.read());

            dest_idex.write(dest_reg_reg.read());
            reg_write_idex.write(reg_write_reg.read());
    }

    // Depois captura entradas na borda de clock
    void latch_inputs() {
        if (!rst_n.read()) {
            pc_reg.write(0);
            read_data1_reg.write(0);
            read_data2_reg.write(0);
            immediate_reg.write(0);
            dest_reg_reg.write(0);

            mem_write_reg.write(false);
            mem_to_reg_reg.write(false);
            ula_op_reg.write(0);
            ula_src_reg.write(false);
            reg_write_reg.write(false);
            pc_src_reg.write(false);
        } else {
            pc_reg.write(pc_in.read());
            read_data1_reg.write(read_data1_in.read());
            read_data2_reg.write(read_data2_in.read());
            immediate_reg.write(immediate_in.read());
            dest_reg_reg.write(dest_reg_in.read());

            mem_write_reg.write(mem_write_in.read());
            mem_to_reg_reg.write(mem_to_reg_in.read());
            ula_op_reg.write(ula_op_in.read());
            ula_src_reg.write(ula_src_in.read());
            reg_write_reg.write(reg_write_in.read());
            pc_src_reg.write(pc_src_in.read());
        }
    }

    SC_CTOR(PipeReg_ID_EX) {
        SC_METHOD(write_outputs);
        sensitive << pc_reg << read_data1_reg << read_data2_reg << immediate_reg
                  << dest_reg_reg << mem_write_reg << mem_to_reg_reg
                  << ula_op_reg << ula_src_reg << reg_write_reg << pc_src_reg;

        SC_METHOD(latch_inputs);
        sensitive << clk.pos();
    }
};

// PipeReg EX/MEM
SC_MODULE(PipeReg_EX_MEM) {
    sc_in<bool> clk, rst_n;

    // Entradas
    sc_in<sc_uint<16>> ula_result_in, write_data_in;
    sc_in<sc_uint<3>>  dest_reg_in;
    sc_in<bool>        mem_write_in, mem_to_reg_in, reg_write_in;

    // Saídas
    sc_out<sc_uint<16>> ula_result_out, write_data_out;
    sc_out<sc_uint<3>>  dest_reg_out;
    sc_out<bool>        mem_write_out, mem_to_reg_out, reg_write_out;

    sc_out<sc_uint<3>> dest_exmem;
    sc_out<bool>       reg_write_exmem;  

    // Registradores internos
    sc_signal<sc_uint<16>> ula_result_reg, write_data_reg;
    sc_signal<sc_uint<3>>  dest_reg_reg;
    sc_signal<bool>        mem_write_reg, mem_to_reg_reg, reg_write_reg;

    // 1) Atualiza as saídas com o valor armazenado no ciclo anterior
    void write_outputs() {
        ula_result_out.write(ula_result_reg.read());
        write_data_out.write(write_data_reg.read());
        dest_reg_out.write(dest_reg_reg.read());
        mem_write_out.write(mem_write_reg.read());
        mem_to_reg_out.write(mem_to_reg_reg.read());
        reg_write_out.write(reg_write_reg.read());

        dest_exmem.write(dest_reg_reg.read());
        reg_write_exmem.write(reg_write_reg.read());
    }

    // 2) Captura as entradas na borda de clock para uso no próximo ciclo
    void latch_inputs() {
        if (!rst_n.read()) {
            ula_result_reg.write(0);
            write_data_reg.write(0);
            dest_reg_reg.write(0);
            mem_write_reg.write(false);
            mem_to_reg_reg.write(false);
            reg_write_reg.write(false);
        } else {
            ula_result_reg.write(ula_result_in.read());
            write_data_reg.write(write_data_in.read());
            dest_reg_reg.write(dest_reg_in.read());
            mem_write_reg.write(mem_write_in.read());
            mem_to_reg_reg.write(mem_to_reg_in.read());
            reg_write_reg.write(reg_write_in.read());
        }
    }

    SC_CTOR(PipeReg_EX_MEM) {

        SC_METHOD(write_outputs);
        sensitive << ula_result_reg << write_data_reg << dest_reg_reg
                  << mem_write_reg << mem_to_reg_reg << reg_write_reg;

        // Captura entradas na borda positiva do clock
        SC_METHOD(latch_inputs);
        sensitive << clk.pos();
    }
};

// PipeReg MEM/WB
SC_MODULE(PipeReg_MEM_WB) {
    sc_in<bool> clk, rst_n;

    // Entradas
    sc_in<sc_uint<16>> mem_read_in, ula_result_in;
    sc_in<sc_uint<3>>  dest_reg_in;
    sc_in<bool>        mem_to_reg_in, reg_write_in;

    // Saídas
    sc_out<sc_uint<16>> mem_read_out, ula_result_out;
    sc_out<sc_uint<3>>  dest_reg_out;
    sc_out<bool>        mem_to_reg_out, reg_write_out;

    sc_out<sc_uint<3>> dest_memwb;
    sc_out<bool>       reg_write_memwb;  

    // Registradores internos
    sc_signal<sc_uint<16>> mem_read_reg, ula_result_reg;
    sc_signal<sc_uint<3>>  dest_reg_reg;
    sc_signal<bool>        mem_to_reg_reg, reg_write_reg;

    // 1) Atualiza as saídas com os valores internos (estado do ciclo anterior)
    void write_outputs() {
        mem_read_out.write(mem_read_reg.read());
        ula_result_out.write(ula_result_reg.read());
        dest_reg_out.write(dest_reg_reg.read());
        mem_to_reg_out.write(mem_to_reg_reg.read());
        reg_write_out.write(reg_write_reg.read());

        dest_memwb.write(dest_reg_reg.read());
        reg_write_memwb.write(reg_write_reg.read());
    }

    // 2) Captura as entradas na borda de clock (para uso no próximo ciclo)
    void latch_inputs() {
        if (!rst_n.read()) {
            mem_read_reg.write(0);
            ula_result_reg.write(0);
            dest_reg_reg.write(0);
            mem_to_reg_reg.write(false);
            reg_write_reg.write(false);
        } else {
            mem_read_reg.write(mem_read_in.read());
            ula_result_reg.write(ula_result_in.read());
            dest_reg_reg.write(dest_reg_in.read());
            mem_to_reg_reg.write(mem_to_reg_in.read());
            reg_write_reg.write(reg_write_in.read());
        }
    }

    void debug_signals() {
        std::cout << "[MEM/WB]---------------------------------------------------------------------------" << std::endl;
        std::cout << "Dest Reg: " << dest_reg_reg.read().to_string(SC_BIN) << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }

    SC_CTOR(PipeReg_MEM_WB) {
        // Atualiza as saídas sempre que algum registrador interno muda
        SC_METHOD(write_outputs);
        sensitive << mem_read_reg << ula_result_reg << dest_reg_reg
                  << mem_to_reg_reg << reg_write_reg;

        // Captura as entradas na borda positiva do clock
        SC_METHOD(latch_inputs);
        sensitive << clk.pos();

        // Método para depuração
        SC_METHOD(debug_signals);
        sensitive << clk.pos();
    }
};
