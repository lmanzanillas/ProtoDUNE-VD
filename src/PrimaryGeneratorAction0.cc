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
/// \file eventgenerator/particleGun/src/PrimaryGeneratorAction0.cc
/// \brief Implementation of the PrimaryGeneratorAction0 class
//
//
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo..........

#include "PrimaryGeneratorAction0.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........
PrimaryGeneratorAction0::PrimaryGeneratorAction0(G4ParticleGun* gun)
    : fParticleGun(gun) {
    // Set neutron energy (change this to your desired value)
    fNeutronEnergy = 2.45 * MeV;  // Example: 2.5 MeV D-T neutrons
    position = G4ThreeVector(0., 0., 0.);
    GeneratePosition();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........
void PrimaryGeneratorAction0::GeneratePosition() {
    G4double xg = -20 * cm + (G4UniformRand() - 0.5) * cm;
    G4double yg = -580 * cm + (G4UniformRand() - 0.5) * cm;
    G4double zg = 190 * cm + (G4UniformRand() - 0.5) * cm;
    position.set(xg, yg, zg);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........
void PrimaryGeneratorAction0::GeneratePrimaries(G4Event* anEvent) {
    const G4double pulseDuration = 400. * microsecond;
    const G4int neutronsPerPulse = 100;  // Number of neutrons in the pulse

    for (G4int i = 0; i < neutronsPerPulse; ++i) {
        G4double emissionTime = G4UniformRand() * pulseDuration;

        G4double theta = 2. * pi * G4UniformRand();
        G4double phi = acos(2. * G4UniformRand() - 1.);
        G4double ux = sin(phi) * cos(theta);
        G4double uy = sin(phi) * sin(theta);
        G4double uz = cos(phi);
        G4ThreeVector direction(ux, uy, uz);

        // Re-generate position for each neutron (optional)
        GeneratePosition();

        G4PrimaryVertex* vertex = new G4PrimaryVertex(position, emissionTime);

        G4ParticleDefinition* particleDefinition =
            G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        G4PrimaryParticle* particle = new G4PrimaryParticle(particleDefinition);
        particle->SetMomentumDirection(direction);
        particle->SetKineticEnergy(fNeutronEnergy);

        vertex->SetPrimary(particle);
        anEvent->AddPrimaryVertex(vertex);
    }
}
