//
// Created by johannes on 29-10-23.
//

#include "SyncRobotMessage.h"
#include <sstream>
#include "MessageTypes.hpp"

namespace Messaging {
    SyncRobotMessage::SyncRobotMessage(const std::string &message) {
        parse(message);
    }
    SyncRobotMessage::SyncRobotMessage(const Model::Robot robot)
        : SyncRobotMessage(robot.getName(), robot.getPosition(), robot.getFront()) {}
    SyncRobotMessage::SyncRobotMessage(const std::string &aName, const wxPoint aPosition, Model::BoundedVector aFront)
        : name(aName), position(aPosition), front(aFront) {}

    void SyncRobotMessage::parse(const std::string &message){
        std::istringstream is(message);

        std::string name;
        std::getline(is, name, '#');

        is >> position.x >> position.y >> front.x >> front.y;
    }

    void SyncRobotMessage::fillMessage(Messaging::Message &msg) const {
        std::stringstream ss;

        ss << name << "#" << position.x << " " << position.y << " " << front.x << " " << front.y;

        msg.setMessageType(MessageType::SynchronizeRobot);
        msg.setBody(ss.str());
    }

    wxPoint SyncRobotMessage::getPosition() const{
        return position;
    }

    Model::BoundedVector SyncRobotMessage::getFront() const{
        return front;
    }

    std::string SyncRobotMessage::getName() const{
        return name;
    }

    void SyncRobotMessage::updateRobot(Model::Robot& robot) const {
        robot.setFront(getFront());
        robot.setPosition(getPosition());
    };

    Model::RobotPtr SyncRobotMessage::newRobot() const {
        return std::make_shared<Model::Robot>(getName(), getPosition());
    };
}
