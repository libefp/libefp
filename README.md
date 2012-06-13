# libefp

## Overview

This is work in progress. Public API can still change substantially.

The Effective Fragment Potential method (EFP) allows one to describe large
molecular systems by replacing chemically inert part of a system by a set of
Effective Fragments while performing regular ab initio calculation on the
chemically active part [1-7]. The libefp library is a full implementation of
the EFP2 method. It allows users to easily add EFP2 support to their favourite
quantum chemistry package.

The EFP2 potential consists of five energy terms: electrostatics, polarization,
dispersion, exchange repulsion, and charge transfer. A good description of the
EFP method (both EFP1 and EFP2 variants) and more references can be found in
the manual for the GAMESS quantum chemistry package (see GAMESS website at
http://www.msg.ameslab.gov/gamess/).

The libefp library is written and maintained by Ilya Kaliman. Send questions
and comments to ilya.kaliman@gmail.com.


## How to Install

To compile libefp from source issue the following commands:

	./configure --prefix=INSTALL_PATH --with-cblas-libs=CBLAS_LIBS

In the above command `INSTALL_PATH` is the path to install the library and
`CBLAS_LIBS` are libraries providing CBLAS implementation.

To compile the library issue:

	make

To run the test suit (optional) issue:

	make check

Finally, install the library:

	make install


## How to Use

To be able to use the library you should include efp.h file in your code:

	#include <efp.h>

To get the idea of how to use the library see `tests/test_common.c` file. The
description of the specific public functions and structures provided by the
library is available at http://libefp.github.com/.

To obtain an executable program link with provided shared (recommended) or
static libraries:

	gcc -o prog prog.c -lefp

or

	gcc -o prog prog.c CBLAS_LIBS -lm libefp.a

In the above command `CBLAS_LIBS` are CBLAS libraries. You should substitute
`CBLAS_LIBS` with the command to link against the CBLAS library implementation
of your choice. Probably you want to use the same `CBLAS_LIBS` as you specified
during configuration.

You can use pkg-config to easily get all required arguments:

	gcc -o prog prog.c $(pkg-config --libs --cflags libefp)

In this case the program will be linked against CBLAS libraries specified
during configuration.

To be able to compute all EFP energy terms the calling program must set
callback functions which will provide various integrals (e.g., overlap or
kinetic energy integrals over Gaussian basis functions) as well as information
about quantum subsystem. Please see API documentation at
http://libefp.github.com/ for the full description of public interface.


## How to create custom EFP fragment types

The library uses EFP potential files in format generated by GAMESS quantum
chemistry package (see http://www.msg.ameslab.gov/gamess/). A version of GAMESS
from August 11, 2011 is the currently a recommended and tested version. A set
of pre generated library fragments are available in fraglib directory. If you
want to generate parameters for custom fragments you should create GAMESS
makefp job input file similar to the `fraglib/makefp.inp`. Using this input file
you can create EFP parameters for custom fragment types.

After you created `.efp` file using GAMESS you should rename the fragment by
replacing `$FRAGNAME` with your name of choice (e.g. rename `$FRAGNAME` to
`$MYH2O`).

For a complete description of EFP data file format consult FRAGNAME section in
the GAMESS manual (see http://www.msg.ameslab.gov/gamess/).


## Information for libefp contributors

- The main design principle for the libefp library is Keep It Simple. All
  code should be easy to read and to understand. It should be easy to
  integrate the library into programs written in different programming
  languages. So the language of choice is C and no fancy OO hierarchies.

- Be consistent in coding style when adding new code. Consistency is more
  important than particular coding style. Use descriptive names for variables
  and functions. The bigger the scope of the symbol the longer its name should
  be. Look at the sources and maintain similar style for new code.

- Use tabs for indentation. Set tab width to 8 spaces. Keep code within 80
  columns. If you have more than 3 levels of indentation in some function this
  usually means that you should rewrite the code.

- As with most quantum chemistry methods EFP can require large amounts of
  memory. The guideline for developers here is simple: ALWAYS check for memory
  allocation errors in your code and return `EFP_RESULT_NO_MEMORY` on error.

- The code is reentrant which means that it is safe to use two different efp
  objects from two different threads. NEVER use mutable global state as it
  will break this. Store all mutable data in the efp object.

- Use `--enable-developer-flags` in configuration and make sure that
  compilation produces no warnings. Use `make check` to make sure that all new
  code passes the test cases.

- To debug using tests do `libtool --mode=execute gdb ./mytest` in tests
  directory. This will start gdb so you will be able to set breakpoints and
  inspect variables in mytest test program.


## References

1. Effective fragment method for modeling intermolecular hydrogen bonding
   effects on quantum mechanical calculations. J.H.Jensen, P.N.Day, M.S.Gordon,
   H.Basch, D.Cohen, D.R.Garmer, M.Krauss, W.J.Stevens in "Modeling the
   Hydrogen Bond" (D.A. Smith, ed.) ACS Symposium Series 569, 1994, pp
   139-151.

2. An effective fragment method for modeling solvent effects in quantum
   mechanical calculations. P.N.Day, J.H.Jensen, M.S.Gordon, S.P.Webb,
   W.J.Stevens, M.Krauss, D.Garmer, H.Basch, D.Cohen. J.Chem.Phys. 105,
   1968-1986 (1996).

3. Solvation of the Menshutkin Reaction: A Rigorous test of the Effective
   Fragment Model. S.P.Webb, M.S.Gordon. J.Phys.Chem.A 103, 1265-73 (1999).

4. The Effective Fragment Potential Method: a QM-based MM approach to modeling
   environmental effects in chemistry. M.S.Gordon, M.A.Freitag,
   P.Bandyopadhyay, J.H.Jensen, V.Kairys, W.J.Stevens. J.Phys.Chem.A 105,
   293-307 (2001).

5. The Effective Fragment Potential: a general method for predicting
   intermolecular interactions. M.S.Gordon, L.V.Slipchenko, H.Li, J.H.Jensen.
   Annual Reports in Computational Chemistry, Volume 3, pp 177-193 (2007).

6. Water-benzene interactions: An effective fragment potential and correlated
   quantum chemistry study. L.V.Slipchenko, M.S.Gordon. J.Phys.Chem.A 113,
   2092-2102 (2009).

7. Damping functions in the effective fragment potential method. L.V.Slipchenko,
   M.S.Gordon. Mol.Phys. 107, 999-1016 (2009).