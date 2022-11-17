/*
 * Materials.cpp
 *
 *  Created on: Nov 16, 2020
 *      Author: petrstepanov
 */

#include "Materials.hh"
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>

Materials *Materials::fgInstance = nullptr;

Materials::Materials() : fMaterialsList { } {
  // ASCII font
  // https://fsymbols.com/generators/carty/

  // █░█ ▄▀█ █▀▀ █░█ █░█ █▀▄▀█
  // ▀▄▀ █▀█ █▄▄ █▄█ █▄█ █░▀░█
  {
    G4Material *material = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
    printMaterialProperties(material);
    fMaterialsList.push_back(material);
  }

  // ▄▀█ █ █▀█
  // █▀█ █ █▀▄
  {
    G4Material *material = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    printMaterialProperties(material);
    fMaterialsList.push_back(material);
  }

  // █▀ █▀▀ █ █▀▀ █░░ ▄▀█ █▀ █▀ ▄▄ █░█ █▀█ █░░
  // ▄█ █▄▄ █ █▄█ █▄▄ █▀█ ▄█ ▄█ ░░ ▀▀█ █▄█ █▄▄
  {
    G4Material *material = getBaseMaterial("SciGlass-40-L", BaseMaterialType::SciGlass);

    G4MaterialPropertiesTable *mpt = getBaseMPT(BaseMaterialType::SciGlass);

    // Transmittance SciGlass-40 (longtitudinal), provided by Josh: Transmission comparison data 8.20.22.ods
    std::vector<G4double> transmittance = { 398, 0, 399, 0.21764, 402, 0.94338, 405, 2.3246, 408, 4.0974, 411, 6.0884,
                                            414, 8.0273, 417, 9.8424, 420, 11.391, 423, 12.731, 426, 13.855, 429,
                                            14.854, 432, 15.67, 435, 16.484, 438, 17.261, 441, 17.986, 444, 18.766, 447,
                                            19.549, 450, 20.591, 453, 21.551, 456, 22.668, 459, 23.822, 462, 24.957,
                                            465, 26.157, 468, 27.388, 471, 28.537, 474, 29.692, 477, 30.954, 480,
                                            32.138, 483, 33.355, 486, 34.498, 489, 35.691, 492, 36.906, 495, 37.919,
                                            498, 38.903, 501, 39.73, 504, 40.46, 507, 41.081, 510, 41.539, 513, 41.908,
                                            516, 42.322, 519, 42.535, 522, 42.552, 525, 42.409, 528, 42.324, 531,
                                            42.345, 534, 42.255, 537, 42.05, 540, 41.665, 543, 41.196, 546, 40.588, 549,
                                            39.897, 552, 39.151, 555, 38.41, 558, 37.544, 561, 36.555, 564, 35.625, 567,
                                            34.498, 570, 33.105, 573, 31.543, 576, 30.275, 579, 29.179, 582, 27.845,
                                            585, 26.634, 588, 25.815, 591, 25.229, 594, 24.586, 597, 23.916, 600, 23.17,
                                            603, 22.341, 606, 21.582, 609, 20.742, 612, 19.991, 615, 19.287, 618,
                                            18.595, 621, 17.89, 624, 17.214, 627, 16.56, 630, 15.932, 633, 15.373, 636,
                                            14.871, 639, 14.341, 642, 13.752, 645, 13.278, 648, 12.79, 651, 12.398, 654,
                                            11.914, 657, 11.251, 660, 10.932, 663, 10.566, 666, 10.013, 669, 9.6445,
                                            672, 9.3481, 675, 8.8192, 678, 8.6346, 681, 8.194, 684, 7.8547, 687, 7.3147,
                                            690, 6.9201, 693, 6.4997, 696, 6.1193, 699, 5.3917, 702, 5.2959, 705,
                                            4.5018, 708, 4.5189, 711, 4.3281, 714, 4.1353, 717, 3.3948, 720, 3.1829,
                                            723, 2.6561, 726, 1.9778, 729, 1.8531, 732, 1.7831, 735, 1.6995, 738,
                                            1.5822, 741, 1.2433, 744, 0.68673, 747, 0.08161, 750, 0.0093703, 751, 0 };
    G4MaterialPropertyVector *transmittanceMPV = nmToMPV(transmittance);
    G4double transmittanceDistance = 40 * cm;

    // Absorption length
    G4MaterialPropertyVector *refractiveIndexMPV = mpt->GetProperty("RINDEX");
    G4MaterialPropertyVector *absLengthMPV = calcAbsorptionLength(mpt, refractiveIndexMPV, transmittanceMPV,
                                                                  transmittanceDistance);
    mpt->AddProperty("ABSLENGTH", absLengthMPV, true);
    material->SetMaterialPropertiesTable(mpt);
    printMaterialProperties(material);
    fMaterialsList.push_back(material);
    generateEmissionMacro(material);
  }
}

Materials::~Materials() {
  // TODO Auto-generated destructor stub
}

G4Material* Materials::getMaterial(const char *materialId) {
  for (int i = 0; i < (int) fMaterialsList.size(); i++) {
    if (fMaterialsList[i]->GetName() == materialId) {
      return fMaterialsList[i];
    }
  }
  G4cout << "ERROR: Materials::getMaterial material " << materialId << " not found." << G4endl;
  return NULL;
}

