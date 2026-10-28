#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4EventManager.hh"
#include "G4ProcessManager.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4SystemOfUnits.hh"
#include "MyTrackInfo.hh"

#include "G4SteppingManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction)
: fOneStepPrimaries(false), fEventAction(eventAction)
{

	fExpectedNextStatus = Undefined;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void SteppingAction::UserSteppingAction(const G4Step* theStep) {
    // Retrieve the track and particle definition
    G4Track* theTrack = theStep->GetTrack();
    G4ParticleDefinition* particleDef = theTrack->GetDefinition();

    // Initialize expected next status
    fExpectedNextStatus = Undefined;

    // Get pre-step point and associated physical volume
    G4StepPoint* thePrePoint = theStep->GetPreStepPoint();
    G4VPhysicalVolume* thePrePV = thePrePoint->GetPhysicalVolume();

    // Default process creator name
    G4String processCreator = "NoProcess";

    // Get particle properties
    G4int pdgCode = particleDef->GetPDGEncoding();
    G4double energy = theTrack->GetKineticEnergy();

    //G4cout<<"thePrePV->GetName(): "<<thePrePV->GetName()<<G4endl;

    //Neutron capture information
    if (particleDef == G4Neutron::Definition()) {
    	const G4VProcess* proc = theStep->GetPostStepPoint()->GetProcessDefinedStep();
	if (proc && (proc->GetProcessName() == "nCapture" ||
             proc->GetProcessName() == "nCaptureHP")){

        	G4String volName = thePrePV ? thePrePV->GetName() : "none";
	        G4double eNeutron = theStep->GetPreStepPoint()->GetKineticEnergy() / eV;
	        G4ThreeVector pos = theStep->GetPostStepPoint()->GetPosition();
	        G4double t = theStep->GetPostStepPoint()->GetGlobalTime() / us;

        	// Daughter nucleus from the secondaries produced in this step
	        G4int Z = 0, A = 0;
        	const auto* secs = theStep->GetSecondaryInCurrentStep();
	        for (const auto* sec : *secs) {
        	    const G4ParticleDefinition* def = sec->GetDefinition();
	            if (def->GetParticleType() == "nucleus") {
        	        Z = def->GetAtomicNumber();
                	A = def->GetAtomicMass();
	            }
       	 	}

        	fEventAction->AddInfoNeutronCapture(volName, Z, A, eNeutron,
                                            pos.x() / cm, pos.y() / cm, pos.z() / cm, t);
    	}
    }

    // Get post-step process and other step-related properties
    G4String process_name = theStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
    G4int stepNo = theTrack->GetCurrentStepNumber();
    G4double edepStep = theStep->GetTotalEnergyDeposit() / keV;

    // Handle neutron capture
    if (process_name == "nCapture" || process_name == "nCaptureHP") {
        G4double time_finish = theStep->GetPostStepPoint()->GetGlobalTime();
        fEventAction->SetTimeCapture(time_finish / us);
        fEventAction->SetTimeDecay(0.);
        G4ThreeVector position_finish = theStep->GetPostStepPoint()->GetPosition();
        fEventAction->SetVertex(position_finish);
    }

    // Handle radioactive decay
    if (process_name == "RadioactiveDecay") {
        G4double partEnergy = theStep->GetPreStepPoint()->GetKineticEnergy() / keV;
        G4double time_finish = theStep->GetPostStepPoint()->GetGlobalTime();
        G4ThreeVector position_finish = theStep->GetPostStepPoint()->GetPosition();
        fEventAction->SetVertex(position_finish);
        fEventAction->SetTimeDecay(time_finish / us);
        fEventAction->SetTimeCapture(0.);
    }

    // Retrieve secondary particles
    G4SteppingManager* steppingManager = fpSteppingManager;
    G4TrackVector* fSecondary = steppingManager->GetfSecondary();
    std::vector<G4double> gamma_vector;

    for (size_t lp1 = 0; lp1 < (*fSecondary).size(); lp1++) {
        const G4ParticleDefinition* particleDefi = (*fSecondary)[lp1]->GetDefinition();
        G4String process = (*fSecondary)[lp1]->GetCreatorProcess()->GetProcessName();
        energy = (*fSecondary)[lp1]->GetKineticEnergy();

        if (particleDefi == G4Gamma::Definition() || particleDefi == G4Alpha::Definition()) {
	    //G4cout<<process<<G4endl;
            if (process == "RadioactiveDecay" || process == "nCapture" || process == "nCaptureHP") {
                gamma_vector.push_back(energy / keV);
            }
        }
    }

    if (!gamma_vector.empty()) {
        fEventAction->AddInfoCaptureGammas(gamma_vector);
    }

    // Store energy deposition for target_1
    if (thePrePV && thePrePV->GetName() == "volTPCActive_PV") {
        G4double time_dep = theStep->GetPostStepPoint()->GetGlobalTime() / us;
        fEventAction->AddInfo(
            thePrePoint->GetPosition().x() / cm,
            thePrePoint->GetPosition().y() / cm,
            thePrePoint->GetPosition().z() / cm,
            edepStep,
            time_dep,
	    pdgCode
        );
    }


    /* -------------   particle-entry filter (neutrons + gammas, including born inside)   -------------------- */
   G4Track*       track     = theStep->GetTrack();
   G4StepPoint*   prePoint  = theStep->GetPreStepPoint();
   G4StepPoint*   postPoint = theStep->GetPostStepPoint();
   
   // Get or create custom track information
   MyTrackInfo* trackInfo = static_cast<MyTrackInfo*>(track->GetUserInformation());
   if (!trackInfo) {
       trackInfo = new MyTrackInfo();
       track->SetUserInformation(trackInfo);
   }
   
   //auto particleDef = track->GetDefinition();
   G4VPhysicalVolume* preVol  = prePoint->GetPhysicalVolume();
   G4VPhysicalVolume* postVol = postPoint->GetPhysicalVolume();
   
   G4bool isInTarget1 = false;
   
   // Case 1: Particle crosses into target_1
   if (postPoint->GetStepStatus() == fGeomBoundary &&
       postVol && postVol->GetName() == "volTPCActive_PV")
   {
       isInTarget1 = true;
   }
   /*// Case 2: Particle born inside target_1
   else if (track->GetCurrentStepNumber() == 1 &&
            preVol && preVol->GetName() == "volTPCActive_PV")
   {
       isInTarget1 = true;
   }
   */ 
   if (isInTarget1)
   {
       G4double ke = track->GetKineticEnergy() / keV;
       G4ThreeVector pos = postPoint->GetPosition();
       G4ThreeVector dir = track->GetMomentumDirection();
   
       G4double dirX = dir.x();
       G4double dirY = dir.y();
       G4double dirZ = dir.z();
   
       // --- Neutrons ---
       if (particleDef == G4Neutron::Definition() && !trackInfo->HasEnteredTarget1Neutron())
       {
           if (fEventAction)
               fEventAction->AddNeutronKinAtLAr(
                   ke,
                   pos.x() / cm, pos.y() / cm, pos.z() / cm,
                   dirX, dirY, dirZ
               );
   
           trackInfo->SetEnteredTarget1Neutron(true);
       }
   
       // --- Gammas ---
       else if (particleDef == G4Gamma::Definition() && !trackInfo->HasEnteredTarget1Gamma())
       {
           if (fEventAction)
               fEventAction->AddGammaKinAtLAr(
                   ke,
                   pos.x() / cm, pos.y() / cm, pos.z() / cm,
                   dirX, dirY, dirZ
               );
   
           trackInfo->SetEnteredTarget1Gamma(true);
       }
    }


    /* -------------  end filter   -------------------- */

    // Check if the post-step volume is valid
    G4StepPoint* thePostPoint = theStep->GetPostStepPoint();
    G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();

    if (!thePostPV) {
        // Out of world
        fExpectedNextStatus = Undefined;
        return;
    }
}
