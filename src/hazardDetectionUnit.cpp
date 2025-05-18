#include <systemc.h>

SC_MODULE(hazardUnit) {
    sc_in<bool> clk;
    sc_in<bool> rst_n;
    sc_in<sc_uint<3>> rs_id, rt_id;
    sc_in<sc_uint<3>> rd_ex, rd_mem, rd_wb;
    sc_in<bool> reg_write_ex, reg_write_mem, reg_write_wb;
    sc_out<bool> stall;

    sc_signal<bool> prev_stall_wb;

    void detect() {
        if (!rst_n.read()) {
            prev_stall_wb.write(false);
            return;
        } else {

            bool hazard_ex = reg_write_ex.read() && (
                (rd_ex.read() == rs_id.read()) || (rd_ex.read() == rt_id.read())
            );
            bool hazard_mem = reg_write_mem.read() && (
                (rd_mem.read() == rs_id.read()) || (rd_mem.read() == rt_id.read())
            );
            bool hazard_wb = reg_write_wb.read() && (
                (rd_wb.read() == rs_id.read()) || (rd_wb.read() == rt_id.read())
            );

            stall.write(hazard_ex || hazard_mem || hazard_wb || prev_stall_wb.read());

            prev_stall_wb.write(hazard_wb);

            std::cout << "[HAZARD DETECTION UNIT]--------------------------------------------------------" << std::endl;
            std::cout << "rd_ex: " << rd_ex.read().to_string(SC_BIN) << std::endl;
            std::cout << "rd_mem: " << rd_mem.read().to_string(SC_BIN) << std::endl;
            std::cout << "rd_wb: " << rd_wb.read().to_string(SC_BIN) << std::endl;
            std::cout << "rs_id: " << rs_id.read().to_string(SC_BIN) << std::endl;
            std::cout << "rt_id: " << rt_id.read().to_string(SC_BIN) << std::endl;
            std::cout << "-------------------------------------------------------------------------------" << std::endl;
        }

    }

    SC_CTOR(hazardUnit) {
        SC_METHOD(detect);
        sensitive << clk.pos() << rs_id << rt_id << rd_ex << rd_mem << rd_wb << reg_write_ex << reg_write_mem;
    }
};

