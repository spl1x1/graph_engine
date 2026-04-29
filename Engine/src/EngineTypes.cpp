#include <EngineTypes.hpp>


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
