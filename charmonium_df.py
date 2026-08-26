import matplotlib.pyplot as plt
import awkward as ak
import pandas as pd
import awkward_pandas as ap
import uproot
import vector as vec

filename = './30552/5BBBE83E-DEA6-934C-9BB4-A88DC319F3A1.root'
event_file = uproot.open(f'{filename}:Events')

muon_data = event_file.arrays(["run", "event", "Muon_pt", "Muon_eta", "Muon_phi", "Muon_charge", "nMuon"])
jet_data = event_file.arrays(["run", "event", "luminosityBlock","Jet_mass", "Jet_pt", "Jet_eta","Jet_phi", "Jet_bRegCorr", "Jet_bRegRes", "Jet_btagCSVV2", "Jet_btagDeepB", "Jet_btagDeepCvB", "Jet_btagDeepCvL", 'Jet_btagDeepFlavB', 'Jet_btagDeepFlavCvB', 'Jet_btagDeepFlavCvL', 'Jet_btagDeepFlavQG'])

mu_df = ak.to_dataframe(muon_data)

pos_mu = mu_df.loc[mu_df["Muon_charge"]>0]
pos_mu.rename(columns = {"Muon_pt":"Mupos_pt", "Muon_eta":"Mupos_eta", "Muon_phi":"Mupos_phi", "Muon_charge":"Mupos_charge", "nMuon":"nmu_drop"}, inplace = True)

neg_mu = mu_df.loc[mu_df["Muon_charge"]<0]
neg_mu.rename(columns = {"Muon_pt":"Muneg_pt", "Muon_eta":"Muneg_eta", "Muon_phi":"Muneg_phi", "Muon_charge":"Muneg_charge"}, inplace = True)

mu_df = pos_mu.merge(neg_mu, on=["run", "event"])
mu_df = mu_df.drop(["nmu_drop"], axis = 1)

jet_df = ak.to_dataframe(jet_data)
jet_df.head()

all_df = pd.merge(mu_df, jet_df, on=["run", "event"])
all_df.dropna(inplace = True)
all_df.insert(loc = 2, value = 0.105658, column = 'Mu_mass')

pos_mu_vec = vec.array({"pt":all_df["Mupos_pt"], "eta":all_df["Mupos_eta"], "phi":all_df["Mupos_phi"], "mass":all_df["Mu_mass"]})
neg_mu_vec = vec.array({"pt":all_df["Muneg_pt"], "eta":all_df["Muneg_eta"], "phi":all_df["Muneg_phi"], "mass":all_df["Mu_mass"]})
jet_vec = vec.array({"pt":all_df["Jet_pt"], "eta":all_df["Jet_eta"], "phi":all_df["Jet_phi"], "mass":all_df["Jet_mass"]})

jpsi_vec = pos_mu_vec + neg_mu_vec
wc_vec = jpsi_vec + jet_vec

all_df.insert(loc = 26, column = 'jpsi_mass', value = jpsi_vec.M)
all_df.insert(loc = 27, column = 'jpsi_pt', value = jpsi_vec.pt)
all_df.insert(loc = 28, column = 'jpsi_eta', value = jpsi_vec.eta)
all_df.insert(loc = 29, column = 'jpsi_phi', value = jpsi_vec.phi)
all_df.insert(loc = 30, column = 'wc_mass', value = wc_vec.M)
all_df.insert(loc = 31, column = 'wc_pt', value = wc_vec.pt)
all_df.insert(loc = 32, column = 'wc_eta', value = wc_vec.eta)
all_df.insert(loc = 33, column = 'wc_phi', value = wc_vec.phi)

all_df1 = all_df.loc[all_df['Jet_mass']<=4.5]

all_df2 = all_df1.loc[(all_df1["wc_mass"]>40) & (all_df1["wc_mass"]<1000)]

all_df3 = all_df2.loc[all_df2.jpsi_mass < 4.9]
all_df3 = all_df3.loc[all_df3.jpsi_mass > 2.9]

all_df4 = all_df1.loc[all_df1.jpsi_mass>2.9]
all_df4 = all_df4.loc[all_df4.jpsi_mass<4.9]
all_df4 = all_df4.loc[all_df4.nMuon == 2]
all_df4 = all_df4.loc[all_df4.wc_mass < 200]

all_df5 = all_df.loc[(all_df.jpsi_mass>2.9) & (all_df.jpsi_mass<4.9) & (all_df.Jet_mass < 3.5)]
all_df5 = all_df5.loc[(all_df5.nMuon == 2) | (all_df5.nMuon == 3)]

all_df6 = all_df5.loc[all_df5.Jet_btagCSVV2<-2]

all_df7 = all_df5.loc[all_df5.Jet_btagCSVV2>-2]

csv_df = all_df.loc[(all_df.jpsi_mass >= 2.9)
                     & (all_df.jpsi_mass <= 4.1) 
                     & (all_df.Jet_eta>=-2.5) 
                     & (all_df.Jet_eta<=2.5) 
                     & (all_df.Jet_mass<=4.5) 
                     & (all_df.wc_mass>=50)]

csv_df.to_csv("charm_df.csv", index=False)
