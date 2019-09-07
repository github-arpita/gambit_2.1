//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Helpers for ColliderBit event loop functions.
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Abram Krislock
///          (a.m.b.krislock@fys.uio.no)
///
///  \author Aldo Saavedra
///
///  \author Andy Buckley
///
///  \author Chris Rogan
///          (crogan@cern.ch)
///  \date 2014 Aug
///  \date 2015 May
///
///  \author Pat Scott
///          (p.scott@imperial.ac.uk)
///  \date 2015 Jul
///  \date 2018 Jan
///  \date 2019 Jan
///
///  \author Anders Kvellestad
///          (anders.kvellestad@fys.uio.no)
///  \date   2017 March
///  \date   2018 Jan
///  \date   2018 May
///
///  *********************************************

#pragma once

#include <string>
#include <iostream>


namespace Gambit
{

  namespace ColliderBit
  {

    /// Special iteration labels for the loop controlled by operateLHCLoop
    enum specialIterations { BASE_INIT = -1,
                             COLLIDER_INIT = -2,
                             START_SUBPROCESS = -3,
                             XSEC_CALCULATION = -4,
                             COLLECT_CONVERGENCE_DATA = -5,
                             CHECK_CONVERGENCE = -6,
                             END_SUBPROCESS = -7,
                             COLLIDER_FINALIZE = -8,
                             BASE_FINALIZE = -9};
  }

}
