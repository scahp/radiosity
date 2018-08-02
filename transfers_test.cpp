////////////////////////////////////////////////////////////////////////
//
// transfers_test.cpp: Tests for transfers.cpp.
//
// (c) Copyright Simon Frankau 2018
//

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "geom.h"
#include "glut_wrap.h"
#include "transfers.h"
#include "weighting.h"

class TransfersTestCase : public CppUnit::TestCase
{
private:
    const int RESOLUTION = 512;

    CPPUNIT_TEST_SUITE(TransfersTestCase);
    CPPUNIT_TEST(eachFaceIsAreaOne);
    CPPUNIT_TEST_SUITE_END();

    void eachFaceIsAreaOne();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TransfersTestCase, "TransfersTestCase");

extern void calcCube(std::vector<Vertex> const &vertices,
                     std::vector<Quad> const &faces,
                     std::vector<double> const &weights);

void TransfersTestCase::eachFaceIsAreaOne()
{
    std::vector<double> weights;
    calcWeights(RESOLUTION, weights);
    TransferCalculator tc(cubeVertices, cubeFaces, RESOLUTION, weights);
    std::vector<double> sums = tc.calcCube();
    for (int i = 0; i < sums.size(); ++i) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, sums[i], 1.0e-6);
    }
}
