#include <systemc.h>
#include "../src/cpu.cpp" // Inclua o caminho correto para o seu módulo CPU

SC_MODULE(cpu_tb) {
    // Sinais
    sc_signal<bool> clk;
    sc_signal<bool> rst_n;

    // Instância da CPU
    cpu* cpu_inst;

    // Geração de clock
    void clock_gen() {
        while (true) {
            clk.write(false);
            wait(20, SC_NS);
            clk.write(true);
            wait(20, SC_NS);
        }
    }

    // Estímulos
    void stimulus() {
        // Aplica reset
        rst_n.write(false);
        wait(20, SC_NS);
        rst_n.write(true);

        // Roda por alguns ciclos
        for (int i = 0; i < 45; ++i) {
            wait(20, SC_NS);
        }

        sc_stop(); // Termina a simulação
    }

    SC_CTOR(cpu_tb) {
        // Instanciar CPU
        cpu_inst = new cpu("cpu_inst");
        cpu_inst->clk(clk);
        cpu_inst->rst_n(rst_n);

        // Processos
        SC_THREAD(clock_gen);
        SC_THREAD(stimulus);
    }

    ~cpu_tb() {
        delete cpu_inst;
    }
};

int sc_main(int argc, char* argv[]) {
    cpu_tb tb("tb");
    sc_start(); // Inicia simulação
    return 0;
}