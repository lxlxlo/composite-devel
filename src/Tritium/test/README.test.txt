TRITIUM UNIT TESTS
==================

These tests are build on the Boost Unit Test Framework... which is
pretty sharp, but lacks some user-friendly features.  Here's a couple
tips for the tests.

For each test, these parameters affect how data is output.  By
default, the output of the tests are sparse.  They are documented in
Boost 1.41.0, however they appear to also apply to 1.34 (which is not
so well documented).

--report_level=detailed

    Gives an itemized summary of the events of the test.

--log_level=all

    Allows all output, e.g. BOOST_MESSAGE()'s.  By default a
    BOOST_MESSAGE() will be suppressed.
