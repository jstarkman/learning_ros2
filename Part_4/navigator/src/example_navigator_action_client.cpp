

#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include <navigator/navigatorAction.h>
#include <ros/ros.h>
#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/Geometry>

geometry_msgs::PoseStamped g_desired_pose;
int g_navigator_rtn_code;
void navigatorDoneCb(const actionlib::SimpleClientGoalState& state, const navigator::navigatorResultConstPtr& result)
{
  ROS_INFO(" navigatorDoneCb: server responded with state [%s]", state.toString().c_str());
  g_navigator_rtn_code = result->return_code;
  ROS_INFO("got object code response = %d; ", g_navigator_rtn_code);
  if (g_navigator_rtn_code == navigator::navigatorResult::DESTINATION_CODE_UNRECOGNIZED)
  {
    ROS_WARN("destination code not recognized");
  }
  else if (g_navigator_rtn_code == navigator::navigatorResult::DESIRED_POSE_ACHIEVED)
  {
    ROS_INFO("reached desired location!");
  }
  else
  {
    ROS_WARN("desired pose not reached!");
  }
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "example_navigator_action_client");
  ros::NodeHandle nh;

  actionlib::SimpleActionClient<navigator::navigatorAction> navigator_ac("navigatorActionServer", true);

  ROS_INFO("waiting for server: ");
  bool server_exists = false;
  while ((!server_exists) && (ros::ok()))
  {
    server_exists = navigator_ac.waitForServer(ros::Duration(0.5));
    ros::spinOnce();
    ros::Duration(0.5).sleep();
    ROS_INFO("retrying...");
  }
  ROS_INFO("connected to navigator action server");

  navigator::navigatorGoal navigation_goal;

  navigation_goal.location_code = navigator::navigatorGoal::HOME;

  ROS_INFO("sending goal: ");
  navigator_ac.sendGoal(navigation_goal, &navigatorDoneCb);

  bool finished_before_timeout = navigator_ac.waitForResult(ros::Duration(30.0));

  if (!finished_before_timeout)
  {
    ROS_WARN("giving up waiting on result ");
    return 1;
  }

  return 0;
}
