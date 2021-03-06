/**
 * \file BmnTrackingQaReport.cxx
 * \author Semen Lebedev <s.lebedev@gsi.de> - Original author. First version of code for CBM experiment.
 * \author Sergey Merts <Sergey.Merts@gmail.com> - Modification for BMN experiment.
 * \date 2011-2014
 */
#include "BmnTrackingQaReport.h"
#include "report/BmnReportElement.h"
#include "report/BmnHistManager.h"
#include "report/BmnDrawHist.h"
#include "BmnUtils.h"
#include "TH1.h"
#include "TF1.h"
#include "map"
#include "TCanvas.h"
#include "TLine.h"
#include "BmnTrackingQa.h"
#include <vector>
#include <set>
#include "TString.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include <fstream>

const Float_t pMax = 5.0;

using namespace std;
using namespace lit;

BmnTrackingQaReport::BmnTrackingQaReport() :
BmnSimulationReport(),
fGlobalTrackVariants() {
    SetReportName("tracking_qa");
}

BmnTrackingQaReport::BmnTrackingQaReport(vector<string> header) :
BmnSimulationReport(),
fGlobalTrackVariants(),
fHeader(header) {
    SetReportName("tracking_qa");
}

BmnTrackingQaReport::~BmnTrackingQaReport() {
}

void BmnTrackingQaReport::Create() {
    Out().precision(3);
    Out() << R()->DocumentBegin();
    Out() << R()->Title(0, GetTitle());
    Out() << PrintEventInfo();
    Out() << "<hr>" << endl;
    PrintCanvases();
    Out() << R()->DocumentEnd();
}

string BmnTrackingQaReport::PrintEventInfo() {
    Out() << "<h2>Event generator: QGSM</h2>" << endl; //FIXME!
    Out() << "<h2>Energy: 4 GeV/n</h2>" << endl; //FIXME!
    if (GetOnlyPrimes()) Out() << "<h2>Results only for primaries presented</h2>" << endl;
    Out() << "<h2>Number of events: " << HM()->H1("hen_EventNo_TrackingQa")->GetEntries() << "</h2>" << endl;
    Out() << "<h2>Mean multiplicity: " << HM()->H1("Multiplicity")->GetMean() << "</h2>" << endl;
    Out() << "<hr>" << endl;
    Out() << "<h3><font color=\"red\">Reconstructable</font> MC-track:</h3>" << "Monte Carlo track with at least <font color=\"red\">4</font> Monte Carlo points in GEM" << endl;
    Out() << "<h3><font color=\"red\">Good</font> track:</h3>" << "Reconstructed track with at least <font color=\"red\">4</font> hits in GEM and <font color=\"red\">60%</font> of them corresponded the same MC-track" << endl;
    Out() << "<h3><font color=\"red\">Clone</font> tracks:</h3>";
    Out() << "Two or more reconstructed tracks with reference to the same MC-track." << endl;
    Out() << "The number of clones is subtracted from number of good tracks before efficiency calculation." << endl;
    Out() << "<hr>" << endl;
    ifstream inTiming;
    inTiming.open("timing.txt");
    if (inTiming.is_open()) {
        TString a1, a2, a3;
        Float_t workTime;
        while (!inTiming.eof()) {
            inTiming >> a1 >> a2 >> a3 >> workTime;
            Out() << "<h3>" << "Average time of " << a1 << " " << a2 << ": " << workTime / HM()->H1("hen_EventNo_TrackingQa")->GetEntries() * 1000.0 << " ms/event " << "</h3>" << endl;
        }
    }
    inTiming.close();
    return "<hr>";
}

