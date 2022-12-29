#ifndef CARTESIAN_IMP_UTILS_HPP
#define CARTESIAN_IMP_UTILS_HPP

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "typedefs.hpp"
#include "model_interface.hpp"

using namespace ModelInterface;

namespace CartesianImpUtils
{

    class CartesianTask
    {
        public:

            typedef std::weak_ptr<CartesianTask> WeakPtr;
            typedef std::shared_ptr<CartesianTask> Ptr;
            typedef std::unique_ptr<CartesianTask> UniquePtr;

            struct CartTask
            {
                utils_defs::RotMat3D rot; // orientation part of the cartesian task

                utils_defs::PosVec3D pos;
            };

            typedef Matrix<double, 6, 1> CartTaskDot;
            typedef Matrix<double, 6, 1> CartTaskDdot;

            typedef Matrix<double, 6, 1> CartTaskErr;
            typedef Matrix<double, 6, 1> CartTaskDotErr;
            typedef Matrix<double, 6, 1> CartTaskDdotErr;

            CartesianTask();

            void update(CartTask chi_ref,
                        CartTaskDot chi_dot_ref,
                        CartTaskDdot chi_ddot_ref);

            void update(utils_defs::PosVec3D pos_ref, utils_defs::RotMat3D rot_ref,
                        CartTaskDot chi_dot_ref,
                        CartTaskDdot chi_ddot_ref);

            // compute task error between last set reference task and the input
            CartTaskErr compute_task_err(utils_defs::PosVec3D pos, utils_defs::RotMat3D rot);
            CartTaskErr compute_task_err(CartTask cart_task);

            CartTaskDotErr compute_task_dot_err(CartTaskDot cart_task_dot);

            CartTaskDdotErr compute_task_ddot_err(CartTaskDdot cart_task_ddot);

        private:

            CartTask _chi_ref;
            CartTaskDot _chi_dot_ref;
            CartTaskDdot _chi_ddot_ref;

            void set_chi_ref(CartTask chi_ref);
            void set_chi_ref(utils_defs::PosVec3D pos_ref, utils_defs::RotMat3D rot_ref);
            void set_chi_dot_ref(CartTaskDot chi_dot_ref);
            void set_chi_ddot_ref(CartTaskDdot chi_ddot_ref);

    };

    /**
    * @brief Class to perform cartesian impedance control.
    *
    */

    class CartesianImpController
    {
        public:

            typedef std::weak_ptr<CartesianImpController> WeakPtr;
            typedef std::shared_ptr<CartesianImpController> Ptr;
            typedef std::unique_ptr<CartesianImpController> UniquePtr;

            typedef Matrix<double, 6, 6> CartInertiaMat;
            typedef Matrix<double, 6, 6> CartStiffMat;
            typedef Matrix<double, 6, 6> CartDampMat;
            typedef Matrix<double, 6, 1> CartStiffVect; // we assume diagonal impedance
            // matrices for simplicity
            typedef Matrix<double, 6, 1> CartDampVect;

            CartesianImpController();

            CartesianImpController(Model::Ptr model_ptr,
                                   CartesianTask::Ptr cart_task);
            CartesianImpController(Model::Ptr model_ptr,
                                   CartesianTask::Ptr cart_task,
                                   std::string cart_cntrl_framename);

            void update();

            void set_cart_impedance(CartStiffMat stifness_mat,
                                    CartDampMat damping_mat);
            void set_cart_impedance(CartStiffVect stifness_vect,
                                    CartDampVect damping_vect);

        private:

            Model::Ptr _model_ptr;

            int _nq = 0, _nv = 0;

            std::string _cart_cntrl_framename;

            CartesianTask::Ptr _cart_task;

            CartStiffMat _cart_stiff;
            CartDampMat _cart_damp;

            CartStiffVect _cart_stiff_vect;
            CartDampVect _cart_damp_vect;

            CartInertiaMat _lambda;

            void map_impedance_vect2mat();

            void compute_cart_inertia_mat();


    };
}

#endif // CARTESIAN_IMP_UTILS_HPP

