# GAMBIT: Global and Modular BSM Inference Tool  
#************************************************
# \file                                          
#                                                
#  Cmake preprocessor variable configuration. 
#    
#************************************************
#                                                
#  Authors (add name and date if you modify):                                    
#                                                
#  \author Antje Putze
#          (antje.putze@lapth.cnrs.fr)              
#  \date 2015 Apr
#
#  \author Pat Scott
#          (p.scott@imperial.ac.uk)              
#  \date 2015 Apr
#                                               
#************************************************

include(CheckIncludeFiles)

message("${Yellow}-- Updating GAMBIT with config data${ColourReset}")

# Check for STL regex
if(COMPILER_SUPPORTS_CXX11 OR COMPILER_SUPPORTS_CXX0X)
  check_include_files(regex.h HAVE_REGEX_H)
endif()

# Check for link.h
check_include_files(link.h HAVE_LINK_H)

# Configure the file
set(outhdr "${PROJECT_SOURCE_DIR}/cmake/include/gambit/cmake/cmake_variables.hpp")
configure_file("${PROJECT_SOURCE_DIR}/cmake/cmake_variables.hpp.in" ${outhdr})
message("${BoldYellow}   Configured ${outhdr}${ColourReset}")

message("${Yellow}-- Updating GAMBIT with config data - done.${ColourReset}")
