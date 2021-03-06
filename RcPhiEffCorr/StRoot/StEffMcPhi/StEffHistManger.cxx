#include "StEffHistManger.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TMath.h"
#include <iostream>
#include "TLorentzVector.h"
#include "TVector3.h"

ClassImp(StEffHistManger)
//
StEffHistManger::StEffHistManger(int energy)
{
  mEnergy = energy;
}

StEffHistManger::~StEffHistManger()
{
  /* */
}

void StEffHistManger::InitHist()
{
  for(int i_cent = 0; i_cent < 10; ++i_cent)
  {
    std::string HistName = Form("h_mMcTracks_%d",i_cent);
    h_mMcTracks[i_cent] = new TH3D(HistName.c_str(),HistName.c_str(),vmsa::BinPt,0.0,vmsa::ptEffMax,vmsa::BinEta,-1.0,1.0,vmsa::BinPhi,-1.0*TMath::Pi(),TMath::Pi());
    HistName = Form("h_mRcTracks_%d",i_cent);
    h_mRcTracks[i_cent] = new TH3D(HistName.c_str(),HistName.c_str(),vmsa::BinPt,0.0,vmsa::ptEffMax,vmsa::BinEta,-1.0,1.0,vmsa::BinPhi,-1.0*TMath::Pi(),TMath::Pi());
    for(int i_pt = vmsa::pt_rebin_first[mEnergy]; i_pt < vmsa::pt_rebin_last[mEnergy]; ++i_pt) // use rebinned pt
    {
      HistName = Form("h_mMcEffCos_Cent_%d_Pt_%d",i_cent,i_pt);
      h_mMcEffCos[i_cent][i_pt] = new TH1D(HistName.c_str(),HistName.c_str(),vmsa::BinCos,0.0,1.0);
      HistName = Form("h_mRcEffCos_Cent_%d_Pt_%d",i_cent,i_pt);
      h_mRcEffCos[i_cent][i_pt] = new TH1D(HistName.c_str(),HistName.c_str(),vmsa::BinCos,0.0,1.0);
    }
  }
  flag_eff = 0;
  flag_eff_PtEtaPhi = 0;
  flag_eff_Cos = 0;
}

void StEffHistManger::FillHistMc(int cent, float pt, float eta, float phi, float cos)
{
  h_mMcTracks[cent]->Fill(pt,eta,phi);
  if(cent >= vmsa::cent_low[0] && cent <= vmsa::cent_up[0])
  {//20-60%
    h_mMcTracks[9]->Fill(pt,eta,phi,vmsa::weight[cent]);
  }
  for(int i_pt = vmsa::pt_rebin_first[mEnergy]; i_pt < vmsa::pt_rebin_last[mEnergy]; ++i_pt) // use rebinned pt
  {
    if(pt > vmsa::pt_low[mEnergy][i_pt] && pt <= vmsa::pt_up[mEnergy][i_pt])
    {
      h_mMcEffCos[cent][i_pt]->Fill(cos);
      if(cent >= vmsa::cent_low[0] && cent <= vmsa::cent_up[0])
      {//20-60%
	h_mMcEffCos[9][i_pt]->Fill(cos,vmsa::weight[cent]);
      }
    }
  }
}

void StEffHistManger::FillHistRc(int cent, float pt, float eta, float phi, float cos)
{
  h_mRcTracks[cent]->Fill(pt,eta,phi);
  if(cent >= vmsa::cent_low[0] && cent <= vmsa::cent_up[0])
  {//20-60%
    h_mRcTracks[9]->Fill(pt,eta,phi,vmsa::weight[cent]);
  }
  for(int i_pt = vmsa::pt_rebin_first[mEnergy]; i_pt < vmsa::pt_rebin_last[mEnergy]; ++i_pt) // use rebinned pt
  {
    if(pt > vmsa::pt_low[mEnergy][i_pt] && pt <= vmsa::pt_up[mEnergy][i_pt])
    {
      h_mRcEffCos[cent][i_pt]->Fill(cos);
      if(cent >= vmsa::cent_low[0] && cent <= vmsa::cent_up[0])
      {//20-60%
	h_mRcEffCos[9][i_pt]->Fill(cos,vmsa::weight[cent]);
      }
    }
  }
}

TH1D* StEffHistManger::CalEffError(TH1D *h_Mc, TH1D *h_Rc, std::string HistName)
{
  TH1D* h_ratio = (TH1D*)h_Rc->Clone();
  h_ratio->Divide(h_Mc);
  for(int i_bin = 1; i_bin < h_ratio->GetNbinsX()+1; ++i_bin)
  {
    double n = h_Mc->GetBinContent(i_bin);
    double k = h_Rc->GetBinContent(i_bin);
    double variance = (k+1.0)*(k+2.0)/((n+2.0)*(n+3.0))-(k+1.0)*(k+1.0)/((n+2.0)*(n+2.0));
    double sigma = TMath::Sqrt(variance);
    if(n > 0.0 && k > 0.0) h_ratio->SetBinError(i_bin,sigma);
  }
  h_ratio->SetName(HistName.c_str());

  return h_ratio;
}

