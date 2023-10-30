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
        // we ""prevent"" seg faults with this sleep.
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // @suppress("Avoid magic numbers")
        acting = true;
        std::thread newRobotThread([this] { startDriving(); });
        robotThread.swap(newRobotThread);
    }

    /**
     *
     */
    void Robot::stopActing() {
        acting = false;
        driving = false;
        robotThread.join();
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

        driving = true;

        goal = RobotWorld::getRobotWorld().getGoal("Goal");
        recalculate();

        drive();
    }

    void Robot::recalculate() {
        calculateRoute(goal);
    }

    /**
     *
     */
    void Robot::stopDriving() {
        driving = false;
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

        Messaging::Client client("localhost",
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

    static std::mutex wallUpdateMutex;

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
                }
                else {
                    Model::WallPtr wall = wallMessage.newWall();
                    TRACE_DEVELOP("CREATING WALL: " + wall->asDebugString());
                    RobotWorld::getRobotWorld().addWall(wall);
                }

                // trigger a redraw of the canvas through some callback spaghetti.
                notifyObservers();
                aMessage.setMessageType(Messaging::EchoResponse);
                recalculate();

                break;
            }
            case Messaging::SynchronizeRobot: {
                Messaging::SyncRobotMessage robotMessage(aMessage.getBody());

                RobotPtr robot = RobotWorld::getRobotWorld().getRobot(robotMessage.getName());
                if(robot) {
                    robotMessage.updateRobot(*robot);
                }
                else {
                    Model::RobotPtr robot = robotMessage.newRobot();
                    RobotWorld::getRobotWorld().addRobot(robot);
                }

                notifyObservers();
                aMessage.setMessageType(Messaging::EchoResponse);
                recalculate();

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

    /**
     *
     */
    std::string Robot::asDebugString() const {
        std::ostringstream os;

        os << "Robot:\n";
        os << "Robot " << name << " at (" << position.x << "," << position.y << ")\n";

        return os.str();
    }

    /**
     *
     */
    void Robot::drive() {
        try {
            // The runtime value always wins!!
            speed = static_cast<float>(Application::MainApplication::getSettings().getSpeed()) /
                    static_cast<float>(10.0);

            // Compare a float/double with another float/double: use epsilon...
            if (std::fabs(speed - 0.0) <= std::numeric_limits<float>::epsilon()) {
                setSpeed(1.0, false); // @suppress("Avoid magic numbers")
            }

            // We use the real position for starters, not an estimated position.
            startPosition = position;

            unsigned pathPoint = 0;
            while (position.x > 0 && position.x < 500 && position.y > 0 && position.y < 500 &&
                   pathPoint < path.size()) // @suppress("Avoid magic numbers")
            {
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

                // If there is no sleep_for here the robot will immediately be on its destination....
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // @suppress("Avoid magic numbers")

                // this should be the last thing in the loop
                if (driving == false) {
                    break;
                }
            } // while
        }
        catch (std::exception &e) {
            Application::Logger::log(__PRETTY_FUNCTION__ + std::string(": ") + e.what());
            std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        }
        catch (...) {
            Application::Logger::log(__PRETTY_FUNCTION__ + std::string(": unknown exception"));
            std::cerr << __PRETTY_FUNCTION__ << ": unknown exception" << std::endl;
        }
    }

    /**
     *
     */
    void Robot::calculateRoute(GoalPtr aGoal) {
        path.clear();
        if (aGoal) {
            // Turn off logging if not debugging AStar
            Application::Logger::setDisable();

            front = BoundedVector(aGoal->getPosition(), position);
            //handleNotificationsFor( astar);
            path = astar.search(position, aGoal->getPosition(), size);
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
