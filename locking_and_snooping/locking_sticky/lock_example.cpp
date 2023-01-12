
// Filename: lock_example.cpp

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
// Version 2  03-Jul-2009

/*

Lock example.

This example demonstrates memory locking implemented using a generic payload extension.
It builds on the AT example (file at_example.cpp), instantiating an extra initiator, extra target,
and extra interconnect component.

In this case, memory locking means a LOAD-LINK/STORE-CONDITIONAL scheme (e.g. see Wikipedia).
A LOAD-LINK from a particular address does a READ and sets a link on that address.
A subsequent WRITE to the same address clears the link.
A subsequent STORE-CONDITIONAL to the same address will only succeed if the link is still set.

(Well, to be pedantic, LOAD-LINK/STORE-CONDITIONAL implements an atomic memory operation WITHOUT
using a lock. But other transaction and memory locking schemes such as READ-MODIFY-WRITE can be
implemented using the same mechanism.)

Lock_LT_initiator generates transactions with the lock extension, and sends them through the
AT interconnect and Lock_interconnect to the LT target. Lock_interconnect is connected in-line
between the AT interconnect and the LT target.

Lock_LT_initiator sends transactions through the Lock_interconnect only, because the other targets
do not support locking. The remaining initiators send regular transactions to all targets,
including the LT target. Hence WRITE transactions from other targets will sometimes cause a
STORE-CONDITIONAL to fail.

Lock_interconnect intercepts the lock extension and implements the locking. Lock_interconnect
propagates regular read/write transactions forward to the LT target, which does not itself understand
locking. When STORE-CONDITIONAL fails, the write transaction is not propagated forward, but the
Lock_interconnect bounces the transaction with a response status of TLM_COMMAND_ERROR_RESPONSE.
The Lock_LT_initiator must then retry the same transaction.

The lock extension is implemented here as an ignorable extension. That is, the sockets use
tlm_base_protocol_types. But the extension is ignorable only in the sense that an interconnect
or target that ignores it will never detect that a STORE-CONDITIONAL should fail and never return
an error response. Really, it would be better either to define a new protocol type or to
implement and protocol negotiation phase to ensure that the lock controller is in place.

You may edit the #defines below to use a different combination of initiator and target types,
BUT beware that the Lock_LT_initiator assumes the LT target is target5 (counting from zero)

The base protocol checkers are included solely for debug and verification.

*/


#include "lock_lt_initiator.h"
#include "lock_interconnect.h"
#include "load_link_interconnect.h"
#include "../common/lock_lt_target.h"

#include "../common/at_typea_initiator.h"
#include "../common/at_typeb_initiator.h"
#include "../common/at_interconnect.h"
#include "../common/at_typea_target.h"
#include "../common/at_typeb_target.h"
#include "../common/at_typec_target.h"
#include "../common/at_typed_target.h"
#include "../common/at_typee_target.h"

#include "../common/gp_mm.h"

#define initiator0_t AT_typeA_initiator
#define initiator1_t AT_typeA_initiator
#define initiator2_t AT_typeB_initiator
#define initiator3_t Lock_LT_initiator

#define target0_t    AT_typeA_target
#define target1_t    AT_typeB_target
#define target2_t    AT_typeC_target
#define target3_t    AT_typeD_target
#define target4_t    AT_typeE_target
#define target5_t    Lock_LT_target


