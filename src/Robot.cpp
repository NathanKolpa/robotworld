#include "Robot.hpp"

#include "Client.hpp"
#include "CommunicationService.hpp"
#include "Goal.hpp"
#include "Logger.hpp"
#include "MainApplication.hpp"
#include "MathUtils.hpp"
#include "Message.hpp"
#include "MessageTypes.hpp"
#include "RobotWorld.hpp"
#include "Server.hpp"
#include "Shape2DUtils.hpp"
#include "Wall.hpp"
#include "WayPoint.hpp"
#include "SyncWallMessage.hpp"
#include "SyncRobotMessage.h"

#include <chrono>
#include <ctime>
#include <sstream>
#include <thread>

namespace Model {
    /**
     *
     */
    Robot::Robot() : Robot("", wxDefaultPosition) {
    }

    /**
     *
     */
    Robot::Robot(const std::string &aName) : Robot(aName, wxDefaultPosition) {
    }

    /**
     *
     */
    Robot::Robot(const std::string &aName,
                 const wxPoint &aPosition) :
            name(aName),
            size(wxDefaultSize),
            position(aPosition),
            front(0, 0),
            speed(0.0),
            acting(false),
            driving(false),
            communicating(false) {
        // We use the real position for starters, not an estimated position.
        startPosition = position;
    }

    /**
     *
     */
    Robot::~Robot() {
        if (driving) {
            Robot::stopDriving();
        }
        if (acting) {
            Robot::stopActing();
        }
        if (communicating) {
            stopCommunicating();
        }

        std::cout << "Remove robot\n";
    }

    /**
     *
     */
    void Robot::setName(const std::string &aName,
                        bool aNotifyObservers /*= true*/) {
        name = aName;
        if (aNotifyObservers == true) {
            notifyObservers();
        }
    }

    /**
     *
     */
    wxSize Robot::getSize() const {
        return size;
    }

    /**
     *
     */
    void Robot::setSize(const wxSize &aSize,
                        bool aNotifyObservers /*= true*/) {
        size = aSize;
        if (aNotifyObservers == true) {
            notifyObservers();
        }
    }

    /**
     *
     */
    void Robot::setPosition(const wxPoint &aPosition,
                            bool aNotifyObservers /*= true*/) {
        position = aPosition;
        if (aNotifyObservers == true) {
            notifyObservers();
        }
    }

    /**
     *
     */
    BoundedVector Robot::getFront() const {
        return front;
    }

    /**
     *
     */
    void Robot::setFront(const BoundedVector &aVector,
                         bool aNotifyObservers /*= true*/) {
        front = aVector;
        if (aNotifyObservers == true) {
            notifyObservers();
        }
    }

    /**
     *
     */
    float Robot::getSpeed() const {
        return speed;
    }

    /**
     *
     */
    void Robot::setSpeed(float aNewSpeed,
                         bool aNotifyObservers /*= true*/) {
        speed = aNewSpeed;
        if (aNotifyObservers == true) {
            notifyObservers();
        }
    }

    /**
     *
     */
    void Robot::startActing() {
        isMaster = true;
        startActingAsSlave();
    }

    void Robot::startActingAsSlave() {
        acting = true;
        startDriving();
    }

    /**
     *
     */
    void Robot::stopActing() {
        acting = false;
        driving = false;
    }

    /**
     *
     */
    void Robot::startDriving() {
        if (isMaster) {
            sendReset();
            syncWorld();
            sendStart();
        }

        pathPoint = 0;
        driving = true;

        start = position;

        goal = RobotWorld::getRobotWorld().getGoal("Goal");
        recalculate();
    }

    /**
     *
     */
    void Robot::stopDriving() {
        driving = false;
    }

    void Robot::recalculate(bool toStart) {
        calculateRoute(toStart);
        pathPoint = 0;
    }

