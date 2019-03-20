/*
The purpose of this tools is to facilitate putting prototypes (declarations) next
to implementations (definitions) in a single source file of a C/C++ programs. 

Splits a single source file into multiple files.  Scans through the single
source file looking for lines of the form:

  #tranche filename ...

With the # mandatory at column 1 (like the old C preprocessor).  Upon finding
such a line, copies all following lines into the listed files, until reaching the
end marker:

  #endtranche

A next improvement of this file would be to support variables to be passed in,
as per a make file.

*/


void tranche(FILE *file){



}
