#include <gtest/gtest.h>
#include <chrono>

#include "tests/generated/cmake_config.h"

#include "awesome_utils/model_interface.hpp"
#include "awesome_utils/contact_est_utils.hpp"
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace ModelInterface;
using namespace ContactEstUtils;

std::string urdf_path;
IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");

namespace
{
    template <typename Func>
    double measure_sec(Func f)
    {
        auto tic = std::chrono::high_resolution_clock::now();

        f();

        auto toc = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::nanoseconds>(toc-tic).count()*1e-9;
    }
}

class TestContactEst: public ::testing::Test {


    protected:

         TestContactEst(){

         }

         virtual ~TestContactEst() {
         }

         virtual void SetUp() {

         }

         virtual void TearDown() {
         }


};

TEST_F(TestContactEst, compute_quantities)
{
    Eigen::VectorXd q, v, a, tau, p,
                    tau_c, f_c, t_c;

    Eigen::MatrixXd B, C, J;

    Model::Ptr model_ptr(new Model(urdf_path));

    model_ptr->get_state(q, v, a, tau);

    model_ptr->update(q, v, tau, a); // computes all terms

    double dt = 0.005;
    double BW = 10.0;
    double lambda = 1.0;

    std::vector<int> selector{0, 1, 2}; // only force

    MomentumBasedFObs::Ptr f_obs_ptr(new MomentumBasedFObs(model_ptr, dt, BW, lambda, true,
                                                           selector));

    std::string contact_linkname = "tip1";
    f_obs_ptr->update(contact_linkname); // compute estimates using the current state in model_ptr

    f_obs_ptr->get_tau_obs(tau_c);
    f_obs_ptr->get_f_est(f_c);
    f_obs_ptr->get_t_est(t_c);

    std::cout << "\nURDF loaded at: "<< model_ptr->get_urdf_path() << "\n " << std::endl;
    std::cout << "** tau_c: \n" << tau_c << "\n " << std::endl;
    std::cout << "** f_c: \n" << f_c << "\n " << std::endl;
    std::cout << "** w_c: \n" << t_c << "\n " << std::endl;

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::string urdf_name = "awesome_leg";
    // You should change here to set up your own URDF file or just pass it as an argument of this example.
    const std::string urdf_fullpath = (argc<=1) ? URDF_PATH + urdf_name + std::string(".urdf"): URDF_PATH + std::string(argv[1]) + std::string(".urdf");
    urdf_path = urdf_fullpath;

    return RUN_ALL_TESTS();
}
