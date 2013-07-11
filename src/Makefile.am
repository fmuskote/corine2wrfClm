TESTS = fractions_test

bin_PROGRAMS = corine2wrfClm
check_PROGRAMS = fractions_test

corine2wrfClm_SOURCES = coordinate.cc coordinate.h \
		      geoRaster.cc  geoRaster.h  \
		      usgs.cc       usgs.h       \
		      corine.cc     corine.h     \
		      wrfFile.cc    wrfFile.h    \
		      modis.cc      modis.h      \
		      corine2wrfClm.cc           \
		      clm.cc        clm.h        \
		      fractions.cc  fractions.h  \
		      notClmFractions.cc notClmFractions.h

fractions_test_SOURCES = fractions_test.cc fractions.h fractions.cc
fractions_test_LDADD = -lboost_test_exec_monitor
