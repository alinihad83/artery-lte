# Artery LTE

Artery LTE merges the two projects [Artery](https://github.com/riebl/artery) and [veins-lte](https://github.com/floxyz/veins-lte).

## Installation

### Prerequisites

#### Boost 1.56.0
At the time of writing, the official Ubuntu packages only contain version 1.54.0, while Artery requires 1.56.0 at least. A shell script [install_boost.sh](https://gitlab.ibr.cs.tu-bs.de/cm-projects/2015-artery-lte/uploads/ae9781d7b95d1b383c453db56aba46fd/install_boost.sh) takes care of the installation process to `/usr/local/boost_1_56_0`.

Afterwards, you might need to set the permissions on that directory:
```bash
$ sudo chown -R root:root /usr/local/boost_1_56_0/
$ sudo chmod -R a+rX /usr/local/boost_1_56_0/
```

#### GeographicLib
Installing vanetza requires (amongst others) GeographicLib in version >= 1.37, which is not available as a binary/package for Ubuntu/Debian-based systems at the time of writing (Okt. 2015). To install the GeoLib, follow the cmake(!) instructions on the ​[GeographicLib Website](http://geographiclib.sourceforge.net/html/install.html).

#### SUMO
1. The following packages are required.  
  `libfox-1.6-dev libgdal-dev libxerces-c-dev libproj-dev`

2. Download and extract SUMO 0.22. Versions 0.21.0 through 0.22.0 are known to be working.

3. Configure the make-file by changing the current path to the SUMO folder and enter `$ ./configure`. Afterwards, make sure that the enabled features contain at least TRACI, PROJ, GDAL and GUI. If something is missing, check that all required packages are installed correctly.

4. `$ make -j5`

#### OMNeT++
1. The following packages are required.  
  `build-essential g++ bison gcc flex perl tcl-dev tk-dev zlib1g-dev 
default-jre doxygen graphviz libwebkitgtk-1.0.0 openmpi-bin libopenmpi-dev libcap-dev cmake`

2. Download and extract OMNeT++ 4.6. (Version 4.4.2 is known to be working as well).

3. From OMNeT++ 4.5 on, you might want to set `USE_CXX11` to `yes` in `configure.user` before calling `$ ./configure`. Consequently, OMNeT++ will be built with C++11.

4. `$ make -j5`

#### Environment Variables
Add the following to your environment (e.g., `.profile` or `.bashrc`):

```bash
export SUMO_HOME="$HOME/bin/Artery/sumo-0.22.0"
export OMNETPP_HOME="$HOME/bin/Artery/omnetpp-4.6"
PATH="$OMNETPP_HOME/bin/:$SUMO_HOME/bin/:$PATH"
```

#### MariaDB

1. Install the following packages:  
  `mariadb-server mariadb-client libmysqlclient18 libmysqlclient-dev 
libmysqlcppconn7 libmysqlcppconn-dev`

2. Secure your MariaDB installation.  
   `$ sudo mysql_secure_installation`

3. Create a standard db user and the artery database.
```bash
$ mysql -u root -p
MariaDB> CREATE DATABASE artery;
MariaDB> GRANT ALL ON artery.* TO omnetpp@localhost IDENTIFIED BY 'omnetpp';
MariaDB> flush privileges;
```

### Build instructions

#### Checkout
```bash
$ git clone git@gitlab.ibr.cs.tu-bs.de:cm-projects/2015-artery-lte.git
$ git checkout artery
$ git submodule update --init
```

#### Vanetza
The following packages are required.  
  `asn1c`

In the vanetza subdir, follow these instructions: 
```bash
$ mkdir build
$ cd build/
$ cmake -D Boost_INCLUDE_DIR=/usr/local/boost_1_56_0 -D VANETZA_GEONET_USE_PACKET_VARIANT=1  ..

You may have to run cmake multiple times. Make sure that two asn libraries are successfully linked.

$ make -j5
```
Optionally, include `-D CMAKE_BUILD_TYPE=Release`.

#### ArteryLTE
If you installed Boost manually (as described above), set the correct path in `veins/local.configure`.
Build ArteryLTE from its main directory:
```bash
$ make makefiles
$ make -j5 [MODE=release]
```
Note: Release mode seems to break the simulation.


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
