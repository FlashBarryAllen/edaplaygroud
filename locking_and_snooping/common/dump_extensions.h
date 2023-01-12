
// Filename: dump_extensions.h

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

// Version 1  03-Jul-2009



#ifndef __DUMP_EXTENSIONS_H__
#define __DUMP_EXTENSIONS_H__

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"

#include <sstream>


void dump_extensions ( tlm::tlm_generic_payload& trans )
{
  std::ostringstream txt;

  txt << "\n\nTransaction details:";
  txt << "\n  has_mm             = " << dec << trans.has_mm() << " (bool)";
  txt << "\n  ref_count          = " << dec << trans.get_ref_count() << " (int)";
  txt << "\n\n  command            = " <<
     (trans.get_command() == tlm::TLM_READ_COMMAND  ? "TLM_READ_COMMAND"
     :trans.get_command() == tlm::TLM_WRITE_COMMAND ? "TLM_WRITE_COMMAND"
                                                    : "TLM_IGNORE_COMMAND");
  txt << "\n  address            = " << hex << trans.get_address() << " (hex)";
  txt << "\n  data_ptr           = " << hex
      << reinterpret_cast<int*>(trans.get_data_ptr()) << " (hex)";
  txt << "\n  data_length        = " << hex << trans.get_data_length() << " (hex)";
  txt << "\n  streaming_width    = " << hex << trans.get_streaming_width() << " (hex)";
  txt << "\n  byte_enable_ptr    = " << hex
      << reinterpret_cast<int*>(trans.get_byte_enable_ptr()) << " (hex)";
  txt << "\n  byte_enable_length = " << hex << trans.get_byte_enable_length() << " (hex)";
  txt << "\n  dmi_allowed        = " << dec << trans.is_dmi_allowed() << " (bool)";
  txt << "\n  response_status    = " << trans.get_response_string();

  txt << "\n\n  size of extension array = " << tlm::max_num_extensions();
  bool extensions_present = false;
  for (unsigned int i = 0; i < tlm::max_num_extensions(); i++)
  {
    tlm::tlm_extension_base* ext = trans.get_extension(i);
    if (ext)
    {
      if (!extensions_present)
        txt << "\n\n  extensions:";
      txt << "\n    index = " << i << "   type = " << typeid(*ext).name();
      extensions_present = true;
    }
  }

  txt << "\n\n";
  SC_REPORT_INFO("JA", txt.str().c_str());
}


#endif
