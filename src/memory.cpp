#include <systemc.h> // include the systemC header file
#include <fstream> // for file operations
using namespace sc_core; // use namespace
using namespace sc_dt; // use namespace
using namespace std;

SC_MODULE(memory) {
    // Ports
    sc_in<bool> clk;
    sc_in<bool> rst_n;
    sc_in<bool> write_enable;
    sc_in<sc_uint<16>> address;
    sc_in<sc_uint<16>> write_data;
    sc_out<sc_uint<16>> read_data;

    // Internal memory (32 words of 16 bits)
    sc_uint<16> mem[32]; // Inicializa com 0

    // Initialization file
    std::string mem_init;

    // Method to read memory file (like $readmemb)
    void load_memory(const std::string& filename) {
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            std::cerr << "Erro ao abrir o arquivo de memória: " << filename << std::endl;
            return;
        }
        
        std::string line;
        int index = 0;
        while (std::getline(infile, line) && index < 32) {
            sc_uint<16> value(line.c_str());  // Construtor direto de string binária
            std::cout << "Carregando valor: " << value.to_string(SC_BIN) << " no índice: " << index << std::endl;
            mem[index++] = value;
        }

        infile.close();
    }

    void save_memory(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo para salvar a memória: " << filename << std::endl;
            return;
        }
    
        for (int i = 0; i < 32; ++i) {
            file << mem[i].to_string(SC_BIN) << "\n";
        }
    
        file.close();
    }

    // Write process
    void write_proc() {
        if (!rst_n.read()) {
          for (int i = 0; i < 32; ++i) {
                mem[i] = 0;
            }
        } else if (write_enable.read()) {
            // Checa se ultimo bit do endereço é 0 (endereço alinhado)
            if ((address.read() & 0x1) == 0) {
                auto word_index_tmp = address.read() >> 1;
                if (word_index_tmp < 32) {
                    sc_uint<5> word_index = word_index_tmp; // só converte após checar
                    mem[word_index] = write_data.read();
                }
            }
        }
    }

    // Read process
    void read_proc() {
        sc_uint<5> word_index = address.read() >> 1;  // divide by 2
        if (word_index < 32) {
            read_data.write(mem[word_index]);
        } else {
            read_data.write(0);  // Optional: undefined behavior for out-of-range
        }
    }

    // Constructor with mem_init as argument
    SC_CTOR(memory) {
        SC_METHOD(read_proc);
        sensitive << address;

        SC_METHOD(write_proc);
        sensitive << clk.pos();
    }
};