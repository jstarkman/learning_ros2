

#include <cartesian_planner/cart_motion_commander.h>

ArmMotionCommander::ArmMotionCommander() : cart_move_action_client_("cartMoveActionServer", true)
{
  ROS_INFO("in constructor of ArmMotionInterface");
  ROS_WARN("test warning...");

  ROS_INFO("waiting for cartMoveActionServer: ");
  bool server_exists = false;
  while ((!server_exists) && (ros::ok()))
  {
    server_exists = cart_move_action_client_.waitForServer(ros::Duration(0.5));
    ros::spinOnce();
    ros::Duration(0.5).sleep();
    ROS_INFO("retrying...");
  }
  ROS_INFO("connected to action server");
  got_done_callback_ = false;
}

void ArmMotionCommander::doneCb_(const actionlib::SimpleClientGoalState& state,
                                 const cartesian_planner::cart_moveResultConstPtr& result)
{
  ROS_INFO(" doneCb: server responded with state [%s]", state.toString().c_str());
  ROS_INFO("got return value= %d", result->return_code);
  cart_result_ = *result;
  got_done_callback_ = true;
}

bool ArmMotionCommander::cb_received_in_time(double max_wait_time)
{
  double wait_time = 0.0;
  double dt = 0.1;
  double print_time = 0.0;
  got_done_callback_ = false;
  finished_before_timeout_ = false;
  while ((!got_done_callback_) & (wait_time < max_wait_time))
  {
    wait_time += dt;
    ros::Duration(dt).sleep();
    print_time += dt;
    if (print_time > 1.0)
    {
      print_time -= 1.0;
      ROS_WARN("ArmMotionCommander still waiting on callback");
    }
  }
  if (wait_time < max_wait_time)
  {
    ROS_INFO("got response in time");
    finished_before_timeout_ = true;
    return true;
  }
  else
  {
    ROS_WARN("did not get callback in time");
    finished_before_timeout_ = false;
    return false;
  }
}

void ArmMotionCommander::send_test_goal(void)
{
  ROS_INFO("sending a test goal");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::ARM_TEST_MODE;
  got_done_callback_ = false;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("giving up waiting on result");
  }
  else
  {
    ROS_INFO("finished before timeout");
    ROS_INFO("return code: %d", cart_result_.return_code);
  }
}

int ArmMotionCommander::plan_move_to_waiting_pose(void)
{
  ROS_INFO("requesting a joint-space motion plan");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::PLAN_PATH_CURRENT_TO_WAITING_POSE;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  ROS_INFO("return code: %d", cart_result_.return_code);
  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("giving up waiting on result");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }

  ROS_INFO("finished before timeout");
  if (cart_result_.return_code == cartesian_planner::cart_moveResult::PATH_NOT_VALID)
  {
    ROS_WARN(" arm plan not valid");
    return (int)cart_result_.return_code;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("unknown return code... not SUCCESS");
    return (int)cart_result_.return_code;
  }

  ROS_INFO("returned SUCCESS from planning request");
  computed_arrival_time_ = cart_result_.computed_arrival_time;
  ROS_INFO("computed move time: %f", computed_arrival_time_);
  return (int)cart_result_.return_code;
}

int ArmMotionCommander::plan_jspace_path_current_to_qgoal(Eigen::VectorXd q_des_vec)
{
  ROS_INFO("requesting a joint-space motion plan");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::PLAN_JSPACE_PATH_CURRENT_TO_QGOAL;
  int njnts = q_des_vec.size();
  cart_goal_.q_goal.resize(njnts);
  for (int i = 0; i < njnts; i++)
    cart_goal_.q_goal[i] = q_des_vec[i];
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  ROS_INFO("return code: %d", cart_result_.return_code);
  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("giving up waiting on result");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }

  ROS_INFO("finished before timeout");
  if (cart_result_.return_code == cartesian_planner::cart_moveResult::PATH_NOT_VALID)
  {
    ROS_WARN(" arm plan not valid");
    return (int)cart_result_.return_code;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("unknown return code... not SUCCESS");
    return (int)cart_result_.return_code;
  }

  ROS_INFO("returned SUCCESS from planning request");
  computed_arrival_time_ = cart_result_.computed_arrival_time;
  ROS_INFO("computed move time: %f", computed_arrival_time_);
  return (int)cart_result_.return_code;
}

int ArmMotionCommander::plan_path_current_to_goal_gripper_pose(geometry_msgs::PoseStamped des_pose)
{
  ROS_INFO("requesting a cartesian-space motion plan");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::PLAN_PATH_CURRENT_TO_GOAL_GRIPPER_POSE;

  cart_goal_.des_pose_gripper = des_pose;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  ROS_INFO("return code: %d", cart_result_.return_code);
  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("giving up waiting on result");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }

  ROS_INFO("finished before timeout");
  if (cart_result_.return_code == cartesian_planner::cart_moveResult::PATH_NOT_VALID)
  {
    ROS_WARN(" arm plan not valid");
    return (int)cart_result_.return_code;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("unknown return code... not SUCCESS");
    return (int)cart_result_.return_code;
  }

  ROS_INFO("returned SUCCESS from planning request");
  computed_arrival_time_ = cart_result_.computed_arrival_time;
  ROS_INFO("computed move time: %f", computed_arrival_time_);
  return (int)cart_result_.return_code;
}

