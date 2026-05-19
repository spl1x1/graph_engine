#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Node.hpp"
#include "Vector.hpp"

class Router: public INode{
    NodePosition Position;
    NodeData Data;

public:
    Router(Vec2 Position) : Position{Position}, Data{.Id = 0, .Type = "Router"} {};
    ~Router() = default;

    virtual void UpdateNodeData(NodeData data) override;
    virtual float GetRadius() const override;
    virtual Vec2 GetPosition() const override;
    virtual Vec2 GetScreenPosition(const Vec2 Camera) const override;
    virtual void SendMessage(Message message) override;
    virtual void ProccesNextMessage() override{}; //Not implemented yet
    virtual void PushMessage(Message message) override;
    virtual NodeData& GetData() override;
    virtual void NodeClicked() override;
};


#endif // ROUTER_HPP