G4MaterialPropertyVector* Materials::calcAbsorptionLength(G4MaterialPropertiesTable *mpt,
  G4MaterialPropertyVector *refractiveIndex, G4MaterialPropertyVector *measuredTransmittance, G4double sampleLength,
  G4int nPoints) {
  // Reflectivity. To be filled from down below
  G4MaterialPropertyVector *reflectivityMPV = new G4MaterialPropertyVector();

  // Ideal Transmittance. To be filled from down below
  G4MaterialPropertyVector *idealTransmittanceMPV = new G4MaterialPropertyVector();

  // Theoretical Transmittance. To be filled from down below
  // std::vector<G4double> realTransmittance = {};

  // Attenuation length
  G4MaterialPropertyVector *attenuationLengthMPV = new G4MaterialPropertyVector();

  // Define energy limits for the output absorption length MPV as the widest between provided RI and T MPVs'
  // G4double eMin = std::min(refractiveIndex->GetMinLowEdgeEnergy(), measuredTransmittance->GetMinLowEdgeEnergy());
  // G4double eMax = std::max(refractiveIndex->GetMaxLowEdgeEnergy(), measuredTransmittance->GetMaxLowEdgeEnergy());

  G4double eMin = std::max(refractiveIndex->GetMinEnergy(), measuredTransmittance->GetMinEnergy());
  G4double eMax = std::min(refractiveIndex->GetMaxEnergy(), measuredTransmittance->GetMaxEnergy());

  for (G4int i = 0; i < nPoints; i++) {
    // Workaround the rounding of the last point... ABSLENGTH = -nan
    G4double e;
    G4double eStep = (eMax - eMin) / (nPoints - 1);
    if (i == 0)
      e = eMin;
    if (i > 0 && i < nPoints - 1)
      e = eMin + i * eStep;
    if (i == nPoints - 1)
      e = eMax;

    // All formulas below taken from:
    // https://sci-hub.do/https://doi.org/10.1016/0168-9002(93)91185-P
    G4double RI = refractiveIndex->Value(e);

    // Reflectivity - formula (3)
    G4double refractiveIndexAir = 1;
    G4double R = std::pow((RI - refractiveIndexAir) / (RI + refractiveIndexAir), 2.);
    reflectivityMPV->InsertValues(e, R);

    // Ideal transmittance with infinite light attenuation length and two parallel end surfaces
    G4double Ts = (1 - R) / (1 + R);
    idealTransmittanceMPV->InsertValues(e, Ts);

    // Real transmittance can also be estimated but we're using the measured value
    // G4double tReal = crystalLength*pow(1-r,2)/(1-pow(l*r,2));

    // Absorption (light attenuation length)
    G4double T = measuredTransmittance->Value(e) / 100.;

    G4double lambda;

    // It can happen that measured transmittance is higher than tha ideal transmittance...
    // Make sure to provide the RINDEX in the whole region.

    if (T > 0) {
      G4double a = T * (1 - Ts) * (1 - Ts);
      G4double b = std::sqrt(4 * Ts * Ts * Ts * Ts + T * T * (1 - Ts * Ts) * (1 - Ts * Ts)) - 2 * Ts * Ts; // @suppress("Ambiguous problem")
      G4double c = a / b;
      G4double d = std::log(c); // @suppress("Ambiguous problem")
      lambda = sampleLength / d;
    } else {
      lambda = 0;
    }
    attenuationLengthMPV->InsertValues(e, lambda);
  }

  // Add custom values to the material properties table (for debugging output)
  mpt->AddProperty("MY_TRANSMITTANCE", measuredTransmittance, true);
  mpt->AddProperty("MY_REFLECTIVITY", reflectivityMPV, true);
  mpt->AddProperty("MY_IDEAL_TRANSM", idealTransmittanceMPV, true);

  // Return MPV
  // attenuationLengthMPV->SetSpline(true); // Removed in Geant 11
  return attenuationLengthMPV;
}

//void Materials::printLine(const char* heading, std::vector<G4double> values, G4double unit){
//  // Print photon wavelength
//  std::cout << std::endl << std::left << std::setw(TAB_COLUMN_1) << heading;
//  for (G4int i = 0; i < (G4int)values.size(); i++){
//    G4double value = (unit == 0.) ? values[i] : values[i]/unit;
//    std::cout << std::left << std::setw(TAB_COLUMN) << value;
//  }
//}
//
//
//void Materials::printHeader(){
//  printLine("Photon wavelength, nm", fOpticalPhotonWavelength);
//  printLine("Photon energy, eV", fOpticalPhotonEnergy, eV);
//}

typedef std::map<G4int, G4MaterialPropertyVector*, std::less<G4int> >::const_iterator MPiterator;

void Materials::printMaterialProperties(G4Material *material) {
  std::cout << "\nMaterial name: " << material->GetName() << std::endl;
  G4MaterialPropertiesTable *mpt = material->GetMaterialPropertiesTable();
  if (!mpt)
    return;

  std::vector<G4double> fOpticalPhotonWavelength = { 360, 400., 440., 480., 520., 560., 600., 640., 680., 720., 760 };

  // Print photon wavelengths
  std::cout << std::left << std::setw(TAB_COLUMN_1) << "PHOTON_WAVELEGTH, nm";
  for (G4int i = 0; i < (G4int) fOpticalPhotonWavelength.size(); i++) {
    std::cout << std::left << std::setw(TAB_COLUMN) << fOpticalPhotonWavelength[i];
  }

  std::cout << std::endl;

  // Print photon energy
  std::cout << std::left << std::setw(TAB_COLUMN_1) << "PHOTON_ENERGY, eV";
  for (G4int i = 0; i < (G4int) fOpticalPhotonWavelength.size(); i++) {
    std::cout << std::left << std::setw(TAB_COLUMN) << wavelengthNmToEnergy(fOpticalPhotonWavelength[i]) / eV;
  }
  std::cout << std::endl;

  // Print material properties
  std::vector<G4String> propertyNames = mpt->GetMaterialPropertyNames();

  // Replaced in Geant4 v.11 with const std::vector<G4MaterialPropertyVector*>& GetProperties() const
  //const std::map<G4int, G4MaterialPropertyVector*, std::less<G4int> >* pMapNew = mpt->GetPropertyMap();
  std::vector<G4String> materialPropertiesNames = mpt->GetMaterialPropertyNames();

  for (G4String propertyName : materialPropertiesNames) {
    G4MaterialPropertyVector *property = mpt->GetProperty(propertyName);
    if (!property)
      continue;

    // Print property name
    std::cout << std::left << std::setw(TAB_COLUMN_1) << propertyName;
    // Print property values
    for (G4int i = 0; i < (G4int) fOpticalPhotonWavelength.size(); i++) {
      G4bool b;
      G4double value = property->GetValue(wavelengthNmToEnergy(fOpticalPhotonWavelength[i]), b);
      std::cout << std::left << std::setw(TAB_COLUMN) << value;
    }

    std::cout << std::endl;
  }

  // Print material constant properties
  std::vector<G4String> constPropertyNames = mpt->GetMaterialConstPropertyNames();
  for (G4String constPropertyName : constPropertyNames) {
    if (!mpt->ConstPropertyExists(constPropertyName))
      continue;
    // Print property name and value
    G4double constPropertyValue = mpt->GetConstProperty(constPropertyName);
    std::cout << std::left << std::setw(TAB_COLUMN_1) << constPropertyName << constPropertyValue << std::endl;
  }
}

void Materials::printMaterialProperties(const char *materialId) {
  G4Material *material = getMaterial(materialId);
  if (material == NULL)
    return;
  printMaterialProperties(material);
}

Materials* Materials::getInstance() {
  static Materials materials;
  if (fgInstance == nullptr) {
    fgInstance = &materials;
  }
  return fgInstance;
}

