#include <iostream>
#include <string> 
#include <map>
#include "TFile.h"
#include "TLorentzVector.h"
#include "TPythia6.h"
#include "TPythia6Decayer.h"
#include "TParticle.h"
#include "TRandom3.h"
#include "TVector3.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TF1.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TClonesArray.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TVector3.h"
#include "TProfile.h"
#include "/global/homes/x/xusun/STAR/VecMesonSpinAlignment/Utility/functions.h"
#include "/global/homes/x/xusun/STAR/VecMesonSpinAlignment/Utility/StSpinAlignmentCons.h"

using namespace std;

TF1* readv2(int energy, int pid, int centrality);
TF1* readspec(int energy, int pid, int centrality);
TH1F* readeta(int energy, int pid, int centrality);
void getKinematics(TLorentzVector& lLambda, double const mass);
void setDecayChannels(int const pid);
void decayAndFill(int const pid, TLorentzVector* lLambda, TClonesArray& daughters);
void fill(int const pid, TLorentzVector* lLambda, TLorentzVector const& lProton, TLorentzVector const& lPion);
void write(int energy,int pid);
TVector3 CalBoostedVector(TLorentzVector const lMcDau, TLorentzVector *lMcVec);
bool passEtaCut(float eta, int BinEta);

int const decayChannelsFirst[2] = {1058,1058};
int const decayChannelsSecond[2] = {1061,1061};
int const decayMother[2] = {3122,-3122};
int const decayChannels[2] = {1058,1058}; // 0: Lambda->p+pi-, 1: Lambdabar->pbar+pi+
float const spinDirection[2] = {1.0,-1.0}; // pbar's momentum is opposite to anti-Lambda spin
float const alphaH[2] = {0.642,-0.642};
float const invMass = 1.116;

// histograms
TH3F *h_Tracks;
TH3F *h_Eta;
TH2F *h_phiRP;
TProfile *p_cosRP, *p_sinRP;
TProfile *p_cosDau[20],     *p_cosLambda[20];
TProfile *p_cosInteDau[20], *p_cosInteLambda[20];
TProfile *p_sinDau[20],     *p_sinLambda[20];
TProfile *p_sinInteDau[20], *p_sinInteLambda[20];

// sampling functions
TF1 *f_v2, *f_spec, *f_flow, *f_EP;
TH1F *h_eta;

TPythia6Decayer* pydecay;

void McLambdaEta(int energy = 6, int pid = 0, int cent = 0, int const NMax = 1000000) // pid = 0 for Lambda, 1 for anti-Lambda
{
  int const BinPt    = vmsa::BinPt;
  int const BinY     = vmsa::BinY;
  int const BinPhi   = vmsa::BinPhi;

  h_Tracks = new TH3F("h_Tracks","h_Tracks",BinPt,vmsa::ptMin,vmsa::ptMax,10.0*BinY,-10.0,10.0,BinPhi,-TMath::Pi(),TMath::Pi());
  h_Eta = new TH3F("h_Eta","h_Eta",10*BinY,-10.0,10.0,10*BinY,-10.0,10.0,10*BinY,-10.0,10.0); // eta for phi, K+ and K-

  h_phiRP = new TH2F("h_phiRP","h_phiRP",BinPt,vmsa::ptMin,vmsa::ptMax,BinPhi,-TMath::Pi(),TMath::Pi());
  p_cosRP = new TProfile("p_cosRP","p_cosRP",BinPt,vmsa::ptMin,vmsa::ptMax);
  p_sinRP = new TProfile("p_sinRP","p_sinRP",BinPt,vmsa::ptMin,vmsa::ptMax);

  for(int i_eta = 0; i_eta < 20; ++i_eta)
  {
    string ProName;
    ProName = Form("p_cosDau_%d",i_eta);
    p_cosDau[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),BinPt,vmsa::ptMin,vmsa::ptMax);
    ProName = Form("p_cosInteDau_%d",i_eta);
    p_cosInteDau[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),1,vmsa::McEtaBin[i_eta]-0.1,vmsa::McEtaBin[i_eta]+0.1);

    ProName = Form("p_cosLambda_%d",i_eta);
    p_cosLambda[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),BinPt,vmsa::ptMin,vmsa::ptMax);
    ProName = Form("p_cosInteLambda_%d",i_eta);
    p_cosInteLambda[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),1,vmsa::McEtaBin[i_eta]-0.1,vmsa::McEtaBin[i_eta]+0.1);

    ProName = Form("p_sinDau_%d",i_eta);
    p_sinDau[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),BinPt,vmsa::ptMin,vmsa::ptMax);
    ProName = Form("p_sinInteDau_%d",i_eta);
    p_sinInteDau[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),1,vmsa::McEtaBin[i_eta]-0.1,vmsa::McEtaBin[i_eta]+0.1);

    ProName = Form("p_sinLambda_%d",i_eta);
    p_sinLambda[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),BinPt,vmsa::ptMin,vmsa::ptMax);
    ProName = Form("p_sinInteLambda_%d",i_eta);
    p_sinInteLambda[i_eta] = new TProfile(ProName.c_str(),ProName.c_str(),1,vmsa::McEtaBin[i_eta]-0.1,vmsa::McEtaBin[i_eta]+0.1);
  }

  f_flow = new TF1("f_flow",flowSample,-TMath::Pi(),TMath::Pi(),1);
  f_v2   = readv2(energy,pid,cent);
  f_spec = readspec(energy,pid,cent);
  h_eta = readeta(energy,pid,cent);

  TStopwatch* stopWatch = new TStopwatch();
  stopWatch->Start();
  if(gRandom) delete gRandom;
  gRandom = new TRandom3();
  gRandom->SetSeed();

  pydecay = TPythia6Decayer::Instance();
  pydecay->Init();
  setDecayChannels(pid); // phi--> K+K-

  TClonesArray ptl("TParticle", 10);
  TLorentzVector *lLambda = new TLorentzVector();
  for(int i_ran = 0; i_ran < NMax; ++i_ran)
  {
    if (floor(10.0*i_ran/ static_cast<float>(NMax)) > floor(10.0*(i_ran-1)/ static_cast<float>(NMax)))
    cout << "=> processing data: " << 100.0*i_ran/ static_cast<float>(NMax) << "%" << endl;

    getKinematics(*lLambda,invMass);
    decayAndFill(pid,lLambda,ptl);
  }
  cout << "=> processing data: 100%" << endl;
  cout << "work done!" << endl;

  write(energy,pid);

  stopWatch->Stop();   
  stopWatch->Print();
}

