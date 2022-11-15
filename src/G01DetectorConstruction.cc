#include "G01DetectorConstruction.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4LogicalVolumeStore.hh"
//#include "G4LogicalVolume.hh"

void print_aux(const G4GDMLAuxListType *auxInfoList, G4String prepend = "|") {
  for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin(); iaux != auxInfoList->end();
      iaux++) {
    G4String str = iaux->type;
    G4String val = iaux->value;
    G4String unit = iaux->unit;

    G4cout << prepend << str << " : " << val << " " << unit << G4endl;

    if (iaux->auxList)
      print_aux(iaux->auxList, prepend + "|");
  }
  return;
}

G01DetectorConstruction::G01DetectorConstruction(G4GDMLParser *parser) {
  fParser = parser;
  fWorld = fParser->GetWorldVolume();
}

G4VPhysicalVolume* G01DetectorConstruction::Construct() {
  ///////////////////////////////////////////////////////////////////////
  //
  // Example how to retrieve Auxiliary Information
  //

  G4cout << std::endl;

  const G4LogicalVolumeStore *lvs = G4LogicalVolumeStore::GetInstance();
  std::vector<G4LogicalVolume*>::const_iterator lvciter;
  for (G4LogicalVolume *volume : *lvs) {
    G4cout << "Logical Volume: " << volume->GetName() << G4endl;

    G4GDMLAuxListType auxInfo = fParser->GetVolumeAuxiliaryInformation(volume);
    if (auxInfo.size() > 0)
      G4cout << "Auxiliary Information is found." << G4endl;
    print_aux(&auxInfo);
  }

  // now the 'global' auxiliary info
  G4cout << std::endl;
  G4cout << "Global auxiliary info:" << std::endl;
  G4cout << std::endl;

  print_aux(fParser->GetAuxList());

  G4cout << std::endl;

  //
  // End of Auxiliary Information block
  //
  ////////////////////////////////////////////////////////////////////////
  return fWorld;
}

void G01DetectorConstruction::ConstructSDandField() {
  // Initialize Primitive Scorers

}