G4MaterialPropertyVector* Materials::evToMPV(std::vector<G4double> digitizedArray) {
  assert(digitizedArray.size() % 2 == 0);
  G4MaterialPropertyVector *mpv = new G4MaterialPropertyVector(); // true for spline
  for (G4int i = 0; i < (G4int) digitizedArray.size(); i += 2) {
    G4double energy = digitizedArray[i] * eV; // E (eV) = 1239.8 / l (nm)
    G4double value = digitizedArray[i + 1];
    mpv->InsertValues(energy, value);
  }
  return mpv;
}

G4MaterialPropertyVector* Materials::nmToMPV(std::vector<G4double> digitizedArrayNm) {
  assert(digitizedArrayNm.size() % 2 == 0);
  G4MaterialPropertyVector *mpv = new G4MaterialPropertyVector(); // true for spline
  // for (G4int i = 0; i < (G4int)digitizedArrayNm.size(); i+=2){
  for (G4int i = (G4int) digitizedArrayNm.size() - 2; i >= 0; i -= 2) {
    G4double hc = 1239.84193;
    G4double energy = hc / digitizedArrayNm[i] * eV; // E (eV) = 1239.8 / l (nm)
    G4double value = digitizedArrayNm[i + 1];
    mpv->InsertValues(energy, value);
  }
  return mpv;
}

// PS: when having a static std::vector member
//std::vector<G4Material*> Materials::fMaterialsList = {};

G4double Materials::energyToWavelangthNm(G4double energy) {
  G4double hc = 1239.84193;
  return hc * eV / energy;
}

G4double Materials::wavelengthNmToEnergy(G4double wavelength) {
  G4double hc = 1239.84193;
  return hc / wavelength * eV; // E (eV) = 1239.8 / l (nm)
}

G4Material* Materials::getBaseMaterial(const char *newName, BaseMaterialType bmt, G4double overrideDensity) {
  // Get elements from the database
  G4Element *Pb = G4NistManager::Instance()->FindOrBuildElement("Pb");
  G4Element *W = G4NistManager::Instance()->FindOrBuildElement("W");
  G4Element *O = G4NistManager::Instance()->FindOrBuildElement("O");
  G4Element *Ba = G4NistManager::Instance()->FindOrBuildElement("Ba");
  G4Element *Gd = G4NistManager::Instance()->FindOrBuildElement("Gd");
  G4Element *Si = G4NistManager::Instance()->FindOrBuildElement("Si");
  G4Element *Ge = G4NistManager::Instance()->FindOrBuildElement("Ge");

  if (bmt == BaseMaterialType::PWO) {
    G4double pwoDensity = 8.28 * g / cm3;
    G4Material *pwo = new G4Material(newName, overrideDensity == 0 ? pwoDensity : overrideDensity, 3, kStateSolid);
    pwo->AddElement(Pb, 1);
    pwo->AddElement(W, 1);
    pwo->AddElement(O, 4);
    return pwo;
  } else if (bmt == BaseMaterialType::SciGlassLight) {
    G4double sciGlassDensity = 3.8 * g / cm3;
    G4Material *sciGlass = new G4Material(newName, overrideDensity == 0 ? sciGlassDensity : overrideDensity, 4,
                                          kStateSolid);
    sciGlass->AddElement(Ba, 0.3875);
    sciGlass->AddElement(Gd, 0.2146);
    sciGlass->AddElement(Si, 0.1369);
    sciGlass->AddElement(O, 0.2610);
    return sciGlass;
  } else if (bmt == BaseMaterialType::SciGlass) {
    G4double sciGlassDensity = 4.22 * g / cm3;
    G4Material *sciGlass = new G4Material(newName, overrideDensity == 0 ? sciGlassDensity : overrideDensity, 4,
                                          kStateSolid);
    sciGlass->AddElement(Ba, 0.3875);
    sciGlass->AddElement(Gd, 0.2146);
    sciGlass->AddElement(Si, 0.1369);
    sciGlass->AddElement(O, 0.2610);
    return sciGlass;
  } else if (bmt == BaseMaterialType::CSGlass) {
    G4double csGlassDensity = 4.9 * g / cm3;
    G4Material *csGlass = new G4Material(newName, overrideDensity == 0 ? csGlassDensity : overrideDensity, 4,
                                         kStateSolid);
    csGlass->AddElement(Ba, 0.3183);
    csGlass->AddElement(Gd, 0.1763);
    csGlass->AddElement(Ge, 0.2905);
    csGlass->AddElement(O, 0.2149);
    return csGlass;
  }
  return NULL;
}

