
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


struct lock_extension: tlm::tlm_extension<lock_extension>
{
  // Lock command extension for atomic memory operations
  // Can be used to implement a READ-MODIFY-WRITE
  // Can be used to implement X-LOCKED-Y, where X and Y could each be READ or WRITE
  // Extension == true => lock given address
  // Extension == false => unlock given address
  // Extension absent and given address locked => bounce transaction
  // If id is non-zero, it is used to identify the initiator; the same initiator must lock and unlock
  // In other words, id == 0 means that the lock is anonymous

  lock_extension() { lock = false; id = 0; valid = false; }

  virtual tlm_extension_base* clone() const
  {
    lock_extension* ext = new lock_extension;
    ext->lock = this->lock;
    ext->id   = this->id;
    return ext;
  }

  virtual void copy_from(tlm_extension_base const &ext)
  {
    lock = static_cast<lock_extension const &>(ext).lock;
    id   = static_cast<lock_extension const &>(ext).id;
  }

  virtual void free()
  {
    valid = false;
  }

  bool lock;
  int  id;
  bool valid;
};


// Memory managers specific to locking extensions
// The allocate() method always returns a transaction with valid locking extensions
// The free() method invalidates the extensions, but does not remove them
// Delegates to a gp_mm, so must call set_mm() before use

class lock_mm: public tlm::tlm_mm_interface
{
public:
  void set_mm(gp_mm* arg) { m_mm = arg; }

  tlm::tlm_generic_payload* allocate();
  virtual void  free(tlm::tlm_generic_payload* trans);

private:
  gp_mm* m_mm;
};

tlm::tlm_generic_payload* lock_mm::allocate()
{
  assert( m_mm );
  tlm::tlm_generic_payload* trans = m_mm->allocate();
  trans->set_mm(this);

  lock_extension* ext;
  trans->get_extension(ext);
  if (!ext)
  {
    ext = new lock_extension;
    trans->set_extension(ext);
  }
  return trans;
}

void lock_mm::free(tlm::tlm_generic_payload* trans)
{
  lock_extension* ext;
  trans->get_extension( ext );
  assert( ext );
  ext->valid = false;

  trans->set_mm(m_mm);
  m_mm->free(trans);
}




struct load_link_extension: tlm::tlm_extension<load_link_extension>
{
  // LOAD-LINK/STORE-CONDITIONAL command extension for atomic memory operations.

  load_link_extension() { cmd = LOAD_LINK; valid = false; }

  virtual tlm_extension_base* clone() const
  {
    load_link_extension* ext = new load_link_extension;
    ext->cmd = this->cmd;
    return ext;
  }

  virtual void copy_from(tlm_extension_base const &ext)
  {
    cmd = static_cast<load_link_extension const &>(ext).cmd;
  }

  virtual void free()
  {
    valid = false;
  }

  enum cmd_t {LOAD_LINK, STORE_CONDITIONAL};
  cmd_t cmd;
  bool valid;
};


class load_link_mm: public tlm::tlm_mm_interface
{
public:
  void set_mm(gp_mm* arg) { m_mm = arg; }

  tlm::tlm_generic_payload* allocate();
  virtual void  free(tlm::tlm_generic_payload* trans);

private:
  gp_mm* m_mm;
};

tlm::tlm_generic_payload* load_link_mm::allocate()
{
  assert( m_mm );
  tlm::tlm_generic_payload* trans = m_mm->allocate();
  trans->set_mm(this);

  load_link_extension* ext;
  trans->get_extension(ext);
  if (!ext)
  {
    ext = new load_link_extension;
    trans->set_extension(ext);
  }
  return trans;
}

void load_link_mm::free(tlm::tlm_generic_payload* trans)
{
  load_link_extension* ext;
  trans->get_extension( ext );
  assert( ext );
  ext->valid = false;

  trans->set_mm(m_mm);
  m_mm->free(trans);
}

#endif

