// $Id$
//
//    File: VertexTrackerCluster_factory.cc
// Created: Thu Jan  5 21:59:51 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "VertexTrackerCluster_factory.h"
using namespace jana;

#include<vtx_mc/MCVertexTrackerHit.h>

//------------------
// init
//------------------
jerror_t VertexTrackerCluster_factory::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t VertexTrackerCluster_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t VertexTrackerCluster_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// Here we want to find hits that belong to the same track.
	// I'm going to cheat here by just peeking at the trackid 
	// of the mchit from which each hit was made. 
	vector<const VertexTrackerHit*> hits;
	loop->Get(hits);
	
	// Container to hold all hits, indexed by trackid
	map<int, vector<const VertexTrackerHit*> > clusters;

	// Loop over all hits
	for(auto hit : hits){
		
		// Grab the MCVertexTrackerHit object that was associated
		// with this hit. We use the GetSingle() method here since
		// we know there is only one MCVertexTrackerHit associated
		// with this. In cases where more than object of the same
		// type is associated use the Get() method and pass it a
		// STL vector just like we do with loop->Get(...) above.
		const MCVertexTrackerHit *mchit = NULL;
		hit->GetSingle(mchit);
		if(!mchit) continue;
		
		// Add this hit to the appropriate cluster
		clusters[mchit->trackid].push_back(hit);
	}
	
	// Loop over all clusters
	for(auto c : clusters){
	
		// Create new cluster object and copy hits into it
		auto cluster = new VertexTrackerCluster;
		cluster->hits = c.second;

		// Hand cluster object to JANA
		_data.push_back(cluster);
	}

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t VertexTrackerCluster_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t VertexTrackerCluster_factory::fini(void)
{
	return NOERROR;
}

