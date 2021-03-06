#include <string>
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLine.h"
#include "TLatex.h"
#include "TGaxis.h"
#include "TLegend.h"
#include "TRandom3.h"
#include "TStyle.h"
#include "../Utility/functions.h"
#include "../Utility/draw.h"
#include "../Utility/StSpinAlignmentCons.h"

using namespace std;

std::pair<double, double> const momentumRange(0.2,5.0);
int const pT_low = 1;
int const pT_high = 14;
int const MarkerColorQA   = kAzure-2;
int const MarkerStyleQA   = 24;
int const MarkerColorRP   = kGray+2;
int const MarkerStyleRP   = 22;
int const MarkerColorGaus = kViolet-2;
int const MarkerStyleGaus = 23;
int const MarkerColorEP   = kRed;
int const MarkerStyleEP   = 29;

void plotMcPhiResCorr(int energy = 6)
{
  gStyle->SetOptDate(0);
  TGaxis::SetMaxDigits(4);
  gRandom->SetSeed();
  int const BinRho = floor(60 * gRandom->Rndm());
  // int const BinRho = 40; 
  float const rhoInPut = 0.01*BinRho;

  string InPutFile = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/Data/Phi_v2_1040.root",vmsa::mBeamEnergy[energy].c_str());
  TFile *File_InPut = TFile::Open(InPutFile.c_str());
  TGraphAsymmErrors *g_v2 = (TGraphAsymmErrors*)File_InPut->Get("g_v2");
  TF1 *f_v2 = new TF1("f_v2",v2_pT_FitFunc,vmsa::ptMin,vmsa::ptMax,5);
  f_v2->FixParameter(0,2);
  f_v2->SetParameter(1,0.1);
  f_v2->SetParameter(2,0.1);
  f_v2->SetParameter(3,0.1);
  f_v2->SetParameter(4,0.1);
  f_v2->SetLineColor(kGray+2);
  f_v2->SetLineWidth(2);
  f_v2->SetLineStyle(2);
  g_v2->Fit(f_v2,"N");

  string InPutHist = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/McPhiResCorr.root",vmsa::mBeamEnergy[energy].c_str());
  // string InPutHist = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/McPhiResCorr_40.root",vmsa::mBeamEnergy[energy].c_str());
  TFile *File_Hist = TFile::Open(InPutHist.c_str());
  string HistTracks = Form("h_Tracks_%d",BinRho);
  TH3F *h_Tracks = (TH3F*)File_Hist->Get(HistTracks.c_str());
  TH1F *h_TracksProj[3];
  h_TracksProj[0] = (TH1F*)h_Tracks->Project3D("x");
  h_TracksProj[1] = (TH1F*)h_Tracks->Project3D("y");
  h_TracksProj[2] = (TH1F*)h_Tracks->Project3D("z");
  string LabelX[3] = {"p_{T} (GeV/c)","y","#phi-#Psi_{RP}"};

  TCanvas *c_Track = new TCanvas("c_Track","c_Track",10,10,1800,600);
  c_Track->Divide(3,1);
  for(int i_pad = 0; i_pad < 3; ++i_pad)
  {
    c_Track->cd(i_pad+1)->SetLeftMargin(0.15);
    c_Track->cd(i_pad+1)->SetBottomMargin(0.15);
    c_Track->cd(i_pad+1)->SetTicks(1,1);
    c_Track->cd(i_pad+1)->SetGrid(0,0);
    h_TracksProj[i_pad]->SetTitle("");
    h_TracksProj[i_pad]->SetStats(0);
    h_TracksProj[i_pad]->GetXaxis()->SetTitle(LabelX[i_pad].c_str());
    h_TracksProj[i_pad]->GetXaxis()->CenterTitle();
    h_TracksProj[i_pad]->GetXaxis()->SetLabelSize(0.04);
    h_TracksProj[i_pad]->GetXaxis()->SetNdivisions(505);

    h_TracksProj[i_pad]->GetYaxis()->SetTitle("Counts");
    h_TracksProj[i_pad]->GetYaxis()->SetTitleSize(0.04);
    h_TracksProj[i_pad]->GetYaxis()->CenterTitle();
    h_TracksProj[i_pad]->GetYaxis()->SetLabelSize(0.04);
    h_TracksProj[i_pad]->GetYaxis()->SetNdivisions(505);
    h_TracksProj[i_pad]->GetYaxis()->SetRangeUser(0.7*h_TracksProj[i_pad]->GetMinimum(),1.2*h_TracksProj[i_pad]->GetMaximum());
    
    h_TracksProj[i_pad]->SetMarkerStyle(24);
    h_TracksProj[i_pad]->SetMarkerSize(1.1);
    h_TracksProj[i_pad]->SetMarkerColor(2);
    h_TracksProj[i_pad]->Draw("pE");
  }

  TF1 *f_spec = new TF1("f_spec",pTLevy,vmsa::ptMin,vmsa::ptMax,3);
  f_spec->SetParameter(0,h_TracksProj[0]->GetMaximum());
  f_spec->SetParameter(1,10);
  f_spec->SetParameter(2,0.4);
  f_spec->SetLineStyle(2);
  f_spec->SetLineColor(2);
  f_spec->SetLineWidth(2);
  h_TracksProj[0]->GetYaxis()->SetTitle("dN/dp_{T}");
  h_TracksProj[0]->Fit(f_spec,"N");
  c_Track->cd(1);
  f_spec->Draw("l same");

  TF1 *f_pol = new TF1("f_pol","pol0",-1.0,1.0);
  f_pol->SetLineColor(2);
  f_pol->SetLineStyle(2);
  f_pol->SetLineWidth(2);
  h_TracksProj[1]->Fit(f_pol,"N");
  c_Track->cd(2);
  f_pol->Draw("l same");

  TF1 *f_phi = new TF1("f_phi",flow,-1.0*TMath::Pi(),1.0*TMath::Pi(),2);
  f_phi->SetParameter(0,0.1);
  f_phi->SetParameter(1,100);
  f_phi->SetLineColor(2);
  f_phi->SetLineStyle(2);
  f_phi->SetLineWidth(2);
  h_TracksProj[2]->Fit(f_phi,"N");
  c_Track->cd(3);
  f_phi->Draw("l same");

  string HistPsi = Form("h_PsiGaus_%d",BinRho);
  TH1F *h_PsiGaus = (TH1F*)File_Hist->Get(HistPsi.c_str());
  HistPsi = Form("h_PsiEP_%d",BinRho);
  TH1F *h_PsiEP = (TH1F*)File_Hist->Get(HistPsi.c_str());
  TCanvas *c_Psi2 = new TCanvas("c_Psi2","c_Psi2",100,10,800,800);
  c_Psi2->cd()->SetLeftMargin(0.15);
  c_Psi2->cd()->SetBottomMargin(0.15);
  c_Psi2->cd()->SetTicks(1,1);
  c_Psi2->cd()->SetGrid(0,0);
  h_PsiEP->SetTitle("");
  h_PsiEP->SetStats(0);
  h_PsiEP->GetXaxis()->SetLabelSize(0.05);
  h_PsiEP->GetXaxis()->SetTitle("#Psi_{2}-#Psi_{RP}");
  h_PsiEP->GetXaxis()->CenterTitle();
  h_PsiEP->GetXaxis()->SetTitleSize(0.05);
  h_PsiEP->GetXaxis()->SetRangeUser(-TMath::PiOver2(),TMath::PiOver2());
  h_PsiEP->GetXaxis()->SetNdivisions(505);

  h_PsiEP->GetYaxis()->SetLabelSize(0.05);
  h_PsiEP->GetYaxis()->SetTitle("Counts");
  h_PsiEP->GetYaxis()->CenterTitle();
  h_PsiEP->GetYaxis()->SetTitleSize(0.05);
  h_PsiEP->GetYaxis()->SetNdivisions(505);
  if(h_PsiEP->GetMaximum() > h_PsiGaus->GetMaximum()) h_PsiEP->GetYaxis()->SetRangeUser(0,1.2*h_PsiEP->GetMaximum());
  else h_PsiEP->GetYaxis()->SetRangeUser(0,1.2*h_PsiGaus->GetMaximum());
  h_PsiEP->SetMarkerStyle(24);
  h_PsiEP->SetMarkerColor(2);
  h_PsiEP->SetMarkerSize(1.0);
  h_PsiEP->Draw("pE");
  TF1 *f_EP = new TF1("f_EP",EventPlaneDist,-TMath::PiOver2(),TMath::PiOver2(),2);
  f_EP->SetParameter(0,1.5);
  f_EP->SetParameter(1,1000);
  h_PsiEP->Fit(f_EP,"N");
  f_EP->SetLineColor(2);
  f_EP->SetLineWidth(4);
  f_EP->SetLineStyle(2);
  f_EP->Draw("l same");
  float chi = f_EP->GetParameter(0);
  TF1 *f_res = new TF1("f_res",EventPlaneResolution,0,10,0);
  float resEP = f_res->Eval(chi);
  string leg_PsiEP = Form("Event Plane: #chi = %2.2f, resolution = %2.2f",chi,resEP);
  plotTopLegend((char*)leg_PsiEP.c_str(),0.2,0.85,0.04,2,0.0,42,1);

  h_PsiGaus->SetMarkerStyle(24);
  h_PsiGaus->SetMarkerColor(1);
  h_PsiGaus->SetMarkerSize(1.0);
  h_PsiGaus->Draw("pE same");
  TF1 *f_gaus = new TF1("f_gaus","gaus",-TMath::PiOver2(),TMath::PiOver2());
  h_PsiGaus->Fit(f_gaus,"N");
  float sig = f_gaus->GetParameter(2);
  f_gaus->SetRange(-3.0*sig,3.0*sig);
  h_PsiGaus->Fit(f_gaus,"NR");

  f_gaus->SetLineColor(1);
  f_gaus->SetLineWidth(4);
  f_gaus->SetLineStyle(2);
  f_gaus->Draw("l same");

  float sigma = f_gaus->GetParameter(2);
  float resGaus = cos(2.0*sigma);
  string leg_PsiGaus = Form("Gaussian: #sigma = %2.2f, resolution = %2.2f",sigma,resGaus);
  plotTopLegend((char*)leg_PsiGaus.c_str(),0.2,0.8,0.04,1,0.0,42,1);

  TCanvas *c_v2fitQA = new TCanvas("c_v2fitQA","c_v2fitQA",10,10,1200,600);
  c_v2fitQA->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_v2fitQA->cd(i_pad+1)->SetLeftMargin(0.15);
    c_v2fitQA->cd(i_pad+1)->SetBottomMargin(0.15);
    c_v2fitQA->cd(i_pad+1)->SetTicks(1,1);
    c_v2fitQA->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistPhiQA = Form("h_phiQA_%d",BinRho);
  TH2F *h_phiQA = (TH2F*)File_Hist->Get(HistPhiQA.c_str());
  TH1F *h_phiQAproj[vmsa::BinPt];
  TGraphAsymmErrors *g_v2QA = new TGraphAsymmErrors();
  float delta_pt = (vmsa::ptMax-vmsa::ptMin)/(float)vmsa::BinPt;
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string phiQAproj = Form("h_phiQAproj_%d",i_pt);
    h_phiQAproj[i_pt] = (TH1F*)h_phiQA->ProjectionY(phiQAproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_flow = new TF1("f_flow",flow,-1.0*TMath::Pi(),1.0*TMath::Pi(),2);
    f_flow->SetParameter(0,0.1);
    f_flow->SetParameter(1,100);
    h_phiQAproj[i_pt]->Fit(f_flow,"N");
    float v2 = f_flow->GetParameter(0);
    float err_v2 = f_flow->GetParError(0);
    g_v2QA->SetPoint(i_pt,pt,v2);
    g_v2QA->SetPointError(i_pt,0.0,0.0,err_v2,err_v2);
    if(i_pt == pT_low) c_v2fitQA->cd(1);
    if(i_pt == pT_high) c_v2fitQA->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_phiQAproj[i_pt]->SetTitle("");
      h_phiQAproj[i_pt]->SetStats(0);
      h_phiQAproj[i_pt]->GetXaxis()->SetTitle("#phi-#Psi_{RP}");
      h_phiQAproj[i_pt]->GetXaxis()->CenterTitle();
      h_phiQAproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_phiQAproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_phiQAproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_phiQAproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_phiQAproj[i_pt]->GetYaxis()->CenterTitle();
      h_phiQAproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_phiQAproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_phiQAproj[i_pt]->GetYaxis()->SetRangeUser(0.7*h_phiQAproj[i_pt]->GetMinimum(),1.2*h_phiQAproj[i_pt]->GetMaximum());

      h_phiQAproj[i_pt]->SetMarkerStyle(MarkerStyleQA);
      h_phiQAproj[i_pt]->SetMarkerSize(1.1);
      h_phiQAproj[i_pt]->SetMarkerColor(MarkerColorQA);
      h_phiQAproj[i_pt]->SetLineColor(MarkerColorQA);
      h_phiQAproj[i_pt]->DrawCopy("pE");
      f_flow->SetLineStyle(2);
      f_flow->SetLineWidth(2);
      f_flow->SetLineColor(2);
      f_flow->Draw("l same");
      string legPt = Form("p_{T} = %2.2f GeV/c",pt);
      string legV2 = Form("v_{2} = %2.3f #pm %0.3f",v2,err_v2);
      plotTopLegend((char*)legPt.c_str(),0.4,0.35,0.04,1,0.0,42,1);
      plotTopLegend((char*)legV2.c_str(),0.4,0.25,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_v2fitRP = new TCanvas("c_v2fitRP","c_v2fitRP",10,10,1200,600);
  c_v2fitRP->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_v2fitRP->cd(i_pad+1)->SetLeftMargin(0.15);
    c_v2fitRP->cd(i_pad+1)->SetBottomMargin(0.15);
    c_v2fitRP->cd(i_pad+1)->SetTicks(1,1);
    c_v2fitRP->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistPhiRP = Form("h_phiRP_%d",BinRho);
  TH2F *h_phiRP = (TH2F*)File_Hist->Get(HistPhiRP.c_str());
  TH1F *h_phiRPproj[vmsa::BinPt];
  TGraphAsymmErrors *g_v2RP = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string phiRPproj = Form("h_phiRPproj_%d",i_pt);
    h_phiRPproj[i_pt] = (TH1F*)h_phiRP->ProjectionY(phiRPproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_flow = new TF1("f_flow",flow,-1.0*TMath::Pi(),1.0*TMath::Pi(),2);
    f_flow->SetParameter(0,0.1);
    f_flow->SetParameter(1,100);
    h_phiRPproj[i_pt]->Fit(f_flow,"N");
    float v2 = f_flow->GetParameter(0);
    float err_v2 = f_flow->GetParError(0);
    g_v2RP->SetPoint(i_pt,pt+0.02,v2);
    g_v2RP->SetPointError(i_pt,0.0,0.0,err_v2,err_v2);
    if(i_pt == pT_low) c_v2fitRP->cd(1);
    if(i_pt == pT_high) c_v2fitRP->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_phiRPproj[i_pt]->SetTitle("");
      h_phiRPproj[i_pt]->SetStats(0);
      h_phiRPproj[i_pt]->GetXaxis()->SetTitle("#phi-#Psi_{RP}");
      h_phiRPproj[i_pt]->GetXaxis()->CenterTitle();
      h_phiRPproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_phiRPproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_phiRPproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_phiRPproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_phiRPproj[i_pt]->GetYaxis()->CenterTitle();
      h_phiRPproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_phiRPproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_phiRPproj[i_pt]->GetYaxis()->SetRangeUser(0.7*h_phiRPproj[i_pt]->GetMinimum(),1.2*h_phiRPproj[i_pt]->GetMaximum());

      h_phiRPproj[i_pt]->SetMarkerStyle(MarkerStyleRP);
      h_phiRPproj[i_pt]->SetMarkerSize(1.1);
      h_phiRPproj[i_pt]->SetMarkerColor(MarkerColorRP);
      h_phiRPproj[i_pt]->SetLineColor(MarkerColorRP);
      h_phiRPproj[i_pt]->DrawCopy("pE");
      f_flow->SetLineStyle(2);
      f_flow->SetLineWidth(2);
      f_flow->SetLineColor(2);
      f_flow->Draw("l same");
      string legPt = Form("p_{T} = %2.2f GeV/c",pt);
      string legV2 = Form("v_{2} = %2.3f #pm %0.3f",v2,err_v2);
      plotTopLegend((char*)legPt.c_str(),0.4,0.35,0.04,1,0.0,42,1);
      plotTopLegend((char*)legV2.c_str(),0.4,0.25,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_v2fitGaus = new TCanvas("c_v2fitGaus","c_v2fitGaus",10,10,1200,600);
  c_v2fitGaus->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_v2fitGaus->cd(i_pad+1)->SetLeftMargin(0.15);
    c_v2fitGaus->cd(i_pad+1)->SetBottomMargin(0.15);
    c_v2fitGaus->cd(i_pad+1)->SetTicks(1,1);
    c_v2fitGaus->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistPhiGaus = Form("h_phiGaus_%d",BinRho);
  TH2F *h_phiGaus = (TH2F*)File_Hist->Get(HistPhiGaus.c_str());
  TH1F *h_phiGausproj[vmsa::BinPt];
  TGraphAsymmErrors *g_v2Gaus = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string phiGausproj = Form("h_phiGausproj_%d",i_pt);
    h_phiGausproj[i_pt] = (TH1F*)h_phiGaus->ProjectionY(phiGausproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_flow = new TF1("f_flow",flow,-1.0*TMath::Pi(),1.0*TMath::Pi(),2);
    f_flow->SetParameter(0,0.1);
    f_flow->SetParameter(1,100);
    h_phiGausproj[i_pt]->Fit(f_flow,"N");
    float v2 = f_flow->GetParameter(0);
    float err_v2 = f_flow->GetParError(0);
    g_v2Gaus->SetPoint(i_pt,pt+0.05,v2/resGaus);
    g_v2Gaus->SetPointError(i_pt,0.0,0.0,err_v2/resGaus,err_v2/resGaus);
    if(i_pt == pT_low) c_v2fitGaus->cd(1);
    if(i_pt == pT_high) c_v2fitGaus->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_phiGausproj[i_pt]->SetTitle("");
      h_phiGausproj[i_pt]->SetStats(0);
      h_phiGausproj[i_pt]->GetXaxis()->SetTitle("#phi-#Psi_{EP}");
      h_phiGausproj[i_pt]->GetXaxis()->CenterTitle();
      h_phiGausproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_phiGausproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_phiGausproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_phiGausproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_phiGausproj[i_pt]->GetYaxis()->CenterTitle();
      h_phiGausproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_phiGausproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_phiGausproj[i_pt]->GetYaxis()->SetRangeUser(0.7*h_phiGausproj[i_pt]->GetMinimum(),1.2*h_phiGausproj[i_pt]->GetMaximum());

      h_phiGausproj[i_pt]->SetMarkerStyle(MarkerStyleGaus);
      h_phiGausproj[i_pt]->SetMarkerSize(1.1);
      h_phiGausproj[i_pt]->SetMarkerColor(MarkerColorGaus);
      h_phiGausproj[i_pt]->SetLineColor(MarkerColorGaus);
      h_phiGausproj[i_pt]->DrawCopy("pE");
      f_flow->SetLineStyle(2);
      f_flow->SetLineWidth(2);
      f_flow->SetLineColor(2);
      f_flow->Draw("l same");
      string legPt = Form("p_{T} = %2.2f GeV/c",pt);
      string legV2 = Form("v_{2} = %2.3f #pm %0.3f",v2,err_v2);
      plotTopLegend((char*)legPt.c_str(),0.4,0.35,0.04,1,0.0,42,1);
      plotTopLegend((char*)legV2.c_str(),0.4,0.25,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_v2fitEP = new TCanvas("c_v2fitEP","c_v2fitEP",10,10,1200,600);
  c_v2fitEP->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_v2fitEP->cd(i_pad+1)->SetLeftMargin(0.15);
    c_v2fitEP->cd(i_pad+1)->SetBottomMargin(0.15);
    c_v2fitEP->cd(i_pad+1)->SetTicks(1,1);
    c_v2fitEP->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistPhiEP = Form("h_phiEP_%d",BinRho);
  TH2F *h_phiEP = (TH2F*)File_Hist->Get(HistPhiEP.c_str());
  TH1F *h_phiEPproj[vmsa::BinPt];
  TGraphAsymmErrors *g_v2EP = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string phiEPproj = Form("h_phiEPproj_%d",i_pt);
    h_phiEPproj[i_pt] = (TH1F*)h_phiEP->ProjectionY(phiEPproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_flow = new TF1("f_flow",flow,-1.0*TMath::Pi(),1.0*TMath::Pi(),2);
    f_flow->SetParameter(0,0.1);
    f_flow->SetParameter(1,100);
    h_phiEPproj[i_pt]->Fit(f_flow,"N");
    float v2 = f_flow->GetParameter(0);
    float err_v2 = f_flow->GetParError(0);
    g_v2EP->SetPoint(i_pt,pt+0.05,v2/resEP);
    g_v2EP->SetPointError(i_pt,0.0,0.0,err_v2/resEP,err_v2/resEP);
    if(i_pt == pT_low) c_v2fitEP->cd(1);
    if(i_pt == pT_high) c_v2fitEP->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_phiEPproj[i_pt]->SetTitle("");
      h_phiEPproj[i_pt]->SetStats(0);
      h_phiEPproj[i_pt]->GetXaxis()->SetTitle("#phi-#Psi_{EP}");
      h_phiEPproj[i_pt]->GetXaxis()->CenterTitle();
      h_phiEPproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_phiEPproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_phiEPproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_phiEPproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_phiEPproj[i_pt]->GetYaxis()->CenterTitle();
      h_phiEPproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_phiEPproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_phiEPproj[i_pt]->GetYaxis()->SetRangeUser(0.7*h_phiEPproj[i_pt]->GetMinimum(),1.2*h_phiEPproj[i_pt]->GetMaximum());

      h_phiEPproj[i_pt]->SetMarkerStyle(MarkerStyleEP);
      h_phiEPproj[i_pt]->SetMarkerSize(1.1);
      h_phiEPproj[i_pt]->SetMarkerColor(MarkerColorEP);
      h_phiEPproj[i_pt]->SetLineColor(MarkerColorEP);
      h_phiEPproj[i_pt]->DrawCopy("pE");
      f_flow->SetLineStyle(2);
      f_flow->SetLineWidth(2);
      f_flow->SetLineColor(2);
      f_flow->Draw("l same");
      string legPt = Form("p_{T} = %2.2f GeV/c",pt);
      string legV2 = Form("v_{2} = %2.3f #pm %0.3f",v2,err_v2);
      plotTopLegend((char*)legPt.c_str(),0.4,0.35,0.04,1,0.0,42,1);
      plotTopLegend((char*)legV2.c_str(),0.4,0.25,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_v2 = new TCanvas("c_v2","c_v2",10,10,800,800);
  c_v2->cd()->SetLeftMargin(0.15);
  c_v2->cd()->SetBottomMargin(0.15);
  c_v2->cd()->SetTicks(1,1);
  c_v2->cd()->SetGrid(0,0);
  TH1F *h_play = new TH1F("h_play","h_play",100,0.0,10.0);
  for(int i_bin = 1; i_bin < 101; ++i_bin)
  {
    h_play->SetBinContent(i_bin,-10.0);
    h_play->SetBinError(i_bin,1.0);
  }
  h_play->SetTitle("");
  h_play->SetStats(0);
  h_play->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  h_play->GetXaxis()->CenterTitle();
  h_play->GetXaxis()->SetRangeUser(vmsa::ptMin,vmsa::ptMax);
  h_play->GetXaxis()->SetNdivisions(505);
  h_play->GetXaxis()->SetLabelSize(0.03);

  h_play->GetYaxis()->SetTitle("v_{2}");
  h_play->GetYaxis()->CenterTitle();
  // h_play->GetYaxis()->SetRangeUser(0.0,0.2);
  float v2Max = g_v2Gaus->GetHistogram()->GetMaximum();
  h_play->GetYaxis()->SetRangeUser(0.0,v2Max);
  h_play->GetYaxis()->SetNdivisions(505);
  h_play->GetYaxis()->SetLabelSize(0.03);
  h_play->Draw("pE");
  f_v2->Draw("l same");

  g_v2QA->SetMarkerStyle(MarkerStyleQA);
  g_v2QA->SetMarkerColor(MarkerColorQA);
  g_v2QA->SetLineColor(MarkerColorQA);
  g_v2QA->SetMarkerSize(1.4);
  g_v2QA->Draw("pE same");

  g_v2RP->SetMarkerStyle(MarkerStyleRP);
  g_v2RP->SetMarkerColor(MarkerColorRP);
  g_v2RP->SetLineColor(MarkerColorRP);
  g_v2RP->SetMarkerSize(1.4);
  g_v2RP->Draw("pE same");

  g_v2Gaus->SetMarkerStyle(MarkerStyleGaus);
  g_v2Gaus->SetMarkerColor(MarkerColorGaus);
  g_v2Gaus->SetLineColor(MarkerColorGaus);
  g_v2Gaus->SetMarkerSize(1.4);
  g_v2Gaus->Draw("pE same");

  g_v2EP->SetMarkerStyle(MarkerStyleEP);
  g_v2EP->SetMarkerColor(MarkerColorEP);
  g_v2EP->SetLineColor(MarkerColorEP);
  g_v2EP->SetMarkerSize(1.4);
  g_v2EP->Draw("pE same");

  g_v2->SetMarkerStyle(30);
  g_v2->SetMarkerColor(2);
  g_v2->SetLineColor(2);
  g_v2->SetMarkerSize(2.4);
  g_v2->Draw("pE same");

  string legEnergy = Form("AuAu %s 10%%-40%%",vmsa::mBeamEnergy[energy].c_str());
  plotTopLegend((char*)legEnergy.c_str(),0.4,0.35,0.04,1,0.0,42,1);
  TLegend *legv2 = new TLegend(0.2,0.7,0.55,0.85);
  legv2->SetBorderSize(0.0);
  legv2->SetFillColor(10);
  legv2->AddEntry(g_v2,"#phi STAR PRC 93 014907","p");
  legv2->AddEntry(f_v2,"fit","l");
  legv2->AddEntry(g_v2QA,"MC","p");
  legv2->AddEntry(g_v2RP,"MC w.r.t. RP & #rho_{00}^{phy}","p");
  legv2->AddEntry(g_v2Gaus,"MC w.r.t. Gaussian EP & #rho_{00}^{obs}","p");
  legv2->AddEntry(g_v2EP,"MC w.r.t. Sergei EP & #rho_{00}^{obs}","p");
  legv2->Draw("same");

  TCanvas *c_rhofitQA = new TCanvas("c_rhofitQA","c_rhofitQA",10,10,1200,600);
  c_rhofitQA->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_rhofitQA->cd(i_pad+1)->SetLeftMargin(0.15);
    c_rhofitQA->cd(i_pad+1)->SetBottomMargin(0.15);
    c_rhofitQA->cd(i_pad+1)->SetTicks(1,1);
    c_rhofitQA->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistCosQA = Form("h_cosQA_%d",BinRho);
  TH2F *h_cosQA = (TH2F*)File_Hist->Get(HistCosQA.c_str());
  TH1F *h_cosQAproj[vmsa::BinPt];
  TGraphAsymmErrors *g_rhoQA = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string rhoQAproj = Form("h_rhoQAproj_%d",i_pt);
    h_cosQAproj[i_pt] = (TH1F*)h_cosQA->ProjectionY(rhoQAproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_rho = new TF1("f_rho",SpinDensity,-1.0,1.0,2);
    f_rho->SetParameter(0,0.33);
    f_rho->SetParameter(1,100);
    h_cosQAproj[i_pt]->Fit(f_rho,"NQ");
    float rho = f_rho->GetParameter(0);
    float err_rho = f_rho->GetParError(0);
    g_rhoQA->SetPoint(i_pt,pt,rho);
    g_rhoQA->SetPointError(i_pt,0.0,0.0,err_rho,err_rho);

    if(i_pt == pT_low) c_rhofitQA->cd(1);
    if(i_pt == pT_high) c_rhofitQA->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_cosQAproj[i_pt]->SetTitle("");
      h_cosQAproj[i_pt]->SetStats(0);
      h_cosQAproj[i_pt]->GetXaxis()->SetTitle("cos(#theta*)");
      h_cosQAproj[i_pt]->GetXaxis()->CenterTitle();
      h_cosQAproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_cosQAproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_cosQAproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_cosQAproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_cosQAproj[i_pt]->GetYaxis()->CenterTitle();
      h_cosQAproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_cosQAproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_cosQAproj[i_pt]->GetYaxis()->SetRangeUser(0.6*h_cosQAproj[i_pt]->GetMinimum(),1.1*h_cosQAproj[i_pt]->GetMaximum());

      h_cosQAproj[i_pt]->SetMarkerStyle(MarkerStyleQA);
      h_cosQAproj[i_pt]->SetMarkerSize(1.1);
      h_cosQAproj[i_pt]->SetMarkerColor(MarkerColorQA);
      h_cosQAproj[i_pt]->SetLineColor(MarkerColorQA);
      h_cosQAproj[i_pt]->DrawCopy("pE");
      f_rho->SetLineStyle(2);
      f_rho->SetLineWidth(2);
      f_rho->SetLineColor(2);
      f_rho->Draw("l same");
      string legPtRho = Form("p_{T} = %2.2f GeV/c, #rho_{00} = %2.3f #pm %0.3f",pt,rho,err_rho);
      plotTopLegend((char*)legPtRho.c_str(),0.2,0.20,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_rhofitRP = new TCanvas("c_rhofitRP","c_rhofitRP",10,10,1200,600);
  c_rhofitRP->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_rhofitRP->cd(i_pad+1)->SetLeftMargin(0.15);
    c_rhofitRP->cd(i_pad+1)->SetBottomMargin(0.15);
    c_rhofitRP->cd(i_pad+1)->SetTicks(1,1);
    c_rhofitRP->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistCosRP = Form("h_cosRP_%d",BinRho);
  TH2F *h_cosRP = (TH2F*)File_Hist->Get(HistCosRP.c_str());
  TH1F *h_cosRPproj[vmsa::BinPt];
  TGraphAsymmErrors *g_rhoRP = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string rhoRPproj = Form("h_rhoRPproj_%d",i_pt);
    h_cosRPproj[i_pt] = (TH1F*)h_cosRP->ProjectionY(rhoRPproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_rho = new TF1("f_rho",SpinDensity,-1.0,1.0,2);
    f_rho->SetParameter(0,0.33);
    f_rho->SetParameter(1,100);
    h_cosRPproj[i_pt]->Fit(f_rho,"NQ");
    float rho = f_rho->GetParameter(0);
    float err_rho = f_rho->GetParError(0);
    g_rhoRP->SetPoint(i_pt,pt,rho);
    g_rhoRP->SetPointError(i_pt,0.0,0.0,err_rho,err_rho);

    if(i_pt == pT_low) c_rhofitRP->cd(1);
    if(i_pt == pT_high) c_rhofitRP->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_cosRPproj[i_pt]->SetTitle("");
      h_cosRPproj[i_pt]->SetStats(0);
      h_cosRPproj[i_pt]->GetXaxis()->SetTitle("cos(#theta*)");
      h_cosRPproj[i_pt]->GetXaxis()->CenterTitle();
      h_cosRPproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_cosRPproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_cosRPproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_cosRPproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_cosRPproj[i_pt]->GetYaxis()->CenterTitle();
      h_cosRPproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_cosRPproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_cosRPproj[i_pt]->GetYaxis()->SetRangeUser(0.6*h_cosRPproj[i_pt]->GetMinimum(),1.1*h_cosRPproj[i_pt]->GetMaximum());

      h_cosRPproj[i_pt]->SetMarkerStyle(MarkerStyleRP);
      h_cosRPproj[i_pt]->SetMarkerSize(1.1);
      h_cosRPproj[i_pt]->SetMarkerColor(MarkerColorRP);
      h_cosRPproj[i_pt]->SetLineColor(MarkerColorRP);
      h_cosRPproj[i_pt]->DrawCopy("pE");
      f_rho->SetLineStyle(2);
      f_rho->SetLineWidth(2);
      f_rho->SetLineColor(2);
      f_rho->Draw("l same");
      string legPtRho = Form("p_{T} = %2.2f GeV/c, #rho_{00} = %2.3f #pm %0.3f",pt,rho,err_rho);
      plotTopLegend((char*)legPtRho.c_str(),0.2,0.20,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_rhofitGaus = new TCanvas("c_rhofitGaus","c_rhofitGaus",10,10,1200,600);
  c_rhofitGaus->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_rhofitGaus->cd(i_pad+1)->SetLeftMargin(0.15);
    c_rhofitGaus->cd(i_pad+1)->SetBottomMargin(0.15);
    c_rhofitGaus->cd(i_pad+1)->SetTicks(1,1);
    c_rhofitGaus->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistCosGaus = Form("h_cosGaus_%d",BinRho);
  TH2F *h_cosGaus = (TH2F*)File_Hist->Get(HistCosGaus.c_str());
  TH1F *h_cosGausproj[vmsa::BinPt];
  TGraphAsymmErrors *g_rhoGaus = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string rhoGausproj = Form("h_rhoGausproj_%d",i_pt);
    h_cosGausproj[i_pt] = (TH1F*)h_cosGaus->ProjectionY(rhoGausproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_rho = new TF1("f_rho",SpinDensity,-1.0,1.0,2);
    f_rho->SetParameter(0,0.33);
    f_rho->SetParameter(1,100);
    h_cosGausproj[i_pt]->Fit(f_rho,"NQ");
    float rho = f_rho->GetParameter(0);
    float err_rho = f_rho->GetParError(0);
    g_rhoGaus->SetPoint(i_pt,pt,rho);
    g_rhoGaus->SetPointError(i_pt,0.0,0.0,err_rho,err_rho);

    if(i_pt == pT_low) c_rhofitGaus->cd(1);
    if(i_pt == pT_high) c_rhofitGaus->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_cosGausproj[i_pt]->SetTitle("");
      h_cosGausproj[i_pt]->SetStats(0);
      h_cosGausproj[i_pt]->GetXaxis()->SetTitle("cos(#theta*)");
      h_cosGausproj[i_pt]->GetXaxis()->CenterTitle();
      h_cosGausproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_cosGausproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_cosGausproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_cosGausproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_cosGausproj[i_pt]->GetYaxis()->CenterTitle();
      h_cosGausproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_cosGausproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_cosGausproj[i_pt]->GetYaxis()->SetRangeUser(0.6*h_cosGausproj[i_pt]->GetMinimum(),1.1*h_cosGausproj[i_pt]->GetMaximum());

      h_cosGausproj[i_pt]->SetMarkerStyle(MarkerStyleGaus);
      h_cosGausproj[i_pt]->SetMarkerSize(1.1);
      h_cosGausproj[i_pt]->SetMarkerColor(MarkerColorGaus);
      h_cosGausproj[i_pt]->SetLineColor(MarkerColorGaus);
      h_cosGausproj[i_pt]->DrawCopy("pE");
      f_rho->SetLineStyle(2);
      f_rho->SetLineWidth(2);
      f_rho->SetLineColor(2);
      f_rho->Draw("l same");
      string legPtRho = Form("p_{T} = %2.2f GeV/c, #rho_{00} = %2.3f #pm %0.3f",pt,rho,err_rho);
      plotTopLegend((char*)legPtRho.c_str(),0.2,0.20,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_rhofitEP = new TCanvas("c_rhofitEP","c_rhofitEP",10,10,1200,600);
  c_rhofitEP->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_rhofitEP->cd(i_pad+1)->SetLeftMargin(0.15);
    c_rhofitEP->cd(i_pad+1)->SetBottomMargin(0.15);
    c_rhofitEP->cd(i_pad+1)->SetTicks(1,1);
    c_rhofitEP->cd(i_pad+1)->SetGrid(0,0);
  }
  string HistCosEP = Form("h_cosEP_%d",BinRho);
  TH2F *h_cosEP = (TH2F*)File_Hist->Get(HistCosEP.c_str());
  TH1F *h_cosEPproj[vmsa::BinPt];
  TGraphAsymmErrors *g_rhoEP = new TGraphAsymmErrors();
  for(int i_pt = 0; i_pt < vmsa::BinPt; ++i_pt)
  {
    float pt = vmsa::ptMin+(i_pt+0.5)*delta_pt;
    string rhoEPproj = Form("h_rhoEPproj_%d",i_pt);
    h_cosEPproj[i_pt] = (TH1F*)h_cosEP->ProjectionY(rhoEPproj.c_str(),i_pt+1,i_pt+1);
    TF1 *f_rho = new TF1("f_rho",SpinDensity,-1.0,1.0,2);
    f_rho->SetParameter(0,0.33);
    f_rho->SetParameter(1,100);
    h_cosEPproj[i_pt]->Fit(f_rho,"NQ");
    float rho = f_rho->GetParameter(0);
    float err_rho = f_rho->GetParError(0);
    g_rhoEP->SetPoint(i_pt,pt,rho);
    g_rhoEP->SetPointError(i_pt,0.0,0.0,err_rho,err_rho);

    if(i_pt == pT_low) c_rhofitEP->cd(1);
    if(i_pt == pT_high) c_rhofitEP->cd(2);
    if(i_pt == pT_low || i_pt == pT_high)
    {
      h_cosEPproj[i_pt]->SetTitle("");
      h_cosEPproj[i_pt]->SetStats(0);
      h_cosEPproj[i_pt]->GetXaxis()->SetTitle("cos(#theta*)");
      h_cosEPproj[i_pt]->GetXaxis()->CenterTitle();
      h_cosEPproj[i_pt]->GetXaxis()->SetLabelSize(0.04);
      h_cosEPproj[i_pt]->GetXaxis()->SetNdivisions(505);

      h_cosEPproj[i_pt]->GetYaxis()->SetTitle("Counts");
      h_cosEPproj[i_pt]->GetYaxis()->SetTitleSize(0.04);
      h_cosEPproj[i_pt]->GetYaxis()->CenterTitle();
      h_cosEPproj[i_pt]->GetYaxis()->SetLabelSize(0.04);
      h_cosEPproj[i_pt]->GetYaxis()->SetNdivisions(505);
      h_cosEPproj[i_pt]->GetYaxis()->SetRangeUser(0.6*h_cosEPproj[i_pt]->GetMinimum(),1.1*h_cosEPproj[i_pt]->GetMaximum());

      h_cosEPproj[i_pt]->SetMarkerStyle(MarkerStyleEP);
      h_cosEPproj[i_pt]->SetMarkerSize(1.1);
      h_cosEPproj[i_pt]->SetMarkerColor(MarkerColorEP);
      h_cosEPproj[i_pt]->SetLineColor(MarkerColorEP);
      h_cosEPproj[i_pt]->DrawCopy("pE");
      f_rho->SetLineStyle(2);
      f_rho->SetLineWidth(2);
      f_rho->SetLineColor(2);
      f_rho->Draw("l same");
      string legPtRho = Form("p_{T} = %2.2f GeV/c, #rho_{00} = %2.3f #pm %0.3f",pt,rho,err_rho);
      plotTopLegend((char*)legPtRho.c_str(),0.2,0.20,0.04,1,0.0,42,1);
    } 
  }

  TCanvas *c_rho = new TCanvas("c_rho","c_rho",100,10,800,800);
  c_rho->cd()->SetLeftMargin(0.15);
  c_rho->cd()->SetBottomMargin(0.15);
  c_rho->cd()->SetTicks(1,1);
  c_rho->cd()->SetGrid(0,0);
  TH1F *h_rho = new TH1F("h_rho","h_rho",100,0.0,10.0);
  for(int i_bin = 1; i_bin < 101; ++i_bin)
  {
    h_rho->SetBinContent(i_bin,-10.0);
    h_rho->SetBinError(i_bin,1.0);
  }
  h_rho->SetTitle("");
  h_rho->SetStats(0);
  h_rho->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  h_rho->GetXaxis()->CenterTitle();
  h_rho->GetXaxis()->SetRangeUser(vmsa::ptMin,vmsa::ptMax);
  h_rho->GetYaxis()->SetTitle("#rho_{00}");
  h_rho->GetXaxis()->SetNdivisions(505);

  h_rho->GetYaxis()->CenterTitle();
  h_rho->GetYaxis()->SetRangeUser(0.0,0.6);
  h_rho->GetYaxis()->SetNdivisions(505);
  h_rho->Draw("pE");

  g_rhoQA->SetMarkerStyle(MarkerStyleQA);
  g_rhoQA->SetMarkerColor(MarkerColorQA);
  g_rhoQA->SetMarkerSize(1.4);
  g_rhoQA->Draw("pE same");
  TF1 *f_polQA = new TF1("f_polQA","pol0",vmsa::ptMin,vmsa::ptMax);
  f_polQA->SetParameter(0,0.33);
  g_rhoQA->Fit(f_polQA,"NQ");
  f_polQA->SetLineStyle(2);
  f_polQA->SetLineColor(MarkerColorQA);
  f_polQA->SetLineWidth(2);
  f_polQA->Draw("l same");
  float rhoQA = f_polQA->GetParameter(0);
  float err_rhoQA = f_polQA->GetParError(0);
  float chi2QA = f_polQA->GetChisquare();
  float NDFQA = f_polQA->GetNDF();
  string leg_rhoQA = Form("#rho_{00}^{QA} = %2.4f #pm %2.4f, #chi^{2}/NDF = %2.2f/%2.2f",rhoQA,err_rhoQA,chi2QA,NDFQA);

  g_rhoRP->SetMarkerStyle(MarkerStyleRP);
  g_rhoRP->SetMarkerColor(MarkerColorRP);
  g_rhoRP->SetMarkerSize(1.4);
  g_rhoRP->Draw("pE same");
  TF1 *f_polRP = new TF1("f_polRP","pol0",vmsa::ptMin,vmsa::ptMax);
  f_polRP->SetParameter(0,0.33);
  g_rhoRP->Fit(f_polRP,"NQ");
  f_polRP->SetLineStyle(2);
  f_polRP->SetLineColor(MarkerColorRP);
  f_polRP->SetLineWidth(2);
  f_polRP->Draw("l same");
  float rhoRP = f_polRP->GetParameter(0);
  float err_rhoRP = f_polRP->GetParError(0);
  float chi2RP = f_polRP->GetChisquare();
  float NDFRP = f_polRP->GetNDF();
  string leg_rhoRP = Form("#rho_{00}^{RP} = %2.4f #pm %2.4f, #chi^{2}/NDF = %2.2f/%2.2f",rhoRP,err_rhoRP,chi2RP,NDFRP);

  g_rhoGaus->SetMarkerStyle(MarkerStyleGaus);
  g_rhoGaus->SetMarkerColor(MarkerColorGaus);
  g_rhoGaus->SetMarkerSize(1.4);
  g_rhoGaus->Draw("pE same");
  TF1 *f_polGaus = new TF1("f_polGaus","pol0",vmsa::ptMin,vmsa::ptMax);
  f_polGaus->SetParameter(0,0.33);
  g_rhoGaus->Fit(f_polGaus,"NQ");
  f_polGaus->SetLineStyle(2);
  f_polGaus->SetLineColor(MarkerColorGaus);
  f_polGaus->SetLineWidth(2);
  f_polGaus->Draw("l same");
  float rhoGaus = f_polGaus->GetParameter(0);
  float err_rhoGaus = f_polGaus->GetParError(0);
  float chi2Gaus = f_polGaus->GetChisquare();
  float NDFGaus = f_polGaus->GetNDF();
  string leg_rhoGaus = Form("#rho_{00}^{EP}(Gaussian) = %2.4f #pm %2.4f, #chi^{2}/NDF = %2.2f/%2.2f",rhoGaus,err_rhoGaus,chi2Gaus,NDFGaus);

  g_rhoEP->SetMarkerStyle(MarkerStyleEP);
  g_rhoEP->SetMarkerColor(MarkerColorEP);
  g_rhoEP->SetMarkerSize(1.4);
  g_rhoEP->Draw("pE same");
  TF1 *f_polEP = new TF1("f_polEP","pol0",vmsa::ptMin,vmsa::ptMax);
  f_polEP->SetParameter(0,0.33);
  g_rhoEP->Fit(f_polEP,"NQ");
  f_polEP->SetLineStyle(2);
  f_polEP->SetLineColor(MarkerColorEP);
  f_polEP->SetLineWidth(2);
  f_polEP->Draw("l same");
  float rhoEP = f_polEP->GetParameter(0);
  float err_rhoEP = f_polEP->GetParError(0);
  float chi2EP = f_polEP->GetChisquare();
  float NDFEP = f_polEP->GetNDF();
  string leg_rhoEP = Form("#rho_{00}^{EP}(Sergei) = %2.4f #pm %2.4f, #chi^{2}/NDF = %2.2f/%2.2f",rhoEP,err_rhoEP,chi2EP,NDFEP);
  string leg_InPut = Form("InPut #rho_{00}^{phy} = %2.2f",rhoInPut);

  TLegend *leg_rhoL = new TLegend(0.2,0.2,0.8,0.4);
  leg_rhoL->SetBorderSize(0);
  leg_rhoL->SetFillColor(0);
  leg_rhoL->AddEntry((TObject*)0,leg_InPut.c_str(),"C");
  leg_rhoL->AddEntry(g_rhoQA,leg_rhoQA.c_str(),"p");
  leg_rhoL->AddEntry(g_rhoRP,leg_rhoRP.c_str(),"p");
  leg_rhoL->AddEntry(g_rhoGaus,leg_rhoGaus.c_str(),"p");
  leg_rhoL->AddEntry(g_rhoEP,leg_rhoEP.c_str(),"p");

  TLegend *leg_rhoH = new TLegend(0.2,0.65,0.8,0.85);
  leg_rhoH->SetBorderSize(0);
  leg_rhoH->SetFillColor(0);
  leg_rhoH->AddEntry((TObject*)0,leg_InPut.c_str(),"C");
  leg_rhoH->AddEntry(g_rhoQA,leg_rhoQA.c_str(),"p");
  leg_rhoH->AddEntry(g_rhoRP,leg_rhoRP.c_str(),"p");
  leg_rhoH->AddEntry(g_rhoGaus,leg_rhoGaus.c_str(),"p");
  leg_rhoH->AddEntry(g_rhoEP,leg_rhoEP.c_str(),"p");

  if(rhoInPut > 1.0/3.0) leg_rhoL->Draw("same");
  else leg_rhoH->Draw("same");

  // c_Track->SaveAs("../figures/TrackSampling.eps");
  c_Psi2->SaveAs("../figures/EventPlaneSmearing.png");
  // c_v2->SaveAs("../figures/McV2.eps");
  // c_v2fitQA->SaveAs("c_v2fitQA.eps");
  // c_v2fitRP->SaveAs("c_v2fitRP.eps");
  // c_v2fitEP->SaveAs("c_v2fitEP.eps");
  // c_rhofitQA->SaveAs("c_rhofitQA.eps");
  // c_rhofitRP->SaveAs("c_rhofitRP.eps");
  // c_rhofitEP->SaveAs("c_rhofitEP.eps");
  // c_rho->SaveAs("c_rho.eps");
}
