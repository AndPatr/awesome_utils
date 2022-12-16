# Various robotics-tailored cpp utilities to perform signal processing, parameter identification and more

These include

- a cpp wrapper ("model interface") around Pinocchio's model class

- signal processing utilities: a numerical differentiator and an integrator, a moving average filter, a couple of sign function implementations

- trajectory utilities: class for computing an 9th-order super smooth polynomial transition between values, a linear interpolator, a class for loading standardized trajectories using MatLogger2

- xbot2-specific utilities (see xbot2): class for getting quadrature current measurements from XBot2 aux ROS topics

- calibration utilities: quadrature current estimator for three-phase BLDC-based actuators, actuator friction model runtime calibrator, friction observer

- contact estimation utilities: implementation of a first order momentum-based force estimator
