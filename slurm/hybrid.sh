#!/bin/bash

#SBATCH --partition=priority
#SBATCH --account=eic
#SBATCH --mem-per-cpu=512
#SBATCH --time=12:00:00
#SBATCH --output=eres_hybrid.out
#SBATCH --error=eres_hybrid.err

# Pass macros to executable
./barrel ./gdml/hybrid/mother.gdml ./macros/batch.mac
