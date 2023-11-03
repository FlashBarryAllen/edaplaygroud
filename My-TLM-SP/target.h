#ifndef TARGET_H
#define TARGET_H

#include "utilities.h"
#include "tlm_utils/simple_target_socket.h"
#include <queue>

// **************************************************************************************
// Target module able to handle two pipelined transactions
// **************************************************************************************

DECLARE_EXTENDED_PHASE(internal_ph);

struct Target: sc_module
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_target_socket<Target> socket;
  sc_in_clk clk;
  int m_count;
  std::vector<std::shared_ptr<A>> vec;

  SC_CTOR(Target)
  : socket("socket"), m_count(0)
  {
    sc_clock* sys_clk = new sc_clock("sys_clk", 1, SC_NS);
    clk(*sys_clk);

    // Register callbacks for incoming interface method calls
    socket.register_nb_transport_fw(this, &Target::nb_transport_fw);
    void main_run();
    SC_METHOD(main_run);
    sensitive << clk.pos();
  }

  // TLM-2 non-blocking transport method

  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    sc_dt::uint64    adr = trans.get_address();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned char*   dat = trans.get_data_ptr();
    unsigned int     wid = trans.get_streaming_width();

    B* b = reinterpret_cast<B*>(dat);
    std::shared_ptr<A> aa = b->m_aa;
    vec.emplace_back(aa);

    std::cout << aa->a << aa->b << b->c << std::endl;

    delete b;

    // Obliged to check the transaction attributes for unsupported features
    // and to generate the appropriate error response
    if (byt != 0) {
      trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
      return tlm::TLM_COMPLETED;
    }
    if (len > 4 || wid < len) {
      trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
      return tlm::TLM_COMPLETED;
    }

    return tlm::TLM_ACCEPTED;
  }

  void main_run() {
    m_count++;
    //std::cout << "m_count = " << m_count << std::endl;
    
    if (m_count % 10 == 0) {
      if (!vec.empty()) {
        std::shared_ptr<A> aa = vec.front();
        vec.erase(vec.begin());
      }
    }
    
  }
};

#endif
