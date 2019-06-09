#!/bin/sh

if [ -z $ROYALE_VERSION_BUILD ]; then
  echo "ERROR Build version not defined - set it with \"export ROYALE_VERSION_BUILD=number\""
  exit
fi

if [ "$#" -gt 0 ] && ([ $1 = "clean" ] || [ $1 = "-clean" ]); then
  if [ -d build ]; then
	rm -rf build
  fi
  if [ -d bin ]; then
    rm -rf bin
  fi
  if [ -d release ]; then
    rm -rf release
  fi
  if [ -d ../behavior/build ]; then
    rm -rf ../behavior/build
  fi
  echo "cleaned, now exiting"
  exit 1
else
  if [ ! -d "build" ]; then
   mkdir build
  fi

  if [ ! -d "bin" ]; then
   mkdir bin
  fi

  if [ -d release ]; then
    rm -rf release/*/
  fi

  rm -rf bin/ReleaseAnalyzer_LOG.txt

  if [ "$#" -gt 0 ] && ([ $1 = "compileonly" ] || [ $1 = "-compileonly" ]); then
    cd build
    cmake -DROYALE_ENABLE_PANDOC=OFF -DROYALE_DO_VERIFICATION=ON -DROYALE_ENABLE_RELEASEANALYZER=ON $2 ../../../
    cmake --build . --config Release
	cd ..
  else
    if [ "$#" -gt 0 ] && ([ $1 = "buildExpectationFiles" ] || [ $1 = "-buildExpectationFiles" ]); then
      cd build
      cmake -DROYALE_ENABLE_PANDOC=OFF -DROYALE_DO_VERIFICATION=ON -DROYALE_ENABLE_RELEASEANALYZER=ON $2 ../../../ | tee -a output ../bin/ReleaseAnalyzer_LOG.txt
      cmake --build . --config Release | tee -a output ../bin/ReleaseAnalyzer_LOG.txt
      cd ../bin
      ./releaseanalyzer -settingsFile settings.xml -usecreateconfig "WINDOWS-32Bit"
	  ./releaseanalyzer -settingsFile settings.xml -usecreateconfig "WINDOWS-64Bit"
	  ./releaseanalyzer -settingsFile settings.xml -usecreateconfig "APPLE-64Bit"
	  ./releaseanalyzer -settingsFile settings.xml -usecreateconfig "ANDROID-32Bit"
	  ./releaseanalyzer -settingsFile settings.xml -usecreateconfig "LINUX-64Bit"
	  ./releaseanalyzer -settingsFile settings.xml -usecreateconfig "LINUX-32Bit"
      cd ..
	else
      cd build
      cmake -DROYALE_ENABLE_PANDOC=OFF -DROYALE_DO_VERIFICATION=ON -DROYALE_ENABLE_RELEASEANALYZER=ON $1 ../../../ | tee -a output ../bin/ReleaseAnalyzer_LOG.txt
      cmake --build . --config Release | tee -a output ../bin/ReleaseAnalyzer_LOG.txt
      cd ../bin
      ./releaseanalyzer -settingsFile settings.xml
      cd ..
	fi
  fi
fi
