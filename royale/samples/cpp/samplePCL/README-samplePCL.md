samplePCL
------------

This C++ example shows how to connect Royale to the Point Cloud Library (PCL) which can be downloaded
from http://pointclouds.org.
It will capture image data, fill a PCL PointCloud and display the data with a CloudViewer. 
It was tested with PCL 1.8.0.

To compile this you need to point CMake to your PCL installation folder (with the PCL_DIR variable) and 
make sure that version of PCL has been compiled with a C++ ABI version that's compatible to Royale.

Due to a bug in the PCL installer on Windows this example might not compile in the MinSizeRel and
RelWithDebInfo configurations, because the wrong VTK libraries are linked (mismatch detected for 
'_ITERATOR_DEBUG_LEVEL').

If you're running Debian or Ubuntu, please install the libvtk6-qt-dev and libfontconfig1-dev packages,
otherwise you might get errors during compilation.