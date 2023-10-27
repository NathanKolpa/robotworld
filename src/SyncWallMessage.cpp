#include "SyncWallMessage.hpp"
#include <sstream>
#include "MessageTypes.hpp"

namespace Messaging {

    SyncWallMessage::SyncWallMessage(const std::string &message) {
        parse(message);
    }

    SyncWallMessage::SyncWallMessage(const Model::Wall &wall)
            : SyncWallMessage(wall.getObjectId(), wall.getPoint1().x, wall.getPoint1().y, wall.getPoint2().x,
                              wall.getPoint2().y) {
    }

    SyncWallMessage::SyncWallMessage(Base::ObjectId id, int ax, int ay, int bx, int by)
            : id(id), ax(ax), ay(ay), bx(bx), by(by) {}

    void SyncWallMessage::parse(const std::string &message) {
        std::istringstream is(message);

        std::string objId;
        std::getline(is, objId, '#');
        id = Base::ObjectId(objId);

        is >> ax >> ay >> bx >> by;
    }

    void SyncWallMessage::fillMessage(Messaging::Message &msg) const {
        std::stringstream ss;

        ss << id.toString() << "#" << ax << " " << ay << " " << bx << " " << by;

        msg.setMessageType(MessageType::SynchronizeWall);
        msg.setBody(ss.str());
    }

    const Base::ObjectId &SyncWallMessage::getId() const { return id; }

    wxPoint SyncWallMessage::getA() const {
        return wxPoint(ax, ay);
    }

    wxPoint SyncWallMessage::getB() const {
        return wxPoint(bx, by);
    }

    void SyncWallMessage::updateWall(Model::Wall &wall) const {
        wall.setPoint1(getA());
        wall.setPoint2(getB());
    }

    Model::WallPtr SyncWallMessage::newWall() const {
        return std::make_shared<Model::Wall>(getId(), getA(), getB());
    }


} // Messaging