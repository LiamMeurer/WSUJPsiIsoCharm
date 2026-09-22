#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include <memory>
#include <string>
#include <vector>

class JetBTagFlatTableProducer : public edm::stream::EDProducer<> {
public:
  explicit JetBTagFlatTableProducer(const edm::ParameterSet& iConfig)
      : srcToken_(consumes<pat::JetCollection>(iConfig.getParameter<edm::InputTag>("src"))),
        name_(iConfig.getParameter<std::string>("name")),
        doc_(iConfig.getParameter<std::string>("doc")),
        singleton_(iConfig.getParameter<bool>("singleton")),
        extension_(iConfig.getParameter<bool>("extension")) {
    produces<nanoaod::FlatTable>();
  }

  ~JetBTagFlatTableProducer() override = default;

private:
  void produce(edm::Event& iEvent, const edm::EventSetup&) override {
    edm::Handle<pat::JetCollection> jets;
    iEvent.getByToken(srcToken_, jets);

    auto table = std::make_unique<nanoaod::FlatTable>(jets->size(), name_, singleton_, extension_);
    table->setDoc(doc_);

    std::vector<float> deepFlavB;
    std::vector<float> deepCSVProbb;
    std::vector<float> deepCSVProbbb;
    std::vector<float> deepCSVProbc;
    deepFlavB.reserve(jets->size());
    deepCSVProbb.reserve(jets->size());
    deepCSVProbbb.reserve(jets->size());
    deepCSVProbc.reserve(jets->size());

    for (const auto& jet : *jets) {
      deepFlavB.push_back(
          static_cast<float>(jet.bDiscriminator("pfDeepFlavourJetTags:probb") +
                             jet.bDiscriminator("pfDeepFlavourJetTags:probbb")));
      deepCSVProbb.push_back(static_cast<float>(jet.bDiscriminator("pfDeepCSVJetTags:probb")));
      deepCSVProbbb.push_back(static_cast<float>(jet.bDiscriminator("pfDeepCSVJetTags:probbb")));
      deepCSVProbc.push_back(static_cast<float>(jet.bDiscriminator("pfDeepCSVJetTags:probc")));
    }

    table->template addColumn<float>("btagDeepFlavB", deepFlavB, "DeepJet b-tag discriminator", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<float>("deepCSV_probb", deepCSVProbb, "DeepCSV probb discriminator", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<float>("deepCSV_probbb", deepCSVProbbb, "DeepCSV probbb discriminator", nanoaod::FlatTable::FloatColumn, 10);
    table->template addColumn<float>("deepCSV_probc", deepCSVProbc, "DeepCSV probc discriminator", nanoaod::FlatTable::FloatColumn, 10);

    iEvent.put(std::move(table));
  }

  edm::EDGetTokenT<pat::JetCollection> srcToken_;
  std::string name_;
  std::string doc_;
  bool singleton_;
  bool extension_;
};

DEFINE_FWK_MODULE(JetBTagFlatTableProducer);
