MS=../contrib/MassSpectra
FS=$MS/flexiblesusy

FSMODEL1=$FS/models/CMSSM

FSINCLUDES="-I$FS -I$FS/src -I$FS/legacy -I$FS/config -I$FSMODEL1"
LIBMODEL=$FSMODEL1/libCMSSM.a
LIBFLEXI=$FS/src/libflexisusy.a
LIBLEGACY=$FS/legacy/liblegacy.a
FSLIBS="$LIBMODEL $LIBFLEXI $LIBLEGACY"

EIGEN=../../extras/eigen3

GSL=/usr/local/gsl/1.12-gcc/include
#GSL=/usr/include/gsl/

g++ -Wfatal-errors -std=c++11 -c -o run_tests.o run_tests.cpp -Iinclude -I$MS $FSINCLUDES -I$EIGEN -I$GSL
g++ -Wfatal-errors -pthread -std=c++11 -o run_tests run_tests.o $FSLIBS -lgsl -lgslcblas -llapack -latlas -lblas -lgfortran -L/usr/lib/gcc/x86_64-linux-gnu/4.6/