    /**
     *
     */
    void Robot::startCommunicating() {
        if (!communicating) {
            communicating = true;

            std::string localPort = "12345";
            if (Application::MainApplication::isArgGiven("-local_port")) {
                localPort = Application::MainApplication::getArg("-local_port").value;
            }

            if (Messaging::CommunicationService::getCommunicationService().isStopped()) {
                TRACE_DEVELOP("Restarting the Communication service");
                Messaging::CommunicationService::getCommunicationService().restart();
            }

            server = std::make_shared<Messaging::Server>(static_cast<unsigned short>(std::stoi(localPort)),
                                                         toPtr<Robot>());
            Messaging::CommunicationService::getCommunicationService().registerServer(server);
        }
    }


    int Robot::minDistance() {
        wxPoint self = position;

        RobotPtr otherRobot = RobotWorld::getRobotWorld().getRobot("Bram");
        if (!otherRobot) {
            return std::numeric_limits<int>().max();
        }

        wxPoint other = otherRobot->position;

        return std::sqrt(std::pow(other.x - self.x, 2) + std::pow(other.y - self.y, 2) * 1.0);
    }

    /**
     *
     */
    void Robot::stopCommunicating() {
        if (communicating) {
            communicating = false;
            Messaging::Message message(Messaging::StopCommunicatingRequest, "stop");
            sendMessage(message);
        }
    }

    void Robot::sendMessage(const Messaging::Message &msg) {
        std::string localPort = "12345";
        if (Application::MainApplication::isArgGiven("-remote_port")) {
            localPort = Application::MainApplication::getArg("-remote_port").value;
        }

        std::string remoteIp = "localhost";
        if (Application::MainApplication::isArgGiven("-remote_ip")) {
            remoteIp = Application::MainApplication::getArg("-remote_ip").value;
        }

        Messaging::Client client(remoteIp,
                                 static_cast<unsigned short>(std::stoi(localPort)),
                                 toPtr<Robot>());

        client.dispatchMessage(msg);
    }

    /**
     *
     */
    wxRegion Robot::getRegion() const {
        wxPoint translatedPoints[] = {getFrontRight(), getFrontLeft(), getBackLeft(), getBackRight()};
        return wxRegion(4, translatedPoints); // @suppress("Avoid magic numbers")
    }

    /**
     *
     */
    bool Robot::intersects(const wxRegion &aRegion) const {
        wxRegion region = getRegion();
        region.Intersect(aRegion);
        return !region.IsEmpty();
    }

    /**
     *
     */
    wxPoint Robot::getFrontLeft() const {
        // x and y are pointing to top left now
        int x = position.x - (size.x / 2);
        int y = position.y - (size.y / 2);

        wxPoint originalFrontLeft(x, y);
        double angle = Utils::Shape2DUtils::getAngle(front) + 0.5 * Utils::PI;

        wxPoint frontLeft(static_cast<int>((originalFrontLeft.x - position.x) * std::cos(angle) -
                                           (originalFrontLeft.y - position.y) * std::sin(angle) + position.x),
                          static_cast<int>((originalFrontLeft.y - position.y) * std::cos(angle) +
                                           (originalFrontLeft.x - position.x) * std::sin(angle) + position.y));

        return frontLeft;
    }

    /**
     *
     */
    wxPoint Robot::getFrontRight() const {
        // x and y are pointing to top left now
        int x = position.x - (size.x / 2);
        int y = position.y - (size.y / 2);

        wxPoint originalFrontRight(x + size.x, y);
        double angle = Utils::Shape2DUtils::getAngle(front) + 0.5 * Utils::PI;

        wxPoint frontRight(static_cast<int>((originalFrontRight.x - position.x) * std::cos(angle) -
                                            (originalFrontRight.y - position.y) * std::sin(angle) + position.x),
                           static_cast<int>((originalFrontRight.y - position.y) * std::cos(angle) +
                                            (originalFrontRight.x - position.x) * std::sin(angle) + position.y));

        return frontRight;
    }