void BmnTrackingQaReport::Draw() {
    DrawEventsInfo("Distribution of impact parameter and multiplicity");
    SetDefaultDrawStyle();

    TString pNamesIn[5] = {"Sim_vs_P_gem", "Rec_vs_P_gem", "Well_vs_P_gem", "Ghost_vs_P_gem", "Split_vs_P_gem"};
    TString pNamesOut[3] = {"Eff_vs_P_gem", "Fake_vs_P_gem", "SplitEff_vs_P_gem"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs P_sim per event for GEM TRACKS", pNamesIn, pNamesOut);
    
    TString pNamesInWide[5] = {"Sim_vs_P_gem_Wide", "Rec_vs_P_gem_Wide", "Well_vs_P_gem_Wide", "Ghost_vs_P_gem_Wide", "Split_vs_P_gem_Wide"};
    TString pNamesOutWide[3] = {"Eff_vs_P_gem_Wide", "Fake_vs_P_gem_Wide", "SplitEff_vs_P_gem_Wide"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs P_sim per event for GEM TRACKS in wide momentum range", pNamesIn, pNamesOut);

    TString pNamesInGlob[5] = {"Sim_vs_P_glob", "Rec_vs_P_glob", "Well_vs_P_glob", "Ghost_vs_P_glob", "Split_vs_P_glob"};
    TString pNamesOutGlob[3] = {"Eff_vs_P_glob", "Fake_vs_P_glob", "SplitEff_vs_P_glob"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs P_sim per event for GLOBAL TRACKS", pNamesInGlob, pNamesOutGlob);

    TString etaNamesIn[5] = {"Sim_vs_Eta_gem", "Rec_vs_Eta_gem", "Well_vs_Eta_gem", "Ghost_vs_Eta_gem", "Split_vs_Eta_gem"};
    TString etaNamesOut[3] = {"Eff_vs_Eta_gem", "Fake_vs_Eta_gem", "SplitEff_vs_Eta_gem"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs Pseudorapidity per event for GEM TRACKS", etaNamesIn, etaNamesOut);

    TString etaNamesInGlob[5] = {"Sim_vs_Eta_glob", "Rec_vs_Eta_glob", "Well_vs_Eta_glob", "Ghost_vs_Eta_glob", "Split_vs_Eta_glob"};
    TString etaNamesOutGlob[3] = {"Eff_vs_Eta_glob", "Fake_vs_Eta_glob", "SplitEff_vs_Eta_glob"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs Pseudorapidity per event for GLOBAL TRACKS", etaNamesInGlob, etaNamesOutGlob);

    TString thetaNamesIn[5] = {"Sim_vs_Theta_gem", "Rec_vs_Theta_gem", "Well_vs_Theta_gem", "Ghost_vs_Theta_gem", "Split_vs_Theta_gem"};
    TString thetaNamesOut[3] = {"Eff_vs_Theta_gem", "Fake_vs_Theta_gem", "SplitEff_vs_Theta_gem"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs theta per event for GEM TRACKS", thetaNamesIn, thetaNamesOut);

    TString thetaNamesInGlob[5] = {"Sim_vs_Theta_glob", "Rec_vs_Theta_glob", "Well_vs_Theta_glob", "Ghost_vs_Theta_glob", "Split_vs_Theta_glob"};
    TString thetaNamesOutGlob[3] = {"Eff_vs_Theta_glob", "Fake_vs_Theta_glob", "SplitEff_vs_Theta_glob"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs theta per event for GLOBAL TRACKS", thetaNamesInGlob, thetaNamesOutGlob);

    TString nhNamesIn[5] = {"Sim_vs_Nh_gem", "Rec_vs_Nh_gem", "Well_vs_Nh_gem", "Ghost_vs_Nh_gem", "Split_vs_Nh_gem"};
    TString nhNamesOut[3] = {"Eff_vs_Nh_gem", "Fake_vs_Nh_gem", "SplitEff_vs_Nh_gem"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs Nh per event for GEM TRACKS", nhNamesIn, nhNamesOut);

    TString nhNamesInGlob[5] = {"Sim_vs_Nh_glob", "Rec_vs_Nh_glob", "Well_vs_Nh_glob", "Ghost_vs_Nh_glob", "Split_vs_Nh_glob"};
    TString nhNamesOutGlob[3] = {"Eff_vs_Nh_glob", "Fake_vs_Nh_glob", "SplitEff_vs_Nh_glob"};
    DrawEffGem("Distribution of MC-tracks, reco-tracks, fakes and clones vs Nh per event for GLOBAL TRACKS", nhNamesInGlob, nhNamesOutGlob);

    DrawNhitsGem("Distribution of GEM RECO-tracks vs number of hits per track");

    DrawTwoH2("Distribution of GEM reconstructable MC-tracks (left) and MC-tracks corresponded to reconstructed tracks (right) vs number of MC-points and Pseudorapidity", "Nh_sim_Eta_sim_gem", "Nh_rec_Eta_rec_gem");
    DrawTwoH2("Distribution of GLOBAL reconstructable MC-tracks (left) and MC-tracks corresponded to reconstructed tracks (right) vs number of MC-points and Pseudorapidity", "Nh_sim_Eta_sim_glob", "Nh_rec_Eta_rec_glob");
    DrawTwoH2("Distribution of GEM reconstructable MC-tracks (left) and MC-tracks corresponded to reconstructed tracks (right) vs number of MC-points and Momentum", "Nh_sim_P_sim_gem", "Nh_rec_P_rec_gem");
    DrawTwoH2("Distribution of GLOBAL reconstructable MC-tracks (left) and MC-tracks corresponded to reconstructed tracks (right) vs number of MC-points and Momentum", "Nh_sim_P_sim_glob", "Nh_rec_P_rec_glob");
    DrawTwoH2("Distribution of GEM reconstructable MC-tracks (left) and MC-tracks corresponded to reconstructed tracks (right) vs number of MC-points and Theta", "Nh_sim_Theta_sim_gem", "Nh_rec_Theta_rec_gem");
    DrawTwoH2("Distribution of GLOBAL reconstructable MC-tracks (left) and MC-tracks corresponded to reconstructed tracks (right) vs number of MC-points and Theta", "Nh_sim_Theta_sim_glob", "Nh_rec_Theta_rec_glob");

    DrawTwoH2("Distribution of MC-tracks and GEM-tracks in Pseudorapidity and Momentum", "EtaP_sim", "EtaP_rec_gem");
    DrawTwoH2("Distribution of MC-tracks and GLOBAL-tracks in Pseudorapidity and Momentum", "EtaP_sim", "EtaP_rec_glob");

    for (Int_t i = 0; i < HM()->H2("EtaP_sim")->GetXaxis()->GetNbins(); ++i) {
        for (Int_t j = 0; j < HM()->H2("EtaP_sim")->GetYaxis()->GetNbins(); ++j) {
            Float_t nom = HM()->H2("EtaP_rec_gem")->GetBinContent(i, j);
            Float_t denom = HM()->H2("EtaP_sim")->GetBinContent(i, j);
            Float_t content = (denom < 0.001) ? 0.0 : nom / denom * 100.0;
            if (content > 100.0) content = 100.0;
            HM()->H2("Eff_vs_EtaP_gem")->SetBinContent(i, j, content);
        }
    }
    for (Int_t i = 0; i < HM()->H2("EtaP_rec_gem")->GetXaxis()->GetNbins(); ++i) {
        for (Int_t j = 0; j < HM()->H2("EtaP_rec_gem")->GetYaxis()->GetNbins(); ++j) {
            Float_t nom = HM()->H2("Clones_vs_EtaP_gem")->GetBinContent(i, j);
            Float_t denom = HM()->H2("EtaP_rec_gem")->GetBinContent(i, j);
            Float_t content = (denom < 0.001) ? 0.0 : nom / denom * 100.0;
            if (content > 100.0) content = 100.0;
            HM()->H2("Clones_vs_EtaP_gem")->SetBinContent(i, j, content);
        }
    }
    for (Int_t i = 0; i < HM()->H2("EtaP_rec_gem")->GetXaxis()->GetNbins(); ++i) {
        for (Int_t j = 0; j < HM()->H2("EtaP_rec_gem")->GetYaxis()->GetNbins(); ++j) {
            Float_t nom = HM()->H2("Fakes_vs_EtaP_gem")->GetBinContent(i, j);
            Float_t denom = HM()->H2("EtaP_rec_gem")->GetBinContent(i, j);
            Float_t content = (denom < 0.001) ? 0.0 : nom / denom * 100.0;
            if (content > 100.0) content = 100.0;
            HM()->H2("Fakes_vs_EtaP_gem")->SetBinContent(i, j, content);
        }
    }
    for (Int_t i = 0; i < HM()->H2("EtaP_sim")->GetXaxis()->GetNbins(); ++i) {
        for (Int_t j = 0; j < HM()->H2("EtaP_sim")->GetYaxis()->GetNbins(); ++j) {
            Float_t nom = HM()->H2("EtaP_rec_glob")->GetBinContent(i, j);
            Float_t denom = HM()->H2("EtaP_sim")->GetBinContent(i, j);
            Float_t content = (denom < 0.001) ? 0.0 : nom / denom * 100.0;
            if (content > 100.0) content = 100.0;
            HM()->H2("Eff_vs_EtaP_glob")->SetBinContent(i, j, content);
        }
    }
    for (Int_t i = 0; i < HM()->H2("EtaP_rec_glob")->GetXaxis()->GetNbins(); ++i) {
        for (Int_t j = 0; j < HM()->H2("EtaP_rec_glob")->GetYaxis()->GetNbins(); ++j) {
            Float_t nom = HM()->H2("Clones_vs_EtaP_glob")->GetBinContent(i, j);
            Float_t denom = HM()->H2("EtaP_rec_glob")->GetBinContent(i, j);
            Float_t content = (denom < 0.001) ? 0.0 : nom / denom * 100.0;
            if (content > 100.0) content = 100.0;
            HM()->H2("Clones_vs_EtaP_glob")->SetBinContent(i, j, content);
        }
    }
    for (Int_t i = 0; i < HM()->H2("EtaP_rec_glob")->GetXaxis()->GetNbins(); ++i) {
        for (Int_t j = 0; j < HM()->H2("EtaP_rec_glob")->GetYaxis()->GetNbins(); ++j) {
            Float_t nom = HM()->H2("Fakes_vs_EtaP_glob")->GetBinContent(i, j);
            Float_t denom = HM()->H2("EtaP_rec_glob")->GetBinContent(i, j);
            Float_t content = (denom < 0.001) ? 0.0 : nom / denom * 100.0;
            if (content > 100.0) content = 100.0;
            HM()->H2("Fakes_vs_EtaP_glob")->SetBinContent(i, j, content);
        }
    }

    DrawThreeH2("Distribution of Efficiency, Ghosts and Clones in Pseudorapidity and Momentum GEM", "Eff_vs_EtaP_gem", "Clones_vs_EtaP_gem", "Fakes_vs_EtaP_gem");
    DrawThreeH2("Distribution of Efficiency, Ghosts and Clones in Pseudorapidity and Momentum GLOBAL", "Eff_vs_EtaP_glob", "Clones_vs_EtaP_glob", "Fakes_vs_EtaP_glob");

    DrawTwoH2("Distribution of MC-tracks and GEM-tracks in theta and Momentum", "ThetaP_sim", "ThetaP_rec_gem");
    DrawTwoH2("Distribution of MC-tracks and GLOBAL-tracks in theta and Momentum", "ThetaP_sim", "ThetaP_rec_glob");
    DrawTwoH2("P_reco vs P_mc for GEM", "P_rec_P_sim_gem", "Pt_rec_Pt_sim_gem");
    DrawTwoH2("P_reco vs P_mc for GLOBAL", "P_rec_P_sim_glob", "Pt_rec_Pt_sim_glob");
    DrawOneH2("Pseudorapidity_reco vs Pseudorapidity_mc GEM", "Eta_rec_Eta_sim_gem");
    DrawOneH2("Pseudorapidity_reco vs Pseudorapidity_mc GLOBAL", "Eta_rec_Eta_sim_glob");
    DrawTwoH2("Tx_reco vs Tx_mc (left) and Ty_reco vs Ty_mc (right) for GEM", "Tx_rec_Tx_sim_gem", "Ty_rec_Ty_sim_gem");
    DrawTwoH2("Tx_reco vs Tx_mc (left) and Ty_reco vs Ty_mc (right) for GLOBAL", "Tx_rec_Tx_sim_glob", "Ty_rec_Ty_sim_glob");
    DrawThreeH2("Reco vs MC for X-, Y- and Z-component of Momentum for GEM", "Px_rec_Px_sim_gem", "Py_rec_Py_sim_gem", "Pz_rec_Pz_sim_gem");
    DrawThreeH2("Reco vs MC for X-, Y- and Z-component of Momentum for GLOBAL", "Px_rec_Px_sim_glob", "Py_rec_Py_sim_glob", "Pz_rec_Pz_sim_glob");
    DrawMomResGem("Momentum resolution for GEM", "momRes_2D_gem", "momRes_1D_gem", "momMean_1D_gem", "momRes_Mean_gem");
    DrawMomResGem("Momentum resolution for GLOBAL", "momRes_2D_glob", "momRes_1D_glob", "momMean_1D_glob", "momRes_Mean_glob");
    DrawTwoH2("Tracks quality distributions GEM", "MomRes_vs_Chi2_gem", "Mom_vs_Chi2_gem");
    DrawTwoH2("Tracks quality distributions GLOBAL", "MomRes_vs_Chi2_glob", "Mom_vs_Chi2_glob");
    DrawTwoH2("Momentum resolution and momentum vs. length of tracks GEM", "MomRes_vs_Length_gem", "Mom_vs_Length_gem");
    DrawTwoH2("Momentum resolution and momentum vs. length of tracks GLOBAL", "MomRes_vs_Length_glob", "Mom_vs_Length_glob");
    DrawOneH2("Momentum resolution vs. Number of hits GEM", "MomRes_vs_nHits_gem");
    DrawOneH2("Momentum resolution vs. Number of hits GLOBAL", "MomRes_vs_nHits_glob");
    DrawOneH2("Momentum resolution vs. theta GEM", "MomRes_vs_Theta_gem");
    DrawOneH2("Momentum resolution vs. theta GLOBAL", "MomRes_vs_Theta_glob");
    DrawTwoH1("Chi-square and length distributions GEM", "Chi2_gem", "Length_gem", "");
    DrawTwoH1("Chi-square and length distributions GLOBAL", "Chi2_glob", "Length_glob", "");

    DrawHitRes("X");
    DrawHitRes("Y");

    TString namesResPullsF_gem[15] = {"ResX_f_gem", "ResY_f_gem", "ResTx_f_gem", "ResTy_f_gem", "ResQp_f_gem", "ErrX_f_gem", "ErrY_f_gem", "ErrTx_f_gem", "ErrTy_f_gem", "ErrQp_f_gem", "PullX_f_gem", "PullY_f_gem", "PullTx_f_gem", "PullTy_f_gem", "PullQp_f_gem"};
    TString namesResPullsL_gem[15] = {"ResX_l_gem", "ResY_l_gem", "ResTx_l_gem", "ResTy_l_gem", "ResQp_l_gem", "ErrX_l_gem", "ErrY_l_gem", "ErrTx_l_gem", "ErrTy_l_gem", "ErrQp_l_gem", "PullX_l_gem", "PullY_l_gem", "PullTx_l_gem", "PullTy_l_gem", "PullQp_l_gem"};

    DrawResAndPull("Residuals and Pulls for first parameters GEM", namesResPullsF_gem);
    DrawResAndPull("Residuals and Pulls for last parameters GEM", namesResPullsL_gem);

    TString namesResPullsF_glob[15] = {"ResX_f_glob", "ResY_f_glob", "ResTx_f_glob", "ResTy_f_glob", "ResQp_f_glob", "ErrX_f_glob", "ErrY_f_glob", "ErrTx_f_glob", "ErrTy_f_glob", "ErrQp_f_glob", "PullX_f_glob", "PullY_f_glob", "PullTx_f_glob", "PullTy_f_glob", "PullQp_f_glob"};
    TString namesResPullsL_glob[15] = {"ResX_l_glob", "ResY_l_glob", "ResTx_l_glob", "ResTy_l_glob", "ResQp_l_glob", "ErrX_l_glob", "ErrY_l_glob", "ErrTx_l_glob", "ErrTy_l_glob", "ErrQp_l_glob", "PullX_l_glob", "PullY_l_glob", "PullTx_l_glob", "PullTy_l_glob", "PullQp_l_glob"};

    DrawResAndPull("Residuals and Pulls for first parameters GLOBAL", namesResPullsF_glob);
    DrawResAndPull("Residuals and Pulls for last parameters GLOBAL", namesResPullsL_glob);

    TString namesParF[5] = {"X_f_gem", "Y_f_gem", "Tx_f_gem", "Ty_f_gem", "Qp_f_gem"};
    TString namesParL[5] = {"X_l_gem", "Y_l_gem", "Tx_l_gem", "Ty_l_gem", "Qp_l_gem"};
    DrawPar("First parameters GEM", namesParF);
    DrawPar("Last parameters GEM", namesParL);

    TString namesParF_glob[5] = {"X_f_glob", "Y_f_glob", "Tx_f_glob", "Ty_f_glob", "Qp_f_glob"};
    TString namesParL_glob[5] = {"X_l_glob", "Y_l_glob", "Tx_l_glob", "Ty_l_glob", "Qp_l_glob"};
    DrawPar("First parameters GLOBAL", namesParF_glob);
    DrawPar("Last parameters GLOBAL", namesParL_glob);

    DrawThreeH1("Vertex", "VertX_gem", "VertY_gem", "VertZ_gem");
    DrawVertResGem("Vertex resolution", "VertResX_gem", "VertResY_gem", "VertResZ_gem");
}

void BmnTrackingQaReport::DrawEffGem(const TString canvasName, TString* inNames, TString* outNames) {

    TString sim = inNames[0];
    TString rec = inNames[1];
    TString well = inNames[2];
    TString ghost = inNames[3];
    TString split = inNames[4];

    TString eff = outNames[0];
    TString fake = outNames[1];
    TString clon = outNames[2];

    Int_t nofEvents = HM()->H1("hen_EventNo_TrackingQa")->GetEntries();
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1200, 600);
    canvas->SetGrid();
    canvas->Divide(2, 1);
    canvas->cd(1);
    HM()->H1(sim)->Sumw2();
    //    HM()->H1(sim)->Scale(1. / nofEvents);
    HM()->H1(rec)->Sumw2();
    //    HM()->H1(rec)->Scale(1. / nofEvents);
    HM()->H1(well)->Sumw2();
    //    HM()->H1(well)->Scale(1. / nofEvents);
    HM()->H1(ghost)->Sumw2();
    //    HM()->H1(ghost)->Scale(1. / nofEvents);
    HM()->H1(split)->Sumw2();
    //    HM()->H1(split)->Scale(1. / nofEvents);

    HM()->H1(sim)->SetMinimum(0.0);
    HM()->H1(well)->SetMinimum(0.0);
    HM()->H1(ghost)->SetMinimum(0.0);
    HM()->H1(split)->SetMinimum(0.0);

    vector<TH1*> histos1;
    histos1.push_back(HM()->H1(sim));
    histos1.push_back(HM()->H1(well));
    histos1.push_back(HM()->H1(ghost));
    histos1.push_back(HM()->H1(split));
    vector<string> labels1;
    labels1.push_back("MC tracks");
    labels1.push_back("Good tracks");
    labels1.push_back("Ghost tracks");
    labels1.push_back("Clones");
    DrawH1(histos1, labels1, kLinear, kLinear, true, 0.5, 0.8, 1.0, 0.99, "PE1", kFALSE);

    canvas->cd(2);
    vector<string> labels2;
    labels2.push_back("Efficiency");
    labels2.push_back("Ghosts");
    labels2.push_back("Clones");

    HM()->H1(eff)->Divide(HM()->H1(well), HM()->H1(sim), 1., 1., "B");
    HM()->H1(eff)->Scale(100.0);
    HM()->H1(fake)->Divide(HM()->H1(ghost), HM()->H1(rec), 1., 1., "B");
    HM()->H1(fake)->Scale(100.0);
    HM()->H1(clon)->Divide(HM()->H1(split), HM()->H1(rec), 1., 1., "B");
    HM()->H1(clon)->Scale(100.0);

    // Boundary checking.
    // These cases shouldn't happen, but they happen sometimes...
    for (Int_t i = 0; i < HM()->H1(eff)->GetNbinsX(); ++i) {
        if (HM()->H1(eff)->GetBinContent(i) > 100.0) {
            HM()->H1(eff)->SetBinContent(i, 100.0);
            HM()->H1(eff)->SetBinError(i, 0.0);
        }
        if (HM()->H1(fake)->GetBinContent(i) > 100.0) {
            HM()->H1(fake)->SetBinContent(i, 100.0);
            HM()->H1(fake)->SetBinError(i, 0.0);
        }
        if (HM()->H1(clon)->GetBinContent(i) > 100.0) {
            HM()->H1(clon)->SetBinContent(i, 100.0);
            HM()->H1(clon)->SetBinError(i, 0.0);
        }
        if (HM()->H1(eff)->GetBinContent(i) < 0.0) {
            HM()->H1(eff)->SetBinContent(i, 0.0);
            HM()->H1(eff)->SetBinError(i, 0.0);
        }
        if (HM()->H1(fake)->GetBinContent(i) < 0.0) {
            HM()->H1(fake)->SetBinContent(i, 0.0);
            HM()->H1(fake)->SetBinError(i, 0.0);
        }
        if (HM()->H1(clon)->GetBinContent(i) < 0.0) {
            HM()->H1(clon)->SetBinContent(i, 0.0);
            HM()->H1(clon)->SetBinError(i, 0.0);
        }
    }

    HM()->H1(eff)->SetMaximum(100.0);
    HM()->H1(fake)->SetMaximum(100.0);
    HM()->H1(clon)->SetMaximum(100.0);
    HM()->H1(eff)->SetMinimum(0.0);
    HM()->H1(fake)->SetMinimum(0.0);
    HM()->H1(clon)->SetMinimum(0.0);

    vector<TH1*> histos2;
    histos2.push_back(HM()->H1(eff));
    histos2.push_back(HM()->H1(fake));
    histos2.push_back(HM()->H1(clon));
    DrawH1(histos2, labels2, kLinear, kLinear, true, 0.5, 0.9, 1.0, 0.99, "PE1X0", kFALSE);
}

void BmnTrackingQaReport::DrawNhitsGem(const string& canvasName) {
    Int_t nofEvents = HM()->H1("hen_EventNo_TrackingQa")->GetEntries();
    TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 600, 600);
    canvas->SetGrid();
    HM()->H1("Well_vs_Nh_gem")->Sumw2();
    HM()->H1("Well_vs_Nh_gem")->Scale(1. / nofEvents);
    HM()->H1("Ghost_vs_Nh_gem")->Sumw2();
    HM()->H1("Ghost_vs_Nh_gem")->Scale(1. / nofEvents);
    vector<TH1*> histos1;
    histos1.push_back(HM()->H1("Well_vs_Nh_gem"));
    histos1.push_back(HM()->H1("Ghost_vs_Nh_gem"));
    vector<string> labels1;
    labels1.push_back("Good tracks");
    labels1.push_back("Ghosts");
    DrawH1(histos1, labels1, kLinear, kLinear, true, 0.5, 0.9, 1.0, 0.99, "PE1", kFALSE);
}

void BmnTrackingQaReport::DrawEventsInfo(const string& canvasName) {
    TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 1500, 500);
    canvas->SetGrid();
    canvas->Divide(3, 1);
    canvas->cd(1);
    DrawH1(HM()->H1("Impact parameter"), kLinear, kLinear, "", kRed, 2, 1, 1.1, 20, 33);
    canvas->cd(2);
    DrawH1(HM()->H1("Multiplicity"), kLinear, kLinear, "", kRed, 2, 1, 1.1, 20, 33);
    canvas->cd(3);
    DrawH2(HM()->H2("Impact_Mult"), kLinear, kLinear, kLinear, "colz");
}

void BmnTrackingQaReport::DrawMomResGem(const string& canvasName, TString name2d, TString nameSigma, TString nameMean, TString nameAver) {
    TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 1600, 400);
    canvas->SetGrid();
    canvas->Divide(4, 1);
    canvas->cd(1);
    DrawH2(HM()->H2(name2d), kLinear, kLinear, kLinear, "colz");

    canvas->cd(2);
    FillAndFitSlice(nameSigma, nameMean, name2d);
    HM()->H1(nameSigma)->SetMaximum(10.0);
    HM()->H1(nameSigma)->SetMinimum(0.0);
    DrawH1(HM()->H1(nameSigma), kLinear, kLinear, "PE1", kRed, 0.7, 0.75, 1.1, 20);

    canvas->cd(3);
    HM()->H1(nameMean)->SetMaximum(10.0);
    HM()->H1(nameMean)->SetMinimum(-10.0);
    DrawH1(HM()->H1(nameMean), kLinear, kLinear, "PE1", kRed, 0.7, 0.75, 1.1, 20);

    canvas->cd(4);
    TH1D* projY = HM()->H2(name2d)->ProjectionY("tmp1");
    for (Int_t iBin = 0; iBin < projY->GetNbinsX(); ++iBin) {
        HM()->H1(nameAver)->SetBinContent(iBin, projY->GetBinContent(iBin));
    }

    DrawH1(HM()->H1(nameAver), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
    HM()->H1(nameAver)->Fit("gaus", "RQWW", "", -10, 10);
    HM()->H1(nameAver)->SetMaximum(HM()->H1(nameAver)->GetMaximum() * 1.05);
    TF1 *fit = HM()->H1(nameAver)->GetFunction("gaus");
    TPaveStats* ps = new TPaveStats(3.0, HM()->H1(nameAver)->GetMaximum() / 1.5, 10.0, HM()->H1(nameAver)->GetMaximum());
    ps->SetFillColor(0);
    ps->SetShadowColor(0);
    ps->AddText(Form("#mu = %2.2f", fit->GetParameter(1)));
    ps->AddText(Form("#sigma = %2.2f", fit->GetParameter(2)));
    ps->Draw();
}

void BmnTrackingQaReport::DrawResAndPull(const TString canvasName, TString* inNames) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1500, 1000);
    canvas->SetGrid();
    canvas->Divide(5, 3);

    for (Int_t i = 0; i < 15; ++i) {
        canvas->cd(i + 1);
        HM()->H1(inNames[i])->Fit("gaus", "RQWW", "", -4, 4);
        DrawH1(HM()->H1(inNames[i]), kLinear, kLog, "", kBlue, 0.7, 0.75, 1.1, 20);
        //if (i > 4) {
        TF1 *fit = HM()->H1(inNames[i])->GetFunction("gaus");
        Float_t xMax = HM()->H1(inNames[i])->GetXaxis()->GetXmax();
        Float_t yMax = HM()->H1(inNames[i])->GetMaximum();
        TPaveStats* ps = new TPaveStats(xMax / 2, yMax / 10, xMax, yMax);
        ps->SetFillColor(0);
        ps->SetShadowColor(0);
        ps->AddText(Form("#mu = %2.2f", fit->GetParameter(1)));
        ps->AddText(Form("#sigma = %2.2f", fit->GetParameter(2)));
        ps->Draw();
        //}
    }
}

void BmnTrackingQaReport::DrawPar(const TString canvasName, TString* inNames) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1500, 300);
    canvas->SetGrid();
    canvas->Divide(5, 1);

    for (Int_t i = 0; i < 5; ++i) {
        canvas->cd(i + 1);
        DrawH1(HM()->H1(inNames[i]), kLinear, kLog, "", kRed, 0.7, 0.75, 1.1, 20);
    }
}

void BmnTrackingQaReport::FillAndFitSlice(TString nameSigma, TString nameMean, TString name2d) {
    Int_t nBins = HM()->H1(nameSigma)->GetXaxis()->GetNbins();
    Int_t momResStep = HM()->H2(name2d)->GetNbinsX() / nBins;
    Int_t bin = 0;
    for (Int_t iBin = 0; iBin < HM()->H2(name2d)->GetNbinsX(); iBin += momResStep) {
        TH1D* proj = HM()->H2(name2d)->ProjectionY("tmp", iBin, iBin + (momResStep - 1));
        proj->Fit("gaus", "SQRww", "", -5.0, 5.0);
        TF1 *fit = proj->GetFunction("gaus");
        Float_t sigma = fit->GetParameter(2);
        Float_t sigmaError = fit->GetParError(2);
        Float_t mean = fit->GetParameter(1);
        Float_t meanError = fit->GetParError(1);
        HM()->H1(nameSigma)->SetBinContent(bin, sigma);
        HM()->H1(nameSigma)->SetBinError(bin, sigmaError);
        HM()->H1(nameMean)->SetBinContent(bin, mean);
        HM()->H1(nameMean)->SetBinError(bin, meanError);
        bin++;
    }
}

void BmnTrackingQaReport::DrawVertResGem(const string& canvasName, TString name1dX, TString name1dY, TString name1dZ) {
    TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 1500, 500);
    canvas->SetGrid();
    canvas->Divide(3, 1);

    canvas->cd(1);
    HM()->H1(name1dX)->Fit("gaus", "RQWW", "", -1, 1);
    DrawH1(HM()->H1(name1dX), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
    DrawMuSigma(canvas->cd(1), HM()->H1(name1dX));
    canvas->cd(2);
    HM()->H1(name1dY)->Fit("gaus", "RQWW", "", -1, 1);
    DrawH1(HM()->H1(name1dY), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
    DrawMuSigma(canvas->cd(2), HM()->H1(name1dY));
    canvas->cd(3);
    HM()->H1(name1dZ)->Fit("gaus", "RQWW", "", -2, 2);
    DrawH1(HM()->H1(name1dZ), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
    DrawMuSigma(canvas->cd(3), HM()->H1(name1dZ));
}

void BmnTrackingQaReport::DrawMuSigma(TVirtualPad* pad, TH1* h) {
    pad->cd();
    TF1 *fit = h->GetFunction("gaus");
    Float_t xMax = h->GetXaxis()->GetXmax();
    Float_t yMax = h->GetMaximum();
    TPaveStats* ps = new TPaveStats(xMax / 2, yMax / 2, xMax, yMax);
    ps->SetFillColor(0);
    ps->SetShadowColor(0);
    ps->AddText(Form("#mu = %2.3f", fit->GetParameter(1)));
    ps->AddText(Form("#sigma = %2.3f", fit->GetParameter(2)));
    ps->Draw();
}

void BmnTrackingQaReport::DrawOneH1(const TString canvasName, const TString name1, const TString drawOpt) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 500, 500);
    canvas->SetGrid();
    DrawH1(HM()->H1(name1), kLinear, kLog, drawOpt.Data(), kRed, 0.7, 0.75, 1.1, 20);
}

void BmnTrackingQaReport::DrawTwoH1(const TString canvasName, const TString name1, const TString name2, const TString drawOpt) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1000, 500);
    canvas->SetGrid();
    canvas->Divide(2, 1);
    canvas->cd(1);
    DrawH1(HM()->H1(name1), kLinear, kLog, drawOpt.Data(), kRed, 0.7, 0.75, 1.1, 20);
    canvas->cd(2);
    DrawH1(HM()->H1(name2), kLinear, kLinear, drawOpt.Data(), kRed, 0.7, 0.75, 1.1, 20);
}