TF1* readv2(int energy, int pid, int centrality)
{
  string InPutV2 = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/Data/Phi_v2_1040.root",vmsa::mBeamEnergy[energy].c_str());
  TFile *File_v2 = TFile::Open(InPutV2.c_str());
  TGraphAsymmErrors *g_v2 = (TGraphAsymmErrors*)File_v2->Get("g_v2");
  TF1 *f_v2 = new TF1("f_v2",v2_pT_FitFunc,vmsa::ptMin,vmsa::ptMax,5);
  f_v2->FixParameter(0,2);
  f_v2->SetParameter(1,0.1);
  f_v2->SetParameter(2,0.1);
  f_v2->SetParameter(3,0.1);
  f_v2->SetParameter(4,0.1);
  f_v2->SetLineColor(2);
  f_v2->SetLineWidth(2);
  f_v2->SetLineStyle(2);
  g_v2->Fit(f_v2,"N");

  /*
  TCanvas *c_v2 = new TCanvas("c_v2","c_v2",10,10,800,800);
  c_v2->cd()->SetLeftMargin(0.15);
  c_v2->cd()->SetBottomMargin(0.15);
  c_v2->cd()->SetTicks(1,1);
  c_v2->cd()->SetGrid(0,0);
  TH1F *h_v2 = new TH1F("h_v2","h_v2",100,0.0,10.0);
  for(int i_bin = 1; i_bin < 101; ++i_bin)
  {
    h_v2->SetBinContent(i_bin,-10.0);
    h_v2->SetBinError(i_bin,1.0);
  }
  h_v2->SetTitle("");
  h_v2->SetStats(0);
  h_v2->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  h_v2->GetXaxis()->CenterTitle();
  h_v2->GetYaxis()->SetTitle("v_{2}");
  h_v2->GetYaxis()->CenterTitle();
  h_v2->GetYaxis()->SetRangeUser(0.0,0.2);
  h_v2->Draw("pE");
  g_v2->Draw("pE same");
  f_v2->Draw("l same");
  */

  return f_v2;
}

