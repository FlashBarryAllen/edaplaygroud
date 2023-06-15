#ifndef TOP_H
#define TOP_H

#include "initiator.h"
#include "bus.h"
#include "target.h"

// *****************************************************************************************
// Top-level module instantiates NUM initiators, a bus, and NUM targets
// *****************************************************************************************

#define NUM 1

SC_MODULE(Top)
{
  Initiator* init[NUM];
  Bus*       bus;
  Target*    target[NUM];

  SC_CTOR(Top)
  {
    bus   = new Bus("bus");

    // ***************************************************************************
    // bus->init_socket and bus->targ_socket are multi-sockets, each bound NUM times
    // ***************************************************************************

    for (int i = 0; i < NUM; i++)
    {
      char txt[20];
      sprintf(txt, "init_%d", i);
      init[i] = new Initiator(txt);
      init[i]->socket.bind( bus->targ_socket );
    }

    for (int i = 0; i < NUM; i++)
    {
      char txt[20];
      sprintf(txt, "target_%d", i);
      target[i] = new Target(txt);

      bus->init_socket.bind( target[i]->socket );
    }
  }
};

#endif
