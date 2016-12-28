// $Id$
//
//    File: EGeneratedEvent.h
// Created: Tue Dec 27 14:56:22 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _EGeneratedEvent_
#define _EGeneratedEvent_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
#include <JLEIC_MC/EGeneratedParticle.h>

/// A generated(simulated) event's initial state particles.
///
/// This contains a list of particles representing the complete hierarchy
/// of a generated event. For the simplest generator (particle gun) the
/// list will be a single particle with no parent and no children. More
/// complex interactions can be represented with decay chains of particles
/// of unlimited depth.
///
/// The "seed" member is a string representing the state of the random
/// number generator obtained via the ">>" operator as described in the
/// C++11 standard.


class EGeneratedEvent:public jana::JObject{
	public:
		JOBJECT_PUBLIC(EGeneratedEvent);

		EGeneratedEvent(const char *generator_name):generator_name(generator_name){}

		const char *generator_name;
		vector<EGeneratedParticle> particles;
		string seed; // seed used to produce event

		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			string clipped_seed = seed.length()<=16 ? seed:(seed.substr(15) + "...");

			AddString(items, "Generator", "%s", generator_name ? generator_name:"unknown");
			AddString(items, "Npart", "%4d", particles.size());
			AddString(items, "seed", "%s", clipped_seed.c_str());
		}
};

#endif // _EGeneratedEvent_
