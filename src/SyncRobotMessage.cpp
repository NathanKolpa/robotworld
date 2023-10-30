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
        : SyncRobotMessage(robot.getPosition(), robot.getFront()) {}
    SyncRobotMessage::SyncRobotMessage(const std::string &aName, const wxPoint aPosition, Model::BoundedVector aFront)
        : aName(name), position(position), front(aFront) {}

    void SyncRobotMessage::parse(const string &message){
        std::istringstream is(message);

        std::string name;
        std::getline()

        is >> position.x >> position.y >> front.x >> front.y;
    }

    void SyncRobotMessage::fillMessage(Messaging::Message &msg) const {
        std::stringstream ss;

        ss << name << "#" << position.x << " " << position.y << " " << front.x << " " << front.y;

        msg.setMessageType(MessageType::SynchronizeRobot);
        msg.setBody(ss.str());
    }
}