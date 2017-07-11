#include <gazebo_msgs/ModelStates.h>
#include <geometry_msgs/Pose.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <stdio.h>
#include <string.h>
#include <random>

geometry_msgs::Pose g_mobot_pose;
geometry_msgs::Pose g_noisy_mobot_pose;
geometry_msgs::Quaternion g_quat;
ros::Publisher g_pose_publisher;
ros::Publisher g_yaw_publisher;
ros::Publisher g_gps_publisher;
std::normal_distribution<double> distribution(0.0, 1.0);
std::default_random_engine generator;

double convertPlanarQuat2Phi(geometry_msgs::Quaternion quaternion)
{
  double quat_z = quaternion.z;
  double quat_w = quaternion.w;
  double phi = 2.0 * atan2(quat_z, quat_w);
  return phi;
}

void model_state_CB(const gazebo_msgs::ModelStates& model_states)
{
  int n_models = model_states.name.size();
  int imodel;

  bool found_name = false;
  for (imodel = 0; imodel < n_models; imodel++)
  {
    std::string model_name(model_states.name[imodel]);
    if (model_name.compare("mobot") == 0)
    {
      ROS_INFO("found match: mobot is model %d", imodel);
      found_name = true;
      break;
    }
  }
  if (found_name)
  {
    g_mobot_pose = model_states.pose[imodel];
    g_pose_publisher.publish(g_mobot_pose);
    g_noisy_mobot_pose = g_mobot_pose;
    g_noisy_mobot_pose.orientation = g_quat;
    g_noisy_mobot_pose.position.x += distribution(generator);
    g_noisy_mobot_pose.position.y += distribution(generator);
    g_gps_publisher.publish(g_noisy_mobot_pose);
    double gazebo_yaw = convertPlanarQuat2Phi(g_mobot_pose.orientation);
    std_msgs::msg::Float64 yaw_msg;
    yaw_msg.data = gazebo_yaw;
    g_yaw_publisher.publish(yaw_msg);
  }
  else
  {
    ROS_WARN("state of mobot model not found");
  }
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "gazebo_model_publisher");
  ros::NodeHandle nh;
  ROS_INFO("gazebo model state publisher");
  g_pose_publisher = nh.advertise<geometry_msgs::Pose>("gazebo_mobot_pose", 1);
  g_yaw_publisher = nh.advertise<std_msgs::msg::Float64>("gazebo_mobot_yaw", 1);
  g_gps_publisher = nh.advertise<geometry_msgs::Pose>("gazebo_mobot_noisy_pose", 1);
  ros::Subscriber state_sub = nh.subscribe("gazebo/model_states", 1, model_state_CB);

  g_quat.x = 0;
  g_quat.y = 0;
  g_quat.z = 0;
  g_quat.w = 1;
  ros::spin();
}
