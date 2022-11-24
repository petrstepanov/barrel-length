#ifndef Constants_H_
#define Constants_H_

#include <G4String.hh>

class Constants {
  public:
    static Constants* GetInstance();
    virtual ~Constants();

    void setOutputFileSuffix(G4String str);
    G4String getOutputFileSuffix();

  protected:
    Constants();

  private:
    static Constants* fgInstance;

    G4String uniqueFileSuffix;
};

#endif