    /**
     *
     */
    wxPoint Robot::getBackLeft() const {
        // x and y are pointing to top left now
        int x = position.x - (size.x / 2);
        int y = position.y - (size.y / 2);

        wxPoint originalBackLeft(x, y + size.y);

        double angle = Utils::Shape2DUtils::getAngle(front) + 0.5 * Utils::PI;

        wxPoint backLeft(static_cast<int>((originalBackLeft.x - position.x) * std::cos(angle) -
                                          (originalBackLeft.y - position.y) * std::sin(angle) + position.x),
                         static_cast<int>((originalBackLeft.y - position.y) * std::cos(angle) +
                                          (originalBackLeft.x - position.x) * std::sin(angle) + position.y));

        return backLeft;
    }

    /**
     *
     */
    wxPoint Robot::getBackRight() const {
        // x and y are pointing to top left now
        int x = position.x - (size.x / 2);
        int y = position.y - (size.y / 2);

        wxPoint originalBackRight(x + size.x, y + size.y);

        double angle = Utils::Shape2DUtils::getAngle(front) + 0.5 * Utils::PI;

        wxPoint backRight(static_cast<int>((originalBackRight.x - position.x) * std::cos(angle) -
                                           (originalBackRight.y - position.y) * std::sin(angle) + position.x),
                          static_cast<int>((originalBackRight.y - position.y) * std::cos(angle) +
                                           (originalBackRight.x - position.x) * std::sin(angle) + position.y));

        return backRight;
    }

    /**
     *
     */
    void Robot::handleNotification() {
        //	std::unique_lock<std::recursive_mutex> lock(robotMutex);

        static int update = 0;
        if ((++update % 200) == 0) // @suppress("Avoid magic numbers")
        {
            notifyObservers();
        }
    }

    /**
     *
     */
    void Robot::handleRequest(Messaging::Message &aMessage) {
        FUNCTRACE_TEXT_DEVELOP(aMessage.asString());

        switch (aMessage.getMessageType()) {
            case Messaging::StopCommunicatingRequest: {
                aMessage.setMessageType(Messaging::StopCommunicatingResponse);
                aMessage.setBody("StopCommunicatingResponse");
                // Handle the request. In the limited context of this works. I am not sure
                // whether this works OK in a real application because the handling is time sensitive,
                // i.e. 2 async timers are involved:
                // see CommunicationService::stopServer and Server::stopHandlingRequests
                Messaging::CommunicationService::getCommunicationService().stopServer(12345,
                                                                                      true); // @suppress("Avoid magic numbers")

                break;
            }
            case Messaging::EchoRequest: {
                aMessage.setMessageType(Messaging::EchoResponse);
                aMessage.setBody("Messaging::EchoResponse: " + aMessage.asString());
                break;
            }
            case Messaging::Reset: {
                RobotWorld::getRobotWorld().resetWorld();
                aMessage.setMessageType(Messaging::EchoResponse);
                break;
            }
            case Messaging::Start: {
                if (!acting) {
                    TRACE_DEVELOP("Start on request of other");
                    startActingAsSlave();
                }
                aMessage.setMessageType(Messaging::EchoResponse);
                break;
            }
            case Messaging::SynchronizeWall: {
                Messaging::SyncWallMessage wallMessage(aMessage.getBody());

                WallPtr wall = RobotWorld::getRobotWorld().getWall(wallMessage.getId());
                if (wall) {
                    wallMessage.updateWall(*wall);
                    TRACE_DEVELOP("UPDATING WALL: " + wall->asDebugString());
                } else {
                    Model::WallPtr wall = wallMessage.newWall();
                    TRACE_DEVELOP("CREATING WALL: " + wall->asDebugString());
                    RobotWorld::getRobotWorld().addWall(wall);
                }

                // trigger a redraw of the canvas through some callback spaghetti.
                notifyObservers();
                aMessage.setMessageType(Messaging::EchoResponse);
                break;
            }
            case Messaging::SynchronizeRobot: {
                Messaging::SyncRobotMessage robotMessage(aMessage.getBody());

                RobotPtr robot = RobotWorld::getRobotWorld().getRobot("Bram");
                if (robot) {
                    robotMessage.updateRobot(*robot);
                } else {
                    Model::RobotPtr robot = robotMessage.newRobot();
                    RobotWorld::getRobotWorld().addRobot(robot);

                    robot->walls.push_back(RobotWorld::getRobotWorld().newWall(wxPoint(0, 0), wxPoint(0, 0)));
                    robot->walls.push_back(RobotWorld::getRobotWorld().newWall(wxPoint(0, 0), wxPoint(0, 0)));
                    robot->walls.push_back(RobotWorld::getRobotWorld().newWall(wxPoint(0, 0), wxPoint(0, 0)));
                    robot->walls.push_back(RobotWorld::getRobotWorld().newWall(wxPoint(0, 0), wxPoint(0, 0)));
                }

                notifyObservers();
                aMessage.setMessageType(Messaging::EchoResponse);
                break;
            }
            default: {
                TRACE_DEVELOP(__PRETTY_FUNCTION__ + std::string(": default not implemented"));
                break;
            }
        }
    }

