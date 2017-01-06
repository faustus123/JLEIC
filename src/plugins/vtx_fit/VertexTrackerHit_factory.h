// $Id$
//
//    File: VertexTrackerHit_factory.h
// Created: Thu Jan  5 21:59:40 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _VertexTrackerHit_factory_
#define _VertexTrackerHit_factory_

#include <JANA/JFactory.h>
#include "VertexTrackerHit.h"

class VertexTrackerHit_factory:public jana::JFactory<VertexTrackerHit>{
	public:
		VertexTrackerHit_factory(){};
		~VertexTrackerHit_factory(){};


	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _VertexTrackerHit_factory_

