#ifndef TOP_H
#define TOP_H

#include "initiator.h"
#include "target.h"

// *****************************************************************************************
// Top-level module instantiates NUM initiators, a bus, and NUM targets
// *****************************************************************************************

SC_MODULE(Top)
{
  Initiator* init;
  Target*    target;

  SC_CTOR(Top)
  {
    init = new Initiator("src");
    target = new Target("targ");

    init->socket.bind( target->socket );
  }
};

#endif
