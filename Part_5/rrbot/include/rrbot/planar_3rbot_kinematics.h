/*
 * File:   planar_3rbot_kinematics.h
 * Author: wsn
 *
 * Created June, 2016
 */

#ifndef PLANAR_3RBOT_KIN_H
#define PLANAR_3RBOT_KIN_H
#include <eigen3/Eigen/src/Geometry/Transform.h>
#include <math.h>
#include <ros/ros.h>
#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <iostream>
#include <string>
#include <vector>

// typedef Eigen::Matrix<double, 6, 1> Vectorq6x1; //can define special cases of VectorXd
// typedef Eigen::Matrix<double, 7, 1> Vectorq7x1;
// a fwd kin solver...
// list DH params here for planar_3rbot:
//
// world frame has origin on ground plane and z-axis pointing up
// DH frame0 has z-axis through joint1, antiparallel to world-y axis
// subject to this constraint, get choices for orientation and origin of frame0;
// choose frame0 x-axis parallel to world-frame x-axis,
// and thus frame0 y-axis is parallel to world-frame z-axis;
// choose frame0 origin offset by 1.95 along world z and -0.1 along world y

const int NJNTS = 3;  // number of degrees of freedom of this robot

const double base_to_frame0_dz = 1.95;
const double base_to_frame0_dy = -0.1;
const double base_to_frame0_dx = 0.0;
const double base_to_frame0_rotx = M_PI / 2.0;

const double DH_a1 = 0.9;   // link length: distance from joint1 to joint2 axes
const double DH_a2 = 0.9;   // dist from j2 to j3 axes
const double DH_a3 = 0.95;  // link length: distance from joint3 axis to flange-z axis

const double DH_d1 = 0.1;  // offset along parent z axis from frame0 to frame1
const double DH_d2 = 0.1;  // offset along parent z axis from frame1 to frame2
const double DH_d3 = 0.0;  // zero offset along parent z axis from frame2 to flange

const double DH_alpha1 = 0;  // joint1 axis is parallel to joint2 axis
const double DH_alpha2 = 0;  // joint2 axis is parallel to joint3 axis
const double DH_alpha3 = 0;  // joint3 axis is parallel to flange z-axis

// could define robot "home" angles different than DH home pose; reconcile with these offsets
const double DH_q_offset1 = 0.0;  // M_PI/2.0;
const double DH_q_offset2 = 0.0;  //-M_PI/2.0;
const double DH_q_offset3 = 0.0;  //-M_PI/2.0;

const double deg2rad = M_PI / 180.0;

// can choose to restrict joint ranges:
const double DH_q_max1 = deg2rad * 160;
const double DH_q_max2 = deg2rad * 160;
const double DH_q_max3 = deg2rad * 160;

const double DH_q_min1 = -deg2rad * 160;
const double DH_q_min2 = -deg2rad * 160;
const double DH_q_min3 = -deg2rad * 160;

const double DH_a_params[NJNTS] = { DH_a1, DH_a2, DH_a3 };
const double DH_d_params[NJNTS] = { DH_d1, DH_d2, DH_d3 };
const double DH_alpha_params[NJNTS] = { DH_alpha1, DH_alpha2, DH_alpha3 };
const double DH_q_offsets[NJNTS] = { DH_q_offset1, DH_q_offset2, DH_q_offset3 };
const double q_lower_limits[NJNTS] = { DH_q_min1, DH_q_min2, DH_q_min2 };
const double q_upper_limits[NJNTS] = { DH_q_max1, DH_q_max2, DH_q_max2 };

class Planar_3rbot_fwd_solver
{
public:
  Planar_3rbot_fwd_solver();  // constructor

  // fwd kin fncs: tool flange w/rt base (link1)
  Eigen::Affine3d fwd_kin_flange_wrt_world_solve(Eigen::VectorXd q_vec);

  Eigen::MatrixXd Jacobian(Eigen::VectorXd q_vec);

  // these are all w/rt base link
  Eigen::Matrix4d get_frame(int i)
  {
    return A_mat_products_[i];
  };
  Eigen::Matrix4d get_frame0()
  {
    return A_mat_products_[0];
  };
  Eigen::Matrix4d get_frame1()
  {
    return A_mat_products_[1];
  };
  Eigen::Matrix4d get_flange_frame()
  {
    return A_mat_products_[NJNTS - 1];
  };  // flange frame

  // inner fwd-kin fnc: computes tool-flange frame w/rt base frame
  Eigen::Matrix4d fwd_kin_solve_(Eigen::VectorXd q_vec);

  Eigen::Matrix4d A_mats_[NJNTS], A_mat_products_[NJNTS], A_base_link_wrt_world_, A_base_link_wrt_world_inv_;
};

class Planar_3rbot_IK_solver : Planar_3rbot_fwd_solver
{
public:
  Planar_3rbot_IK_solver();                                    // constructor;
  bool fit_q_to_range(double q_min, double q_max, double &q);  // applies periodicity and tests if in jnt range
  // given angle of jnt1, reach dist to goal implies angle of q3; expect 2 solns
  bool solve_for_j3_ang(Eigen::Vector3d O_flange_wrt_world, double q1, std::vector<double> &q3_solns);
  // generic formula to find solns q that satisfy K = A*cos(q) + B*sin(q); expect 2 solns
  bool solve_K_eq_Acos_plus_Bsin(double K, double A, double B, std::vector<double> &q_solns);
  // given q1 and q3, solve for q2
  bool solve_for_j2_ang(Eigen::Vector3d O_flange_wrt_world, double q1, double q3, double &q2_soln);
  // given q1, solve for combinations (q1,q2,q3) to achieve O_flange_wrt_world; rtn number of solns
  int solve_for_qsolns_given_q1(Eigen::Vector3d O_flange_wrt_world, double q1, std::vector<Eigen::Vector3d> &q_solns);
  // ik_solve generates a vector of 3D solns by sampling candidate values of q1, then solving for corresponding q2,q3
  // choose dq1_sample_res for resolution of samples of q1
  int ik_solve(double dq1_sample_res, Eigen::Affine3d desired_flange_pose_wrt_base,
               std::vector<Eigen::Vector3d> &q_solns);
};

#endif /* PLANAR_3RBOT_KIN_H */
