void tof700_tracks_aclic(int runnumber = 1700)
{
  gROOT->ProcessLine(".x $VMCWORKDIR/macro/run/bmnloadlibs.C");
  gSystem->SetIncludePath(" -Wno-overloaded-virtual -I$SIMPATH/include -I$SIMPATH/include/root -I$VMCWORKDIR/base/field -I$VMCWORKDIR/base/steer -I$VMCWORKDIR/base/sim -I$VMCWORKDIR/base/event -I$VMCWORKDIR/base/source -I$VMCWORKDIR/geobase -I$VMCWORKDIR/parbase -I$VMCWORKDIR/fairtools -I$VMCWORKDIR/bmndata -I$VMCWORKDIR/globaltracking -I$VMCWORKDIR/gem -I$VMCWORKDIR/silicon -I$VMCWORKDIR/uni_db -I$VMCWORKDIR/uni_db/db_classes");
  gROOT->ProcessLine(".L recoRunDCH_VP.C++");
  recoRunDCH_VP(runnumber);
}
