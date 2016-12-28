// $Id$
//
//    File: EGeneratedEvent_factory.h
// Created: Tue Dec 27 14:56:22 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _EGeneratedEvent_factory_
#define _EGeneratedEvent_factory_

#include <JANA/JFactory.h>
#include "EGeneratedEvent.h"

/// Placeholder to warn user that they are using a tagless
/// EGeneratedEvent_factory. The tag is used to provide the
/// name of the generator so a tagless factory could lead to
/// confusion later and so is discouraged.

class EGeneratedEvent_factory:public jana::JFactory<EGeneratedEvent>{
	public:
		EGeneratedEvent_factory(){};
		~EGeneratedEvent_factory(){};

		jerror_t EGeneratedEvent_factory::init(void)
		{
			jerr << " This class is a placeholder to prevent the use of an untagged" << endl;
			jerr << " EGeneratedEvent_factory class. Please check your configuration" << endl;
			jerr << " and code to ensure a generator is specified." << endl;

			return VALUE_OUT_OF_RANGE;
		}

	private:
		jerror_t init(void);						///< Called once at program start.
};

#endif // _EGeneratedEvent_factory_
