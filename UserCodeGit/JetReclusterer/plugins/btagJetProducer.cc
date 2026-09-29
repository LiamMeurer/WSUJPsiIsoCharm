// producer for applying b-tagging information to slimmed jets 
// produces a new jet collection with btagging information with similar output to ak1ReclusterProducer.cc
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
//MiniAOD DataFormats
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
//fastjet headers
#include <fastjet/ClusterSequence.hh>
#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>
#include <memory>
#include <vector>
class btagJetProducer : public edm::stream::EDProducer<> {
public:
  explicit btagJetProducer(const edm::ParameterSet& iConfig);

  ~btagJetProducer() override = default;

private:
  void produce(edm::Event& iEvent, const edm::EventSetup&) override;

  edm::EDGetTokenT<pat::JetCollection> jetToken_;
};

btagJetProducer::btagJetProducer(const edm::ParameterSet& iConfig)
    : jetToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jets"))) {
  produces<pat::JetCollection>();
}

void btagJetProducer::produce(edm::Event& iEvent, const edm::EventSetup&) {
  edm::Handle<pat::JetCollection> jetHandle;
  iEvent.getByToken(jetToken_, jetHandle);

  auto jets = std::make_unique<pat::JetCollection>();
  for (const auto& jet : *jetHandle) {
    jets->push_back(jet);
    jets->back().addUserFloat("btagDeepFlavB",
        jet.bDiscriminator("pfDeepFlavourJetTags:probb") +
        jet.bDiscriminator("pfDeepFlavourJetTags:probbb"));
    jets->back().addUserFloat("deepCSV_probb",
        jet.bDiscriminator("pfDeepCSVJetTags:probb"));
    jets->back().addUserFloat("deepCSV_probbb",
        jet.bDiscriminator("pfDeepCSVJetTags:probbb"));
    jets->back().addUserFloat("deepCSV_probc",
        jet.bDiscriminator("pfDeepCSVJetTags:probc"));
  }

  iEvent.put(std::move(jets));
}

DEFINE_FWK_MODULE(btagJetProducer);