G4MaterialPropertiesTable* Materials::getBaseMPT(BaseMaterialType bmt) {
  const G4int SciGlasstoPWOLightYieldCoefficient = 10;
  G4MaterialPropertiesTable *mpt = new G4MaterialPropertiesTable();
  if (bmt == BaseMaterialType::PWO) {
    // Some of the GEANT4 optical parameters are outlined on pp.55 of
    // http://epubs.surrey.ac.uk/811039/1/Sion%20Richards-Thesis-Final-Version.pdf

    // PbWO4 ordinary refractive index taken from page 5 of
    // https://sci-hub.do/https://doi.org/10.1016/S0168-9002(96)01016-9
    // point 330, 2.8 can be added artificially
    // point 900, 2.21 can be added artificially
    // Researchers refer to this RI data as well: http://geb.uni-giessen.de/geb/volltexte/2013/10382/pdf/EissnerTobias_2013_11_18.pdf
    std::vector<G4double> refractiveIndex = { 375, 2.452, 400, 2.393, 425, 2.353, 450, 2.323, 475, 2.301, 500, 2.284,
                                              525, 2.270, 550, 2.259, 575, 2.250, 600, 2.242, 625, 2.236, 650, 2.230,
                                              675, 2.225, 700, 2.222 };
    G4MaterialPropertyVector *refractiveIndexMPV = nmToMPV(refractiveIndex);
    mpt->AddProperty("RINDEX", refractiveIndexMPV, true);

    // Older values used for simulation:
    // std::vector<G4double> wavelength         = { 400., 440., 480., 520., 560., 600.,  640., 680.}
    // std::vector<G4double> transmittancePbWO4 = { 0.33, 0.48, 0.62, 0.67, 0.68, 0.689, 0.69, 0.69};

    // PbWO4 measured longitudinal transmittance (I/I_0) of a crystal of 21.3 cm long (R. Zhu, 1996)
    // Digitized from the undoped sample # 728
    // https://sci-hub.do/10.1016/0168-9002(96)00286-0
    // std::vector<G4double> transmittance = {360.2, 0, 374.3, 9.2, 387.3, 21.3, 402.4, 34.5, 427.6, 46.0, 454.0, 55.2, 483.4, 64.9, 519.0, 69.0, 579.2, 69.0, 636.3, 69.0, 702.7, 69.0, 755.7, 70.1, 797.6, 69.5};
    // G4MaterialPropertyVector* transmittanceMPV = nmToMPV(transmittance);
    // G4double pbwo4BlockLength = 21.3*cm;

    // Digitized from below article for the sample befor irradiation (2001) by P.Yang
    // https://sci-hub.do/10.1016/S0022-0248(01)02385-5
    // std::vector<G4double> transmittance = {339.7, 0.0, 348.2, 1.4, 353.4, 9.9, 357.2, 18.9, 362.0, 31.1, 366.7, 40.7, 372.9, 47.1, 381.9, 52.9, 392.4, 57.0, 410.9, 62.0, 429.5, 64.8, 446.1, 66.3, 475.5, 67.9, 500.7, 69.3, 525.9, 70.1, 550.1, 70.9, 574.8, 71.2, 600.5, 71.6, 625.2, 71.9, 650.4, 72.1, 675.1, 72.3, 700.2, 72.4, 724.9, 72.4, 750.6, 72.6, 775.8, 72.3, 799.5, 73.1};
    // G4MaterialPropertyVector* transmittanceMPV = nmToMPV(transmittance);
    // G4double transmittanceDistance = 22*cm;

    // Transmittance CRYTUR, provided by V.Berdnikov
    // std::vector<G4double> transmittance = {330, 0, 340, 1.566694, 350, 13.223613, 360, 45.441407, 370, 62.071972, 380, 67.140230, 390, 68.931434, 400, 69.772116, 410, 70.262557, 420, 70.646469, 430, 71.216135, 440, 71.501950, 450, 71.723380, 460, 71.975302, 470, 72.019692, 480, 72.192253, 490, 72.614533, 500, 72.789630, 510, 72.788232, 520, 72.912663, 530, 73.042799, 540, 73.235979, 550, 73.354468, 560, 73.453758, 570, 73.639631, 580, 73.695232, 590, 73.819757, 600, 73.957622, 610, 73.997395, 620, 74.067210, 630, 74.073362, 640, 74.278860, 650, 74.253225, 660, 74.346432, 670, 74.327692, 680, 74.598802, 690, 74.607074, 700, 74.542008, 710, 74.542790, 720, 74.641923, 730, 74.787570, 740, 74.585494, 750, 74.804944, 760, 74.843619, 770, 74.728880, 780, 74.860927, 790, 74.742771, 800, 74.925695};
    // G4MaterialPropertyVector* transmittanceMPV = nmToMPV(transmittance);
    // G4double transmittanceDistance = 20*cm;

    // Transmittance SICCAS (S047), provided by V.Berdnikov
    // std::vector<G4double> transmittance = {340, 0., 350, 3.48074, 360, 29.3949, 370, 48.346, 380, 55.3979, 390, 58.6286, 400, 60.9593, 410, 62.6595, 420, 63.9743, 430, 64.9291, 440, 65.631, 450, 66.1062, 460, 66.7927, 470, 67.1896, 480, 67.3996, 490, 67.7139, 500, 68.053, 510, 68.3073, 520, 68.519, 530, 68.8111, 540, 68.9737, 550, 69.0832, 560, 69.4211, 570, 69.5605, 580, 69.6869, 590, 69.8266, 600, 69.9968, 610, 70.0129, 620, 70.1723, 630, 70.2061, 640, 70.3685, 650, 70.4813, 660, 70.4112, 670, 70.5207, 680, 70.6612, 690, 70.6313, 700, 70.6932, 710, 70.7846, 720, 70.7335, 730, 70.8962, 740, 70.934, 750, 70.9511, 760, 71.0478, 770, 71.0046, 780, 71.0511, 790, 71.0082, 800, 70.973};
    // G4MaterialPropertyVector* transmittanceMPV = nmToMPV(transmittance);
    // G4double transmittanceDistance = 20*cm;

    // G4MaterialPropertyVector* absLengthMPV = calcAbsorptionLength(mpt, refractiveIndexMPV, transmittanceMPV, transmittanceDistance);

    // Absorption length PbWO4 - calculated https://sci-hub.do/https://doi.org/10.1016/0168-9002(93)91185-P
    // TODO: compare with https://sci-hub.do/https://doi.org/10.1016/S0168-9002(96)01016-9
    // TODO: compare with https://sci-hub.do/https://link.springer.com/article/10.1557/PROC-348-475
    // mpt->AddProperty("ABSLENGTH", absLengthMPV, true);

    // Energy spectrum for the fast scintillation component PbWO4 (arbitrary units)
    // PbWO4 emission spectrum (Berd sent paper): https://sci-hub.do/10.1088/1742-6596/293/1/012004
    // std::vector<G4double> digitizedEmissionSpectrum = {411.29848229342326, 10, 420.4047217537943, 14.606741573033702, 425.9696458684655, 21.68539325842697, 433.55817875210795, 30, 439.62900505902195, 37.64044943820225, 447.72344013490726, 44.49438202247191, 457.3355817875211, 50, 466.4418212478921, 51.79775280898876, 478.58347386172005, 49.7752808988764, 490.7251264755481, 42.92134831460674, 499.8313659359191, 35.1685393258427, 508.9376053962901, 27.752808988764045, 518.043844856661, 20.561797752809, 527.6559865092748, 13.932584269662925, 544.8566610455313, 7.752808988764045, 564.080944350759, 3.2584269662921344, 581.2816188870152, 2.0224719101123583};

    // Scintillation light yield - number of photons per unit energy deposition ~ 300 for SICCAS PbWO4
    // https://sci-hub.do/10.1016/j.phpro.2015.05.033 - says 300/MeV
    mpt->AddConstProperty("SCINTILLATIONYIELD", 300. / MeV, true);

    // However we used 100/MeV to ensure correct PE output
    // Siccas Light Yield is 120/MeV? check here: https://sci-hub.ru/10.1109/23.603694
  } else if (bmt == BaseMaterialType::SciGlass) {
    // Refractive index for SciGlass. T.Horn said use 1.58 for now.
    // std::vector<G4double> refractiveIndex = { 400, 1.58, 800, 1.58};

    // Refractive index used from 40% BaO, 60% SiO2
    // https://sci-hub.do/10.1016/S0022-3093(96)00609-6
    std::vector<G4double> refractiveIndex = { 297, 1.699, 310, 1.691, 322, 1.682, 348, 1.673, 379, 1.662, 429, 1.653,
                                              511, 1.644, 617, 1.637, 718, 1.633, 1031, 1.626, 1389, 1.621 };
    G4MaterialPropertyVector *refractiveIndexMPV = nmToMPV(refractiveIndex);
    mpt->AddProperty("RINDEX", refractiveIndexMPV, true);

    // Emission spectrum for SciGlass provided by T.Horn, normalized:
    std::vector<G4double> emissionSpectrum = { 364, 0, 365, 0.000216196129458, 366, 0.00019669833529, 367,
                                               0.000217343058527, 368, 0.000216769593993, 369, 0.000223651168405, 370,
                                               0.000223077703871, 371, 0.00021906345213, 372, 0.000209314555046, 373,
                                               0.000244869356177, 374, 0.000244295891643, 375, 0.000263793685811, 376,
                                               0.000276409905568, 377, 0.000285585338118, 378, 0.000309670848561, 379,
                                               0.000297054628805, 380, 0.000313111635767, 381, 0.000339491004349, 382,
                                               0.000384794702564, 383, 0.00042493721997, 384, 0.000394543599648, 385,
                                               0.000424363755436, 386, 0.000463932808307, 387, 0.000478269421667, 388,
                                               0.000518985403607, 389, 0.000568876818098, 390, 0.00058493382506, 391,
                                               0.000569450282632, 392, 0.000684143189507, 393, 0.000750091610959, 394,
                                               0.000817186961481, 395, 0.000858476407956, 396, 0.001041411594421, 397,
                                               0.00103739734268, 398, 0.001098184583324, 399, 0.001292589060476, 400,
                                               0.001401547322007, 401, 0.001516240228881, 402, 0.001618890380534, 403,
                                               0.00177315234028, 404, 0.001990495398807, 405, 0.002171136727135, 406,
                                               0.002373569707768, 407, 0.002567400720386, 408, 0.002824886296319, 409,
                                               0.002979148256066, 410, 0.003179287378562, 411, 0.003425303663808, 412,
                                               0.0037338275833, 413, 0.003940274815674, 414, 0.004243064089823, 415,
                                               0.004536677931422, 416, 0.004854377283464, 417, 0.005014947353088, 418,
                                               0.005458235438158, 419, 0.005693929361786, 420, 0.005965178086544, 421,
                                               0.006241587992111, 422, 0.006554126163344, 423, 0.00692343732348, 424,
                                               0.007141353846542, 425, 0.007523854690969, 426, 0.007703349090227, 427,
                                               0.007974597814985, 428, 0.008213159061284, 429, 0.008436236765155, 430,
                                               0.008740172968373, 431, 0.008992497363497, 432, 0.00915421436219, 433,
                                               0.009405965292779, 434, 0.009579725046694, 435, 0.009622161422238, 436,
                                               0.00995419738764, 437, 0.010063155649171, 438, 0.010083226907874, 439,
                                               0.010123942889814, 440, 0.010297129179195, 441, 0.010629165144596, 442,
                                               0.01040149972445, 443, 0.010645222151559, 444, 0.01057984719464, 445,
                                               0.010644648687024, 446, 0.010754180413089, 447, 0.01072149293463, 448,
                                               0.010710597108477, 449, 0.010586728769053, 450, 0.010738123406127, 451,
                                               0.010660132229452, 452, 0.010798910646771, 453, 0.010602212311481, 454,
                                               0.010599344988809, 455, 0.010661279158521, 456, 0.010432466809306, 457,
                                               0.010512751844119, 458, 0.010373399962266, 459, 0.010243796977498, 460,
                                               0.010230607293207, 461, 0.01008666769508, 462, 0.010071757617186, 463,
                                               0.009867604242949, 464, 0.009820006686596, 465, 0.009828035190078, 466,
                                               0.009683522127416, 467, 0.009476501430507, 468, 0.00944668127472, 469,
                                               0.009189769163321, 470, 0.009106616805837, 471, 0.009027478700094, 472,
                                               0.008731571000357, 473, 0.008805547925291, 474, 0.008542901168549, 475,
                                               0.008352510943137, 476, 0.008374302595443, 477, 0.008107068122426, 478,
                                               0.00788341695402, 479, 0.007775032157024, 480, 0.007611594764728, 481,
                                               0.007531309729915, 482, 0.007482565244494, 483, 0.007167733215123, 484,
                                               0.007195259512773, 485, 0.007176335183139, 486, 0.006976769525177, 487,
                                               0.006807024023003, 488, 0.006660790566738, 489, 0.006671112928357, 490,
                                               0.006567889312169, 491, 0.006382660267567, 492, 0.006263379644418, 493,
                                               0.006125174691634, 494, 0.006039728476012, 495, 0.005903243916832, 496,
                                               0.00577192053846, 497, 0.005691062039114, 498, 0.005588985351995, 499,
                                               0.005419239849821, 500, 0.005337234421406, 501, 0.00530626733655, 502,
                                               0.005231716947081, 503, 0.005060251051304, 504, 0.004982833339164, 505,
                                               0.00484864263812, 506, 0.004819969411402, 507, 0.004596891707531, 508,
                                               0.004581408165103, 509, 0.00442886659896, 510, 0.004364065106576, 511,
                                               0.004306145188604, 512, 0.004205788895089, 513, 0.004128944647483, 514,
                                               0.004064716619633, 515, 0.003814112618112, 516, 0.003897264975596, 517,
                                               0.003734974512369, 518, 0.003672466878122, 519, 0.003592755307844, 520,
                                               0.003419569018464, 521, 0.003379426501058, 522, 0.003332975873774, 523,
                                               0.003283657923818, 524, 0.003205093282608, 525, 0.00310301659549, 526,
                                               0.003039935496709, 527, 0.003027892741487, 528, 0.002879938891619, 529,
                                               0.002766392913813, 530, 0.002726250396407, 531, 0.002640804180786, 532,
                                               0.002599514734311, 533, 0.002590339301761, 534, 0.002572561901196, 535,
                                               0.002387906321128, 536, 0.002362673881615, 537, 0.002293284672956, 538,
                                               0.00233055986769, 539, 0.002284109240406, 540, 0.002112643344629, 541,
                                               0.002222748535228, 542, 0.00203407870342, 543, 0.001968130281967, 544,
                                               0.001970424140104, 545, 0.001914224615736, 546, 0.001858025091367, 547,
                                               0.00175078722344, 548, 0.001827631471045, 549, 0.001704336596155, 550,
                                               0.001712365099637, 551, 0.001637241245634, 552, 0.001621757703206, 553,
                                               0.001570719359647, 554, 0.001534017629447, 555, 0.001429647084191, 556,
                                               0.001445130626619, 557, 0.001445130626619, 558, 0.00131667457092, 559,
                                               0.001331011184279, 560, 0.001362551733669, 561, 0.001312660319179, 562,
                                               0.001267930085498, 563, 0.001220905993679, 564, 0.001182483869876, 565,
                                               0.001171588043723, 566, 0.001146929068745, 567, 0.001063203246727, 568,
                                               0.001068364427536, 569, 0.001038544271749, 570, 0.000956538843334, 571,
                                               0.001017326083977, 572, 0.000941055300905, 573, 0.000884855776537, 574,
                                               0.000865357982368, 575, 0.000903206641637, 576, 0.000865357982368, 577,
                                               0.000796542238244, 578, 0.000792527986503, 579, 0.000774750585937, 580,
                                               0.000749518146425, 581, 0.000731167281325, 582, 0.000776470979541, 583,
                                               0.000646294530238, 584, 0.000641133349429, 585, 0.000694465551125, 586,
                                               0.000659484214528, 587, 0.000659484214528, 588, 0.000610739729107, 589,
                                               0.000583213431457, 590, 0.000584360360526, 591, 0.000559127921013, 592,
                                               0.000568303353563, 593, 0.000528734300692, 594, 0.000493752964095, 595,
                                               0.000526440442554, 596, 0.000478842886201, 597, 0.000488591783285, 598,
                                               0.000422069897298, 599, 0.000451316588551, 600, 0.000417482181023, 601,
                                               0.000427231078108, 602, 0.000411174071145, 603, 0.000423790290901, 604,
                                               0.000377913128152, 605, 0.000392823206045, 606, 0.000373325411877, 607,
                                               0.00040257210313, 608, 0.00036587037293, 609, 0.00035669494038, 610,
                                               0.000334903288074, 611, 0.000313111635767, 612, 0.000334329823539, 613,
                                               0.000325154390989, 614, 0.000291319983461, 615, 0.000305656596821, 616,
                                               0.00029246691253, 617, 0.000287879196255, 618, 0.000264367150346, 619,
                                               0.000268954866621, 620, 0.000254618253261, 621, 0.000263793685811, 622,
                                               0.000255191717796, 623, 0.000232826600955, 624, 0.000251177466055, 625,
                                               0.000221930774802, 626, 0.000220783845733, 627, 0.000221357310268, 628,
                                               0.00021504920039, 629, 0.000190390225412, 630, 0.000202432980634, 631,
                                               0.00020071258703, 632, 0.000184655580068, 633, 0.000183508650999, 634,
                                               0.000161716998693, 635, 0.000155982353349, 636, 0.000163437392296, 637,
                                               0.000158849676021, 638, 0.000154261959746, 639, 0.000140498810921, 640,
                                               0.000138778417318, 641, 0.000151394637074, 642, 0.00013247030744, 643,
                                               0.000134190701043, 644, 0.000145659991731, 645, 0.000151968101609, 646,
                                               0.000122147945821, 647, 0.000123868339424, 648, 0.000127882591165, 649,
                                               0.000133617236509, 650, 0.000127882591165, 651, 0 };
    // std::vector<G4double> emissionSpectrum = {364, 0, 365, 0.000216196129458, 375, 0.000263793685811, 385, 0.000424363755436, 395, 0.000858476407956, 405, 0.002171136727135, 415, 0.004536677931422, 425, 0.007523854690969, 435, 0.009622161422238, 445, 0.010644648687024, 455, 0.010661279158521, 465, 0.009828035190078, 475, 0.008352510943137, 485, 0.007176335183139, 495, 0.005903243916832, 505, 0.00484864263812, 515, 0.003814112618112, 525, 0.00310301659549, 535, 0.002387906321128, 545, 0.001914224615736, 555, 0.001429647084191, 565, 0.001171588043723, 575, 0.000903206641637, 585, 0.000694465551125, 595, 0.000526440442554, 605, 0.000392823206045, 615, 0.000305656596821, 625, 0.000221930774802, 635, 0.000155982353349, 645, 0.000151968101609, 650, 0.000127882591165, 651, 0};

    // change contents of vector
//    std::vector<G4double>::iterator p;
//    p = emissionSpectrum.begin();
//      while(p != emissionSpectrum.end()) {
//        *p = *p+100;
//        p++;
//      }
    // Tweak: try not normalized
    // for (G4double i : emissionSpectrum) i = i*100;
    G4MaterialPropertyVector *fastComponent = nmToMPV(emissionSpectrum);
    G4MaterialPropertyVector *slowComponent = nmToMPV(emissionSpectrum);
    mpt->AddProperty("SCINTILLATIONCOMPONENT1", fastComponent, true);
    mpt->AddProperty("SCINTILLATIONCOMPONENT2", slowComponent, true);

    // Overleaf French studies: test size samples have a light yield 11-35 (avg. 20) times that of PWO. We take ~ 10 times more
    mpt->AddConstProperty("SCINTILLATIONYIELD", SciGlasstoPWOLightYieldCoefficient * 300. / MeV, true);
  } else if (bmt == BaseMaterialType::CSGlass) {
    // Refractive index for SciGlass. T.Horn said use 1.58 for now.
    // std::vector<G4double> refractiveIndex = { 400, 1.58, 800, 1.58};

    // Refractive index used from 40% BaO, 60% SiO2
    // https://sci-hub.do/10.1016/S0022-3093(96)00609-6
    std::vector<G4double> refractiveIndex = { 297, 1.699, 310, 1.691, 322, 1.682, 348, 1.673, 379, 1.662, 429, 1.653,
                                              511, 1.644, 617, 1.637, 718, 1.633, 1031, 1.626, 1389, 1.621 };
    G4MaterialPropertyVector *refractiveIndexMPV = nmToMPV(refractiveIndex);
    mpt->AddProperty("RINDEX", refractiveIndexMPV, true);

    // Emission spectrum for SciGlass provided by T.Horn, normalized:
    // Custom emission spectrum (added 0 values on the sides)
    std::vector<G4double> emissionSpectrum = { 449, 0.0, 450, 0.002661594724972, 451, 0.002369886292445, 452,
                                               0.002279828655557, 453, 0.002357160756797, 454, 0.002226968738253, 455,
                                               0.00207328342313, 456, 0.002014550181682, 457, 0.002002803533392, 458,
                                               0.002018465731111, 459, 0.001931344756296, 460, 0.001950922503446, 461,
                                               0.00188729482521, 462, 0.001947985841373, 463, 0.00189512592407, 464,
                                               0.001897083698785, 465, 0.001816814935472, 466, 0.001857928204486, 467,
                                               0.001962669151735, 468, 0.002079156747275, 469, 0.001932323643654, 470,
                                               0.001841287119409, 471, 0.001629847450194, 472, 0.001630826337552, 473,
                                               0.001821709372259, 474, 0.001553494236311, 475, 0.0016043963789, 476,
                                               0.001620058576619, 477, 0.001497697656935, 478, 0.001526085390302, 479,
                                               0.00141840778098, 480, 0.001433091091342, 481, 0.001566219771958, 482,
                                               0.001424281105125, 483, 0.00149965543165, 484, 0.001467352148854, 485,
                                               0.001404703357975, 486, 0.00143015442927, 487, 0.001348906778599, 488,
                                               0.00132541348202, 489, 0.001357716764816, 490, 0.001443858852274, 491,
                                               0.001421344443052, 492, 0.00148790878336, 493, 0.001512380967297, 494,
                                               0.001508465417867, 495, 0.001401766695903, 496, 0.001380231174038, 497,
                                               0.00141449223155, 498, 0.00130192018544, 499, 0.0013949144844, 500,
                                               0.001291152424508, 501, 0.001285279100363, 502, 0.001274511339431, 503,
                                               0.001346949003884, 504, 0.001286257987721, 505, 0.001269616902644, 506,
                                               0.001294089086581, 507, 0.001238292507205, 508, 0.001259828029069, 509,
                                               0.001320519045232, 510, 0.00121675698534, 511, 0.001352822328029, 512,
                                               0.001278426888861, 513, 0.001288215762436, 514, 0.001238292507205, 515,
                                               0.001213820323268, 516, 0.001223609196843, 517, 0.001278426888861, 518,
                                               0.001235355845132, 519, 0.001322476819947, 520, 0.001213820323268, 521,
                                               0.00132541348202, 522, 0.001360653426889, 523, 0.001246123606064, 524,
                                               0.001268638015286, 525, 0.001276469114146, 526, 0.001276469114146, 527,
                                               0.001361632314246, 528, 0.00131366683373, 529, 0.001287236875078, 530,
                                               0.001251996930209, 531, 0.001287236875078, 532, 0.001323455707305, 533,
                                               0.001342054567097, 534, 0.001218714760055, 535, 0.001163897068037, 536,
                                               0.001230461408345, 537, 0.001295067973938, 538, 0.001270595790001, 539,
                                               0.001247102493422, 540, 0.001323455707305, 541, 0.001260806916427, 542,
                                               0.001379252286681, 543, 0.00140666113269, 544, 0.001532937601804, 545,
                                               0.001488887670718, 546, 0.001423302217767, 547, 0.001571114208746, 548,
                                               0.001547620912166, 549, 0.001623974126049, 550, 0.00158481863175, 551,
                                               0.001731651735372, 552, 0.001878484838993, 553, 0.002035106816188, 554,
                                               0.002302343064779, 555, 0.002421767322391, 556, 0.002755567911289, 557,
                                               0.003086431838116, 558, 0.003537698909911, 559, 0.003835280666583, 560,
                                               0.004556720649041, 561, 0.006647624044606, 562, 0.009690984838993, 563,
                                               0.01150192645032, 564, 0.010942981769202, 565, 0.008264745959153, 566,
                                               0.006972614647287, 567, 0.006193420310738, 568, 0.006193420310738, 569,
                                               0.006384303345445, 570, 0.006459677671971, 571, 0.006329485653427, 572,
                                               0.005580636824959, 573, 0.004914014534519, 574, 0.004367795389049, 575,
                                               0.00395274714948, 576, 0.00347211345696, 577, 0.003380098045358, 578,
                                               0.003159848389926, 579, 0.003024761934595, 580, 0.003031614146097, 581,
                                               0.002947429833354, 582, 0.00280940671595, 583, 0.002740884600927, 584,
                                               0.002927852086205, 585, 0.002826047801027, 586, 0.002917084325273, 587,
                                               0.003143207304849, 588, 0.003341921438416, 589, 0.003662996491668, 590,
                                               0.004124052437038, 591, 0.004641883849142, 592, 0.005077488723218, 593,
                                               0.005619792319258, 594, 0.006873747024182, 595, 0.00761084920436, 596,
                                               0.008925494925448, 597, 0.012198894248841, 598, 0.017030682245333, 599,
                                               0.02249581036211, 600, 0.026223413419371, 601, 0.025178940608946, 602,
                                               0.022193334168651, 603, 0.019057957962661, 604, 0.016301411164015, 605,
                                               0.015392024808921, 606, 0.015084654178674, 607, 0.014759663575993, 608,
                                               0.01430741761684, 609, 0.013605555381531, 610, 0.012847896566846, 611,
                                               0.011746648289688, 612, 0.01075014095978, 613, 0.009520658438792, 614,
                                               0.008820753978198, 615, 0.007582461470994, 616, 0.006759217203358, 617,
                                               0.005986875078311, 618, 0.005099024245082, 619, 0.004147545733617, 620,
                                               0.003814724032076, 621, 0.003101115148478, 622, 0.002734032389425, 623,
                                               0.002505951635133, 624, 0.00225927202105, 625, 0.002152573299085, 626,
                                               0.00208894562085, 627, 0.002120270016289, 628, 0.00207719897256, 629,
                                               0.002118312241574, 630, 0.002078177859917, 631, 0.002164319947375, 632,
                                               0.002233820949756, 633, 0.002560769327152, 634, 0.002522592720211, 635,
                                               0.00271249686756, 636, 0.002924915424132, 637, 0.003201940546297, 638,
                                               0.00348777565468, 639, 0.00394491605062, 640, 0.004278716639519, 641,
                                               0.004378563149981, 642, 0.004601749467485, 643, 0.005172440796893, 644,
                                               0.00601036837489, 645, 0.007221252036086, 646, 0.008409621288059, 647,
                                               0.009547088397444, 648, 0.009747760305726, 649, 0.00913008238316, 650,
                                               0.007974995301341, 651, 0.007178180992357, 652, 0.006523305350207, 653,
                                               0.006444015474251, 654, 0.006284456834983, 655, 0.006042671657687, 656,
                                               0.005862556383912, 657, 0.005451423693773, 658, 0.004916951196592, 659,
                                               0.00471236373888, 660, 0.004273822202731, 661, 0.003965472685127, 662,
                                               0.003617967673224, 663, 0.003258716013031, 664, 0.002819195589525, 665,
                                               0.002629291442175, 666, 0.002317026375141, 667, 0.002106565593284, 668,
                                               0.001959732489663, 669, 0.001769828342313, 670, 0.001637678549054, 671,
                                               0.001504549868438, 672, 0.001415471118907, 673, 0.001268638015286, 674,
                                               0.001293110199223, 675, 0.001211862548553, 676, 0.001145298208245, 677,
                                               0.001205989224408, 678, 0.001089501628869, 679, 0.001032726162135, 680,
                                               0.001074818318506, 681, 0.001087543854154, 682, 0.001117889362235, 683,
                                               0.001048388359855, 684, 0.001133551559955, 685, 0.001091459403584, 686,
                                               0.001095374953013, 687, 0.001151171532389, 688, 0.001138445996742, 689,
                                               0.001203052562336, 690, 0.001148234870317, 691, 0.001204031449693, 692,
                                               0.001207946999123, 693, 0.001159981518607, 694, 0.001206968111765, 695,
                                               0.001200115900263, 696, 0.001247102493422, 697, 0.001249060268137, 698,
                                               0.001251018042852, 699, 0.001211862548553, 700, 0.001213820323268, 701,
                                               0.00130192018544, 702, 0.001322476819947, 703, 0.001300941298083, 704,
                                               0.001434069978699, 705, 0.001413513344192, 706, 0.001477141022428, 707,
                                               0.001574050870818, 708, 0.001459521049994, 709, 0.001565240884601, 710,
                                               0.001516296516727, 711, 0.001575029758176, 712, 0.001517275404085, 713,
                                               0.001546642024809, 714, 0.001506507643152, 715, 0.001387083385541, 716,
                                               0.001445816626989, 717, 0.001408618907405, 718, 0.001351843440672, 719,
                                               0.001408618907405, 720, 0.001346949003884, 721, 0.001345970116527, 722,
                                               0.001387083385541, 723, 0.001276469114146, 724, 0.001280384663576, 725,
                                               0.001260806916427, 726, 0.001185432589901, 727, 0.001164875955394, 728,
                                               0.001199137012906, 729, 0.00114725598296, 730, 0.001129636010525, 731,
                                               0.001093417178298, 732, 0.001205989224408, 733, 0.001067966107004, 734,
                                               0.001096353840371, 735, 0.001081670530009, 736, 0.001027831725348, 737,
                                               0.001114952700163, 738, 0.001035662824208, 739, 0.00111201603809, 740,
                                               0.001055240571357, 741, 0.000990634005764, 742, 0.001015106189701, 743,
                                               0.00101902173913, 744, 0.001020000626488, 745, 0.00102293728856, 746,
                                               0.001013148414986, 747, 0.001027831725348, 748, 0.001045451697782, 749,
                                               0.001064050557574, 750, 0.001060135008144, 751, 0 };

    // Avoiding the exceptions: trying non-normalized emission (not worked)
    // Avoiding the exceptions: trying extend emission spectrum to the left (not worked)
    G4MaterialPropertyVector *fastComponent = nmToMPV(emissionSpectrum);
    G4MaterialPropertyVector *slowComponent = nmToMPV(emissionSpectrum);
    mpt->AddProperty("SCINTILLATIONCOMPONENT1", fastComponent, true);
    mpt->AddProperty("SCINTILLATIONCOMPONENT2", slowComponent, true);

    // Overleaf French studies: test size samples have a light yield 11-35 (avg. 20) times that of PWO. We take ~ 10 times more
    mpt->AddConstProperty("SCINTILLATIONYIELD", SciGlasstoPWOLightYieldCoefficient * 300. / MeV, true);
  }

  // Fluctuation of mean number of optical photons produces for the step
  mpt->AddConstProperty("RESOLUTIONSCALE", 1.0, true);

  mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 13 * ns, true);
  mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 412 * ns, true);
  mpt->AddConstProperty("SCINTILLATIONYIELD1", 0.9);
  mpt->AddConstProperty("SCINTILLATIONYIELD2", 0.1);

  // Scintillation rise time
  mpt->AddConstProperty("SCINTILLATIONRISETIME1", 10 * ps, true);
  mpt->AddConstProperty("SCINTILLATIONRISETIME2", 50 * ps, true);
  return mpt;
}

