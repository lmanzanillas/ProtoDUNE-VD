#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "SteelSupportConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4MaterialTable.hh"
#include "UlalapMaterials.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4Torus.hh"
#include "G4EllipticalTube.hh"
#include "G4Hype.hh"

#include "G4Transform3D.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4NistManager.hh"
#include <G4UserLimits.hh>

#include "G4MultiFunctionalDetector.hh"
#include "G4SDManager.hh"
#include "G4PSEnergyDeposit.hh"
#include <G4VPrimitiveScorer.hh>

#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4VoxelLimits.hh"

#include "G4MTRunManager.hh"
#include "G4PhysicalConstants.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4Navigator.hh"
#include "G4TransportationManager.hh"

#include "G4GDMLParser.hh"

#include <G4VisAttributes.hh>
#include <iostream>
#include <fstream>
#include <iterator>

using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/*
Constructs DetectorConstruction, defines default values.
*/

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),  fd2DetectorBox(nullptr), 
logicSteelSupport(nullptr), 
logicCryoPrimMembrane(nullptr), 
fd2LogicVolume(nullptr) 
{
  fDetectorMessenger = new DetectorMessenger(this);
  halfSizeCavernFD2Y = halfSizeCavernFD2Z = 30.*m;
  halfSizeCavernFD2X = 100.*m;
  fSetupName = "DUNE";
  fDataType = "csv";
  halfDetectorX = 62.0/2.0*m;
  halfDetectorXActiveAr = 60.0/2.0*m;
  halfDetectorZ = 15.1/2.0*m;
  halfDetectorZActiveAr = 13.6/2.0*m;
  halfDetectorY = 14.0/2.0*m;
  halfDetectorYActiveAr = 13.44/2.0*m;
  cryostatThicknessPrimMembraneSS = 1.2*mm;
  cryostatThicknessSecondaryBarrierAl = 0.8*mm;
  cryostatThicknessVaporBarrierSS = 12.0*mm;
  cryostatThicknessInnerPU = 0.4*m;
  cryostatThicknessOuterPU = 0.4*m;
  cryostatThicknessInnerPlywood = 10.0*mm;
  cryostatThicknessOuterPlywood = 10.0*mm;
  cryostatThicknessOuterSteelSupport = 1.20*m;
  shieldingThickness = 23.0*cm;
  BottomShieldingThickness = 30.0*cm;
  BottomLeadShieldingThickness = 2.5*cm;
  n_captureLayerThickness = 0.1*cm;
  n_captWaffleBottomThickness = 0.00236*cm;
  fBiSourcePosition = G4ThreeVector(0.*cm, 15.*cm, 0.*cm);
  fBiSourcePosition2 = G4ThreeVector(10.*cm, 45.*cm, 10.*cm);
  fDetectorType = 1;
  fDetectorName = "FD2";
  fVolName = "World";
  materialConstruction = new UlalapMaterials;
  DefineMaterials();
  data_output_directory = "./";  
  fd2Material = G4Material::GetMaterial("G4_lAr");
  fShieldingMaterial = G4Material::GetMaterial("G4_AIR");
  materialShieldingWaffle = G4Material::GetMaterial("G4_WATER");
  materialNeutronCapture = G4Material::GetMaterial("Gd2O3_Powder");
  materialLead = G4Material::GetMaterial("G4_AIR");
  //I beams
  // IBeams
  fIFlangeWidth = 0.402 * m; // all m here.
  fIFlangeThick = 0.040 * m;
  fIFlangeWaist = 0.022 * m;
  fIFlangeHeight = 1.108 * m - 2*fIFlangeThick;
  //fIFlangeHeightInside = fIFlangeHeight - 2*fIFlangeThick;
  fIFlangeHeightInside = fIFlangeHeight;
  fITopLength = 18.94 *m ;
  fISideLength = 17.8 * m  - 2*fIFlangeHeight - 2*fIFlangeThick; // need a little space with these side beams

  fIPortHoleRad = 0.800/2 * m;
  fISidePortLoc = 5.907 * m - fIFlangeHeight/2. ;
  fIPortSpacing = 4.0*m ;
  fIBotPortLoc = 5.0*m;

  fBeltFlangeBotWidth = 0.2*m;
  fht = 4*m; // m, for beam and belt placement
  fst = (17.832/2 + 0.030)*m;
  fzpl = 64.732*m;
  fSpacing = 64.732/41 *m; // m
  //materialNeutronCapture = G4Material::GetMaterial("G4_WATER");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction(){
  //delete physicWorldBox;
  delete fDetectorMessenger;
  delete materialConstruction;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::GetPhysicalVolumeByName(const G4String &name)
{
  // access the store of physical volumes
  G4PhysicalVolumeStore * pvs= G4PhysicalVolumeStore::GetInstance();
  G4VPhysicalVolume* pv;
  G4int npv= pvs->size();
  G4int ipv;
  for (ipv=0; ipv<npv; ipv++) {
    pv= (*pvs)[ipv];
    if (!pv)
      break;
    //G4cout<<" pv->GetName() "<<pv->GetName()<<G4endl;
    if (pv->GetName() == name)
      return pv;
  }
  return NULL;
}




/*
Sets which detector geometry is used.
*/
void DetectorConstruction::SetDetectorType(G4int value){
  fDetectorType=value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  //G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();
}

//Sets dimmensions of target, thickness corresponds to the Z coordinate, Length to x.
void DetectorConstruction::SetLArDetectorLength(G4double value){
  halfDetectorX = (value/2.)*mm;
  //UpdateGeometry();
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
void DetectorConstruction::SetLArDetectorThickness(G4double value){
  halfDetectorZ = (value/2.)*mm;
  //UpdateGeometry();
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
void DetectorConstruction::SetLArDetectorWidth(G4double value){
  halfDetectorY = (value/2.)*mm;
  //UpdateGeometry();
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::SetshieldingThickness(G4double value){
  shieldingThickness = (value/1.)*mm;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
//Bi sources position
void DetectorConstruction::SetBiSourcePosition(G4ThreeVector value){
  fBiSourcePosition = value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::SetBiSourcePosition2(G4ThreeVector value){
  fBiSourcePosition2 = value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
void DetectorConstruction::SetWaffleThickness(G4double value){
  BottomShieldingThickness = (value/1.)*mm;
  //UpdateGeometry();
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
void DetectorConstruction::SetBottomWafflenCaptThickness(G4double value){
  n_captWaffleBottomThickness = (value/1.)*mm;
  //UpdateGeometry();
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
//Sets material of target.
void DetectorConstruction::SetTargetMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial = G4Material::GetMaterial(materialChoice);

  if (pttoMaterial) {
    fd2Material = pttoMaterial;
    if(fd2LogicVolume)fd2LogicVolume->SetMaterial(fd2Material);
    G4cout<<" material: "<<fd2Material->GetName()<<G4endl;  
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterial : "
           << materialChoice << " not found" << G4endl;
  }
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();
}

//Sets material of Ge Container coating

void DetectorConstruction::SetDetectorName(G4String detectorNAME)
{
	fDetectorName = detectorNAME;
}

void DetectorConstruction::SetSetupName(G4String setupNAME)
{
	fSetupName = setupNAME;
}
void DetectorConstruction::SetDataType(G4String dataType)
{
	fDataType = dataType;
}
/*
Sets material of sample.
*/
void DetectorConstruction::SetShieldingMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial = G4Material::GetMaterial(materialChoice);

  if (pttoMaterial) {
    fShieldingMaterial = pttoMaterial;
    if(logicShieldingBoxLongLatWall)logicShieldingBoxLongLatWall->SetMaterial(fShieldingMaterial);
    if(logicShieldingBoxShortLatWall)logicShieldingBoxShortLatWall->SetMaterial(fShieldingMaterial);
    G4cout<<" material "<<fShieldingMaterial->GetName()<<G4endl;  
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterial : "
           << materialChoice << " not found" << G4endl;
  }
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();
}

void DetectorConstruction::SetShieldingBottomLeadLayer(G4String materialChoice)
{ // search the material by its name
  G4Material* pttoMaterial = G4Material::GetMaterial(materialChoice);

  if (pttoMaterial) {
    materialLead = pttoMaterial;
    if(logicLeadWaffleLayer)logicLeadWaffleLayer->SetMaterial(materialLead);
    G4cout<<" material Lead"<<materialLead->GetName()<<G4endl;  
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterialLead : "
           << materialChoice << " not found" << G4endl;
  }
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();

}
void DetectorConstruction::SetShieldingMaterialWaffle(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial = G4Material::GetMaterial(materialChoice);

  if (pttoMaterial) {
    materialShieldingWaffle = pttoMaterial;
    if(logicWaffleBottomShielding)logicWaffleBottomShielding->SetMaterial(materialShieldingWaffle);
    G4cout<<" material waffle"<<materialShieldingWaffle->GetName()<<G4endl;  
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterialWaffle : "
           << materialChoice << " not found" << G4endl;
  }
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();
}

void DetectorConstruction::SetMaterialWaffleNeutronAbsorber(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial = G4Material::GetMaterial(materialChoice);

  if (pttoMaterial) {
    materialNeutronCapture = pttoMaterial;
    if(logicNeutronAbsorberWaffle)logicNeutronAbsorberWaffle->SetMaterial(materialNeutronCapture);
    G4cout<<" material n absorber: "<<materialNeutronCapture->GetName()<<G4endl;  
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterialWaffleNeutronAbsorber : "
           << materialChoice << " not found" << G4endl;
  }
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();
}



void DetectorConstruction::SetOutputDirectory(G4String output_directory)
{
    data_output_directory = output_directory;  
}
/*
Sets material of world volume.
*/
void DetectorConstruction::SetWorldMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial =
     G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);
  if (pttoMaterial) {
    fWorldMaterial = pttoMaterial;
    if ( logicWorldBox ) { logicWorldBox->SetMaterial(fWorldMaterial); }
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterial : "
           << materialChoice << " not found" << G4endl;
  }
  G4RunManager::GetRunManager()->ReinitializeGeometry();
  //G4MTRunManager::GetRunManager()->PhysicsHasBeenModified();
}


/*
Defines materials used in simulation. Sets material properties for PEN and other optical components.
*/
void DetectorConstruction::DefineMaterials(){
  // ============================================================= Materials =============================================================
  //materialConstruction = new PenMaterials;
  materialConstruction-> Construct();
  materialAir = G4Material::GetMaterial("Air");
  materialFR4 = G4Material::GetMaterial("FR4");
  //materialB4C = G4Material::GetMaterial("FLEXIBORE");
  materialB4C = G4Material::GetMaterial("MinwaxGdComposite");
  materialSteel = G4Material::GetMaterial("Steel_EN8");
  materialSS304L = G4Material::GetMaterial("Steel_SS304L");
  materialPUfoam = G4Material::GetMaterial("PU_foam");
  materialPlywood = G4Material::GetMaterial("Plywood");
  materialAcrylic = G4Material::GetMaterial("Acrylic");
  materialAlCryostat = G4Material::GetMaterial("Aluminium_6061");
  materialRock = G4Material::GetMaterial("G4_CONCRETE");
  materialTitanium = G4Material::GetMaterial("titanium");
  //materialLead = G4Material::GetMaterial("G4_Pb");

  G4cout<<" materials imported succesfully "<<G4endl;

}

void DetectorConstruction::SetVolName(G4ThreeVector thePoint){
  G4Navigator* theNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  G4VPhysicalVolume* myVolume= theNavigator->LocateGlobalPointAndSetup(thePoint);
  fVolName =  myVolume->GetName();
}


void DetectorConstruction::UpdateGeometry(){

  //define new one
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::Clean();
  G4LogicalVolumeStore::Clean();
  G4SolidStore::Clean();

  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

/*
Clears stored geometry, then constructs all volumes that can be used in the simulation.

Builds and places volumes in world.

Defines detector sensitivities and properties.
*/
G4VPhysicalVolume* DetectorConstruction::Construct()
{
	fParser.Read("../input_files/protodunevd_v5_ggd.gdml");

	return fParser.GetWorldVolume();
} 
