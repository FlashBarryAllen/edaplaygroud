
// Filename: load_link_extension.h

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


#ifndef __LOCK_EXTENSION_H__
#define __LOCK_EXTENSION_H__

#include "tlm.h"

#include "../common/common_header.h"

struct load_link_guard_ext: tlm::tlm_extension<load_link_guard_ext>
{
  // LOAD-LINK/STORE-CONDITIONAL command extension for atomic memory operations.

  load_link_guard_ext() {}

  virtual tlm_extension_base* clone() const
  {
    load_link_guard_ext* ext = new load_link_guard_ext;
    return ext;
  }

  virtual void copy_from(tlm_extension_base const &ext) {}

  // Override free so that it does not attempt to delete the static object
  virtual void free() {}
};


struct load_link_data_ext: tlm::tlm_extension<load_link_data_ext>
{
  load_link_data_ext() { cmd = LOAD_LINK; }

  virtual tlm_extension_base* clone() const
  {
    load_link_data_ext* ext = new load_link_data_ext;
    ext->cmd = this->cmd;
    return ext;
  }

  virtual void copy_from(tlm_extension_base const &ext)
  {
    cmd = static_cast<load_link_data_ext const &>(ext).cmd;
  }

  enum cmd_t {LOAD_LINK, STORE_CONDITIONAL};
  cmd_t cmd;
};



struct lock_guard_ext: tlm::tlm_extension<lock_guard_ext>
{
  // Lock command extension for atomic memory operations
  // Can be used to implement a READ-MODIFY-WRITE
  // Can be used to implement X-LOCKED-Y, where X and Y could each be READ or WRITE
  // Extension == true => lock given address
  // Extension == false => unlock given address
  // Extension absent and given address locked => bounce transaction
  // If id is non-zero, it is used to identify the initiator; the same initiator must lock and unlock
  // In other words, id == 0 means that the lock is anonymous

  lock_guard_ext() {}

  virtual tlm_extension_base* clone() const
  {
    lock_guard_ext* ext = new lock_guard_ext;
    return ext;
  }

  virtual void copy_from(tlm_extension_base const &ext) {}

  // Override free so that it does not attempt to delete the static object
  virtual void free() {}
};

struct lock_data_ext: tlm::tlm_extension<lock_data_ext>
{
  lock_data_ext() { lock = false; id = 0; }

  virtual tlm_extension_base* clone() const
  {
    lock_data_ext* ext = new lock_data_ext;
    ext->lock = this->lock;
    ext->id   = this->id;
    return ext;
  }

  virtual void copy_from(tlm_extension_base const &ext)
  {
    lock = static_cast<lock_data_ext const &>(ext).lock;
    id   = static_cast<lock_data_ext const &>(ext).id;
  }

  bool lock;
  int  id;
};

#endif