TF1* readspec(int energy, int pid, int centrality)
{
  string InPutSpec = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/Data/Phi_Spec.root",vmsa::mBeamEnergy[energy].c_str());
  TFile *File_Spec = TFile::Open(InPutSpec.c_str());
  TGraphAsymmErrors *g_spec = (TGraphAsymmErrors*)File_Spec->Get("g_spec");
  TF1 *f_Levy = new TF1("f_Levy",Levy,vmsa::ptMin,vmsa::ptMax,3);
  f_Levy->SetParameter(0,1);
  f_Levy->SetParameter(1,10);
  f_Levy->SetParameter(2,0.1);
  f_Levy->SetLineStyle(2);
  f_Levy->SetLineColor(4);
  f_Levy->SetLineWidth(2);
  g_spec->Fit(f_Levy,"N");

  TF1 *f_spec = new TF1("f_spec",pTLevy,vmsa::ptMin,vmsa::ptMax,3);
  f_spec->SetParameter(0,f_Levy->GetParameter(0));
  f_spec->SetParameter(1,f_Levy->GetParameter(1));
  f_spec->SetParameter(2,f_Levy->GetParameter(2));
  f_spec->SetLineStyle(2);
  f_spec->SetLineColor(2);
  f_spec->SetLineWidth(2);

  /*
  TCanvas *c_spec = new TCanvas("c_spec","c_spec",10,10,800,800);
  c_spec->cd()->SetLeftMargin(0.15);
  c_spec->cd()->SetBottomMargin(0.15);
  c_spec->cd()->SetTicks(1,1);
  c_spec->cd()->SetGrid(0,0);
  c_spec->SetLogy();
  TH1F *h_spec = new TH1F("h_spec","h_spec",100,0.0,10.0);
  for(int i_bin = 1; i_bin < 101; ++i_bin)
  {
    h_spec->SetBinContent(i_bin,-10.0);
    h_spec->SetBinError(i_bin,1.0);
  }
  h_spec->SetTitle("");
  h_spec->SetStats(0);
  h_spec->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  h_spec->GetXaxis()->CenterTitle();
  h_spec->GetYaxis()->SetTitle("dN/p_{T}dp_{T}");
  h_spec->GetYaxis()->CenterTitle();
  h_spec->GetYaxis()->SetRangeUser(1E-6,10);
  h_spec->Draw("pE");
  g_spec->Draw("pE same");
  f_Levy->Draw("l same");
  f_spec->Draw("l same");
  */

  return f_spec;
}

TH1F* readeta(int energy, int pid, int centrality)
{
  string InPutEta = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/Data/Phi_Eta.root",vmsa::mBeamEnergy[energy].c_str());
  TFile *File_Eta = TFile::Open(InPutEta.c_str());
  TH2F *h_mEta_Cos_sig = (TH2F*)File_Eta->Get("h_mEta_Cos_sig");
  TH1F *h_eta = (TH1F*)h_mEta_Cos_sig->ProjectionX();

  return h_eta;
}

void getKinematics(TLorentzVector& lLambda, double const mass)
{
  // double const pt = f_spec->GetRandom(vmsa::ptMin, vmsa::ptMax);
  // double const eta = h_eta->GetRandom();
  // f_flow->ReleaseParameter(0);
  // f_flow->SetParameter(0,f_v2->Eval(pt));
  // double const phi = f_flow->GetRandom();

  double const pt = gRandom->Uniform(vmsa::ptMin, vmsa::ptMax);
  double const eta = gRandom->Uniform(-4.0*vmsa::acceptanceRapidity, 4.0*vmsa::acceptanceRapidity);
  double const phi = TMath::TwoPi() * gRandom->Rndm();

  lLambda.SetPtEtaPhiM(pt,eta,phi,mass);
}

void setDecayChannels(int const pid)
{
  int const mdme = decayChannels[pid];
  cout << "mdme = " << mdme << endl;
  for (int idc = decayChannelsFirst[pid]; idc < decayChannelsSecond[pid] + 1; idc++) TPythia6::Instance()->SetMDME(idc, 1, 0); // close all decay channel
  TPythia6::Instance()->SetMDME(mdme, 1, 1); // open the one we need
  int *PYSeed = new int;
  TPythia6::Instance()->SetMRPY(1,(int)PYSeed); // Random seed
}

void decayAndFill(int const pid, TLorentzVector* lLambda, TClonesArray& daughters)
{
  pydecay->Decay(decayMother[pid], lLambda);
  pydecay->ImportParticles(&daughters);

  TLorentzVector lProton;
  TLorentzVector lPion;

  int nTrk = daughters.GetEntriesFast();
  // cout << "nTrk = " << nTrk << endl;
  for (int iTrk = 0; iTrk < nTrk; ++iTrk)
  {
    TParticle* ptl0 = (TParticle*)daughters.At(iTrk);
    // cout << "PdgCode = " << ptl0->GetPdgCode() << endl;

    switch (TMath::Abs(ptl0->GetPdgCode()))
    {
      case 2212:
	ptl0->Momentum(lProton);
	break;
      case 211:
	ptl0->Momentum(lPion);
	break;
      default:
	break;
    }
  }
  daughters.Clear("C");
  // cout << "lProton.M() = " << lProton.M() << endl;
  // cout << "lPion.M() = " << lPion.M() << endl;

  fill(pid,lLambda,lProton,lPion);
}

