// $Id$
//
//    File: EKinematicData.hh
// Created: Tue Dec 27 09:48:51 EST 2016
// Creator: davidl (on Darwin harriet 15.6.0 i386)
//

#ifndef _EKinematicData_
#define _EKinematicData_

#include <TLorentzVector.h>
#include <TVector3.h>

#include <JANA/jerror.h>
#include <EParticle.h>

/// A particle with kinematic data attached.
///
/// This is used for both generated and reconstructed particles.
/// The EParticle base class provides general particle properties
/// and this adds the kinematic data. This is used as a base class
/// for EGeneratedParticle (and possibly others in the future.)

class EKinematicData:public EParticle, public jana::JObject{
	public:
		EKinematicData(EParticle::Type ptype=EParticle::Unknown):EParticle(ptype){}
		virtual ~EKinematicData(){}

		TLorentzVector& p4(void){ return kMomentum; }
		TLorentzVector& x4(void){ return kPosition; }
		      TVector3  p3(void){ return kMomentum.Vect(); }
		      TVector3  x3(void){ return kPosition.Vect(); }
				  int32_t ptype(void) { return (int32_t)type; }
				  int32_t PDGid(void){ return PDGtype(type); }


	protected:

		TLorentzVector kMomentum;
		TLorentzVector kPosition;

};

#endif // _EKinematicData_
