
// Filename: lock_lt_initiator.h

//----------------------------------------------------------------------
//  Copyright (c) 2008-2009 by Doulos Ltd.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//----------------------------------------------------------------------

// Version 1  09-Sep-2008
// Version 2  02-Jul-2009


#ifndef __LOCK_LT_INITIATOR_H__
#define __LOCK_LT_INITIATOR_H__

#include "../common/common_header.h"
#include "lock_extension.h"

struct Lock_LT_initiator: sc_module
{
  // Simple initiator that calls b_transport and uses lock extension

  tlm_utils::simple_initiator_socket<Lock_LT_initiator, 32> socket;

  Lock_LT_initiator(sc_module_name _n, gp_mm* mm)
  : socket("socket")
  , m_mm(mm)
  {
    SC_THREAD(thread_process_1);
    SC_THREAD(thread_process_2);
  }

  SC_HAS_PROCESS(Lock_LT_initiator);

  void thread_process_1()
  {
    // Exercises lock extension

    tlm::tlm_generic_payload* trans;
    sc_time delay = SC_ZERO_TIME;

    bool next_read  = true;
    bool next_write = false;
    tlm::tlm_command cmd;
    int  addr = 0;
    int  store_data = 0;

    for (int i = 0; i < 1000; i++)
    {
      trans = m_mm->allocate();
      trans->acquire();

      // Add sticky data extension once only
      lock_data_ext* ext;
      trans->get_extension(ext);
      if ( !ext )
      {
        ext = new lock_data_ext;
        trans->set_extension(ext);
      }

      trans->set_auto_extension( &lock_guard_ext_instance );

      if (next_read)
      {
        // Start a fresh read-modify-write command cycle
        addr = 0x400 | ((rand() % 256) & 0xFC); // Assuming memory is 5th target (or 4, counting from 0)
        next_read  = false;
        next_write = true;
      }

      if (i % 2 == 0)
      {
        cmd = tlm::TLM_READ_COMMAND;
        ext->lock = true;
        data1 = 0;
      }
      else
      {
        cmd = tlm::TLM_WRITE_COMMAND;
        ext->lock = false;
        if (next_write)
        {
          // Generate the data to be stored for this read-write cycle
          // The read may fail if the address is already locked, forcing the read to be repeated
          // with the same address and data, only moving on to a fresh read-write cycle
          // when it succeeds.
          store_data = (rand() << 16) | rand();
          next_write = false;
        }
        data1 = store_data;
      }

      trans->set_command( cmd );
      trans->set_address( addr );
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data1) );
      trans->set_data_length( 4 );
      trans->set_streaming_width( 4 );
      trans->set_byte_enable_ptr( 0 );
      trans->set_dmi_allowed( false );
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

      fout << hex << addr << " new, cmd=" << (ext->lock ? "lock" : "unlock")
           << ", data=" << hex << data1 << " at time " << sc_time_stamp()
           << " in " << name() << endl;

      socket->b_transport( *trans, delay );

      // When read fails, retry the command
      bool retry = false;
      if ( ext->lock )
      {
        if ( trans->get_response_status() == tlm::TLM_COMMAND_ERROR_RESPONSE )
          retry = true;
        else
          next_read = true;
      }

      if ( trans->is_response_error() && !retry )
      {
        char txt[100];
        sprintf(txt, "Transaction returned with error, response status = %s",
                     trans->get_response_string().c_str());
        SC_REPORT_ERROR("TLM-2", txt);
      }

      trans->release();

      wait(delay);
    }
  }

  void thread_process_2()
  {
    // Exercises LOAD-LINK, STORE-CONDITIONAL commands

    tlm::tlm_generic_payload* trans;
    sc_time delay = SC_ZERO_TIME;

    bool next_load  = true;
    bool next_store = false;
    tlm::tlm_command cmd;
    int  addr = 0;
    int  store_data = 0;

    for (int i = 0; i < 1000; i++)
    {
      trans = m_mm->allocate();
      trans->acquire();

      // Add sticky data extension once only
      load_link_data_ext* ext;
      trans->get_extension(ext);
      if ( !ext )
      {
        ext = new load_link_data_ext;
        trans->set_extension(ext);
      }

      trans->set_auto_extension( &load_link_guard_ext_instance );

      if (next_load)
      {
        // Start a fresh LOAD-STORE command cycle
        addr = 0x500 | ((rand() % 256) & 0xFC); // Assuming memory is 6th target (or 5, counting from 0)
        next_load  = false;
        next_store = true;
      }

      if (i % 2 == 0)
      {
        cmd = tlm::TLM_READ_COMMAND;
        ext->cmd = load_link_data_ext::LOAD_LINK;
        data2 = 0;
      }
      else
      {
        cmd = tlm::TLM_WRITE_COMMAND;
        ext->cmd = load_link_data_ext::STORE_CONDITIONAL;
        if (next_store)
        {
          // Generate the data to be stored for this LOAD-STORE cycle
          // The STORE_CONDITIONAL may fail, forcing the LOAD-STORE to be repeated
          // with the same address and data, only moving on to a fresh LOAD-STORE cycle
          // when it succeeds.
          store_data = (rand() << 16) | rand();
          next_store = false;
        }
        data2 = store_data;
      }

      trans->set_command( cmd );
      trans->set_address( addr );
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data2) );
      trans->set_data_length( 4 );
      trans->set_streaming_width( 4 );
      trans->set_byte_enable_ptr( 0 );
      trans->set_dmi_allowed( false );
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

      fout << hex << addr << " new, cmd=" << (ext->cmd ? "store" : "load")
           << ", data=" << hex << data2 << " at time " << sc_time_stamp()
           << " in " << name() << endl;

      socket->b_transport( *trans, delay );

      // When STORE_CONDITIONAL fails, retry the command
      bool retry = false;
      if (  ext->cmd == load_link_data_ext::STORE_CONDITIONAL )
      {
        if ( trans->get_response_status() == tlm::TLM_COMMAND_ERROR_RESPONSE )
          retry = true;
        else
          next_load = true;
      }

      if ( trans->is_response_error() && !retry )
      {
        char txt[100];
        sprintf(txt, "Transaction returned with error, response status = %s",
                     trans->get_response_string().c_str());
        SC_REPORT_ERROR("TLM-2", txt);
      }

      trans->release();

      wait(delay);
    }
  }

  gp_mm* m_mm; // Memory manager
  int data1;   // Internal data buffer used by initiator with generic payload
  int data2;   // Internal data buffer used by initiator with generic payload

  static lock_guard_ext      lock_guard_ext_instance;
  static load_link_guard_ext load_link_guard_ext_instance;
};

lock_guard_ext      Lock_LT_initiator::lock_guard_ext_instance;
load_link_guard_ext Lock_LT_initiator::load_link_guard_ext_instance;


#endif

