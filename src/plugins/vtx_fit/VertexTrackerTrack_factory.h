// $Id$
//
//    File: VertexTrackerTrack_factory.h
// Created: Thu Jan  5 21:58:34 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _VertexTrackerTrack_factory_
#define _VertexTrackerTrack_factory_

#include <JANA/JFactory.h>
#include "VertexTrackerTrack.h"

#include "DQuickFit.h"

class VertexTrackerTrack_factory:public jana::JFactory<VertexTrackerTrack>{
	public:
		VertexTrackerTrack_factory(){};
		~VertexTrackerTrack_factory(){};

		DQuickFit qfit;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _VertexTrackerTrack_factory_

