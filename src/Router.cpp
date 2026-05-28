#include "Router.hpp"
#include "Node.hpp"
#include <iostream>
//Router

void Router::UpdateNodeData(NodeData data){
    Data = data;
}

float Router::GetRadius() const{
    return Position.Radius;
}

Vec2 Router::GetPosition() const{
    return Position.Position;
}

Vec2 Router::GetScreenPosition(const Vec2 Camera) const{
    return Position.TransformPosition(Camera);
}

void Router::SendMessage(Message message){
    if (Data.Network == nullptr) {
        std::cerr << "Error: Router with id " << Data.Id << " is not connected to a network. Message not sent." << std::endl;
        return;
    };
    message.SenderAddress = Data.Address;
    BasicNodeOperations::SendMessage(message, *Data.Network);
}

void Router::PushMessage(Message message){
    Data.MessageQueue.push(message);
}

NodeData& Router::GetData(){
    return Data;
}

void Router::NodeClicked() {
    std::cout << "Router " << Data.Id << " " << Data.Address.ToString()  << " clicked. Message queue size: " << Data.MessageQueue.size() << std::endl;
};
