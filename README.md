# A study for the EIC barrel Panda calorimeter design

![Standard and hybrid barrel sleeve designs](https://github.com/petrstepanov/barrel-length/blob/main/resources/design.png?raw=true "Standard and hybrid barrel sleeve designs")
<p align="center"><i>Standard and hybrid barrel sleeve designs.</i></p>

Project goal is to determine the discrepances between energy deposition in two different detector barrel geometries.

For the sake of simplicity the simulation is narrowed down from a 3D case to a 2D case:

* Instead of a complete barrel, a single sleeve is imported into the simulation.
* Particle source is setup to emit electrons from a single point in a 2π angle within the sleeve plane.

GDML files for the standard and hybrid are provided by [Joshua Crafts](mailto:crafts@cua.edu).

Energy deposition studies should predict the optimal geometry to be used in the detector design.

Particle emission point is set on the intersection of the lines going through the volume centers of the very left and very right crystals. Electrons are shot rendomly in the plane of the crystals' sleeve in a 2π angle. Screenshot below visualizes one 10 GeV e− event.

![One 10 GeV e− event](https://github.com/petrstepanov/barrel-length/blob/main/resources/beam-on.png?raw=true "One 10 GeV e− event")
<p align="center"><i>One 10 GeV e− event visualization in Geant4.</i></p>

## How to Compile the Program

1. Have your Geant4 environment ready.
2. Check out and compile the program:

```
git clone https://github.com/petrstepanov/barrel-length
mkdir barrel-length-build
cd barrel-length-build
cmake ./../barrel-length/
make -j`nproc`
```

## Visual Validation of the Setup

Launch the program and pass corresponding GDML file as a command-line parameter:

```
barrell ./gdml/40cm/mother.gdml
```

Shoot desired number of the events via entering `/run/beamOn 10` command in the Geant4 interactive session.

## Obtained Results

To obtain results with high statistics, simulation should be preferrably run on a super computer. Pass batch input macro file as a second parameter:

```
barrell ./gdml/standard/mother.gdml ./macros/batch.mac
```

Two SLURM scripts are added to facilitate the program run on the supercomputer environment:
```
sbatch ./slurm/40cm.sh
sbatch ./slurm/hybrid.sh
sbatch ./slurm/45cm.sh
```

## Results

ROOT script `./draw/src/plotEdep.cpp` plots and calculates energy resolutions for selected output files. Plot below demonstrates Crystal Ball fits of the energy deposition histograms for above configurations.

![Comparison of the energy resolutions](https://github.com/petrstepanov/barrel-length/blob/main/resources/results.png?raw=true "Comparison of the energy resolutions")
<p align="center"><i>Comparison of the energy resolutions obtained with 6 GeV electrons.</i></p>

Energy resolutions can also be obtained from the ROOT histograms' parameters. Table below outlines the results:

| Energy Resolution at 6 GeV, electrons     | 40 cm         | 45 cm         | Hybrid        |
| ----------------------------------------- | ------------- | ------------- | ------------- |
| Crystal ball gauss sigma / gauss mean, %  | 1.703 ± 0.007 | 1.586 ± 0.007 | 1.713 ± 0.007 |
| Histogram std. dev. / histogram mean, %   | 2.983 ± 0.007 | 2.370 ± 0.007 | 2.978 ± 0.007 |

Feel free to reach me out at stepanovps@gmail.com with questions and concerns.