//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"
#include "Constants.hh"
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include <G4RunManager.hh>
#include <G4Run.hh>
#include <G4AccumulableManager.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4LogicalVolume.hh>
#include <G4UnitsTable.hh>
#include <G4SystemOfUnits.hh>
#include <G4AnalysisManager.hh>
#include <chrono>
#include <iostream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction() {
  // add new units for dose
  //
//  const G4double milligray = 1.e-3 * gray;
//  const G4double microgray = 1.e-6 * gray;
//  const G4double nanogray = 1.e-9 * gray;
//  const G4double picogray = 1.e-12 * gray;
//
//  new G4UnitDefinition("milligray", "milliGy", "Dose", milligray);
//  new G4UnitDefinition("microgray", "microGy", "Dose", microgray);
//  new G4UnitDefinition("nanogray", "nanoGy", "Dose", nanogray);
//  new G4UnitDefinition("picogray", "picoGy", "Dose", picogray);

  // Register accumulable to the accumulable manager
  G4AccumulableManager *accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->RegisterAccumulable(depositedEnergies);

  // Create analysis manager
  // Example with merging ntuples copied from B5
  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true);

  G4String fileName = "edep";
  fileName += Constants::GetInstance()->getOutputFileSuffix();
  analysisManager->SetFileName(fileName);

  // Open an output file "edep-####.root"
  // G4String fileName = "edep";
  // auto t = std::time(nullptr);
  // auto tm = *std::localtime(&t);
  // std::ostringstream oss;
  // oss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S");
  // fileName += oss.str();
  // fileName += ".root";

  // Create ntuple
  analysisManager->CreateNtuple("barrel", "Values per Geant4 event");
  analysisManager->CreateNtupleDColumn("edep");
  analysisManager->CreateNtupleIColumn("eventNumber");
  analysisManager->FinishNtuple();
  // analysisManager->CreateH1("edepHist", "Energy Deposition", run->GetNumberOfEvent(), 0, run->GetNumberOfEvent());

  // analysisManager->SetNtupleFileName(0, "ntuple");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction() {
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*) {
  // Inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  // Reset accumulables to their initial values
  G4AccumulableManager *accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();

  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Reset histograms from previous run
  analysisManager->Reset();

  // Open an output file
  // The default file name is set in RunAction::RunAction(), it can be overwritten in a macro
  analysisManager->OpenFile();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run *run) {
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0)
    return;

  // Merge accumulables
  G4AccumulableManager *accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  // Run conditions
  const PrimaryGeneratorAction *generatorAction =
      static_cast<const PrimaryGeneratorAction*>(G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  if (generatorAction) {
    const G4GeneralParticleSource *particleGun = generatorAction->GetParticleGun();
    runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    runCondition += " of ";
    G4double particleEnergy = particleGun->GetParticleEnergy();
    runCondition += G4BestUnit(particleEnergy, "Energy");
  }

  // Print output
  if (IsMaster()) {
    G4cout << G4endl << "------------------- End of Global Run ----------------------";
  } else {
    G4cout << G4endl << "------------------- End of Local Run -----------------------";
  }
  G4int eTotal = depositedEnergies.GetValue();
  G4cout << G4endl << " Run consists of " << nofEvents << " " << runCondition << G4endl
         << " Total energy depositred in crystals: " << G4BestUnit(eTotal, "Energy") << G4endl
         << "------------------------------------------------------------" << G4endl << G4endl;

  // Write output ROOT file
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile(false);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::AddEnergyPerEvent(G4double energyPerEvent) {
  depositedEnergies += energyPerEvent;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
