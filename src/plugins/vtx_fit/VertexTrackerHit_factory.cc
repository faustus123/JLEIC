// $Id$
//
//    File: VertexTrackerHit_factory.cc
// Created: Thu Jan  5 21:59:40 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "VertexTrackerHit_factory.h"
using namespace jana;

#include <vtx_mc/MCVertexTrackerHit.h>

//------------------
// init
//------------------
jerror_t VertexTrackerHit_factory::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t VertexTrackerHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t VertexTrackerHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// Get the inputs needed for this factory. Here, only the 
	// MCVertexTrackerHit objects are needed.
	vector<const MCVertexTrackerHit*> mchits;
	loop->Get(mchits);

	// Loop over all MC hits
	for( auto mchit : mchits){
	
		// Create an object of the type this factory makes. In this
		// case there is not really any algorithm. The values are
		// just copied from the MC hit. 
		auto hit = new VertexTrackerHit;
		hit->planeid = mchit->planeid;
		hit->Xpos    = mchit->Xpos;
		hit->Ypos    = mchit->Ypos;
		hit->Zpos    = mchit->Zpos;
		
		// One can add any number of objects as associated objects.
		// This allows you to trace back to the objects that were used
		// to create the object you're working with.
		hit->AddAssociatedObject(mchit);

		// Adding an object point to the _data container is how
		// the data is published to JANA. It actually just stays
		// in the container, but if JANA gets a request for the 
		// objects from this factory, that is where it will look.
		// JANA will also automatically delete the objects in _data
		// at the start of the next event.
		_data.push_back(hit);
	}

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t VertexTrackerHit_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t VertexTrackerHit_factory::fini(void)
{
	return NOERROR;
}

