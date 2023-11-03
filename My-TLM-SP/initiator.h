#ifndef INITIATOR_H
#define INITIATOR_H

#include "utilities.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include <memory>

// **************************************************************************************
// Initiator module generating multiple pipelined generic payload transactions
// **************************************************************************************

struct Initiator: sc_module
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_initiator_socket<Initiator> socket;

  SC_CTOR(Initiator)
  : socket("socket")  // Construct and name socket
  {
    // Register callbacks for incoming interface method calls
    //socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);

    SC_THREAD(thread_process);
  }

  void thread_process()
  {
    tlm::tlm_generic_payload trans;
    tlm::tlm_phase phase;
    sc_time delay;

    // Generate a sequence of random transactions
    for (int i = 0; i < 10000000; i++)
    {
      int adr = rand();
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(rand() % 2);
      if (cmd == tlm::TLM_WRITE_COMMAND) data[i % 16] = rand();

      std::shared_ptr<A> aa = std::make_shared<A>();
      aa->a = 10;
      aa->b = 20;
      B* b = new B();
      b->c = 30;
      b->m_aa = aa;

      // Set all attributes except byte_enable_length and extensions (unused)
      trans.set_command( cmd );
      trans.set_address( adr );
      trans.set_data_ptr( reinterpret_cast<unsigned char*>(b) );
      trans.set_data_length( 4 );
      trans.set_streaming_width( 4 ); // = data_length to indicate no streaming
      trans.set_byte_enable_ptr( 0 ); // 0 indicates unused
      trans.set_dmi_allowed( false ); // Mandatory initial value
      trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

      // Timing annotation models processing time of initiator prior to call
      delay = sc_time(rand_ps(), SC_PS);

      //fout << hex << adr << " " << name() << " new, cmd=" << (cmd ? 'W' : 'R')
      //     << ", data=" << hex << data[i % 16] << " at time " << sc_time_stamp() << endl;

      // Non-blocking transport call on the forward path
      tlm::tlm_sync_enum status;
      status = socket->nb_transport_fw( trans, phase, delay );

      // Check value returned from nb_transport_fw
      if (status == tlm::TLM_UPDATED)
      {
      }
    }
  }

  mm   m_mm;
  int  data[16];
};

#endif
