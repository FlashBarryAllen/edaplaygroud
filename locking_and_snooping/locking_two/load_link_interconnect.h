
// Filename: load_link_interconnect.h

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


#ifndef __LOAD_LINK_INTERCONNECT_H__
#define __LOAD_LINK_INTERCONNECT_H__

#include "../common/common_header.h"
#include "lock_extension.h"

struct Load_link_interconnect: sc_module
{
  // Interconnect component that implements LOAD-LINK/STORE-CONDITIONAL commmands using extension

  tlm_utils::passthrough_target_socket<Load_link_interconnect, 32> targ_socket;
  tlm_utils::simple_initiator_socket  <Load_link_interconnect, 32> init_socket;

  SC_CTOR(Load_link_interconnect)
  : targ_socket("targ_socket")
  , init_socket("init_socket")
  {
    targ_socket.register_b_transport              (this, &Load_link_interconnect::b_transport);
    targ_socket.register_nb_transport_fw          (this, &Load_link_interconnect::nb_transport_fw);
    targ_socket.register_get_direct_mem_ptr       (this, &Load_link_interconnect::get_direct_mem_ptr);
    targ_socket.register_transport_dbg            (this, &Load_link_interconnect::transport_dbg);
    init_socket.register_nb_transport_bw          (this, &Load_link_interconnect::nb_transport_bw);
    init_socket.register_invalidate_direct_mem_ptr(this, &Load_link_interconnect::invalidate_direct_mem_ptr);
  }

  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    if ( load_link_store_conditional( trans ) )
      init_socket->b_transport( trans, delay );
  }


  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    if ( load_link_store_conditional( trans ) )
      return init_socket->nb_transport_fw( trans, phase, delay );
    else
      return tlm::TLM_COMPLETED;
  }

  virtual bool get_direct_mem_ptr( tlm::tlm_generic_payload& trans,
                                           tlm::tlm_dmi& dmi_data)
  {
    return init_socket->get_direct_mem_ptr( trans, dmi_data );
  }

  virtual unsigned int transport_dbg( tlm::tlm_generic_payload& trans )
  {
    return init_socket->transport_dbg( trans );
  }


  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    return targ_socket->nb_transport_bw( trans, phase, delay );
  }

  virtual void invalidate_direct_mem_ptr( sc_dt::uint64 start_range,
                                                  sc_dt::uint64 end_range )
  {
    targ_socket->invalidate_direct_mem_ptr(start_range, end_range);
  }


  bool load_link_store_conditional( tlm::tlm_generic_payload& trans )
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();

    load_link_guard_ext* ext;
    trans.get_extension(ext);

    if (ext)
    {
      load_link_data_ext* ext;
      trans.get_extension(ext);

      if (ext->cmd == load_link_data_ext::LOAD_LINK)
      {
        lock_map[adr] = true;
      }
      else if (ext->cmd == load_link_data_ext::STORE_CONDITIONAL)
      {
        if ( !lock_map[adr] )
        {
          trans.set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
          return false;
        }
        lock_map[adr] = false;
      }
    }
    else if (cmd == tlm::TLM_WRITE_COMMAND)
    {
      if (lock_map[adr])
        lock_map[adr] = false;
    }
    return true;
  }

  std::map <sc_dt::uint64, bool> lock_map;
};

#endif

