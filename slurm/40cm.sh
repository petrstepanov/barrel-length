#!/bin/bash

#SBATCH --partition=priority
#SBATCH --account=eic
#SBATCH --mem-per-cpu=512
#SBATCH --time=12:00:00
#SBATCH --output=eres_40cm.out
#SBATCH --error=eres_40cm.err

# Pass macros to executable
./barrel ./gdml/40cm/mother.gdml ./macros/batch.mac
