
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

// MiniAOD DataFormats
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

// FastJet Headers
#include <fastjet/ClusterSequence.hh>
#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>

#include <memory>
#include <vector>

class AK1ReclusterProducer : public edm::stream::EDProducer<> {
public:
explicit AK1ReclusterProducer(const edm::ParameterSet& iConfig);
~AK1ReclusterProducer() override = default;

private:
void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override;

edm::EDGetTokenT<pat::JetCollection> jetToken_;
edm::EDGetTokenT<pat::PackedCandidateCollection> pfcandToken_;
edm::EDGetTokenT<reco::VertexCollection> vtxToken_;

double jetR_;
double minPt_;
bool useAK4ConstituentsOnly_;
bool applyCHS_;
};

AK1ReclusterProducer::AK1ReclusterProducer(const edm::ParameterSet& iConfig)
: jetToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("jets"))),
pfcandToken_(consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("pfCandidates"))),
vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
jetR_(iConfig.getUntrackedParameter<double>("jetR", 0.1)),
minPt_(iConfig.getUntrackedParameter<double>("minPt", 3.0)),
useAK4ConstituentsOnly_(iConfig.getParameter<bool>("useAK4ConstituentsOnly")),
applyCHS_(iConfig.getParameter<bool>("applyCHS")) {

produces<pat::JetCollection>();
}

void AK1ReclusterProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
auto reclusteredJets = std::make_unique<pat::JetCollection>();

// Fetch Primary Vertices for CHS checks
edm::Handle<reco::VertexCollection> vertices;
iEvent.getByToken(vtxToken_, vertices);

std::vector<fastjet::PseudoJet> fjInputs;

if (useAK4ConstituentsOnly_) {
// Mode 1: Recluster constituents of existing AK4 MiniAOD jets (e.g., slimmedJets)
edm::Handle<pat::JetCollection> jets;
iEvent.getByToken(jetToken_, jets);

for (const auto& jet : *jets) {
for (unsigned int i = 0; i < jet.numberOfDaughters(); ++i) {
const auto* cand = dynamic_cast<const pat::PackedCandidate*>(jet.daughter(i));
if (!cand) continue;

// Apply CHS: Ignore charged hadrons coming from secondary vertices (PV association != 0)
if (applyCHS_ && cand->charge() != 0) {
if (cand->fromPV() < pat::PackedCandidate::PVUsedInFit) continue;
}

fastjet::PseudoJet pjet(cand->px(), cand->py(), cand->pz(), cand->energy());
pjet.set_user_index(i);
fjInputs.push_back(pjet);
}
}
} else {
// Mode 2: Recluster from ALL event-wide packed PFCandidates
edm::Handle<pat::PackedCandidateCollection> pfcands;
iEvent.getByToken(pfcandToken_, pfcands);

for (size_t i = 0; i < pfcands->size(); ++i) {
const auto& cand = pfcands->at(i);

if (cand.pt() < 0.3) continue; // Basic noise floor cut

// Apply CHS
if (applyCHS_ && cand.charge() != 0) {
if (cand.fromPV() < pat::PackedCandidate::PVUsedInFit) continue;
}

fastjet::PseudoJet pjet(cand.px(), cand.py(), cand.pz(), cand.energy());
pjet.set_user_index(i);
fjInputs.push_back(pjet);
}
}

// Setup FastJet Anti-kT R=0.1
fastjet::JetDefinition jetDef(fastjet::antikt_algorithm, jetR_);
fastjet::ClusterSequence clustSeq(fjInputs, jetDef);

// Cluster and sort by pT
std::vector<fastjet::PseudoJet> inclusiveJets = fastjet::sorted_by_pt(clustSeq.inclusive_jets(minPt_));

// Convert FastJet output back to pat::Jets
for (const auto& fjJet : inclusiveJets) {
reco::Candidate::LorentzVector p4(fjJet.px(), fjJet.py(), fjJet.pz(), fjJet.e());

pat::Jet patJet;
patJet.setP4(p4);

reclusteredJets->push_back(patJet);
}

iEvent.put(std::move(reclusteredJets));
}

DEFINE_FWK_MODULE(AK1ReclusterProducer);