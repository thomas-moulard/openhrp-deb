#include "Mobility.h"
#include "PathPlanner.h"
#include "Algorithm.h"
#include "Roadmap.h"

using namespace PathEngine;

Algorithm::Algorithm(PathPlanner* planner) 
  : isRunning_(false), planner_(planner)
{
  properties_["interpolation-distance"] = "0.1"; 
  roadmap_ = new Roadmap(planner_);
}

Algorithm::~Algorithm()
{
}

void Algorithm::setProperty(const std::string& key, const std::string& value)
{
    properties_[key] = value;
}

void Algorithm::setProperties(const std::map<std::string, std::string> &properties) 
{
  std::map<std::string, std::string>::const_iterator it;
  it = properties.begin();
  while (it != properties.end()) {
    properties_[it->first] = it->second;
    it++;
  }
}

void Algorithm::getProperties(std::vector<std::string> &names,
			      std::vector<std::string> &values) {
  names.clear();
  values.clear();

  std::map<std::string, std::string>::iterator it;
  it = properties_.begin();
  while (it != properties_.end()) {
    names.push_back((*it).first);
    values.push_back((*it).second);
    it++;
  }
}

bool Algorithm::tryDirectConnection()
{
  Mobility *mobility = planner_->getMobility();

  if (mobility->isReachable(start_, goal_)){
    path_.push_back(start_);
    path_.push_back(goal_);
    return true;
  }else{
    return false;
  }
}

bool Algorithm::preparePlanning()
{
  isRunning_ = true;

  Mobility::interpolationDistance(atof(properties_["interpolation-distance"].c_str()));
#if 1
  std::map<std::string, std::string>::iterator it;
  it = properties_.begin();
  std::cout << "properties:" << std::endl;
  while (it != properties_.end()) {
      std::cout << "  " << it->first << " = " << it->second << std::endl;
      it++;
  }
  std::cout << std::endl;
#endif

  path_.clear();

  std::cout << "start:" << start_ << std::endl;
  std::cout << "goal:" << goal_ << std::endl;

  // validity checks of start&goal configurations
  if (!start_.isValid()){
      std::cerr << "start configuration is invalid" << std::endl;
      return false;
  }
  if (planner_->checkCollision(start_)){
      std::cerr << "start configuration is not collision-free" << std::endl;
      return false;
  }
  if (!goal_.isValid()){
      std::cerr << "goal configuration is invalid" << std::endl;
      return false;
  }
  if (planner_->checkCollision(goal_)){
      std::cerr << "goal configuration is not collision-free" << std::endl;
      return false;
  }
#if 0
  Mobility *mobility = planner_->getMobility();
  if (!mobility->isReachable(start_, goal_, false)){
      std::cerr << "goal is not reachable even if collision is not checked"
                << std::endl;
      return false;
  }
#endif

  return true;
}
