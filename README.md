# A study for the EIC barrel Panda calorimeter design

Project goal is to determine the discrepances between energy deposition in two different detector barrel geometries.

![Standard and hybrid barrel sleeve designs](https://github.com/petrstepanov/barrel-length/blob/main/resources/<img-path>?raw=true "Standard and hybrid barrel sleeve designs")
*Standard and hybrid barrel sleeve designs.*

For the sake of simplicity the simulation is narrowed down from a 3D case to a 2D case:

* Instead of a complete barrel, a single sleeve is imported into the simulation.
* Particle source is setup to emit electrons from a single point in a 2π angle within the sleeve plane.

GDML files for the standard and hybrid are provided by [Joshua Crafts](mailto:crafts@cua.edu).

Energy deposition studies should predict the optimal geometry to be used in the detector design.

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
barrell ./gdml/standard/mother.gdml
```

Shoot desired number of the events via entering `/run/beamOn 10` command in the Geant4 interactive session.

## Obtaining results

To obtain results with high statistics, simulation should be preferrably run on a super computer. Pass batch input macro file as a second parameter:

```
barrell ./gdml/standard/mother.gdml ./macros/batch.mac
```

## Results