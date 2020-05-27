//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Fronted header for the DarkSUSY backend
///
///  Compile-time registration of available
///  functions and variables from this backend.
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Torsten Bringmann
///          (torsten.bringmann@fys.uio.no)
///  \date 2020 February, 2020
///
///
///  *********************************************

#define BACKENDNAME DarkSUSY_generic_wimp
#define BACKENDLANG FORTRAN
#define VERSION 6.2.2
#define SAFE_VERSION 6_2_2

// Load the library
LOAD_LIBRARY

// Functions used in DS frontend
BE_FUNCTION(dsinit, void, (), "dsinit_", "dsinit")

// Convenience functions (registration)
// (none needed so far, for application in DarkBit_standalone_WIMP)

// Functions used in RelicDensity.cpp
BE_FUNCTION(dsanwx, double, (double&), "dsanwx_", "dsanwx")
BE_FUNCTION(dsrdcom, void, (), "dsrdcom_", "dsrdcom")
BE_FUNCTION(dsrdstart, void, (int&, double(&)[1000], double(&)[1000], int&, double(&)[1000], double(&)[1000], int&, double(&)[1000]), "dsrdstart_", "dsrdstart")
BE_FUNCTION(dsrdens, void, (double(*)(double&),double&,double&,int&,int&,int&), "dsrdens_", "dsrdens")

// Functions used in GamYields.cpp
BE_FUNCTION(dsanyield_sim, double, (double&,double&,int&,char*,int&,int&,int&), "dsanyield_sim_", "dsanyield_sim")

// Functions used in SunNeutrinos.cpp
BE_FUNCTION(dssenu_capsuntab, double, (const double&, const double&, const double&, const double&), "dssenu_capsuntab_", "cap_Sun_v0q0_isoscalar_DS")

// Halo model common blocks
BE_VARIABLE(dshmcom, DS_HMCOM, "dshmcom_", "dshmcom")
BE_VARIABLE(dshmframevelcom, DS_HMFRAMEVELCOM, "dshmframevelcom_", "dshmframevelcom")
BE_VARIABLE(dshmisodf, DS_HMISODF, "dshmisodf_", "dshmisodf")
BE_VARIABLE(dshmnoclue, DS_HMNOCLUE, "dshmnoclue_", "dshmnoclue")

// Common blocks in the DarkSUSY core library
BE_VARIABLE(ddcomlegacy, DS_DDCOMLEGACY, "ddcomlegacy_", "ddcomlegacy") //DD
BE_VARIABLE(rdtime, DS_RDTIME,     "rdtime_",    "rdtime")    // RD timeout
BE_VARIABLE(rdpars, DS_RDPARS,     "rdpars_",    "rdpars")    // gRD Parameters

// Convenience functions (registration)
BE_CONV_FUNCTION(dsgenericwimp_nusetup, void, (const double(&)[29], const double(&)[29][3], const double(&)[15], const double(&)[3], const double&, const double&), "DS_nuyield_setup")
BE_CONV_FUNCTION(DS_neutral_h_decay_channels, std::vector<std::vector<str>>, (), "get_DS_neutral_h_decay_channels")
BE_CONV_FUNCTION(DS_charged_h_decay_channels, std::vector<std::vector<str>>, (), "get_DS_charged_h_decay_channels")
BE_CONV_FUNCTION(neutrino_yield, double, (const double&, const int&, void*&), "nuyield")
// Functions used by convenience functions
BE_FUNCTION(dsseyield_sim_ls, double, (const double&, const double&, const double&, const int&, const int&, const int&, const int&, const int&, const int&, const char*, const int&, const int&, const int&), "dsseyield_sim_ls_", "raw_nuyield_sim")

// Undefine macros to avoid conflict with other backends
#include "gambit/Backends/backend_undefs.hpp"
