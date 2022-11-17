#include <DetectorConstruction.hh>
#include "Materials.hh"

#include <G4MultiFunctionalDetector.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4VisAttributes.hh>
#include <G4Colour.hh>
#include <G4SDManager.hh>
#include <G4PSEnergyDeposit.hh>

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

DetectorConstruction::DetectorConstruction(G4GDMLParser *parser) {
  fParser = parser;
  fWorld = fParser->GetWorldVolume();
  mfd = nullptr;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  // Set material for volumes imported from GDML
  G4VisAttributes *crystalVisAttr = new G4VisAttributes();
  crystalVisAttr->SetColour(0, 1, 1); // cyan

  const G4LogicalVolumeStore *lvs = G4LogicalVolumeStore::GetInstance();
  for (G4LogicalVolume *volume : *lvs) {
    G4cout << "Logical volume name: " << volume->GetName() << G4endl;
    if (volume->GetName() == "World") {
      // Volume is world
      G4Material *material = Materials::getInstance()->getMaterial("G4_AIR");
      volume->SetMaterial(material);
    } else {
      // Volume is crystal
      G4Material *material = Materials::getInstance()->getMaterial("SciGlass-40-L");
      volume->SetMaterial(material);
      volume->SetVisAttributes(crystalVisAttr);
      // Remember volume to attach sensitive detector
      crystalLogicalVolumes.push_back(volume);
    }

    // Print auxilary info:
    // G4GDMLAuxListType auxInfo = fParser->GetVolumeAuxiliaryInformation(volume);
    // if (auxInfo.size() > 0)
    //   G4cout << "Auxiliary Information is found." << G4endl;
    // print_aux (&auxInfo);
  }

  // Print the 'global' auxiliary info
  // G4cout << std::endl;
  // G4cout << "Global auxiliary info:" << std::endl;
  // G4cout << std::endl;
  // print_aux(fParser->GetAuxList());

  return fWorld;
}

void DetectorConstruction::ConstructSDandField() {
  // Initialize Primitive Scorers
  mfd = new G4MultiFunctionalDetector("crystals");
  G4SDManager::GetSDMpointer()->AddNewDetector(mfd);

  G4VPrimitiveScorer *primitiveScorer = new G4PSEnergyDeposit("edep");
  mfd->RegisterPrimitive(primitiveScorer);

  // Assign sensitive detector to all crystals' logical volumes
  for (G4LogicalVolume* lv : crystalLogicalVolumes){
    SetSensitiveDetector(lv, mfd);
  }
}
