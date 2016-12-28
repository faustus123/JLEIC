// $Id$
//
//    File: EGeneratedParticle.h
// Created: Tue Dec 27 14:57:35 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _EGeneratedParticle_
#define _EGeneratedParticle_

#include <set>

#include <EKinematicData.h>

class EGeneratedEvent;

/// Generated (simulated) particle info.
///
/// This inherits from DKinematicData and adds pointers to
/// its parent, children, and sibilings. This is meant to
/// allow easy navigation of the hierarchy of particles in
/// a simulated event. A EGeneratedEvent object contains a
/// list of EGeneratedParticle objects that together,
/// represent a complete event, including any intermediary
/// particles.

class EGeneratedParticle:public EKinematicData{
	public:
		EGeneratedParticle():event(NULL),parent(NULL){}
		virtual ~EGeneratedParticle();

		              EGeneratedEvent *event;
		           EGeneratedParticle *parent;
		std::set<EGeneratedParticle*> children;
		std::set<EGeneratedParticle*> siblings;

		bool IsFinalState(void) { return children.empty(); }
};

#endif // _EGeneratedParticle_
