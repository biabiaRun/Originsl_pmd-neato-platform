#!/usr/bin/python3

import sys
import os
import importlib
import importlib.util

def importFile (sourceFile):
    spec = importlib.util.spec_from_file_location ("register_file", sourceFile)
    if not spec:
        raise Exception ("Either couldn't find the directory containing the --device, or there is no __init__.py in that directory")
    module = importlib.util.module_from_spec (spec)
    try:
        spec.loader.exec_module (module)
    except:
        return []
    if hasattr(module, 'full_cfg'):
        return module.full_cfg
    else:
        return []
    
def main ():

    startaddress = 0x9000
    stopaddress = 0x93FF

    initmap = set()
    usedfiles = []
    
    for file in os.listdir("."):
        if file.endswith(".py"):
            regs = importFile(file)
            if len(regs) > 0:
                usedfiles.append(file)
                for reg in regs:
                    if reg[0] < startaddress or reg[0] > stopaddress:
                        initmap.add(reg)

    print("Used files :")
    for file in usedfiles:
        print("   " + file)

    print("Init registers :")
    for reg in initmap:
        print ("   (0x%0.4X, 0x%0.4X, 0)" % (reg[0], reg[1]))
    
if (__name__ == "__main__"):
    main()
