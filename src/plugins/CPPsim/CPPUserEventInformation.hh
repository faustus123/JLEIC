

#ifndef _CPPUSEREVENTINFORMATION_
#define _CPPUSEREVENTINFORMATION_

#include "G4UImanager.hh"
#include "G4VUserEventInformation.hh"

//#include <HDDM/hddm_s.hpp>

using namespace std;

class CPPUserEventInformation: public G4VUserEventInformation
{
public:
	CPPUserEventInformation(void):keep_event(true),Nprimaries(0){
//		if(hddmevent){
			// Count number of primaries already in event
			// const hddm_s::ProductList &products = hddmevent->getProducts();
			// Nprimaries = products.size();

//			if(hddm_s->physicsEvents && hddm_s->physicsEvents->mult>0){
//				if(hddm_s->physicsEvents->in[0].reactions){
//					s_Reaction_t *reac = &hddm_s->physicsEvents->in[0].reactions->in[0];
//					for(uint32_t i=0; i<reac->vertices->mult; i++){
//						Nprimaries += reac->vertices->in[i].products->mult;
//					}
//				}
//			}
//		}
	}
	CPPUserEventInformation(int geanttype, G4ThreeVector &pos, G4ThreeVector &mom):keep_event(true),Nprimaries(1){
		vertex_t v(geanttype, pos, mom);
		vertices.push_back(v);
	}
	~CPPUserEventInformation(){
		// Free memory from hddm event (if any)
//		if(hddmevent != NULL) delete hddmevent;
	}

	void Print() const {
		G4cout << "CPPUserEventInformation: nothing yet" << endl;
	}

	bool keep_event;

	// These hold info. on particles whose parameters are generated within CPPsim
	// This complicated structure matches what is already in HDDM and will allow
	// more complicated things like pi0 decay by GEANT4 or Lambda decay to be
	// captured here (eventually).
	class partdef_t{
		public:
			partdef_t(int mygeanttype, G4ThreeVector &mymom, int myid=1, int myparentid=0, int mymech=0):geanttype(mygeanttype),mom(mymom),id(myid),parentid(myparentid),mech(mymech){}

			int geanttype;
			G4ThreeVector mom;
			int id;
			int parentid;
			int mech;
	};
	class vertex_t{
		public:
			vertex_t(int geanttype, G4ThreeVector &mypos, G4ThreeVector &mom):pos(mypos){ partdefs.push_back( partdef_t(geanttype, mom) ); }

			G4ThreeVector pos;
			vector<partdef_t> partdefs;
	};
	vector<vertex_t> vertices;
	int Nprimaries;
};

#endif // _CPPUSEREVENTINFORMATION_
