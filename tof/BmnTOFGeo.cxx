//--------------------------------------------------------------------------------------------------------------------------------------
#include<iostream>

#include "BmnTOFGeo.h"
#include "FairGeoNode.h"

ClassImp(BmnTOFGeo)
//--------------------------------------------------------------------------------------------------------------------------------------
BmnTOFGeo::BmnTOFGeo() 
{
	fName = "TOFB1";
	maxSectors = 0;
	maxModules = 200;
}
//--------------------------------------------------------------------------------------------------------------------------------------
const char* BmnTOFGeo::getModuleName(Int_t m) 
{
	sprintf(modName, "TOFB%i", m+1);

return modName;
}
//--------------------------------------------------------------------------------------------------------------------------------------
const char* BmnTOFGeo::getEleName(Int_t m) 
{
	sprintf(eleName, "TOFB%i", m+1);
  
return eleName;
}
//--------------------------------------------------------------------------------------------------------------------------------------
