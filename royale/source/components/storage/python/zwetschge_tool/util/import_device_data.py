import importlib
import importlib.util
import sys
import os


def importDeviceData (sourceFile):
    """This loads the data that is to be converted to a Zwetschge file.

    Currently the input format is itself an executable Python package directory, in which sourceFile
    points to the main module will be loaded and then have the `getZwetschgeDeviceData` method
    called.

    This expects there to be one top-level file for the device, which is given with the --device
    argument.  It also expects that file to be in an implicit package (to have a __init__.py file in
    the same directory), which makes it easier to split the device in to multiple files (the
    provided ExampleDevice.py includes documentation about this).
    """
    spec = importlib.util.spec_from_file_location ("device_package", os.path.join (os.path.dirname(sourceFile), "__init__.py"))
    if not spec:
        raise Exception ("Either couldn't find the directory containing the --device, or there is no __init__.py in that directory")
    module = importlib.util.module_from_spec (spec)
    spec.loader.exec_module (module)
    sys.modules["device_package"] = module

    # Now load the device file itself.
    spec = importlib.util.spec_from_file_location ("device_package.device", sourceFile)
    module = importlib.util.module_from_spec (spec)
    spec.loader.exec_module (module)

    return module.getZwetschgeDeviceData()