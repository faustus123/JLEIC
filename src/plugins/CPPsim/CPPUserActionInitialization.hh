
#ifndef _CPPUserActionInitialization_
#define _CPPUserActionInitialization_

#include "G4VUserActionInitialization.hh"

#include "CPPPrimaryGeneratorAction.hh"
//#include "CPPHDDMout.hh"


class CPPUserActionInitialization:public G4VUserActionInitialization
{
public:

	CPPUserActionInitialization(){}
	~CPPUserActionInitialization(){}

	virtual void Build() const{
		SetUserAction(new CPPPrimaryGeneratorAction);
		//SetUserAction(new CPPHDDMout);		
	}

};



#endif // _CPPUserActionInitialization_
