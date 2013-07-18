//////////////////////////////////////////////////////////
// Simple example for GAMBIT Ini-file parser
//
// Christoph Weniger <c.weniger@uva.nl>
// May, June, July 2013
//////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include "yaml-cpp/yaml.h"
#include <yaml_parser.hpp>

namespace GAMBIT
{
  namespace IniParser
  {
    int IniFile::readFile(std::string filename)
    {
      // Read inifile file
      std::vector<YAML::Node> roots = YAML::LoadAllFromFile(filename);

      // Set central nodes
      parametersNode = roots[0];
      YAML::Node outputNode = roots[1];
      YAML::Node auxNode = roots[2];
      keyValuePairNode = roots[3];

      // Read likelihood/observables
      for(YAML::const_iterator it=outputNode.begin(); it!=outputNode.end(); ++it)
      {
        observables.push_back((*it).as<Types::Observable>());
      }

      // Read auxiliaries
      for(YAML::const_iterator it=auxNode.begin(); it!=auxNode.end(); ++it)
      {
        auxiliaries.push_back((*it).as<Types::Observable>());
      }
    }
  }
}
