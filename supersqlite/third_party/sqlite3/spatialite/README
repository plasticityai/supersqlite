  --------------------- libspatialite ------------------------

PLEASE read the following information.

1 - Installation
2 - Required dependencies
3 - Build notes
  3.1: Building on Linux
  3.2: Building on Mac OS X
  3.3: Building on Windows
    3.3.1: using MinGW / MSYS
    3.3.2: using Visual Studio .NET
  
=====================================================================

1. Installation:
=================

The default destination path prefix for installed files is /usr/local.
Results from the installation script will be placed into subdirectories
include and lib.  If this default path prefix is appropriate, then execute:

    ./configure

If another path prefix is required, then execute:

    ./configure --prefix=/my/path

In either case, the directory of the prefix path must exist and be
writable by the installer.

After executing configure, execute:

    make
    make install

Or even better, you can *strip* any executable binary, in order
to eliminate any debugging symbol, and thus widely reducing
their size:

    make install-strip

2. Required dependencies:
=========================
    
The main external dependencies needed in order to build 'libspatialite' 
are:
 - SQLite 3 (http://www.sqlite.org)
    This is a hard dependency - you can't build libspatialite without it. SQLite
    version 3.7.3 or later is strongly preferred - if you have an earlier
    version then you will need to pass --enable-geocallbacks=no to the
    ./configure script.
    
 - PROJ.4 (http://trac.osgeo.org/proj/)
    This is strongly recommended, unless you have a particular purpose in mind
    for your libspatialite build, and know that you won't need it. It is usually
    available as a package, and libspatialite is pretty flexible about versions.

 - GEOS (http://trac.osgeo.org/geos/)
    This is strongly recommended, unless you have a particular purpose in mind
    for your libspatialite build, and know that you won't need it. It is usually
    available as a package, but libspatialite will have more capability if you
    use version 3.3.0 or later so make sure that the package is recent enough.
    Use --enable-geosadvanced=no argument to the ./configure script if you want
    to use an earlier version of GEOS.

 - FreeXL (http://www.gaia-gis.it/FreeXL/)
    This is recommended if you want to be able to import data from Microsoft
    Excel format (.xls suffix) files. If you do not wish to use it, you will
    need to pass --enable-freexl=no to the ./configure script. Version 0.0.4
    or later is required.

Note that you need development code (e.g. -dev packages on Debian Linux and 
derivatives such as Ubuntu, or -devel packages on most other Linux
distributions).


ICONV [Windows]
---------------
When building on Windows, then you also need to provide iconv to ensure that
appropriate character set conversions are available. This dependency is not 
usually an issue when building on Linux or Mac OS X, because these systems 
provide iconv as a standard component.

For Windows the preferred solution is to download and install the pre-built 
iconv binaries and related files from:
http://gnuwin32.sourceforge.net/packages/libiconv.htm

3 - Build notes
===============

3.1: Building on Linux and similar systems
------------------------------------------

Building libspatialite on Linux and similar systems such as BSD or other Unix
variants does not require any special settings. If you have unpacked the sources
as ./libspatialite-3.1.0, then the required steps are:

# cd libspatialite-3.1.0
# ./configure
# make
# sudo make install
#     or (in order to save some disk space)
# sudo make install-strip

3.2: Building on Mac OS X
-------------------------

Building 'libspatialite' on Mac OS X very similar to Linux. You simply have to
set explicitly some environment variables. If you have unpacked the sources as 
./libspatialite-3.1.0, then the required steps are:

# cd libspatialite-3.1.0
# export "CFLAGS=-I/opt/local/include"
# export "LDFLAGS=-I/opt/local/lib"
# ./configure 
# make
# sudo make install
#     or (in order to save some disk space)
# sudo make install-strip

IMPORTANT NOTICE: this will build an executable for your specific platform.
That is, when building on a PPC Mac, the resulting binary will be be for PPC.
Similarly, when building on Intel Mac, resulting binary will be for  Intel.

3.3: Building on Windows
------------------------

On Windows systems you can choose using two different compilers:
- MinGW / MSYS
  This represents a smart porting of a minimalistic Linux-like
  development toolkit
- Microsoft Visual Studio .NET
  This is the standard platform development toolkit from Microsoft.

3.3.1: using MinGW / MSYS
-------------------------

We assume that you have already installed the MinGW compiler and the MSYS shell.
Building 'libspatialite' under Windows is then more or less like building
on any other UNIX-like system. If you have unpacked the sources as 
C:\libspatialite-3.1.0, then the required steps are:

$ cd c:/libspatialite-3.1.0
$ export "CFLAGS=-I/usr/local/include"
$ export "LDFLAGS=-L/usr/local/lib"
$ ./configure --target=mingw32
$ make
$ make install-strip
$     or (in order to save some disk space)
$ make install-strip


3.3.2: using Microsoft Visual Studio .NET
-----------------------------------------

We assume that you have already installed Visual Studio enabling the command
line tools. Note that you are expected to the Visual Studio command prompt shell
rather than the GUI build environment. If you have unpacked the sources as 
C:\libspatialite-3.1.0, then the required steps are:

> cd c:\libspatialite-3.1.0
> nmake /f makefile.vc
> nmake /f makefile.vc install

Please note: standard definitions in 'makefile.vc' assumes:
- enabling PROJ
- disabling GEOS

If you want to alter the default behaviour then make modifications in 
'makefile.vc'. Also note that 'libspatialite-geos.def' contains those external
symbols to be exported from the DLL when you build GEOS.

