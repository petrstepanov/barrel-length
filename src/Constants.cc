#include "Constants.hh"
#include <algorithm>

Constants* Constants::fgInstance = nullptr;

Constants::Constants(){}

Constants::~Constants(){}

Constants* Constants::GetInstance(){
  static Constants instance;
  if (fgInstance == nullptr){
    fgInstance = &instance;
  }
  return fgInstance;
}

void Constants::setOutputFileSuffix(G4String str){
  // Remember input filename to produce unique output filename
  std::replace( str.begin(), str.end(), '/', '-');
  std::replace( str.begin(), str.end(), '\\', '-');
  std::replace( str.begin(), str.end(), '.', '-');

  uniqueFileSuffix = str;
}

G4String Constants::getOutputFileSuffix(){
  return uniqueFileSuffix;
}
