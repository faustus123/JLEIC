// $Id$
//
//    File: VertexTrackerTrack_factory.cc
// Created: Thu Jan  5 21:58:34 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "VertexTrackerTrack_factory.h"
using namespace jana;

#include "VertexTrackerCluster.h"

//------------------
// init
//------------------
jerror_t VertexTrackerTrack_factory::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t VertexTrackerTrack_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t VertexTrackerTrack_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	vector<const VertexTrackerCluster*> clusters;
	loop->Get(clusters);

	// Loop over clusters
	for(auto cluster : clusters){
		
		// Do a fast helical fit to hits
		qfit.Clear();
		for(auto h : cluster->hits) qfit.AddHitXYZ( h->Xpos, h->Ypos, h->Zpos );
		qfit.FitTrack();
	
		auto trk = new VertexTrackerTrack;
		trk->p     = qfit.p;
		trk->theta = qfit.theta;
		trk->phi   = qfit.phi;
		trk->chisq = qfit.chisq;
		trk->ndf   = (double)qfit.GetNhits();
		
		_data.push_back(trk);
	}


	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t VertexTrackerTrack_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t VertexTrackerTrack_factory::fini(void)
{
	return NOERROR;
}

