#include <iostream>
#include <vector>
#include <algorithm>

#include "test_motion_pepper.h"


test_motion_pepper::test_motion_pepper(ros::NodeHandle &nh)
{
  // read in config options
  n = nh;
  n.param( "frequency", freq, 25);
  n.param<std::string>("ip", m_ip, "131.254.10.126");

  m_session = qi::makeSession();
  std::string ip_port = "tcp://" + m_ip + ":9559";
  m_session->connect(ip_port);
  m_qiProxy = m_session->service("ALMotion");
}

test_motion_pepper::~test_motion_pepper(){
  float vel_x = 0.0;
  float vel_y = 0.0;
  float vel_th = 0.0;

  m_qiProxy.async<void>("move", vel_x, vel_y, vel_th);


}

void test_motion_pepper::spin()
{
  ros::Rate loop_rate(freq);

  std::vector<std::string> m_bodyJointNames = m_qiProxy.call< std::vector<std::string> >("getBodyNames", "Body");
  // Erase wheel joints ( the last 3)
  m_bodyJointNames.resize(m_bodyJointNames.size()-3);

  std::vector<double> m_bodyJointValues = m_qiProxy.call< std::vector<double> >("getAngles", m_bodyJointNames, 1);

  std::cout << "Joints:" << std::endl;
  for (unsigned int i = 0; i<m_bodyJointNames.size(); i++)
    std::cout << m_bodyJointNames[i] << " - " << m_bodyJointValues[i] << std::endl;

  // These instructions have no effect
  //  m_qiProxy.call< void >("setSmartStiffnessEnabled",false);
  //  m_qiProxy.call< void >("setMoveArmsEnabled",false,false);
  //  m_qiProxy.call< void >("setStiffnesses","Body", 1.0);


  while(ros::ok()){

    float vel_x = 0.001;
    float vel_y = 0.001;
    float vel_th = 0.001;

    m_qiProxy.async<void>("move", vel_x, vel_y, vel_th);

    qi::AnyValue names_qi = fromStringVectorToAnyValue(m_bodyJointNames);
    qi::AnyValue angles_qi = fromDoubleVectorToAnyValue(m_bodyJointValues);

    m_qiProxy.async<void>("setAngles", names_qi, angles_qi, 0.7f);

    ros::spinOnce();
    loop_rate.sleep();
  }

}



qi::AnyValue test_motion_pepper::fromStringVectorToAnyValue(const std::vector<std::string> &vector)
{
  qi::AnyValue res;
  try
  {
    std::vector<qi::AnyValue> vector_qi;
    vector_qi.reserve(vector.size());
    vector_qi.resize(vector.size());

    std::vector<std::string>::const_iterator it = vector.begin();
    std::vector<qi::AnyValue>::iterator it_qi = vector_qi.begin();
    for(; it != vector.end(); ++it, ++it_qi)
    {
      *it_qi = qi::AnyValue(qi::AnyReference::from(*it), false, false);
    }
    res = qi::AnyValue(qi::AnyReference::from(vector_qi), false, false);
  }
  catch(const std::exception& e)
  {
    std::cout << "Could not convert to qi::AnyValue \n\tTrace: " << e.what() << std::endl;
  }
  return res;
}

qi::AnyValue test_motion_pepper::fromDoubleVectorToAnyValue(const std::vector<double> &vector)
{
  qi::AnyValue res;
  try
  {
    std::vector<qi::AnyValue> vector_qi;
    vector_qi.reserve(vector.size());
    vector_qi.resize(vector.size());

    std::vector<double>::const_iterator it = vector.begin();
    std::vector<qi::AnyValue>::iterator it_qi = vector_qi.begin();
    for(; it != vector.end(); ++it, ++it_qi)
    {
      *it_qi = qi::AnyValue(qi::AnyReference::from(static_cast<float>(*it)), false, false);
    }
    res = qi::AnyValue(qi::AnyReference::from(vector_qi), false, false);
  }
  catch(const std::exception& e)
  {
    std::cout << "Could not convert to qi::AnyValue \n\tTrace: " << e.what() << std::endl;
  }
  return res;
}