void StEffHistManger::CalEfficiency()
{
  for(int i_cent = 0; i_cent < 10; ++i_cent)
  {
    std::string HistName;
    HistName = Form("h_mMcEffPt_Cent_%d",i_cent);
    h_mMcEffPt[i_cent] = (TH1D*)h_mMcTracks[i_cent]->Project3D("x")->Clone(HistName.c_str());
    HistName = Form("h_mRcEffPt_Cent_%d",i_cent);
    h_mRcEffPt[i_cent] = (TH1D*)h_mRcTracks[i_cent]->Project3D("x")->Clone(HistName.c_str());
    HistName = Form("h_mEffPt_Cent_%d",i_cent);
    h_mEffPt[i_cent] = CalEffError(h_mMcEffPt[i_cent],h_mRcEffPt[i_cent],HistName.c_str());

    HistName = Form("h_mMcEffEta_Cent_%d",i_cent);
    h_mMcEffEta[i_cent] = (TH1D*)h_mMcTracks[i_cent]->Project3D("y")->Clone(HistName.c_str());
    HistName = Form("h_mRcEffEta_Cent_%d",i_cent);
    h_mRcEffEta[i_cent] = (TH1D*)h_mRcTracks[i_cent]->Project3D("y")->Clone(HistName.c_str());
    HistName = Form("h_mEffEta_Cent_%d",i_cent);
    h_mEffEta[i_cent] = CalEffError(h_mMcEffEta[i_cent],h_mRcEffEta[i_cent],HistName.c_str());

    HistName = Form("h_mMcEffPhi_Cent_%d",i_cent);
    h_mMcEffPhi[i_cent] = (TH1D*)h_mMcTracks[i_cent]->Project3D("z")->Clone(HistName.c_str());
    HistName = Form("h_mRcEffPhi_Cent_%d",i_cent);
    h_mRcEffPhi[i_cent] = (TH1D*)h_mRcTracks[i_cent]->Project3D("z")->Clone(HistName.c_str());
    HistName = Form("h_mEffPhi_Cent_%d",i_cent);
    h_mEffPhi[i_cent] = CalEffError(h_mMcEffPhi[i_cent],h_mRcEffPhi[i_cent],HistName.c_str());
  }
  flag_eff = 1;
}

void StEffHistManger::CalEffPtEtaPhi()
{
  for(int i_cent = 0; i_cent < 10; ++i_cent)
  {
    for(int i_eta = 0; i_eta < vmsa::BinEta; ++i_eta)
    {
      for(int i_phi = 0; i_phi < vmsa::BinPhi; ++i_phi)
      {
	std::string HistNameMc = Form("h_mMcEff_Cent_%d_Eta_%d_Phi_%d",i_cent,i_eta,i_phi);
	h_mMcEffPEP[HistNameMc] = (TH1D*)h_mMcTracks[i_cent]->ProjectionX(HistNameMc.c_str(),i_eta+1,i_eta+1,i_phi+1,i_phi+1);

	std::string HistNameRc = Form("h_mRcEff_Cent_%d_Eta_%d_Phi_%d",i_cent,i_eta,i_phi);
	h_mRcEffPEP[HistNameRc] = (TH1D*)h_mRcTracks[i_cent]->ProjectionX(HistNameRc.c_str(),i_eta+1,i_eta+1,i_phi+1,i_phi+1);

	std::string HistNameEff = Form("h_mEff_Cent_%d_Eta_%d_Phi_%d",i_cent,i_eta,i_phi);
	h_mEffPEP[HistNameEff] = CalEffError(h_mMcEffPEP[HistNameMc],h_mRcEffPEP[HistNameRc],HistNameEff.c_str());
      }
    }
  }
  flag_eff_PtEtaPhi = 1;
}

void StEffHistManger::CalEffCosThetaStar()
{
  for(int i_cent = 0; i_cent < 10; ++i_cent)
  {
    for(int i_pt = vmsa::pt_rebin_first[mEnergy]; i_pt < vmsa::pt_rebin_last[mEnergy]; ++i_pt)
    {
      std::string HistName = Form("h_mEffCos_Cent_%d_Pt_%d",i_cent,i_pt);
      h_mEffCos[i_cent][i_pt] = CalEffError(h_mMcEffCos[i_cent][i_pt],h_mRcEffCos[i_cent][i_pt],HistName.c_str());
    }
  }
  flag_eff_Cos = 1;
}

void StEffHistManger::WriteHist()
{
  for(int i_cent = 0; i_cent < 10; ++i_cent)
  {
    h_mMcTracks[i_cent]->Write();
    h_mRcTracks[i_cent]->Write();

    if(flag_eff > 0.5)
    {
      h_mEffPt[i_cent]->Write();
      h_mEffEta[i_cent]->Write();
      h_mEffPhi[i_cent]->Write();
    }

    if(flag_eff_PtEtaPhi > 0.5)
    {
      for(int i_eta = 0; i_eta < vmsa::BinEta; ++i_eta)
      {
	for(int i_phi = 0; i_phi < vmsa::BinPhi; ++i_phi)
	{
	  std::string HistNameEff = Form("h_mEff_Cent_%d_Eta_%d_Phi_%d",i_cent,i_eta,i_phi);
	  h_mEffPEP[HistNameEff]->Write();
	}
      }
    }

    if(flag_eff_Cos > 0.5)
    {
      for(int i_pt = vmsa::pt_rebin_first[mEnergy]; i_pt < vmsa::pt_rebin_last[mEnergy]; ++i_pt)
      {
	// h_mMcEffCos[i_cent][i_pt]->Write();
	// h_mRcEffCos[i_cent][i_pt]->Write();
	h_mEffCos[i_cent][i_pt]->Write();
      }
    }
  }
}