void BmnTrackingQaReport::DrawOneH2(const TString canvasName, const TString name1) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 500, 500);
    canvas->SetGrid();
    DrawH2(HM()->H2(name1), kLinear, kLinear, kLinear, "colz");
}

void BmnTrackingQaReport::DrawHitRes(TString axis) {
    TString name = Form("%s Residuals between hits and tracks", axis.Data());
    TCanvas* canvas = CreateCanvas(name.Data(), name.Data(), 1200, 800);
    canvas->SetGrid();
    canvas->Divide(3, 2);
    for (Int_t i = 0; i < 6; ++i) {
        canvas->cd(i + 1);
        DrawH1(HM()->H1(Form("Res%s_%dst", axis.Data(), i)), kLinear, kLog, "", kRed, 0.7, 0.75, 1.1, 20);
    }
}

void BmnTrackingQaReport::DrawTwoH2(const TString canvasName, const TString name1, const TString name2) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1000, 500);
    canvas->SetGrid();
    canvas->Divide(2, 1);
    canvas->cd(1);
    DrawH2(HM()->H2(name1), kLinear, kLinear, kLinear, "colz");
    canvas->cd(2);
    DrawH2(HM()->H2(name2), kLinear, kLinear, kLinear, "colz");
}

void BmnTrackingQaReport::DrawThreeH2(const TString canvasName, const TString name1, const TString name2, const TString name3) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1500, 500);
    canvas->SetGrid();
    canvas->Divide(3, 1);
    canvas->cd(1);
    DrawH2(HM()->H2(name1), kLinear, kLinear, kLinear, "colz");
    canvas->cd(2);
    DrawH2(HM()->H2(name2), kLinear, kLinear, kLinear, "colz");
    canvas->cd(3);
    DrawH2(HM()->H2(name3), kLinear, kLinear, kLinear, "colz");
}

void BmnTrackingQaReport::DrawThreeH1(const TString canvasName, const TString name1, const TString name2, const TString name3) {
    TCanvas* canvas = CreateCanvas(canvasName.Data(), canvasName.Data(), 1500, 500);
    canvas->SetGrid();
    canvas->Divide(3, 1);
    canvas->cd(1);
    DrawH1(HM()->H1(name1), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
    canvas->cd(2);
    DrawH1(HM()->H1(name2), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
    canvas->cd(3);
    DrawH1(HM()->H1(name3), kLinear, kLinear, "", kBlue, 0.7, 0.75, 1.1, 20);
}

ClassImp(BmnTrackingQaReport)
