// $Id$
//
//    File: EGeneratedEvent_factory_ParticleGun.cc
// Created: Tue Dec 27 17:47:37 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "EGeneratedEvent_factory_ParticleGun.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t EGeneratedEvent_factory_ParticleGun::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t EGeneratedEvent_factory_ParticleGun::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t EGeneratedEvent_factory_ParticleGun::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	// Code to generate factory data goes here. Add it like:
	//
	// EGeneratedEvent *myEGeneratedEvent = new EGeneratedEvent;
	// myEGeneratedEvent->x = x;
	// myEGeneratedEvent->y = y;
	// ...
	// _data.push_back(myEGeneratedEvent);
	//
	// Note that the objects you create here will be deleted later
	// by the system and the _data vector will be cleared automatically.

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t EGeneratedEvent_factory_ParticleGun::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t EGeneratedEvent_factory_ParticleGun::fini(void)
{
	return NOERROR;
}

