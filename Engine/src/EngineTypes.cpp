#include <EngineTypes.hpp>
#include <cstdint>
#include <sys/types.h>


bool Background::operator==(const Background& other){
    return this->TexturePath == other.TexturePath &&
           this->Width == other.Width &&
           this->Height == other.Height;
};

Vec2 operator+(Vec2 vec, Vector2 RaylibVec2){
    return Vec2{vec[0] + RaylibVec2.x, vec[1] + RaylibVec2.y};
}

Vec2 operator-(Vec2 vec, Vector2 RaylibVec2){
    return Vec2{vec[0] - RaylibVec2.x, vec[1] - RaylibVec2.y};
}

bool operator==(Vec2 vec, Vector2 RaylibVec2){
    return vec[0] == RaylibVec2.x && vec[1] == RaylibVec2.y;
}

Vec2 ConvertRayVec2(Vector2 RaylibVec2){
    return Vec2{RaylibVec2.x, RaylibVec2.y};
}

Vec2 NodeAbstract::GetPosition() const {
    return Position;
}

Vec2 NodeAbstract::GetScreenPosition(const Vec2 Camera) const {
    return Position - Camera;
}

void Nodes::AddNode(std::unique_ptr<NodeAbstract> node) {
    //Use free node id if available, otherwise add to the end of the list
    if (!FreeNodeIds.empty()){
        const auto id = FreeNodeIds.back();

        NodeList[id] = std::move(node);
        node->SetId(id);

        FreeNodeIds.pop_back();
        return;
    }

    node->SetId(GetNextNodeId());
    NodeList.push_back(std::move(node));
}

void Nodes::RemoveNode(const uint id) {
    if (static_cast<size_t>(id) >= NodeList.size() || !NodeList[id]) {
        return; // Invalid ID or node already removed
    }
    NodeList[id].reset(); // Remove the node
    FreeNodeIds.push_back(id); // Add the ID to the free list
}

void NodeAbstract::SetId(const uint16_t id) {
    Id = id;
}


uint16_t Nodes::GetNextNodeId() {
    return NextNodeId++;
}
