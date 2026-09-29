// muon producer to store muon information similar to ak1ReclusterProducer.cc 
// produces standard muon object with no modification pushes muon charge and isGlobal as userint to the muon object
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

// MiniAOD DataFormats
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include <memory>
#include <vector>
class muonProducer : public edm::stream::EDProducer<> {
public:
   explicit muonProducer(const edm::ParameterSet& iConfig);
   ~muonProducer() override = default;
private:
   void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
      auto muons = std::make_unique<pat::MuonCollection>();
      edm::Handle<pat::MuonCollection> muonHandle;
      iEvent.getByToken(muonToken_, muonHandle);
      for (const auto& muon : *muonHandle) {
         muons->push_back(muon);
         muons->back().addUserInt("charge", muon.charge());
         muons->back().addUserInt("isGlobal", muon.isGlobalMuon());
      }
      iEvent.put(std::move(muons));
   }
   edm::EDGetTokenT<pat::MuonCollection> muonToken_;
   bool applyCHS_;
};

muonProducer::muonProducer(const edm::ParameterSet& iConfig)
: muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons"))),
  applyCHS_(iConfig.getParameter<bool>("applyCHS")) {
   produces<pat::MuonCollection>();

}



DEFINE_FWK_MODULE(muonProducer);