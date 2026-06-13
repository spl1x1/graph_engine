#ifndef ENGINE_TYPES_HPP
#define ENGINE_TYPES_HPP

#include <Node.hpp>
#include <string>
#include <sys/types.h>
#include <Vector.hpp>
#include <raylib.h>

Vec2 operator+(Vec2 vec, Vector2 RaylibVec2);
Vec2 operator-(Vec2 vec, Vector2 RaylibVec2);
bool operator==(Vec2 vec, Vector2 RaylibVec2);
Vec2 ConvertRayVec2(Vector2 RaylibVec2);
float GetDistance(Vec2 a, Vec2 b);


struct EnvWindow{
    uint Width;
    uint Height;
    uint FrameRate;
};
struct EnvSandbox{
    const uint Width;
    const uint Height;
};

struct Background{
    std::string TexturePath;
    // Optional width and height for the background texture, if not provided, it will be loaded with its original size
    uint Width{0};
    // Optional width and height for the background texture, if not provided, it will be loaded with its original size
    uint Height{0};

    bool operator==(const Background& other);
};

struct Enviroment{
    Background Background;
    EnvWindow Window;
    EnvSandbox Sandbox;

    std::string Title;
    float DeltaTime;
};

struct EditMode{
    bool Enabled;
    std::string SelectedMode;
    IPAddress SelectedNetworkArea;
    LinkSpeed SelectedSpeed{LinkSpeed::MEDIUM};
};

#define INPUT_TABLE \
        ENTRY(None, (void)0) \
        ENTRY(Button, instance->ProcessButtons()) \
        ENTRY(TextInput, instance->ProcessTextInput()) \
        ENTRY(Node, instance->ProcessNodeClick()) \
        ENTRY(Camera, instance->ProcessCameraMovement()) \
        ENTRY(Edit, instance->ProcessEditInputs()) \
        ENTRY(KeyboardInput, instance->ProcessKeyboard())

struct InputBlock{
    bool Blocked;
    int BlockLoop{0};

    enum class BlockType{
    #define ENTRY(type,func) type,
        INPUT_TABLE
    #undef ENTRY
    } Type{BlockType::None};

    static std::string BlockTypeToString(BlockType type){
        switch (type) {
            #define ENTRY(a, b) case BlockType::a: return #a;
            INPUT_TABLE
            #undef ENTRY
        }
    };
    static BlockType StringToBlockType(const std::string& str){
        #define ENTRY(a, b) if (str == #a) return BlockType::a;
        INPUT_TABLE
        #undef ENTRY
        return BlockType::None;
    };
};

struct SandboxData{
    Vec2 Camera;
    float Zoom;
    EditMode Edit;
    bool TextInputActive{false};
    bool ShowSpeed{false};
};
#endif // ENGINE_TYPES_HPP
