#!/bin/bash

#SBATCH --partition=priority
#SBATCH --account=eic
#SBATCH --mem-per-cpu=512
#SBATCH --time=12:00:00
#SBATCH --output=eres_standard.out
#SBATCH --error=eres_standard.err

# Pass macros to executable
./barrel ./gdml/standard/mother.gdml ./macros/batch.mac
