// $Id$
//
//    File: EGeneratedEvent_factory_ParticleGun.h
// Created: Tue Dec 27 17:47:37 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _EGeneratedEvent_factory_ParticleGun_
#define _EGeneratedEvent_factory_ParticleGun_

#include <JANA/JFactory.h>
#include "EGeneratedEvent.h"

class EGeneratedEvent_factory_ParticleGun:public jana::JFactory<EGeneratedEvent>{
	public:
		EGeneratedEvent_factory_ParticleGun(){};
		~EGeneratedEvent_factory_ParticleGun(){};
		const char* Tag(void){return "ParticleGun";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop,  int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _EGeneratedEvent_factory_ParticleGun_
