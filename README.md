## Artery LTE

Artery LTE merges the two projects [Artery](https://github.com/riebl/artery) and [veins-lte](https://github.com/floxyz/veins-lte).

## Installation

### Prerequisites

* Install [OMNeT++](http://www.omnetpp.org/).
  Version 4.4.2 is known to be working.
  * Adjust the C++ compiler flags (for use with artery-lte later) by changing
    the line starting with `CXXFLAGS` in `Makefile.inc` to
    ```
    CXXFLAGS = -std=c++11
    ```
* Install [SUMO](http://www.dlr.de/ts/en/desktopdefault.aspx/tabid-9883/16931_read-41000/).
 Versions 0.21.0 through 0.22.0 are known to be working.

### Build instructions

* Clone this repository and switch to the checked-out directory
* Switch to the `artery` branch:
  ```
  git checkout artery
  ```
* Switch into vanetza subdirectory and build vanetza with CMake first:
  ```
  cd vanetza
  mkdir build
  cd build
  cmake ..
  make
  ```
* Switch back to top-level directory and build artery-lte:
  ```
  make makefiles
  make
  ```

## Run the example:

* In a shell switch to the `veins/` subdirectory and run the script `sumo-launchd.py`:
  ```
  cd veins/
  ./sumo-launchd.py -vv -c sumo-gui
  ```
* In another shell, run the artery-lte example:
  ```
  cd veins/examples/artery/
  ./run
  ```