SC_MODULE(Top)
{
  initiator0_t *initiator0;
  initiator1_t *initiator1;
  initiator2_t *initiator2;
  initiator3_t *initiator3;

  AT_interconnect        *at_interconnect;
  Lock_interconnect      *lock_interconnect;
  Load_link_interconnect *load_link_interconnect;

  target0_t    *target0;
  target1_t    *target1;
  target2_t    *target2;
  target3_t    *target3;
  target4_t    *target4;
  target5_t    *target5;

  tlm_utils::tlm2_base_protocol_checker<> *check_init0;
  tlm_utils::tlm2_base_protocol_checker<> *check_init1;
  tlm_utils::tlm2_base_protocol_checker<> *check_init2;
  tlm_utils::tlm2_base_protocol_checker<> *check_init3;

  tlm_utils::tlm2_base_protocol_checker<> *check_targ0;
  tlm_utils::tlm2_base_protocol_checker<> *check_targ1;
  tlm_utils::tlm2_base_protocol_checker<> *check_targ2;
  tlm_utils::tlm2_base_protocol_checker<> *check_targ3;
  tlm_utils::tlm2_base_protocol_checker<> *check_targ4;
  tlm_utils::tlm2_base_protocol_checker<> *check_targ5;


  SC_CTOR(Top)
  {
    // Single memory manager common to all initiators
    m_mm = new gp_mm;

    initiator0   = new initiator0_t("initiator0", m_mm);
    initiator1   = new initiator1_t("initiator1", m_mm);
    initiator2   = new initiator2_t("initiator2", m_mm);
    initiator3   = new initiator3_t("LT_initiator3", m_mm);

    at_interconnect        = new AT_interconnect  ("at_interconnect");
    lock_interconnect      = new Lock_interconnect("lock_interconnect");
    load_link_interconnect = new Load_link_interconnect("load_link_interconnect");

    target0      = new target0_t("target0");
    target1      = new target1_t("target1");
    target2      = new target2_t("target2");
    target3      = new target3_t("target3");
    target4      = new target4_t("target4");
    target5      = new target5_t("LT_memory5");

    check_init0  = new tlm_utils::tlm2_base_protocol_checker<>("check_init0");
    check_init1  = new tlm_utils::tlm2_base_protocol_checker<>("check_init1");
    check_init2  = new tlm_utils::tlm2_base_protocol_checker<>("check_init2");
    check_init3  = new tlm_utils::tlm2_base_protocol_checker<>("check_init3");

    check_targ0  = new tlm_utils::tlm2_base_protocol_checker<>("check_targ0");
    check_targ1  = new tlm_utils::tlm2_base_protocol_checker<>("check_targ1");
    check_targ2  = new tlm_utils::tlm2_base_protocol_checker<>("check_targ2");
    check_targ3  = new tlm_utils::tlm2_base_protocol_checker<>("check_targ3");
    check_targ4  = new tlm_utils::tlm2_base_protocol_checker<>("check_targ4");
    check_targ5  = new tlm_utils::tlm2_base_protocol_checker<>("check_targ5");

    initiator0->socket.bind(check_init0->target_socket);
    check_init0->initiator_socket.bind(at_interconnect->targ_socket);

    initiator1->socket.bind(check_init1->target_socket);
    check_init1->initiator_socket.bind(at_interconnect->targ_socket);

    initiator2->socket.bind(check_init2->target_socket);
    check_init2->initiator_socket.bind(at_interconnect->targ_socket);

    initiator3->socket.bind(check_init3->target_socket);
    check_init3->initiator_socket.bind(at_interconnect->targ_socket);

    at_interconnect->init_socket.bind(check_targ0->target_socket);
    check_targ0->initiator_socket.bind(target0->socket);

    at_interconnect->init_socket.bind(check_targ1->target_socket);
    check_targ1->initiator_socket.bind(target1->socket);

    at_interconnect->init_socket.bind(check_targ2->target_socket);
    check_targ2->initiator_socket.bind(target2->socket);

    at_interconnect->init_socket.bind(check_targ3->target_socket);
    check_targ3->initiator_socket.bind(target3->socket);

    at_interconnect->init_socket.bind(lock_interconnect->targ_socket);
    lock_interconnect->init_socket.bind(check_targ4->target_socket);
    check_targ4->initiator_socket.bind(target4->socket);

    at_interconnect->init_socket.bind(load_link_interconnect->targ_socket);
    load_link_interconnect->init_socket.bind(check_targ5->target_socket);
    check_targ5->initiator_socket.bind(target5->socket);
  }

  ~Top()
  {
    delete m_mm;
  }

  gp_mm* m_mm;
};


int sc_main(int argc, char* argv[])
{
  Top top("top");
  sc_start();
  return 0;
}
