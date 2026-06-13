#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Node.hpp"
#include "Vector.hpp"
#include "LSDB.hpp"

class Router: public INode{
    NodePosition Position;
    NodeData Data;
    LSDB topology;  // Composed LSDB for topology management

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
    virtual void OnEdgeAdded(INode* neighbor, const Edge& edge) override;
    
    // LSDB-related methods
    LSDB& GetLSDB();
    const LSDB& GetLSDB() const;
    void InitializeLSDB();
    void PrintTopologyDatabase() const;
    void PrintLSDBStatistics() const;
    uint32_t SyncWithNetwork();
};


#endif // ROUTER_HPP