int ArmMotionCommander::plan_jspace_path_current_to_cart_gripper_pose(geometry_msgs::PoseStamped des_pose_gripper)
{
  ROS_WARN("requesting a joint-space motion plan to cartesian gripper pose");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::PLAN_JSPACE_PATH_CURRENT_TO_CART_GRIPPER_POSE;
  cart_goal_.des_pose_gripper = des_pose_gripper;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  ROS_INFO("return code: %d", cart_result_.return_code);
  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("giving up waiting on result");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }

  ROS_INFO("finished before timeout");
  if (cart_result_.return_code == cartesian_planner::cart_moveResult::PATH_NOT_VALID)
  {
    ROS_WARN(" arm plan not valid");
    return (int)cart_result_.return_code;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("unknown return code... not SUCCESS");
    return (int)cart_result_.return_code;
  }

  ROS_INFO("returned SUCCESS from planning request");
  computed_arrival_time_ = cart_result_.computed_arrival_time;
  ROS_INFO("computed move time: %f", computed_arrival_time_);
  return (int)cart_result_.return_code;
}

int ArmMotionCommander::plan_path_current_to_goal_dp_xyz(Eigen::Vector3d dp_displacement)
{
  ROS_INFO("requesting a cartesian-space motion plan along vector");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::PLAN_PATH_CURRENT_TO_GOAL_DP_XYZ;

  cart_goal_.arm_dp.resize(3);
  for (int i = 0; i < 3; i++)
    cart_goal_.arm_dp[i] = dp_displacement[i];
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  ROS_INFO("return code: %d", cart_result_.return_code);
  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("giving up waiting on result");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }

  ROS_INFO("finished before timeout");
  if (cart_result_.return_code == cartesian_planner::cart_moveResult::PATH_NOT_VALID)
  {
    ROS_WARN(" arm plan not valid");
    return (int)cart_result_.return_code;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("unknown return code... not SUCCESS");
    return (int)cart_result_.return_code;
  }

  ROS_INFO("returned SUCCESS from planning request");
  computed_arrival_time_ = cart_result_.computed_arrival_time;
  ROS_INFO("computed move time: %f", computed_arrival_time_);
  return (int)cart_result_.return_code;
}

int ArmMotionCommander::execute_planned_path(void)
{
  ROS_INFO("requesting execution of planned path");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::EXECUTE_PLANNED_PATH;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  if (!cb_received_in_time(computed_arrival_time_ + 2.0))
  {
    ROS_WARN("did not complete move in expected time");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("move did not return success; code = %d", cart_result_.return_code);
    return (int)cart_result_.return_code;
  }

  ROS_INFO("move returned success");
  return (int)cart_result_.return_code;
}

int ArmMotionCommander::request_q_data(void)
{
  ROS_INFO("requesting arm joint angles");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::GET_Q_DATA;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  if (!cb_received_in_time(computed_arrival_time_ + 2.0))
  {
    ROS_WARN("did not respond within timeout");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("move did not return success; code = %d", cart_result_.return_code);
    return (int)cart_result_.return_code;
  }

  q_vec_ = cart_result_.q_arm;
  ROS_INFO("move returned success;  arm angles: ");
  int njnts = q_vec_.size();
  for (int ijnt = 0; ijnt < njnts; ijnt++)
  {
    ROS_INFO("%f", q_vec_[ijnt]);
  }
  return (int)cart_result_.return_code;
}

Eigen::VectorXd ArmMotionCommander::get_joint_angles(void)
{
  request_q_data();
  Eigen::VectorXd angs_vecXd;
  int njnts = q_vec_.size();
  angs_vecXd.resize(njnts);
  for (int i = 0; i < njnts; i++)
  {
    angs_vecXd[i] = q_vec_[i];
  }

  return angs_vecXd;
}

int ArmMotionCommander::request_tool_pose(void)
{
  ROS_INFO("requesting right-arm tool pose");
  cart_goal_.command_code = cartesian_planner::cart_moveGoal::GET_TOOL_POSE;
  cart_move_action_client_.sendGoal(cart_goal_, boost::bind(&ArmMotionCommander::doneCb_, this, _1, _2));

  if (!cb_received_in_time(2.0))
  {
    ROS_WARN("did not respond within timeout");
    return (int)cartesian_planner::cart_moveResult::NOT_FINISHED_BEFORE_TIMEOUT;
  }
  if (cart_result_.return_code != cartesian_planner::cart_moveResult::SUCCESS)
  {
    ROS_WARN("move did not return success; code = %d", cart_result_.return_code);
    return (int)cart_result_.return_code;
  }

  tool_pose_stamped_ = cart_result_.current_pose_gripper;
  ROS_INFO("move returned success; right arm tool pose: ");
  ROS_INFO("origin w/rt torso = %f, %f, %f ", tool_pose_stamped_.pose.position.x, tool_pose_stamped_.pose.position.y,
           tool_pose_stamped_.pose.position.z);
  ROS_INFO("quaternion x,y,z,w: %f, %f, %f, %f", tool_pose_stamped_.pose.orientation.x,
           tool_pose_stamped_.pose.orientation.y, tool_pose_stamped_.pose.orientation.z,
           tool_pose_stamped_.pose.orientation.w);
  return (int)cart_result_.return_code;
}
