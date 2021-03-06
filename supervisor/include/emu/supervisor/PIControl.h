#ifndef __emu_supervisor_PIControl_h__
#define __emu_supervisor_PIControl_h__

#include "emu/supervisor/TCDSControl.h"

namespace emu{
  namespace supervisor{
    class PIControl : public TCDSControl{
    public:
      PIControl( xdaq::Application *parent, 
		 xdaq::ApplicationDescriptor* tcdsApplicationDescriptor, 
		 xdata::String partition );
      TCDSControl& setUsePrimaryTCDS( xdata::Boolean& usePrimaryTCDS );
    };
  }
}

#endif
