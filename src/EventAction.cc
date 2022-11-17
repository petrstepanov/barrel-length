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
/// \file EventAction.cc
/// \brief Implementation of the B1::EventAction class

#include "EventAction.hh"
#include "RunAction.hh"

// #include <GEvent.hh>
#include <G4RunManager.hh>
#include <G4AnalysisManager.hh>
#include <G4SDManager.hh>
#include <G4THitsMap.hh>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction *runAction) : fRunAction(runAction) {
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction() {
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*) {
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event *event) {
  // Get hits colection
  G4HCofThisEvent *HCE = event->GetHCofThisEvent();
  if (!HCE)
    return;

  // Obtain the hits collection ID
  G4int eDepCollectionId = G4SDManager::GetSDMpointer()->GetCollectionID("crystals/edep");

  // Get event map
  G4THitsMap<G4double> *eventMap = static_cast<G4THitsMap<G4double>*>(HCE->GetHC(eDepCollectionId));

  //
  G4double depositedEnergyPerEvent = 0;
  std::map<G4int, G4double*>::iterator itr;

  G4cout << "Event number: " << event->GetEventID() << G4endl;
  for (itr = eventMap->GetMap()->begin(); itr != eventMap->GetMap()->end(); itr++) {
    G4int copyNumber = itr->first;
    depositedEnergyPerEvent += *(itr->second);
    G4cout << "  Crystal Scorer, copy N " << copyNumber << ": total E " << depositedEnergyPerEvent << " MeV." << G4endl;
  }

  // Accumulate statistics in run action
  fRunAction->AddEnergyPerEvent(depositedEnergyPerEvent);

  // Populate analysis manager
  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
  analysisManager->FillNtupleDColumn(0, depositedEnergyPerEvent);
  analysisManager->FillNtupleIColumn(1, event->GetEventID());
  analysisManager->AddNtupleRow();

  // analysisManager->FillH1(event->GetEventID(), depositedEnergyPerEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
