#include <UiUtils.h>
#include <FileUtils.h>
#include <FitUtils.h>

#include <CanvasHelper.h>

#include <RtypesCore.h>
#include <TObjString.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TApplication.h>
#include <TPRegexp.h>

int plotEdep() {
  // Get list of files from the UI
  UiUtils::showMessageBox("Select ROOT files energy deposition in Panda sleeves");
  TList *files = UiUtils::getFilePaths();

  // Iterate through files and find maximum nPE branch value across all files
  // This is for the light yield plots for each light guide length to have uniform scale along X axis
  Double_t xAxisMax = FileUtils::getBranchMaximumInFiles(files, "barrel", "edep");
  Double_t xAxisMin = FileUtils::getBranchMinimumInFiles(files, "barrel", "edep")*6;

  // Create canvas for plotting
  TCanvas* canvas = new TCanvas();
  canvas->SetWindowSize(files->GetSize()*560, 500);
  canvas->Divide(files->GetSize(), 1, 1e-5, 1e-5);

  // Iterate through files, plot and fir energy resolutions
  for (TObject *o : *files) {
    static Int_t counter = 0;
    TVirtualPad* pad = canvas->cd(counter + 1);
    pad->SetGrid();

    // Get file name path
    TObjString *objString = (TObjString*) o;
    TString fileNamePath = objString->GetString();

    // Open ROOT file and get the tree
    TFile *file = FileUtils::openFile(fileNamePath.Data());
    TTree *tree = FileUtils::getTree(file, "barrel");

    PathComponents filePath = FileUtils::parseFilePath(fileNamePath.Data());

    // Prepare histogram with branch distribution
    TString histName = TString::Format("hist-%s", filePath.name.Data());
    TString histTitle = "";
    // Int_t maxPE = FileUtils::getBranchMaximum(tree, "nPE")*0.3;
    TH1 *hist = new TH1I(histName.Data(), histTitle.Data(), 200, xAxisMin, xAxisMax);
    hist->GetXaxis()->SetTitle("Deposited energy per Event, MeV");
    TString title = TString::Format("Counts / %.2f MeV", hist->GetBinWidth(1));
    hist->GetYaxis()->SetTitle(title.Data());

    // Populate histogram (should pass "y:x >> histName")
    TString drawOption = TString::Format("edep >> %s", histName.Data());
    tree->Draw(drawOption, "", "goff");

    // Draw Histogram on the canvas
    hist->SetLineWidth(0);
    hist->SetFillColor(EColor::kGray);
    hist->Draw();

    // Fit histogram
    TF1 *fBall = FitUtils::getCrystalBallFunction(hist);
    fBall->SetLineColor(kBlack);
    gStyle->SetOptFit(111);
    hist->Fit(fBall);

    // Plot gaussian part
    TF1* fBallGauss = FitUtils::getCrystalBallGaussFunction(fBall);
    fBallGauss->SetLineColor(kBlack);
    fBallGauss->SetLineStyle(ELineStyle::kDashed);
    fBallGauss->Draw("SAME");

    // Get fitting parameters
    Double_t m = hist->GetFunction("fBall")->GetParameter(2); // mean
    Double_t Dm = hist->GetFunction("fBall")->GetParError(2); // mean error
    Double_t s = hist->GetFunction("fBall")->GetParameter(3); // sigma
    Double_t Ds = hist->GetFunction("fBall")->GetParError(3); // sigma error

    // GAUSS RESOLUTION
    TVector2 gaussResolution = FitUtils::evalResolution(m, Dm, s, Ds);
    TString gaussLine = TString::Format("Res. (gauss), %% = %f #pm %f", gaussResolution.X(), gaussResolution.Y());
    CanvasHelper::addTextToStats(gaussLine.Data(), pad);

    // HISTOGRAM RESOLUTION
    TVector2 histResolution = FitUtils::evalResolution(hist->GetMean(), hist->GetMeanError(), hist->GetStdDev(), hist->GetStdDevError());
    TString histLine = TString::Format("Res. (hist), %% = %f #pm %f", histResolution.X(), histResolution.Y());
    CanvasHelper::addTextToStats(histLine.Data(), pad);

    // CRYSTAL BALL RESOLUTION
    // TVector2 cbResolution = FitUtils::getCrystalBallResolution(fBall);
    // TString cbLine = TString::Format("Resolution (momenta), %% = %f #pm %f", cbResolution.X(), cbResolution.Y());
    // CanvasHelper::addTextToStats(cbLine.Data(), pad);

    // Align statistics box
    TPaveStats *pave = CanvasHelper::getDefaultPaveStats(pad);
    CanvasHelper::setPaveAlignment(pave, kPaveAlignLeft | kPaveAlignTop);
    // pave->SetFillColorAlpha(EColor::kWhite, 0.5);

    // Increase static counter
    counter++;
  }

  // Add canvas title
  CanvasHelper::addMultiCanvasTitle(canvas, "Energy Deposition in Calorimeter Sleeves");

  // Beautify the canvas
  CanvasHelper::getInstance()->addCanvas(canvas);

  // Save canvas to file
  canvas->SetName("eres-all");
  CanvasHelper::saveCanvas(canvas, kFormatPng);

  return 0;
}

#ifndef __CINT__

int main(int argc, char **argv) {
  // Instantiate TApplication
  TApplication *app = new TApplication("plotEdep", &argc, argv);

  // Launch the script
  int returnValue = 0 || plotEdep();

  // Enter the event loop
  app->Run();

  // Return success
  return returnValue;
}

#endif
