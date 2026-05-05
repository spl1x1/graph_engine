#ifndef OSPF_HPP
#define OSPF_HPP

#include <EngineTypes.hpp>

class RouterNode : public NodeAbstract {
public:
    RouterNode(Vec2 position) : NodeAbstract(position) {};
    Color GetColor() override;
};


#endif // OSPF_HPP
