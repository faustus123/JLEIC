
// $Id$
//
//    File: JFactoryGenerator_vtx_fit.cc
// Created: Thu Jan  5 22:00:26 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JApplication.h>
using namespace jana;

#include "VertexTrackerHit_factory.h"
#include "VertexTrackerCluster_factory.h"
#include "VertexTrackerTrack_factory.h"

class JFactoryGenerator_vtx_fit: public jana::JFactoryGenerator{
	public:
		JFactoryGenerator_vtx_fit(){}
		virtual ~JFactoryGenerator_vtx_fit(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JFactoryGenerator_vtx_fit";}
		
		jerror_t GenerateFactories(jana::JEventLoop *loop){
			loop->AddFactory(new VertexTrackerHit_factory());
			loop->AddFactory(new VertexTrackerCluster_factory());
			loop->AddFactory(new VertexTrackerTrack_factory());
			return NOERROR;
		}

};

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddFactoryGenerator(new JFactoryGenerator_vtx_fit());
}
} // "C"
