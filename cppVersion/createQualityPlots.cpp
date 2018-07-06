//
//  createQualityPlots.cpp
//
//  Created by Jeremi Niedziela on 28/06/2018.
//

#include "Event.hpp"
#include "RecHitCalibration.hpp"
#include "ImagingAlgo.hpp"
#include "Helpers.hpp"
#include "ConfigurationManager.hpp"
#include "ClusterMatcher.hpp"

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>

#include <string>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <utility>
#include <memory>

using namespace std;

int main(int argc, char* argv[])
{
  if(argc != 2){
    cout<<"Usage: createQualityPlots path_to_config"<<endl;
    exit(0);
  }
  string configPath(argv[1]);
  ConfigurationManager *config = ConfigurationManager::Instance(configPath);
  
  gROOT->ProcessLine(".L loader.C+");
  
  std::system(("mkdir -p "+config->GetOutputPath()).c_str());
  
  ImagingAlgo *algo = new ImagingAlgo();
  ClusterMatcher *matcher = new ClusterMatcher();
  
  for(int nTupleIter=config->GetMinNtuple();nTupleIter<=config->GetMaxNtuple();nTupleIter++){
    cout<<"\nCurrent ntup: "<<nTupleIter<<endl;
    
    TFile *inFile = TFile::Open(Form("%s%i.root",config->GetInputPath().c_str(),nTupleIter));
    if(!inFile) continue;
    TTree *tree = (TTree*)inFile->Get("ana/hgc");
    if(!tree) continue;
    long long nEvents = tree->GetEntries();
    cout<<"n entries:"<<nEvents<<endl;
    
    unique_ptr<Event> hgCalEvent(new Event(tree));
    
    // start event loop
    for(int iEvent=0;iEvent<nEvents;iEvent++){
      if(iEvent>config->GetMaxEventsPerTuple()) break;
      
      hgCalEvent->GoToEvent(iEvent);
      
      // check if particles reached EE
      if(config->GetReachedEEonly()){
        bool skipEvent = false;
        for(auto reachedEE : *(hgCalEvent->GetGenParticles()->GetReachedEE())){
          if(reachedEE==0){
            skipEvent = true;
            break;
          }
        }
        if(skipEvent) continue;
      }
      string eventDir = config->GetOutputPath()+"/ntup"+to_string(nTupleIter)+"/event"+to_string(iEvent);
      std::system(("mkdir -p "+eventDir).c_str());
      
      cout<<"\nCurrent event:"<<iEvent<<endl;
      
      shared_ptr<RecHits> recHitsRaw = hgCalEvent->GetRecHits();
      shared_ptr<SimClusters> simClusters = hgCalEvent->GetSimClusters();
      
      // get simulated hits associated with a cluster
      vector<RecHits*> simHitsPerClusterArray;
      recHitsRaw->GetHitsPerSimCluster(simHitsPerClusterArray, simClusters);
      
      // re-run clustering with HGCalAlgo
      std::vector<shared_ptr<Hexel>> recClusters;
      algo->getRecClusters(recClusters, recHitsRaw);
      
      vector<RecHits*> recHitsPerClusterArray;
      recHitsRaw->GetRecHitsPerHexel(recHitsPerClusterArray, recClusters);
    
      
      // perform final analysis, fill in histograms and save to files
      TH2D *ErecEsimVsEta = new TH2D("ErecEsim vs. eta","ErecEsim vs. eta",100,1.5,3.2,100,0,2.5);
      TH2D *sigmaEvsEta = new TH2D("sigma(E) vs. eta","sigma(E) vs. eta",100,1.5,3.2,100,-10,10);
      TH2D *sigmaEvsEtaEsim = new TH2D("sigma(E)Esim vs. eta","sigma(E)Esim vs. eta",100,1.5,3.2,500,-1,4);
      TH1D *twoSeparation = new TH1D("two clusters separation","two clusters separation",1000,0,10);
      
      for(int layer=config->GetMinLayer();layer<config->GetMaxLayer();layer++){
        
        vector<MatchedClusters*> matchedClusters;
        matcher->MatchClustersClosest(matchedClusters,recHitsPerClusterArray,simHitsPerClusterArray,layer);
        
        for(MatchedClusters *clusters : matchedClusters){
          double recEnergy = clusters->recCluster->GetEnergy();
          double recEta    = clusters->recCluster->GetEta();
          double simEnergy = clusters->GetTotalSimEnergy();
          
          ErecEsimVsEta->Fill(fabs(recEta),recEnergy/simEnergy);
          sigmaEvsEta->Fill(fabs(recEta),(recEnergy-simEnergy)/recEnergy);
          sigmaEvsEtaEsim->Fill(fabs(recEta),(recEnergy-simEnergy)/simEnergy);
        }
        for(uint i=0;i<matchedClusters.size();i++){
          for(uint j=i;j<matchedClusters.size();j++){
            BasicCluster *recCluster1 = matchedClusters[i]->recCluster;
            BasicCluster *recCluster2 = matchedClusters[j]->recCluster;
            
            double distance = sqrt(pow(recCluster1->GetX()-recCluster2->GetX(),2)+
                                   pow(recCluster1->GetY()-recCluster2->GetY(),2));
            
            double sigma1 = recCluster1->GetRadius();
            double sigma2 = recCluster2->GetRadius();
            
            twoSeparation->Fill(distance/sqrt(sigma1*sigma1+sigma2*sigma2));
          }
        }
      }
      ErecEsimVsEta->SaveAs(Form("%s/ErecEsimVsEta.root",eventDir.c_str()));
      sigmaEvsEta->SaveAs(Form("%s/simgaEVsEta.root",eventDir.c_str()));
      sigmaEvsEtaEsim->SaveAs(Form("%s/simgaEVsEtaEsim.root",eventDir.c_str()));
      twoSeparation->SaveAs(Form("%s/twoSeparation.root",eventDir.c_str()));
      
      recHitsPerClusterArray.clear();
      simHitsPerClusterArray.clear();
      
    }
    delete tree;
    inFile->Close();
    delete inFile;
    
  }
  delete algo;
  delete matcher;
  return 0;
}

