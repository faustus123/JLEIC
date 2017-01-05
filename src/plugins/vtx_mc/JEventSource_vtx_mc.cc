// $Id$
//
//    File: JEventSource_vtx_mc.cc
// Created: Wed Jan  4 19:53:11 EST 2017
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#include <iostream>
using namespace std;

#include "JEventSource_vtx_mc.h"
using namespace jana;

//----------------
// Constructor
//----------------
JEventSource_vtx_mc::JEventSource_vtx_mc(const char* source_name):JEventSource(source_name)
{
	fFile = new TFile(source_name);
	if(!fFile->IsOpen()){
		jerr << " Unable to open ROOT file: " << source_name << endl;
		exit(-1);
	}
	
	fReader = new TTreeReader("EVENT_HITS", fFile);

	EVENT_SIZE = new TTreeReaderValue<Int_t>(*fReader, "EVENT_SIZE");
	EVENT_NUM  = new TTreeReaderValue<Int_t>(*fReader, "EVENT_NUM");
	TrackID    = new TTreeReaderArray<Int_t>(*fReader, "TrackID");
	PlaneID    = new TTreeReaderArray<Int_t>(*fReader, "PlaneID");
	Xpos       = new TTreeReaderArray<Double_t>(*fReader, "Xpos");
	Ypos       = new TTreeReaderArray<Double_t>(*fReader, "Ypos");
	Zpos       = new TTreeReaderArray<Double_t>(*fReader, "Zpos");
	//Ipos       = new TTreeReaderArray<Int_t>(*fReader, "Ipos");
	//Jpos       = new TTreeReaderArray<Int_t>(*fReader, "Jpos");
	
}

//----------------
// Destructor
//----------------
JEventSource_vtx_mc::~JEventSource_vtx_mc()
{
	if(fReader   ) delete fReader;
	if(fFile     ) delete fFile;
	
	if(EVENT_SIZE) delete EVENT_SIZE;
	if(EVENT_NUM ) delete EVENT_NUM;
	if(TrackID   ) delete TrackID;
	if(PlaneID   ) delete PlaneID;
	if(Xpos      ) delete Xpos;
	if(Ypos      ) delete Ypos;
	if(Zpos      ) delete Zpos;
	//if(Ipos      ) delete Ipos;
	//if(Jpos      ) delete Jpos;
}

//----------------
// GetEvent
//----------------
jerror_t JEventSource_vtx_mc::GetEvent(JEvent &event)
{

	// Read an event from the source and copy the vital info into
	// the JEvent structure. The "Ref" of the JEventSource class
	// can be used to hold a pointer to an arbitrary object, though
	// you'll need to cast it to the correct pointer type 
	// in the GetObjects method.
	event.SetJEventSource(this);
	event.SetEventNumber(++Nevents_read);
	event.SetRunNumber(1234);
	event.SetRef(NULL);

	// If an event was sucessfully read in, return NOERROR. Otherwise,
	// return NO_MORE_EVENTS_IN_SOURCE. By way of example, this
	// will return NO_MORE_EVENTS_IN_SOURCE after 100k events.
	if( (!fFile) || (!fReader) ) return NO_MORE_EVENTS_IN_SOURCE;
	
	if(fReader->Next()){
	
		Event *evt = new Event;
		evt->event_num = **EVENT_NUM;
		evt->am_owner = true;
		for(uint32_t i=0; i<**EVENT_SIZE; i++){
			MCVertexTrackerHit *hit = new MCVertexTrackerHit;
			
			hit->trackid = (*TrackID)[i];
			hit->planeid = (*PlaneID)[i];
			hit->Xpos = (*Xpos)[i];
			hit->Ypos = (*Ypos)[i];
			hit->Zpos = (*Zpos)[i];
			//hit->Ipos = (*Ipos)[i];
			//hit->Jpos = (*Jpos)[i];

			evt->mcvtxhits.push_back(hit);
		}
	
		event.SetEventNumber(evt->event_num);
		event.SetRef(evt);
	
	}else{
		return NO_MORE_EVENTS_IN_SOURCE;
	}
	
	return NOERROR;
}

//----------------
// FreeEvent
//----------------
void JEventSource_vtx_mc::FreeEvent(JEvent &event)
{
	Event *evt = (Event*)event.GetRef();
	if(evt){
	
		// If GetObjects was called, then the hit objects will
		// be deleted by JANA automatically. If not then we need
		// to delete them here to avoid a memory leak.
		if(evt->am_owner){
			for( auto hit : evt->mcvtxhits ) delete hit;
		}
		delete evt;
	}
}

//----------------
// GetObjects
//----------------
jerror_t JEventSource_vtx_mc::GetObjects(JEvent &event, JFactory_base *factory)
{
	// This callback is called to extract objects of a specific type from
	// an event and store them in the factory pointed to by JFactory_base.
	// The data type desired can be obtained via factory->GetDataClassName()
	// and the tag via factory->Tag().
	//
	// If the object is not one of a type this source can provide, then
	// it should return OBJECT_NOT_AVAILABLE. Otherwise, it should return
	// NOERROR;
	
	// We must have a factory to hold the data
	if(!factory)throw RESOURCE_UNAVAILABLE;

	// Get name of data class we're trying to extract and the factory tag
	string dataClassName = factory->GetDataClassName();
	string tag = factory->Tag();
	
	if(dataClassName != "MCVertexTrackerHit") return OBJECT_NOT_AVAILABLE;
	
	Event *evt = (Event*)event.GetRef();
	if(!evt) return NOERROR;
	
	JFactory<MCVertexTrackerHit> *fac = dynamic_cast<JFactory<MCVertexTrackerHit>*>(factory);
	if(fac){
		fac->CopyTo(evt->mcvtxhits);
		evt->am_owner = false;
	}
		
	return NOERROR;
}

