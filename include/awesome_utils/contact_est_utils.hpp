#include <Eigen/Core>
#include <Eigen/Geometry>

#include <string>

#include "awesome_utils/sign_proc_utils.hpp"
#include "awesome_utils/model_interface.hpp"

using namespace SignProcUtils;
using namespace ModelInterface;

namespace Eigen
{
    typedef Eigen::Matrix<double, 6, 1> Vector6d;
    typedef Eigen::Matrix<double, 3, 1> Vector3d;
}

namespace ContactEstUtils
{
    typedef Eigen::MatrixXd MatrixXd;
    typedef Eigen::VectorXd VectorXd;

    /// \brief Class to perform a momentum-based force estimation.
    ///
    /// Consider the following expression of the rigid-body dynamics:
    ///
    /// (1) B(q)*q_ddot + C(q, q_dot)*q_dot + g = tau + tau_c
    ///
    /// where
    /// - B is the joint-space inertia matrix
    /// - C is the matrix of Coriolis and centrifugal terms (recall that (2) B_dot = C + C^T)
    /// - g is the joint-space gravitational vector
    /// - tau is a vector of joint-space effort measurements
    /// - tau_c is a "disturbance" vector, which can be used
    ///   to estimate the contact forces, reflected to the joints.
    ///
    /// The dynamics of the observer is given by
    ///
    /// (3) y_dot = K * (p_dot - tau + g - C^T * q_dot - y)
    ///
    /// which is an asymptotically stable dynamics when is
    /// K positive definite square matrix (usually is chosen as a diagonal matrix).
    ///
    /// - p is the joint-space momentum, given by B(q) * q_dot
    ///
    /// Equation (3) can be obtained easily by considering that
    ///
    /// (4) p_dot = B_dot * q_dot + B * q_ddot
    ///
    /// substituting (1) and (2) into (3) and extracting tau_c as the desired variable
    /// to be observed, one obtains the observer dynamics (3).
    ///
    /// How to use (3)? It can be easily integrated (numerically) to obtain an estimate of tau_c
    /// with some noise rejection properties: the higher the gains, the lower the rejection
    /// of noise, the faster the convergence.
    /// In particular, supposing a matrix of diagonals values, the bandwidth of this observer
    /// can be approximated as (5) BW = - k/log(1 - 0.707)
    ///
    /// Clearly, the quality of the estimate depends on the quality of the measurements (accuracy, noise, etc..)
    /// but also on the accuracy up to which the inertial properties of the system are known
    ///
    /// a numerical implementation to obtain y at the current time (y_k) can be implemented integrating both sides of (3)
    /// over a sample interval:
    ///
    /// int_0^{h} [ y_dot * d_t] = K * int_0^{h} [ (p_dot - tau + g - C^T * q_dot - y) * dt] --->
    ///
    /// y_k - y_km1 = K * (p_k - p_km1 + int_0^{h} [ g - tau - C^T * q_dot - y ] * dt)
    ///
    /// let us approximate  int_0^{h} [ y * dt] as (y_k + y_km1)/2.0 * h (trapezoidal integration).
    ///
    /// Rearranging,  we obtain
    ///
    /// (6) (I + h/2.0 * K) * y_k = (I - h/2.0 * K) * y_km1 + K * (p_k - p_km1 + int_0^{h} [ g - tau - C^T * q_dot - y ] * dt)
    ///
    /// The term " int_0^{h} [ g - tau - C^T * q_dot - y ] * dt " can be simply approximated using again trapezoidal integration.
    ///
    /// Inverting (6) w.r.t. y_k gives the update equation for y, i.e. the observer of the residual joint torques
    ///
    /// Note that (6) does not require the differentiation of q_dot and is hence less prone to noise than other possible model-based
    /// observer implementations.

    class MomentumBasedFObs
    {
    public:

        typedef std::weak_ptr<MomentumBasedFObs> WeakPtr;
        typedef std::shared_ptr<MomentumBasedFObs> Ptr;
        typedef std::unique_ptr<MomentumBasedFObs> UniquePtr;

        MomentumBasedFObs();

        MomentumBasedFObs(Model::Ptr model_ptr, double data_dt);
        MomentumBasedFObs(Model::Ptr model_ptr, double data_dt, double bandwidth = 10.0,
                          double lambda = 1.0, bool regularize_f = false,
                          std::vector<int> selector = std::vector<int>{0, 1, 2, 3, 4, 5});

        void update(std::string contact_framename);

        void get_tau_obs(VectorXd& tau_c); // get contact joint efforts estimate
        void get_w_est(Model::Wrench& w_c); // get the wrench estimate
        void get_f_est(Model::Force3D& f_c); // get force estimate
        void get_t_est(Model::Torque3D& t_c); // get wrench estimate

    private:

        Model::Ptr _model_ptr;

        int _nv = -1.0;

        bool _regularize_f = false;// will use previous solution of f_c to regularize the new solution
        // instead of using always a constant value

        double _dt = -1.0;

        double _lambda = 1.0; // regularization term
        // for the solution of the QP problem to retrieve
        // the contact forces and efforts

        double _BW_red_factor = 0.707; // attenuation of the signal
        // of 3dB (definition of bandwidth)

        double _bandwidth = 10.0; // [Hz]
        double _k = 12.3; // [1/s] (or [Hz])

        std::string _current_cont_frame;

        std::vector<int> _selector{0, 1, 2, 3, 4, 5};
        std::vector<int> _Jt_selector{0, 1, 2, 3, 4, 5};

        MatrixXd _K;
        MatrixXd _Skp1, _Skp1_inv, _Sk; // state transition matrices for the discrete integration
        // of the observer dynamics

        NumInt _integrator;

        VectorXd _tau_c_k; // observed joint contact efforts

        VectorXd _p_km1; // last joint-space momentum at k - 1 instant (i.e. previous)

        MatrixXd _A,
                 _I_lambda;
        VectorXd _b, _b_lambda;

        Model::Wrench _w_c; // estimated contact wrenches (6 x 1 -> linear + angular)
        Model::Wrench _w_c_reg; // regularization vector for the contact f_c estimation

        void compute_tau_c(); // computes the observed value of tau_c, i.e. the residual joint efforts

        void apply_selector(Model::Wrench& vector);
        void apply_selector(Model::SpatialJacT& matrix);

    };

}
