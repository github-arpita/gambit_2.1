//  GAMBIT: Global and Modular BSM Inference Tool
//  *********************************************
///  \file
///
///  declaration for scanner module
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Gregory Martinez
///          (gregory.david.martinez@gmail.com)
///  \date 2013 August
///        2014 Feb
///
///  \author Pat Scott
///          (p.scott@imperial.ac.uk)   
///  \date 2014 Dec
///
///  *********************************************

#ifndef GAMBIT_PLUGIN_DEFS_HPP
#define GAMBIT_PLUGIN_DEFS_HPP

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <typeinfo>

#include "plugin_exception.hpp"

namespace Gambit
{
        namespace Plugin
        {
                struct factoryBase
                {
                        virtual void *operator()() = 0;
                        virtual void remove(void *) = 0;
                        virtual ~factoryBase(){}
                };
                
                template <typename T>
                struct funcFactory : factoryBase
                {
                        T *func;
                        funcFactory (T *in) : func(in) {}
                        void *operator()(){return *(void**)&func;}
                        void remove(void * in){};
                };
                
                template <typename T>
                struct classFactory : factoryBase
                {
                        void *operator()(){return (void*) new T;}
                        void remove(void *in){delete (T *) in;}
                };
                
                /*Structure that contains all the data inputed by ScannerBit*/
                struct gambitData
                {
                        std::string name;
                        std::string version;
                        std::vector <void *> inputData;
                        std::vector <void (*)(gambitData &)> inits;
                        std::map<std::string, factoryBase *> outputFuncs;
                        std::type_info const &(*main_type)(void);
                        
                        gambitData(std::string name) : name(name) {}
                        ~gambitData()
                        {
                                for (auto it = outputFuncs.begin(), end = outputFuncs.end(); it != end; it++)

                                {
                                        delete it->second;
                                }
                        }
                };  
        }
}

#endif