    /**
     *
     */
    void Robot::handleResponse(const Messaging::Message &aMessage) {
        FUNCTRACE_TEXT_DEVELOP(aMessage.asString());

        switch (aMessage.getMessageType()) {
            case Messaging::StopCommunicatingResponse: {
                break;
            }
            case Messaging::EchoResponse: {
                break;
            }
            default: {
                TRACE_DEVELOP(__PRETTY_FUNCTION__ + std::string(": default not implemented, ") + aMessage.asString());
                break;
            }
        }
    }

    /**
     *
     */
    std::string Robot::asString() const {
        std::ostringstream os;

        os << "Robot " << name << " at (" << position.x << "," << position.y << ")";

        return os.str();
    }

    void Robot::updateWalls() {
        if (walls.size() != 4) {
            return;
        }

        // front
        walls[0]->setPoint1(getFrontLeft());
        walls[0]->setPoint2(getFrontRight());

        // back
        walls[1]->setPoint1(getBackLeft());
        walls[1]->setPoint2(getBackRight());

        // left
        walls[2]->setPoint1(getFrontLeft());
        walls[2]->setPoint2(getBackLeft());

        // right
        walls[3]->setPoint1(getFrontRight());
        walls[3]->setPoint2(getBackRight());
    }

    /**
     *
     */
    std::string Robot::asDebugString() const {
        std::ostringstream os;

        os << "Robot:\n";
        os << "Robot " << name << " at (" << position.x << "," << position.y << ")\n";

        return os.str();
    }

    void Robot::step(int msInterval) {
        if (!driving) {
            return;
        }

        int avoidModeDist = 70;

        if (isBackTracking) {
            backTrackingFor += msInterval;

            if(backTrackingFor > 5000) {
                backTrackingFor = 0;
                avoidingForMs = 0;
                recalculate();
                isBackTracking = false;
            }

        } else if (inAvoidMode) {
            avoidingForMs += msInterval;

            if (minDistance() > avoidModeDist) {
                TRACE_DEVELOP("NOT AVOIDING ANYMORE");
                inAvoidMode = false;
                avoidingForMs = 0;
            }

            if (!isMaster) {
                if (avoidingForMs > 8000) {
                    TRACE_DEVELOP("START BACKTRACKING");
                    isBackTracking = true;
                    avoidingForMs = 0;
                    recalculate(true);
                }

                return;
            }
            else {
                if (avoidingForMs > 100) {
                    avoidingForMs = 0;
                    inAvoidMode = false;
                    recalculate();
                }
            }
        } else if (!isBackTracking) {
            if (minDistance() < avoidModeDist) {
                inAvoidMode = true;

                if (isMaster) {
                    TRACE_DEVELOP("AVOIDING");
                    recalculate();
                }
            }
        }

        if (path.size() == 0) {
            recalculate();
        }

        // The runtime value always wins!!
        speed = static_cast<float>(Application::MainApplication::getSettings().getSpeed()) /
                static_cast<float>(10.0);

        // Compare a float/double with another float/double: use epsilon...
        if (std::fabs(speed - 0.0) <= std::numeric_limits<float>::epsilon()) {
            setSpeed(1.0, false); // @suppress("Avoid magic numbers")
        }

        // We use the real position for starters, not an estimated position.
        startPosition = position;

        if (position.x > 0 && position.x < 500 && position.y > 0 && position.y < 500 &&
            pathPoint < path.size()) {
            // Do the update
            const PathAlgorithm::Vertex &vertex = path[pathPoint += static_cast<unsigned int>(speed)];
            front = BoundedVector(vertex.asPoint(), position);
            position.x = vertex.x;
            position.y = vertex.y;

            // Stop on arrival or collision
            if (arrived(goal) || collision()) {
                Application::Logger::log(__PRETTY_FUNCTION__ + std::string(": arrived or collision"));
                driving = false;
            }

            notifyObservers();
            sendPosition();
        }
    }

