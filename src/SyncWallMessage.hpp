#ifndef ROBOTWORLD_SYNCWALLMESSAGE_HPP
#define ROBOTWORLD_SYNCWALLMESSAGE_HPP

#include <string>
#include "Wall.hpp"
#include "Message.hpp"
#include "ObjectId.hpp"

namespace Messaging {

    class SyncWallMessage {
    public:
        SyncWallMessage(const std::string& message);
        SyncWallMessage(const Model::Wall& wall);
        SyncWallMessage(Base::ObjectId id, int ax, int ay, int bx, int by);

        virtual ~SyncWallMessage() = default;


        void fillMessage(Messaging::Message& msg) const;

        const Base::ObjectId& getId() const;
        wxPoint getA() const;
        wxPoint getB() const;

        void updateWall(Model::Wall& wall) const;
        Model::WallPtr newWall() const;
    protected:
        void parse(const std::string& message);
    private:
        Base::ObjectId id;
        int ax, ay;
        int bx, by;
    };

} // Messaging

#endif