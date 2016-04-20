ArteryLTE - An Inter-Vehicle Communication (IVC) simulation framework.
======================================================================

ArteryLTE is a holistic IVC simulation framework capable of simulating
different communication technologies, namely the [ETSI
ITS-G5](https://www.etsi.org/deliver/etsi_es/202600_202699/202663/01.01.00_50/es_202663v010100m.pdf)
and the cellular LTE protocol stacks. It thus allows for the analysis of
different communication strategies based on heterogeneous vehicular networks.

It combines [Artery](https://github.com/riebl/artery) and
[VeinsLTE](https://github.com/floxyz/veins-lte). Artery's middleware is
extended by the option of choosing either the ITS-G5 or the LTE stack for
communication. Upon message generation, the Artery services provide information
to the middleware in order to choose the appropriate communication technology.
A backend service is implemented as a static network node connected to the base
stations (eNodeBs) of the LTE network.

## Publication

ArteryLTE has been developed as part of a research project at the [Institute of
Operating Systems and Computer Networks (IBR)](https://www.ibr.cs.tu-bs.de) at
the [Technische Universität Braunschweig](https://www.tu-braunschweig.de).  It
will be released with a corresponding paper describing the detailed
architecture as well as simulation results.

## Installation

### Prerequisites

#### Boost 1.58.0
At the time of writing, the official Ubuntu packages contain version 1.58.0, the same version as artery-lte requires. So no other action is needed here at this time.

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
4. Tune InnoDB performance in /etc/mysql/my.cnf:
```
innodb_flush_log_at_trx_commit = 0
innodb_flush_method = O_DIRECT
innodb_buffer_pool_size = 2G
innodb_buffer_pool_instances = 2
innodb_log_file_size = 512M
```
This should improve write performance according to the following resources:
 - https://www.percona.com/blog/2014/01/28/10-mysql-settings-to-tune-after-installation/
 - https://mariadb.com/kb/en/mariadb/xtradbinnodb-server-system-variables/#innodb_flush_log_at_trx_commit
 - https://www.percona.com/blog/2014/05/23/improve-innodb-performance-write-bound-loads/
 - http://www.innovation-brigade.com/index.php?module=Content&type=user&func=display&tid=1&pid=2
 - https://www.percona.com/blog/2007/11/01/innodb-performance-optimization-basics/
 - http://ajaydivakaran.com/mysql-innodb-are-inserts-slowing-down/

### Build instructions

#### Checkout
```bash
$ git clone https://github.com/ibr-cm/artery-lte.git
$ git checkout artery-lte
```

#### Vanetza
The following packages are required.
  `asn1c cmake`

Make sure to use cmake 3.x.

In the vanetza subdir, follow these instructions:
```bash
$ mkdir build
$ cd build/
$ cmake -D VANETZA_GEONET_USE_PACKET_VARIANT=1  ..

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
