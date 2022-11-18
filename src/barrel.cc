#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include <G4Types.hh>
#include <G4RunManagerFactory.hh>
#include <G4UImanager.hh>
#include <FTFP_BERT.hh>
#include <QBBC.hh>
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>
#include <G4GDMLParser.hh>

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

  // G4RunManager* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);
  G4RunManager* runManager = G4RunManagerFactory::CreateRunManager();

  runManager->SetUserInitialization(new DetectorConstruction(parser));
  // G4VModularPhysicsList* physicsList = new FTFP_BERT();
  G4VModularPhysicsList* physicsList = new QBBC();
  physicsList->SetVerboseLevel(1);
  runManager->SetUserInitialization(physicsList);
  runManager->SetUserInitialization(new ActionInitialization());
  runManager->Initialize();

  // Initialize visualization
  G4VisManager *visManager = new G4VisExecutive();
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
