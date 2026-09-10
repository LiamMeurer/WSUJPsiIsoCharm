#runs jet reclustering on miniAOD samples, using the ak1 reclustering module and writes the result into a nanoaod file
import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.NanoAODEDMEventContent_cff import NanoAODEDMEventContent
from PhysicsTools.NanoAOD.common_cff import Var, ExtVar
process = cms.Process("RECLUSTER")
process.options = cms.untracked.PSet(
    SkipEvent = cms.untracked.vstring('ProductNotFound'),
)
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

# load geometry/conditions so jet corrections and b-tag records are available
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['run2_data']
#multithreading (keep single-thread for testing)
process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(8),
    numberOfStreams = cms.untracked.uint32(8)
)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

# Open Data 2016 MiniAOD sample
process.source = cms.Source("PoolSource",
fileNames = cms.untracked.vstring(
'root://eospublic.cern.ch//eos/opendata/cms/Run2016H/Charmonium/MINIAOD/UL2016_MiniAODv2-v2/100000/070EB912-8A19-9649-A335-42AC81ACA6EB.root'
)
)

# Load your custom module
process.reclusterAK1 = cms.EDProducer('AK1ReclusterProducer',
    jets = cms.InputTag("slimmedJets"),
    pfCandidates = cms.InputTag("packedPFCandidates"),
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    jetR = cms.untracked.double(0.1),
    minPt = cms.untracked.double(3.0),
    useAK4ConstituentsOnly = cms.bool(True), # Set to False to recluster full event
    applyCHS = cms.bool(True) # Remove pileup charged tracks
)
# small EventId producer and event-level flat table
process.eventIdProducer = cms.EDProducer("EventIdProducer")

process.eventTable = cms.EDProducer("GlobalVariablesTableProducer",
    name = cms.string("Event"),
    extension = cms.bool(False),
    variables = cms.PSet(
        runNumber = ExtVar(cms.InputTag("eventIdProducer","run"), int, doc = "Run number"),
        eventNumber = ExtVar(cms.InputTag("eventIdProducer","event"), int, doc = "Event number"),
    )
)

# save custom output without running the full NanoAOD sequence
# This avoids NanoAOD producers that require missing EventSetup records
# for MiniAOD data input.
# run the NanoAOD jet sequence to produce `updatedJets` and related products
from PhysicsTools.NanoAOD.jets_cff import jetSequence
process.load("PhysicsTools.NanoAOD.nano_cff")
if hasattr(process, 'updatedJets'):
    process.updatedJets.addBTagInfo = True
#write btagged from updatedJets to output btagging variables at the event level
process.btagTable = cms.EDProducer("JetBTagFlatTableProducer",
    src = cms.InputTag("updatedJetsWithUserData"),
    cut = cms.string(""),
    name = cms.string("BTag"),
    doc  = cms.string("BTag info for updated jets"),
    singleton = cms.bool(False),
    extension = cms.bool(False),
)
#write slimmed muon object data
process.muonTable = cms.EDProducer("MuonFlatTableProducer",
    src = cms.InputTag("slimmedMuons"),
    cut = cms.string(""),
    name = cms.string("Muon"),
    doc  = cms.string("Muon info for slimmed muons"),
    singleton = cms.bool(False),
    extension = cms.bool(False),
)
# schedule: produce updated jets, recluster AK1, eventId and event table
process.p = cms.Path(jetSequence + process.reclusterAK1 + process.eventIdProducer + process.eventTable + process.btagTable  + process.muonTable)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('nanoAOD_reclustered_AK1.root'),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_reclusterAK1_*_*',
        'keep *_patJets*_*_*',
        'keep *_slimmedJets_*_*',
        'keep *_updatedJetsWithUserData_*_*',
        'keep *_btagTable_*_*',
        'keep *_muonTable_*_*',
        'keep *_finalJets_*_*',
        'keep *_eventIdProducer_*_*',
    )
)
process.e = cms.EndPath(process.out)
