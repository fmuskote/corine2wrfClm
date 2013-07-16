#define BOOST_TEST_MODULE Fractions
#include <boost/test/unit_test.hpp>
#include "fractions.h"

BOOST_AUTO_TEST_CASE( fractions_test )
{
    double tolerance = 1.0e-10;
    size_t size = 15;
    Fractions f0 (size);

    // check for initialization with 0.0
    for (size_t i = 0; i < size; ++i)
        BOOST_CHECK_EQUAL (f0[i], 0.0);

    // check for range check
    BOOST_CHECK_THROW (f0[size], FractionOutOfRange);
    BOOST_CHECK_THROW (f0[-1],   FractionOutOfRange);
    BOOST_CHECK_THROW (f0.set(size, 0.0), FractionOutOfRange);
    BOOST_CHECK_THROW (f0.set(-1,   0.0), FractionOutOfRange);
    BOOST_CHECK_THROW (f0.add(size, 0.0), FractionOutOfRange);
    BOOST_CHECK_THROW (f0.add(-1,   0.0), FractionOutOfRange);

    // check for inconsistency exceptions
    BOOST_CHECK_THROW (f0.set (0, 2.0), FractionInconsistent);
    Fractions f1 (size);
    BOOST_CHECK_THROW (f1.add (0, 2.0), FractionInconsistent);
    
    // check setter
    Fractions f2 (size);
    double value = 0.5;
    f2.set (0, value);
    BOOST_CHECK_CLOSE (f2[0],         value,       tolerance);
    BOOST_CHECK_CLOSE (f2.missing (), 1.0 - value, tolerance);

    // check cumulative setter
    Fractions f3 (size);
    double value1 = 0.5;
    double value2 = 0.3;
    f3.add (0, value1);
    BOOST_CHECK_CLOSE (f3[0],         value1,       tolerance);
    BOOST_CHECK_CLOSE (f3.missing (), 1.0 - value1, tolerance);
    f3.add (0, value2);
    BOOST_CHECK_CLOSE (f3[0],         value1 + value2,       tolerance);
    BOOST_CHECK_CLOSE (f3.missing (), 1.0 - value1 - value2, tolerance);
}
