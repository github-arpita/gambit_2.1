//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Sterile RH Neutrino Model with differential masses
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///   
///  \author Tomas Gonzao  
///          (t.e.gonzalo@fys.uio.no)
///  \date 2017 December
///
///  *********************************************

#include "gambit/Models/model_macros.hpp"
#include "gambit/Models/model_helpers.hpp"
#include "gambit/Logs/logger.hpp"

#include "gambit/Models/models/SN_differential.hpp"


#define MODEL SN_differential 
  void MODEL_NAMESPACE::SN_differential_to_SN_dev (const ModelParameters &myP, ModelParameters &targetP)
  {

     logger()<<"Running interpret_as_parent calculations for SN_differential --> SN_dev."<<LogTags::info<<EOM;
     
     // Send all parameter values upstream to matching parameters in parent.
     // Ignore that some parameters don't exist in the parent, as these are set below.
     targetP.setValues(myP,false);

     // M2
     targetP.setValue("M_2", myP["M_1"]+myP["delta_M_2"]);

     // M3
     targetP.setValue("M_3", myP["M_1"]+myP["delta_M_3"]);

  }

#undef MODEL

