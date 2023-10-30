//
// Created by johannes on 29-10-23.
//

#ifndef ROBOTWORLD_SYNCROBOTMESSAGE_H
#define ROBOTWORLD_SYNCROBOTMESSAGE_H

#include "Robot.hpp"
#include "BoundedVector.hpp"

namespace Messaging {

    class SyncRobotMessage {
        SyncRobotMessage(const std::string &message);
        SyncRobotMessage(const Model::Robot robot);
        SyncRobotMessage(const std::string &aName, const wxPoint aPosition, Model::BoundedVector aFront);

        //getPosition, getFront
        virtual ~SyncRobotMessage() = default;


        void fillMessage(Messaging::Message &msg) const;

        wxPoint getPosition() const;
        Model::BoundedVector getFront() const;
        std::string getName() const;

        void updateRobot(Model::Robot& robot) const;
        Model::RobotPtr newRobot() const;


    protected:
        void parse(const std::string& message);
    private:
        std::string name;
        wxPoint position;
        Model::BoundedVector front;
    };
}

#endif //ROBOTWORLD_SYNCROBOTMESSAGE_H