// Automatically generate macros for emission
void Materials::generateEmissionMacro(G4Material *material) {
  G4MaterialPropertiesTable *mpt = material->GetMaterialPropertiesTable();

  // TODO: sum vectors from sfast and slow components
  G4MaterialPropertyVector *mpv1 = mpt->GetProperty("SCINTILLATIONCOMPONENT1");
  G4MaterialPropertyVector *mpv2 = mpt->GetProperty("SCINTILLATIONCOMPONENT2");
  G4double sy1 = mpt->GetConstProperty("SCINTILLATIONYIELD1");
  G4String emissionFilename = "macros/gps-op-emission-" + material->GetName() + ".mac";

  std::ofstream myfile;
  myfile.open(emissionFilename.c_str());
  myfile << "# Auto-generated: OP Emission for " << material->GetName() << std::endl;
  myfile << "# http://hurel.hanyang.ac.kr/Geant4/Geant4_GPS/reat.space.qinetiq.com/gps/examples/examples.html"
         << std::endl;
  myfile << std::endl;
  myfile << "/gps/particle opticalphoton" << std::endl;
  myfile << "/gps/pos/type Point" << std::endl;
  myfile << "/gps/ang/type iso" << std::endl;
  myfile << std::endl;
//  myfile << "/gps/number 1000000" << std::endl;
//  myfile << std::endl;
  myfile << "# In example #18" << std::endl;

  // These line for user-defined histogram
  // myfile << "/gps/ene/type User" << std::endl;
  // myfile << "/gps/hist/type energy" << std::endl;

  // These lines for arbitrary point-wise energy function with linear interpolation
  myfile << "/gps/ene/type Arb" << std::endl;
  myfile << "/gps/hist/type arb" << std::endl;

  myfile << std::endl;
  for (int i = 0; i < (int) mpv1->GetVectorLength(); i++) {
    G4double e = mpv1->Energy(i);
    G4bool b = true;
    G4double value1 = mpv1->GetValue(e, b);
    G4double value2 = mpv2->GetValue(e, b);
    G4double value = sy1 * value1 + (1 - sy1) * value2;
    myfile << "/gps/hist/point " << e << " " << value << std::endl;
  }
  myfile << std::endl;
  myfile << "/gps/hist/inter Lin" << std::endl;

  myfile.close();
}
