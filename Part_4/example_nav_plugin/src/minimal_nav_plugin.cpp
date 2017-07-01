// package name, header name for new plugin library
#include <example_nav_plugin/minimal_nav_plugin.h>

#include <pluginlib/class_list_macros.h>

PLUGINLIB_EXPORT_CLASS(MinimalPlanner, nav_core::BaseLocalPlanner);

<<<<<<< Updated upstream:Part_4/example_nav_plugin/src/minimal_nav_plugin.cpp
MinimalPlanner::MinimalPlanner()
{
  // nothing ot fill in here; "initialize" will do the initializations
=======
test_planner::TestPlanner::TestPlanner()
{
	
>>>>>>> Stashed changes:Part_4/test_plugin/src/test_planner_orig.cpp
}

// put inits here:
void MinimalPlanner::initialize(std::string name, tf::TransformListener *tf, costmap_2d::Costmap2DROS *costmap_ros)
{
  ros::NodeHandle nh(name);

  old_size = 0;
  handed_tf = tf;
}

<<<<<<< Updated upstream:Part_4/example_nav_plugin/src/minimal_nav_plugin.cpp
bool MinimalPlanner::isGoalReached()
{
  // For demonstration purposes, sending a single navpoint will cause five seconds of activity before exiting.
  return ros::Time::now() > tg;
}

bool MinimalPlanner::setPlan(const std::vector<geometry_msgs::PoseStamped> &plan)
{
  // The "plan" that comes in here is a bunch of poses of varaible length, calculated by the global planner(?).
  // We're just ignoring it entirely, but an actual planner would probably take this opportunity
  // to store it somewhere and maybe update components that refrerence it.
=======
bool test_planner::TestPlanner::isGoalReached()
{
  return ros::Time::now() > tg;
}

bool test_planner::TestPlanner::setPlan(const std::vector<geometry_msgs::PoseStamped> &plan)
{
	
	
	
>>>>>>> Stashed changes:Part_4/test_plugin/src/test_planner_orig.cpp
  ROS_INFO("GOT A PLAN OF SIZE %lu", plan.size());

  if (plan.size() != old_size)
  {
    old_size = plan.size();
    tg = ros::Time::now() + ros::Duration(5.0);
  }

  return true;
}

<<<<<<< Updated upstream:Part_4/example_nav_plugin/src/minimal_nav_plugin.cpp
bool MinimalPlanner::computeVelocityCommands(geometry_msgs::Twist &cmd_vel)
{
  // This is the meat-and-potatoes of the plugin, where velocities are actually generated.
  // in this minimal case, simply specify constants; more generally, choose vx and omega_z
  // intelligently based on the goal and the environment
  // When isGoalReached() is false, computeVelocityCommands will be called each iteration
  // of the controller--which is a settable parameter.  On each iteration, values in
  // cmd_vel should be computed and set, and these values will be published by move_base
  // to command robot motion
=======
bool test_planner::TestPlanner::computeVelocityCommands(geometry_msgs::Twist &cmd_vel)
{
	
	
>>>>>>> Stashed changes:Part_4/test_plugin/src/test_planner_orig.cpp
  cmd_vel.linear.x = 0.2;
  cmd_vel.linear.y = 0.0;
  cmd_vel.linear.z = 0.0;
  cmd_vel.angular.z = 0.2;
  return true;
}