    /**
     *
     */
    void Robot::calculateRoute(bool toStart) {
        path.clear();
        GoalPtr aGoal = goal;

        if (aGoal) {
            // Turn off logging if not debugging AStar
            Application::Logger::setDisable();

            front = BoundedVector(aGoal->getPosition(), position);
            //handleNotificationsFor( astar);

            if (toStart) {
                path = astar.search(position, start, size);
            } else {
                path = astar.search(position, aGoal->getPosition(), size);
            }
            //stopHandlingNotificationsFor( astar);

            Application::Logger::setDisable(false);
        }
    }

    /**
     *
     */
    bool Robot::arrived(GoalPtr aGoal) {
        if (aGoal && intersects(aGoal->getRegion())) {
            return true;
        }
        return false;
    }

    void Robot::sendReset() {
        Messaging::Message msg;
        msg.setMessageType(Messaging::Reset);
        sendMessage(msg);
    }

    void Robot::sendStart() {
        Messaging::Message msg;
        msg.setMessageType(Messaging::Start);
        sendMessage(msg);
    }

    void Robot::sendPosition() {
        Messaging::SyncRobotMessage robotMessage(*this);
        Messaging::Message msg;
        robotMessage.fillMessage(msg);
        sendMessage(msg);
    }

    void Robot::syncWorld() {
        Messaging::Message msg;

        TRACE_DEVELOP(__PRETTY_FUNCTION__);

        const std::vector <WallPtr> &walls = RobotWorld::getRobotWorld().getWalls();
        for (WallPtr wall: walls) {
            if (wall->takeIsModified()) {
                Messaging::SyncWallMessage syncWallMessage(*wall);
                syncWallMessage.fillMessage(msg);
                sendMessage(msg);
                TRACE_DEVELOP("SENDING WALL: " + wall->asDebugString());
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // @suppress("Avoid magic numbers")
            }
        }
    }

    /**
     *
     */
    bool Robot::collision() {
        wxPoint frontLeft = getFrontLeft();
        wxPoint frontRight = getFrontRight();
        wxPoint backLeft = getBackLeft();
        wxPoint backRight = getBackRight();

        const std::vector <WallPtr> &walls = RobotWorld::getRobotWorld().getWalls();
        for (WallPtr wall: walls) {
            if (Utils::Shape2DUtils::intersect(frontLeft, frontRight, wall->getPoint1(), wall->getPoint2()) ||
                Utils::Shape2DUtils::intersect(frontLeft, backLeft, wall->getPoint1(), wall->getPoint2()) ||
                Utils::Shape2DUtils::intersect(frontRight, backRight, wall->getPoint1(), wall->getPoint2())) {
                return true;
            }
        }
        const std::vector <RobotPtr> &robots = RobotWorld::getRobotWorld().getRobots();
        for (RobotPtr robot: robots) {
            if (getObjectId() == robot->getObjectId()) {
                continue;
            }
            if (intersects(robot->getRegion())) {
                return true;
            }
        }
        return false;
    }

} // namespace Model
