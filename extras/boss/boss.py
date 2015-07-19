#!/usr/bin/python

########################################
#                                      #
#   BOSS - Backend-On-a-Stick Script   #
#                                      #
########################################

#
# This is a Python package for making the classes of a C++ library 
# available for dynamic use through the 'dlopen' system.
# BOSS relies on 'gccxml' being installed and callable from the command line.
#
# Default usage:
# ./boss [list of class header files]
# 

import xml.etree.ElementTree as ET
import os
import sys
import warnings
import shutil
import glob
import subprocess
import pickle
import copy
from collections import OrderedDict
from optparse import OptionParser

import modules.cfg as cfg
import modules.gb as gb
import modules.classutils as classutils
import modules.classparse as classparse
import modules.funcparse as funcparse
import modules.funcutils as funcutils
import modules.utils as utils
import modules.filehandling as filehandling
import modules.infomsg as infomsg


# ====== main ========

def main():

    print
    print
    print '  ========================================'
    print '  ||                                    ||'
    print '  ||  BOSS - Backend-On-a-Stick-Script  ||'
    print '  ||                                    ||'
    print '  ========================================'
    print 
    print 


    # Parse command line arguments and options
    parser = OptionParser(usage="usage: %prog [options] <input files>",
                          version="%prog 0.1")
    parser.add_option("-c", "--gccxml-compiler",
                      dest="gccxml_compiler_in",
                      default="",
                      help="Set gccxml to mimic COMPILER.",
                      metavar="COMPILER")
    parser.add_option("-l", "--list",
                      action="store_true",
                      dest="list_flag",
                      default=False,
                      help="Output a list of the available classes and functions.")
    parser.add_option("-g", "--generate-only",
                      action="store_true",
                      dest="generate_only_flag",
                      default=False,
                      help="Stop BOSS after code generation step.")
    parser.add_option("-t", "--types-header",
                      action="store_true",
                      dest="types_header_flag",
                      default=False,
                      help="Generate loaded_types.hpp. (BOSS continues from a previous saved state, no other input required.)")
    parser.add_option("-r", "--reset-source",
                      dest="reset_info_file_name",
                      default="",
                      help="Reset source code that has been mangled by BOSS. Requires a RESET_INFO_FILE generated by BOSS.",
                      metavar="RESET_INFO_FILE")
    # parser.add_option("-G", "--to-gambit",
    #                   dest="main_gambit_path",
    #                   default="",
    #                   help="Copy all the code needed by GAMBIT to the correct locations within the main GAMBIT path GAMBIT_PATH.",
    #                   metavar="GAMBIT_PATH")


    (options, args) = parser.parse_args()

    # Check for conflicting options
    if options.generate_only_flag and options.types_header_flag:
        print 
        print 'Conflicting flags: --generate-only and --types-header'
        print 

        sys.exit()


    # Check that arguments list is not empty
    if (len(args) == 0) and not (options.types_header_flag or options.reset_info_file_name):

        print 
        print 'Missing input arguments. For instructions, run: boss.py --help'
        print 

        sys.exit()

    # If gccxml compiler is given as command line input, update cfg.gccxml_compiler 
    if options.gccxml_compiler_in != '':
        cfg.gccxml_compiler = options.gccxml_compiler_in


    #
    # If types_header_flag is True: Load saved variables, parse factory function files and generate loaded_types.hpp
    #
    if options.types_header_flag:
        with open('savefile.boss') as f:
            gb.classes_done, gb.factory_info = pickle.load(f)

        print '(Continuing from saved state.)'
        print 
        print 
        print 'Parsing the generated factory function source files:'
        print '----------------------------------------------------'
        print 

        factory_xml_files = filehandling.parseFactoryFunctionFiles()

        print
        print
        print 'Generating file loaded_types.hpp:'
        print '---------------------------------'
        print 

        filehandling.createLoadedTypesHeader(factory_xml_files)

        print
        print 'Done!'
        print '-----' 
        print

        sys.exit()

    #
    # If reset option is used: Run the reset function and then quit
    #
    if options.reset_info_file_name != '':
        print
        print
        print 'Reset source code:'
        print '------------------'
        print 
        print '  Input file: ' + options.reset_info_file_name
        print 

        filehandling.resetSourceCode(options.reset_info_file_name)

        sys.exit()



    # If the output directory is to be used, 
    # make sure it does not already exist.
    if (not options.list_flag) and (not options.types_header_flag):
        if os.path.isdir(cfg.extra_output_dir):
            print 
            print "The output directory '%s' already exists." % (cfg.extra_output_dir)
            print "Please remove it or set a different output directory."
            print 

            sys.exit()



    # Get the input file names from command line input. 
    input_files = args

    # Sort them to make sure screen output is identical regardless of ordering of input files.
    input_files.sort()




    #
    # Run gccxml for all input header/source files
    #

    print
    print 'Parsing the input files:'
    print '------------------------'
    print 

    # Create the temp output dir if it does not exist
    filehandling.createOutputDirectories(selected_dirs=['temp'])

    xml_files = []
    for input_file_path in input_files:

        # Get path and filename for the input file
        input_file_dir, input_file_short_name = os.path.split(input_file_path)

        # Construct file name for xml file produced by gccxml
        xml_output_path = os.path.join(gb.boss_temp_dir, input_file_path.replace('/','_').replace('.','_') + '.xml' )

        # List all include paths
        # include_paths_list = [cfg.include_path] + cfg.additional_include_paths

        # Timeout limit and process poll interval [seconds]
        timeout = 20.
        poll = 0.2

        # Run gccxml
        try:
            # utils.gccxmlRunner(input_file_path, include_paths_list, xml_output_path, timeout_limit=timeout, poll_interval=poll)
            utils.gccxmlRunner(input_file_path, cfg.include_paths, xml_output_path, timeout_limit=timeout, poll_interval=poll)
        except:
            raise

        # Append xml file to list of xml files
        xml_files.append(xml_output_path)

    #
    # END: Run gccxml on input files
    #
    print


    #
    # If -l option is given, print a list of all classes and functions, then exit.
    #

    if options.list_flag:

        all_class_names    = []
        all_function_names = []

        for xml_file in xml_files:

            tree = ET.parse(xml_file)
            root = tree.getroot()

            # Set the global xml id dict. (Needed by the functions called from utils.)
            gb.id_dict = OrderedDict([ (el.get('id'), el) for el in root.getchildren() ]) 
           
            # Find all available classes
            for el in (root.findall('Class') + root.findall('Struct')):
                
                # Skip classes that are not loadable (incomplete, abstract, ...)
                try:
                    is_loadable = utils.isLoadable(el)
                except KeyError:
                    continue

                if not is_loadable:
                    continue

                demangled_class_name = el.get('demangled')
                if utils.isNative(el):
                    all_class_names.append(demangled_class_name)

            # Find all available functions
            for el in root.findall('Function'):
                if 'demangled' in el.keys():

                    func_name_full = el.get('demangled')

                    # demangled_name = el.get('demangled')

                    # # Template functions have more complicated 'demangled' entries...
                    # if '<' in demangled_name:
                    #     func_name_full = demangled_name.split(' ',1)[1].split('(',1)[0]
                    # else:
                    #     func_name_full = demangled_name.split('(',1)[0]

                    if utils.isNative(el):
                        all_function_names.append(func_name_full)

        # END: Loop over xml files

        # Remove duplicates
        all_class_names    = list(OrderedDict.fromkeys(all_class_names))
        all_function_names = list(OrderedDict.fromkeys(all_function_names))
        
        # Output lists
        print 'Classes:'
        print '--------'
        for demangled_class_name in all_class_names:
            print '  - ' + demangled_class_name
        print
        print 'Functions:'
        print '----------'
        for demangled_func_name in all_function_names:
            print '  - ' + demangled_func_name
        print

        # Exit
        sys.exit()


    #
    # Analyse types and functions
    #

    print 'Analysing types and functions:'
    print '------------------------------'
    print

    #
    # Read all xml elements of all files and store in two dict of dicts: 
    #
    # 1. all_id_dict:    file name --> xml id --> xml element
    # 2. all_name_dict:  file name --> name   --> xml element
    #
    utils.xmlFilesToDicts(xml_files)


    #
    # Look up potential parent classes and add to cfg.loaded_classes
    #

    if cfg.load_parent_classes:
        utils.addParentClasses()



    #
    # Remove from cfg.loaded_classes all classes that are not loadable (not found, incomplete, abstract, ...)
    #

    # Remove duplicates from cfg.loaded_classes
    cfg.loaded_classes = list(OrderedDict.fromkeys(cfg.loaded_classes))

    is_loadable = dict.fromkeys(cfg.loaded_classes, False)

    # Determine which requested classes are actually loadable
    for xml_file in xml_files:

        # Initialise global dicts
        gb.xml_file_name = xml_file
        utils.initGlobalXMLdicts(xml_file, id_and_name_only=True)

        # # Set the global dicts for the current xml file
        # gb.id_dict   = gb.all_id_dict[xml_file]
        # gb.name_dict = gb.all_name_dict[xml_file]

        # Loop over all named elements in the xml file
        for full_name, el in gb.name_dict.items():

            if el.tag in ['Class', 'Struct']:

                # If a requested class is loadable, set the entry in is_loadable to True
                if full_name in cfg.loaded_classes:

                    if utils.isLoadable(el, print_warning=False):

                        is_loadable[full_name] = True

    # Remove from cfg.loaded_classes those that are not loadable
    for full_name in is_loadable.keys():

        if not is_loadable[full_name]:

            cfg.loaded_classes.remove(full_name)

    # Output info on why classes are not loadable
    for xml_file in xml_files:

        # Initialise global dicts
        gb.xml_file_name = xml_file
        utils.initGlobalXMLdicts(xml_file, id_and_name_only=True)

        for full_name in is_loadable.keys():

            if not is_loadable[full_name]:

                # If the class exists, print reason why it is not loadable.
                if full_name in gb.name_dict.keys():
                    el = gb.name_dict[full_name]
                    utils.isLoadable(el, print_warning=True)            

                # If the class simply does not exist
                else:
                    reason = "Class not found."
                    infomsg.ClassNotLoadable(full_name, reason).printMessage()



    #
    # Fill the gb.accepted_types list
    #
    utils.fillAcceptedTypesList()
    


    #
    # Remove from cfg.loaded_functions all functions that are not loadable
    #

    # Remove duplicates from cfg.loaded_functions
    cfg.loaded_functions = list(OrderedDict.fromkeys(cfg.loaded_functions))

    for xml_file in xml_files:

        # Initialise global dicts
        gb.xml_file_name = xml_file
        utils.initGlobalXMLdicts(xml_file, id_and_name_only=True)

        # # Set the global dicts for the current xml file
        # gb.id_dict   = gb.all_id_dict[xml_file]
        # gb.name_dict = gb.all_name_dict[xml_file]

        # Loop over all named elements in the xml file
        for full_name, el in gb.name_dict.items():

            if el.tag == 'Function':

                # Get function name
                try:
                    func_name = funcutils.getFunctionNameDict(el)
                    func_name_long_templ_args = func_name['long_templ_args']
                except KeyError:
                    func_name_long_templ_args = 'UNKNOWN_NAME'
                except:
                    print '  ERROR: Unexpected error!'
                    raise

                if func_name_long_templ_args in cfg.loaded_functions:

                    is_loadable = not funcutils.ignoreFunction(el, limit_pointerness=True, print_warning=True)

                    if not is_loadable:

                        cfg.loaded_functions.remove(func_name_long_templ_args)



    #
    # Main loop over all xml files
    #

    # Check that we have something to do...
    if (len(cfg.loaded_classes) == 0) and (len(cfg.loaded_functions) == 0):
        print
        print
        print '  - No classes or functions to load!'
        print
        print 'Done!'
        print '-----' 

        sys.exit()



    print
    print
    print 'Generating code:'
    print '----------------'

    for xml_file in xml_files:

        # Output xml file name
        print 
        print
        print '  Current XML file: %s' % xml_file
        print '  ------------------' + '-'*len(xml_file)
        print 

        #
        # Initialise global dicts with the current XML file
        #

        gb.xml_file_name = xml_file
        utils.initGlobalXMLdicts(xml_file)
        

        #
        # Parse classes
        #

        classparse.run()


        #
        # Parse functions
        #

        funcparse.run()


        #
        # Create header with forward declarations of all abstract classes
        #

        abs_frwd_decls_header_path = os.path.join(cfg.extra_output_dir, gb.frwd_decls_abs_fname + cfg.header_extension)
        utils.constrAbsForwardDeclHeader(abs_frwd_decls_header_path)


        #
        # Create header with forward declarations of all wrapper classes
        #

        wrp_frwd_decls_header_path = os.path.join(cfg.extra_output_dir, gb.frwd_decls_wrp_fname + cfg.header_extension)
        utils.constrWrpForwardDeclHeader(wrp_frwd_decls_header_path)
        

        # #
        # # Create header with declarations of all enum types
        # #

        # enum_decls_header_path = os.path.join(cfg.extra_output_dir, gb.enum_decls_wrp_fname + cfg.header_extension)
        # utils.constrEnumDeclHeader(root.findall('Enumeration'), enum_decls_header_path)


    #
    # END: loop over xml files
    #

    # Check that we have done something...
    if (len(gb.classes_done) == 0) and (len(gb.functions_done) == 0):
        print
        print
        print '  - No classes or functions loaded!'
        print
        print 'Done!'
        print '-----' 
        print 

        sys.exit()

    #
    # Clear global dicts
    #

    utils.clearGlobalXMLdicts()



    #
    # Write new files
    #

    # Create all output directories that do not exist.
    filehandling.createOutputDirectories()

    # File writing loop
    for src_file_name, code_dict in gb.new_code.iteritems():

        add_include_guard = code_dict['add_include_guard']
        code_tuples = code_dict['code_tuples']

        code_tuples.sort( key=lambda x : x[0], reverse=True )

        new_src_file_name  = os.path.join(cfg.extra_output_dir, os.path.basename(src_file_name))

        if code_tuples == []:
            continue

        boss_backup_exists = False
        if os.path.isfile(src_file_name):
            try:
                f = open(src_file_name + '.backup.boss', 'r')
                boss_backup_exists = True
            except IOError, e:
                if e.errno != 2:
                    raise e
                f = open(src_file_name, 'r')

            f.seek(0)
            file_content = f.read()
            f.close()
            new_file_content = file_content
        else:
            new_file_content = ''

        if not boss_backup_exists and new_file_content:
            f = open(src_file_name + '.backup.boss', 'w')
            f.write(new_file_content)
            f.close()

        for pos,code in code_tuples:

            if pos == -1:
                new_file_content = new_file_content + code    
            else:
                new_file_content = new_file_content[:pos] + code + new_file_content[pos:]

        # Add include guard where requested
        if add_include_guard:

            short_new_src_file_name = os.path.basename(new_src_file_name)

            if 'include_guard_prefix' in code_dict.keys():
                prefix = code_dict['include_guard_prefix']
            else:
                prefix = ''

            new_file_content = utils.addIncludeGuard(new_file_content, short_new_src_file_name, prefix=prefix ,suffix=gb.gambit_backend_name_full)

        # Do the writing!
        f = open(new_src_file_name, 'w')
        f.write(new_file_content)
        f.close()



    # 
    # Copy files from common_headers/ and common_source_files/ and replace any code template tags
    # 

    filehandling.createCommonHeaders()
    filehandling.createCommonSourceFiles()


    #
    # Move files to correct directories
    #

    filehandling.moveFilesAround()


    #
    # Run through all the generated files and use the code tags __START_GAMBIT_NAMESPACE__ and __END_GAMBIT_NAMESPACE__ to construct
    # the correct namespace.
    #

    construct_namespace_in_files = glob.glob( os.path.join(gb.gambit_backend_dir_complete, '*') )

    filehandling.replaceNamespaceTags(construct_namespace_in_files, gb.gambit_backend_namespace, '__START_GAMBIT_NAMESPACE__', '__END_GAMBIT_NAMESPACE__')


    #
    # Run through all the generated files and remove tags that are no longer needed
    #

    all_generated_files = glob.glob( os.path.join(cfg.extra_output_dir, '*') ) + glob.glob( os.path.join(gb.gambit_backend_dir_complete, '*') )
    remove_tags_list = [ '__START_GAMBIT_NAMESPACE__', 
                         '__END_GAMBIT_NAMESPACE__', 
                         '__INSERT_CODE_HERE__' ]

    filehandling.removeCodeTagsFromFiles(all_generated_files, remove_tags_list)


    #
    # Copy files to the correct locations within the source tree of the original code
    #

    print
    print
    print 'Copying generated files to original source tree:'
    print '------------------------------------------------'
    print 

    manipulated_files, new_files, new_dirs = filehandling.copyFilesToSourceTree(verbose=True)

    # Save source_target_tuples to be able to undo the changes at a later time
    reset_info_file_name = 'reset_info.' + gb.gambit_backend_name_full + '.boss'
    with open(reset_info_file_name, 'w') as f:
        pickle.dump([manipulated_files, new_files, new_dirs], f)


    #
    # If generate_only_flag is True, save state and quit
    #

    if options.generate_only_flag:
        with open('savefile.boss', 'w') as f:
            pickle.dump([gb.classes_done, gb.factory_info], f)
        print 
        print
        print 'Done with code generation. Program state saved.' 
        print 'To generate loaded_types.hpp, run: boss.py --types-header'
        print
        sys.exit()



    #
    # Parse all factory function source files using gccxml
    #

    print 
    print 
    print 'Parsing the generated factory function source files:'
    print '----------------------------------------------------'
    print 

    factory_xml_files = filehandling.parseFactoryFunctionFiles()



    #
    # Generate header file 'loaded_types.hpp'
    #

    print
    print
    print 'Generating file loaded_types.hpp:'
    print '---------------------------------'
    print 

    filehandling.createLoadedTypesHeader(factory_xml_files)


    #
    # Parse any source files for global functions using gccxml
    #

    print 
    print 
    print 'Parsing the generated function source files:'
    print '--------------------------------------------'
    print 

    function_xml_files = filehandling.parseFunctionSourceFiles()


    #
    # Generate GAMBIT frontend header file ''
    #

    print
    print
    print 'Generating GAMBIT frontend header file:'
    print '---------------------------------------'
    print 

    filehandling.createFrontendHeader(function_xml_files)


    #
    # Done!
    #

    print
    print 'Done!'
    print '-----' 
    print
    print "  To prepare this backend for use with GAMBIT, do the following:"
    print 
    print "    1. BOSS has generated new source files in '%s'. Make sure that these are included when building '%s'." % (cfg.source_path, cfg.gambit_backend_name)
    print "    2. Build a shared library (.so) from the '%s' source code that BOSS has edited." % (cfg.gambit_backend_name)
    print "    3. Set the correct path to this library in the 'backends_locations.yaml' file in GAMBIT."
    print "    4. Copy the '%s' directory from '%s' to the 'backend_types' directory within GAMBIT." % (gb.gambit_backend_name_full, gb.gambit_backend_dir_complete)
    print "    5. Copy the file '%s' from '%s' to the GAMBIT 'frontends' directory." % (gb.frontend_fname, gb.frontend_path)
    print 
    print 

# ====== END: main ========

if  __name__ =='__main__':main()

