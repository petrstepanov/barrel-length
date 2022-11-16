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
/// \file persistency/gdml/G01/load_gdml.cc
/// \brief Main program of the persistency/gdml/G01 example
//
//
//
//
// --------------------------------------------------------------
//      GEANT 4 - load_gdml
//
// --------------------------------------------------------------

#include <vector>

#include "G4Types.hh"

#include "G4RunManagerFactory.hh"

#include "G4UImanager.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4TransportationManager.hh"

#include "G01PrimaryGeneratorAction.hh"
#include "G01DetectorConstruction.hh"
#include "G01ActionInitialization.hh"

#include "FTFP_BERT.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "G4GDMLParser.hh"

// --------------------------------------------------------------

int main(int argc, char **argv) {
  G4cout << "Usage: barrel <intput_gdml_file:mandatory>" << " <output_gdml_file:optional>" << G4endl;

  if (argc <= 1) {
    G4cout << "Error! GDML input file is not specified!" << G4endl;
    return -1;
  }

  G4GDMLParser* parser = new G4GDMLParser();

  // Uncomment below to avoid names stripping
  // parser->SetStripFlag(false);

  parser->SetOverlapCheck(true);
  parser->Read(argv[1]);

  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

  runManager->SetUserInitialization(new G01DetectorConstruction(parser));
  runManager->SetUserInitialization(new FTFP_BERT);
  runManager->SetUserInitialization(new G01ActionInitialization());
  runManager->Initialize();

  // Initialize visualization
  G4VisManager *visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager *UImanager = G4UImanager::GetUIpointer();

  // Set macros folder path
  UImanager->ApplyCommand("/control/macroPath macros");

  // Process macro or start UI session
  if (argc > 2) {
    // Batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[2];
    UImanager->ApplyCommand(command + fileName);
  }
  else {
    // Interactive mode
    G4UIExecutive *ui = new G4UIExecutive(argc, argv);
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;

  return 0;
}
