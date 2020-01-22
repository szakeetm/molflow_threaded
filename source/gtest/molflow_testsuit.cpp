//
// Created by pascal on 8/8/19.
//

#include "gtest/gtest.h"
//#define MOLFLOW_PATH ""

namespace {

// The fixture for testing class Foo.
    class MolflowTestFixture : public ::testing::TestWithParam<std::string> {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        MolflowTestFixture() {
            // You can do set-up work for each test here.
        }

        ~MolflowTestFixture() override {
            // You can do clean-up work that doesn't throw exceptions here.
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:

        void SetUp() override {
            // Code here will be called immediately after the constructor (right
            // before each test).
            std::string str = "../../../bin/"+(std::string)MOLFLOW_PATH+"/molflow &";
            //std::string str = "../../../bin/linux/debug/molflow &";
            const char *command = str.c_str();
            system(command);
        }

        void TearDown() override {
            // Code here will be called immediately after each test (right
            // before the destructor).
            std::string str = "killall molflow";
            const char *command = str.c_str();
            system(command);
        }

        // Objects declared here can be used by all tests in the test suite for Foo.
    };

// Tests that the Foo::Bar() method does Abc.
    /*TEST_F(MolflowTest, MethodBarDoesAbc) {
        const int f = 0;
        const double d = 0.0;
        EXPECT_EQ(f, d);
    }*/

// Tests that Foo does Xyz.
    TEST_P(MolflowTestFixture, SimulationResultsAreEqual) {

        std::string testFile = GetParam();

        sleep(2);
        std::string pathToScript = "../../../molflow_tests/molflow_test.py";
        pathToScript = "python3 "+pathToScript+" -i "+testFile;
        const char *command = pathToScript.c_str();
        int status = system(command);
        int returnVal = WEXITSTATUS(status);
        std::cout << "Return val: "<< returnVal << std::endl;
        EXPECT_EQ(returnVal,0);

    }

    INSTANTIATE_TEST_CASE_P(
            MolflowTests,
            MolflowTestFixture,
            ::testing::Values(
                    "results10.100_tex.xml", "pumpmodel.geo"
            ));

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);



    return RUN_ALL_TESTS();
}