void fill(int const pid, TLorentzVector* lLambda, TLorentzVector const& lProton, TLorentzVector const& lPion)
{
  // TVector3 vMcKpBoosted = spinDirection[pid]*CalBoostedVector(lProton,lLambda); // boost Lambda back to Lambda rest frame
  TVector3 vMcKpBoosted = CalBoostedVector(lProton,lLambda); // boost Lambda back to Lambda rest frame

  float Pt_Lambda = lLambda->Pt();
  float Eta_Lambda = lLambda->Eta();
  float Eta_Proton = lProton.Eta();
  float Eta_Pion = lPion.Eta();

  float Psi = 0.0;
  TVector3 nQ(TMath::Sin(Psi),-TMath::Cos(Psi),0.0); // direction of angular momentum with un-smeared EP
  // float CosThetaStarRP = (3.0/alphaH[pid])*vMcKpBoosted.Dot(nQ);
  // // float SinPhiStar = TMath::Sin(vMcKpBoosted.Theta())*TMath::Sin(Psi-vMcKpBoosted.Phi());
  // float SinPhiStar = (8.0/(alphaH[pid]*TMath::Pi()))*TMath::Sin(Psi-vMcKpBoosted.Phi());
  float CosThetaStarRP = vMcKpBoosted.Dot(nQ);
  // float SinPhiStar = TMath::Sin(vMcKpBoosted.Theta())*TMath::Sin(Psi-vMcKpBoosted.Phi());
  float SinPhiStar = TMath::Sin(Psi-vMcKpBoosted.Phi());

  h_phiRP->Fill(Pt_Lambda,lLambda->Phi());
  h_Tracks->Fill(Pt_Lambda,Eta_Lambda,lLambda->Phi());
  h_Eta->Fill(Eta_Lambda,Eta_Proton,Eta_Pion);
  p_cosRP->Fill(Pt_Lambda,CosThetaStarRP);
  p_sinRP->Fill(Pt_Lambda,SinPhiStar);

  for(int i_eta = 0; i_eta < 20; ++i_eta)
  {
    if( passEtaCut(Eta_Lambda,i_eta) ) 
    {
      p_cosLambda[i_eta]->Fill(Pt_Lambda,CosThetaStarRP);
      p_cosInteLambda[i_eta]->Fill(vmsa::McEtaBin[i_eta],CosThetaStarRP);
      p_sinLambda[i_eta]->Fill(Pt_Lambda,SinPhiStar);
      p_sinInteLambda[i_eta]->Fill(vmsa::McEtaBin[i_eta],SinPhiStar);
    }

    if( passEtaCut(Eta_Proton,i_eta) && passEtaCut(Eta_Pion,i_eta) && passEtaCut(Eta_Lambda,i_eta) )
    {
      p_cosDau[i_eta]->Fill(Pt_Lambda,CosThetaStarRP);
      p_cosInteDau[i_eta]->Fill(vmsa::McEtaBin[i_eta],CosThetaStarRP);
      p_sinDau[i_eta]->Fill(Pt_Lambda,SinPhiStar);
      p_sinInteDau[i_eta]->Fill(vmsa::McEtaBin[i_eta],SinPhiStar);
    }
  }
}

TVector3 CalBoostedVector(TLorentzVector const lMcDau, TLorentzVector *lMcVec)
{
  TVector3 vMcBeta = -1.0*lMcVec->BoostVector(); // boost vector

  TLorentzVector lProton = lMcDau;
  lProton.Boost(vMcBeta); // boost proton back to Lambda rest frame
  TVector3 vMcDauStar = lProton.Vect().Unit(); // momentum direction of Kplus in phi-meson rest frame

  return vMcDauStar;
}

bool passEtaCut(float eta, int BinEta)
{
  if(TMath::Abs(eta) >= vmsa::McEtaBin[BinEta]) return kFALSE;

  return kTRUE;
}

void write(int energy, int pid)
{
  string OutPutFile = Form("/project/projectdirs/starprod/rnc/xusun/OutPut/AuAu%s/SpinAlignment/Phi/MonteCarlo/McLambdaEta_%d.root",vmsa::mBeamEnergy[energy].c_str(),pid);
  TFile *File_OutPut = new TFile(OutPutFile.c_str(),"RECREATE");
  File_OutPut->cd();

  h_Tracks->Write();
  h_phiRP->Write();
  h_Eta->Write();

  p_cosRP->Write();
  p_sinRP->Write();

  for(int i_eta = 0; i_eta < 20; ++i_eta)
  {
    p_cosDau[i_eta]->Write();
    p_cosInteDau[i_eta]->Write();
    p_cosLambda[i_eta]->Write();
    p_cosInteLambda[i_eta]->Write();

    p_sinDau[i_eta]->Write();
    p_sinInteDau[i_eta]->Write();
    p_sinLambda[i_eta]->Write();
    p_sinInteLambda[i_eta]->Write();
  }

  File_OutPut->Close();
}
