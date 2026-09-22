#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <memory>

class EventIdProducer : public edm::stream::EDProducer<> {
public:
  explicit EventIdProducer(const edm::ParameterSet&) {
    produces<int>("run");
    produces<int>("event");
  }
  ~EventIdProducer() override = default;

  void produce(edm::Event& iEvent, const edm::EventSetup&) override {
    auto runp = std::make_unique<int>(static_cast<int>(iEvent.id().run()));
    auto evp  = std::make_unique<int>(static_cast<int>(iEvent.id().event()));
    iEvent.put(std::move(runp), "run");
    iEvent.put(std::move(evp),  "event");
  }
};

DEFINE_FWK_MODULE(EventIdProducer);
