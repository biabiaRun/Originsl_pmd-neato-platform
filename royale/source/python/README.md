Royale Python 3 Wrapper
=======================

To build the Python 3 wrapper for Royale you have to install Python 3 and SWIG
(also the Python 3 development packages if you're under a Unix system) and 
make sure that they are found in CMake (PYTHON\_LIBRARY has to be found, but if 
you want to be able to debug PYTHON\_LIBRARY\_DEBUG has to be found too).

If no Python 3 debug library is found the debug wrapper will link against the
release library of Python 3.

All C++ functions that return CameraStatus are wrapped with code that throws an
exception if they return non-SUCCESS.

In the samples/python folder you will also find several examples using the
wrapper, these files will also be copied to the binary output folder (together
with roypy.py and the \_roypy shared library).

Please keep in mind that the API is not finalized yet and might change
in the next releases!
