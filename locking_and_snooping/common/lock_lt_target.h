
// Filename: lock_lt_target.h

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


#ifndef __LOCK_LT_TARGET_H__
#define __LOCK_LT_TARGET_H__

#include "../common/common_header.h"

struct Lock_LT_target: sc_module
{
  // Simple LT target for use with Lock_LT example.

  tlm_utils::simple_target_socket<Lock_LT_target, 32> socket;

  const sc_time LATENCY;

  SC_CTOR(Lock_LT_target)
  : socket("socket")
  , LATENCY(10, SC_NS)
  {
    socket.register_b_transport       (this, &Lock_LT_target ::b_transport);
  }


  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    if (byt != 0) {
      trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
      return;
    }

    if (len > 4 || wid < len) {
      trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
      return;
    }

    if ( cmd == tlm::TLM_READ_COMMAND )
    {
      *reinterpret_cast<int*>(ptr) = -int(adr);
      fout << hex << adr << " Execute READ, data = " << *reinterpret_cast<int*>(ptr)
           << " in " << name() << endl;
    }
    else if ( cmd == tlm::TLM_WRITE_COMMAND )
      fout << hex << adr << " Execute WRITE, data = " << *reinterpret_cast<int*>(ptr)
           << " in " << name() << endl;

    delay = delay + LATENCY;

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
  }
};

#endif

