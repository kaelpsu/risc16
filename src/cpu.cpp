#include <systemc.h>
#include "memory.cpp"
#include "control.cpp"
#include "reg_file.cpp"
#include "signext.cpp"
#include "ula.cpp"

SC_MODULE(cpu) {
    // Entradas
    sc_in<bool> clk;
    sc_in<bool> rst_n;

    // INTERNOS
    sc_signal<sc_uint<16>> pc, pc_next;

    // INSTRUCTION
    sc_signal<sc_uint<16>> instruction;

    // CONTROLE
    sc_signal<sc_uint<4>> op;
    sc_signal<bool> ula_zero;
    sc_signal<bool> ula_negative;

    sc_signal<sc_uint<3>> ula_op;
    sc_signal<bool> imm_source;
    sc_signal<bool> mem_write;
    sc_signal<bool> reg_write;
    sc_signal<bool> ula_src;
    sc_signal<bool> mem_to_reg;
    sc_signal<bool> pc_src;

    sc_signal<bool> rst_mem; // reset da memória de dados
    sc_signal<bool> write_rom; // habilita escrita na memória de instruções
    sc_signal<sc_uint<16>> null_data; // dado nulo para escrita na memória de instruções
    
    // REGISTROS
    sc_signal<sc_uint<3>> source_reg1, source_reg2, dest_reg;
    sc_signal<sc_uint<16>> read_reg1, read_reg2, write_back_data;

    // IMEDIATO
    sc_signal<sc_uint<12>> raw_imm;
    sc_signal<sc_uint<16>> immediate;

    // ULA
    sc_signal<sc_uint<16>> ula_result;
    sc_signal<sc_uint<16>> ula_src2;

    // MEMÓRIA DE DADOS
    sc_signal<sc_uint<16>> mem_read;

    // PROCESSO: Program Counter
    void pc_logic() {
        if (rst_n.read() == false) {
            pc.write(0);
        } else {
            pc.write(pc_next.read());
        }
    }

    // PROCESSO: Próximo PC
    void pc_next_logic() {
        switch (pc_src)
        {
        case 0b0: // PC + 2
            pc_next.write(pc.read() + 2);
            break;
        case 0b1: // PC + 2 + (imediato << 1)
            pc_next.write(pc.read() + 2 + (immediate.read() << 1));
            break;
        
        default:
            break;
        }
    }

    void debug_signals() {
        std::cout << "=====================================CICLO======================================" << std::endl;
        std::cout << "[BASE]--------------------------------------------------------------------------" << std::endl;
        std::cout << "PC: " << pc.read().to_string(SC_BIN) << std::endl;
        std::cout << "PC Next: " << pc_next.read().to_string(SC_BIN) << std::endl;
        std::cout << "Instruction: " << instruction.read().to_string(SC_BIN) << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "[CONTROLE]----------------------------------------------------------------------" << std::endl;
        std::cout << "Op: " << op.read().to_string(SC_BIN) << std::endl;
        std::cout << "ULA Op: " << ula_op.read().to_string(SC_BIN) << std::endl;
        std::cout << "Imm Source: " << imm_source.read() << std::endl;
        std::cout << "Mem Write: " << mem_write.read() << std::endl;
        std::cout << "Reg Write: " << reg_write.read() << std::endl;
        std::cout << "ULA Src: " << ula_src.read() << std::endl;
        std::cout << "Mem to Reg: " << mem_to_reg.read() << std::endl;
        std::cout << "PC Src: " << pc_src.read() << std::endl;
        std::cout << "ULA Zero: " << ula_zero.read() << std::endl;
        std::cout << "ULA Negative: " << ula_negative.read() << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "[REGISTROS]---------------------------------------------------------------------" << std::endl;
        std::cout << "Source Reg 1: " << source_reg1.read().to_string(SC_BIN) << std::endl;
        std::cout << "Source Reg 2: " << source_reg2.read().to_string(SC_BIN) << std::endl;
        std::cout << "Dest Reg: " << dest_reg.read().to_string(SC_BIN) << std::endl;
        std::cout << "Read Reg 1: " << read_reg1.read().to_string(SC_BIN) << std::endl;
        std::cout << "Read Reg 2: " << read_reg2.read().to_string(SC_BIN) << std::endl;
        std::cout << "Write Back Data: " << write_back_data.read().to_string(SC_BIN) << std::endl;
        std::cout << "-------------------------------------------------------------------------------" << std::endl;
        std::cout << "[IMEDIATO]---------------------------------------------------------------------" << std::endl;
        std::cout << "Raw Imm: " << raw_imm.read().to_string(SC_BIN) << std::endl;
        std::cout << "Immediate: " << immediate.read().to_string(SC_BIN) << std::endl;
        std::cout << "-------------------------------------------------------------------------------" << std::endl;
        std::cout << "[ULA]--------------------------------------------------------------------------" << std::endl;
        std::cout << "ULA Result: " << ula_result.read().to_string(SC_BIN) << std::endl;
        std::cout << "ULA Src 2: " << ula_src2.read().to_string(SC_BIN) << std::endl;
        std::cout << "-------------------------------------------------------------------------------" << std::endl;
        std::cout << "[MEMORIA]----------------------------------------------------------------------" << std::endl;
        std::cout << "Mem Read: " << mem_read.read().to_string(SC_BIN) << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "================================================================================" << std::endl;
    }

    // PROCESSO: sinais derivados da instrução
    void decode_instruction() {
        sc_uint<16> instr = instruction.read();
        op.write(instr.range(15, 12));
        dest_reg.write(instr.range(11, 9));
        source_reg1.write(instr.range(8, 6));
        // mudar depois pra ser responsabilidade do controle
        source_reg2.write( (op.read() == 0b1000) ? instr.range(11, 9) : instr.range(5, 3));
        raw_imm.write(instr.range(11, 0));
    }


    void ula_mux() {
        switch (ula_src.read())
        {
        case 0b0:  // Case for ula_src = 0
            ula_src2.write(read_reg2.read());
            break;
            
        case 0b1:  // Case for ula_src = 1
            ula_src2.write(immediate.read());
            break;

        default:
            break;
        }
    }

    void write_back_mux() {
        switch (mem_to_reg.read())
        {
        case false:  // Case for mem_to_reg = 0
            write_back_data.write(ula_result.read());
            break;
            
        case true:  // Case for mem_to_reg = 1
            write_back_data.write(mem_read.read());
            break;

        default:
            break;
        }
    }

    // INSTÂNCIAS DE MÓDULOS
    memory* instruction_memory;
    control* control_unit;
    regfile* regfile_inst;
    signext* sign_extender;
    ula* ula_inst;
    memory* data_memory;

    SC_CTOR(cpu) {
        // Módulo: Instruction Memory (ROM)
        std::cout << "Carregando memória de instruções..." << std::endl;
        instruction_memory = new memory("instruction_memory");
        instruction_memory->load_memory("../test_imemory.bin"); // Adicione esse arquivo no seu diretório de simulação

        // Módulo: Memória de Dados
        std::cout << "Carregando memória de dados..." << std::endl;
        data_memory = new memory("data_memory");
        data_memory->load_memory("../test_dmemory.bin"); // Adicione esse arquivo no seu diretório de simulação

        //Debug
        SC_METHOD(debug_signals);
        sensitive << clk.pos();
        dont_initialize();
        
        // Processos
        SC_METHOD(pc_logic);
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(pc_next_logic);
        sensitive << pc << pc_src << immediate;

        SC_METHOD(decode_instruction);
        sensitive << clk.pos();
        dont_initialize();
        
        SC_METHOD(ula_mux);
        sensitive << ula_src << read_reg2 << immediate;
        dont_initialize();

        SC_METHOD(write_back_mux);
        sensitive << mem_to_reg << ula_result << mem_read;
        dont_initialize();

        rst_mem.write(true); // desativa reset da memoria
        write_rom.write(false); // desativa escrita na memoria de instruções
        null_data.write(0); // dado nulo para escrita na memória de instruções
        
        // Módulo: Instruction Memory (ROM)
        instruction_memory->clk(clk);
        instruction_memory->address(pc);
        instruction_memory->write_data(null_data); // usar dado nulo para escrita
        instruction_memory->write_enable(write_rom);
        instruction_memory->rst_n(rst_mem);
        instruction_memory->read_data(instruction);

        // Módulo: Unidade de Controle
        control_unit = new control("control_unit");
        control_unit->op(op);
        control_unit->ula_zero(ula_zero);
        control_unit->ula_negative(ula_negative);
        control_unit->ula_op(ula_op);
        control_unit->imm_source(imm_source);
        control_unit->mem_write(mem_write);
        control_unit->reg_write(reg_write);
        control_unit->ula_src(ula_src);
        control_unit->mem_to_reg(mem_to_reg);
        control_unit->pc_src(pc_src);

        // Módulo: Banco de Registradores
        regfile_inst = new regfile("regfile");
        regfile_inst->clk(clk);
        regfile_inst->rst_n(rst_n);
        regfile_inst->address1(source_reg1);
        regfile_inst->address2(source_reg2);
        regfile_inst->address3(dest_reg);
        regfile_inst->write_enable(reg_write);
        regfile_inst->write_data(write_back_data);
        regfile_inst->read_data1(read_reg1);
        regfile_inst->read_data2(read_reg2);

        // Módulo: Sign Extender
        sign_extender = new signext("signext");
        sign_extender->raw_src(raw_imm);
        sign_extender->imm_source(imm_source);
        sign_extender->immediate(immediate);

        // Módulo: ULA
        ula_inst = new ula("ula");
        ula_inst->ula_op(ula_op);
        ula_inst->src1(read_reg1);
        ula_inst->src2(ula_src2);
        ula_inst->ula_result(ula_result);
        ula_inst->zero(ula_zero);
        ula_inst->negative(ula_negative);

        // Módulo: Memória de Dados
        data_memory->clk(clk);
        data_memory->address(ula_result);
        data_memory->write_data(read_reg2); // presumivelmente
        data_memory->write_enable(mem_write);
        data_memory->rst_n(rst_mem);
        data_memory->read_data(mem_read);
        
    }
};
