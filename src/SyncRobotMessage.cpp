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
    SyncRobotMessage::SyncRobotMessage(const Model::Robot& robot)
        : SyncRobotMessage(robot.getPosition(), robot.getFront()) {}
    SyncRobotMessage::SyncRobotMessage( const wxPoint aPosition, Model::BoundedVector aFront)
        :position(aPosition), front(aFront) {}

    void SyncRobotMessage::parse(const std::string &message){
        std::istringstream is(message);

        is >> position.x >> position.y >> front.x >> front.y;
    }

    void SyncRobotMessage::fillMessage(Messaging::Message &msg) const {
        std::stringstream ss;

        ss <<  position.x << " " << position.y << " " << front.x << " " << front.y;

        msg.setMessageType(MessageType::SynchronizeRobot);
        msg.setBody(ss.str());
    }

    wxPoint SyncRobotMessage::getPosition() const{
        return position;
    }

    Model::BoundedVector SyncRobotMessage::getFront() const{
        return front;
    }

    void SyncRobotMessage::updateRobot(Model::Robot& robot) const {
        robot.setFront(getFront());
        robot.setPosition(getPosition());
        robot.updateWalls();
    };

    Model::RobotPtr SyncRobotMessage::newRobot() const {
        return std::make_shared<Model::Robot>("Bram", getPosition());
    };
}
