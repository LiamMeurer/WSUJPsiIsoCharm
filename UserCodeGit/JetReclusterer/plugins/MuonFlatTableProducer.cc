#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include <memory>
#include <string>
#include <vector>

class MuonFlatTableProducer : public edm::stream::EDProducer<> {
public:
  explicit MuonFlatTableProducer(const edm::ParameterSet& iConfig)
      : srcToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("src"))),
        name_(iConfig.getParameter<std::string>("name")),
        doc_(iConfig.getParameter<std::string>("doc")),
        singleton_(iConfig.getParameter<bool>("singleton")),
        extension_(iConfig.getParameter<bool>("extension")) {
    produces<nanoaod::FlatTable>();
  }

  ~MuonFlatTableProducer() override = default;

private:
  void produce(edm::Event& iEvent, const edm::EventSetup&) override {
    edm::Handle<pat::MuonCollection> muons;
    iEvent.getByToken(srcToken_, muons);

    auto table = std::make_unique<nanoaod::FlatTable>(muons->size(), name_, singleton_, extension_);
    table->setDoc(doc_);

    std::vector<float> pt, eta, phi, mass;
    std::vector<int> charge, isGlobal;
    pt.reserve(muons->size());
    eta.reserve(muons->size());
    phi.reserve(muons->size());
    mass.reserve(muons->size());
    charge.reserve(muons->size());
    isGlobal.reserve(muons->size());

    for (const auto& muon : *muons) {
      pt.push_back(muon.pt());
      eta.push_back(muon.eta());
      phi.push_back(muon.phi());
      mass.push_back(muon.mass());
      charge.push_back(muon.charge());
      isGlobal.push_back(muon.isGlobalMuon() ? 1 : 0);
    }

    table->template addColumn<float>("pt", pt, "transverse momentum", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<float>("eta", eta, "pseudorapidity", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<float>("phi", phi, "azimuthal angle", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<float>("mass", mass, "mass", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<int>("charge", charge, "electric charge", nanoaod::FlatTable::IntColumn);
    table->template addColumn<int>("isGlobal", isGlobal, "global muon flag", nanoaod::FlatTable::IntColumn);

    iEvent.put(std::move(table));
  }

  edm::EDGetTokenT<pat::MuonCollection> srcToken_;
  std::string name_;
  std::string doc_;
  bool singleton_;
  bool extension_;
};

DEFINE_FWK_MODULE(MuonFlatTableProducer);
