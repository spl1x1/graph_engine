#include <Engine.hpp>
#include <EngineTypes.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include "Node.hpp"
#include "Router.hpp"
#include <Button.hpp>
#include <Widget.hpp>

int main(int argc, char *argv[]) {
    constexpr int windowWidth = 860;
    constexpr int windowHeight = 640;

    constexpr int sandboxWidth = 2000;
    constexpr int sandboxHeight = 2000;

    constexpr float sandboxCenterX = sandboxWidth / 2.f - windowWidth / 2.f;
    constexpr float sandboxCenterY = sandboxHeight / 2.f - windowHeight / 2.f;

    Enviroment env = {
        .Bg = {"assets/nebula_background.png"},
        .Window = {windowWidth, windowHeight, 60},
        .Sandbox = {sandboxWidth, sandboxHeight},
        .Title = "Graph Engine",
    };

    SandboxData sandboxData = {
        .Camera = Vec2{sandboxCenterX, sandboxCenterY},
        .Zoom = 1.0
    };

    Engine::Init(&env, &sandboxData);
    Engine::RegisterNodeType("Router", [](Vec2 position) {
        return std::make_unique<Router>(position);
    });

    std::string saveFile{argc > 1 ? argv[1] : ""};
    Engine::InitSave(saveFile);

    // Register Sync LSDB button – floods LSAs to all neighbors of the selected router
    WidgetData syncWidgetData{
        .PosX = 170.0f,
        .PosY = 130.0f,
        .Width = 120.0f,
        .Height = 20.0f,
        .Border = {0.0f, WHITE}
    };
    ButtonData syncButtonData{
        .HoverColor = LIGHTGRAY,
        .Text = "Sync LSDB",
        .OnClick = [](){
            bool anyUpdates = true;
            while (anyUpdates) {
                anyUpdates = false;
                for (INode* node : Engine::GetAllNodes()) {
                    if (auto* router = dynamic_cast<Router*>(node)) {
                        if (router->SyncWithNetwork() > 0) anyUpdates = true;
                    }
                }
            }
        }
    };
    Widget::Register("SyncButton", std::make_unique<Button>(syncWidgetData, syncButtonData));

    WidgetData sendWidgetData{
        .PosX = 300.0f,
        .PosY = 130.0f,
        .Width = 140.0f,
        .Height = 20.0f,
        .Border = {0.0f, WHITE}
    };
    ButtonData sendButtonData{
        .HoverColor = LIGHTGRAY,
        .Text = "Send Message",
        .OnClick = [&sandboxData](){
            sandboxData.Edit.SelectedMode = "SEND_MESSAGE";
            std::cout << "📨 Message mode enabled. Click source router, then destination router.\n";
        }
    };
    Widget::Register("SendMessageButton", std::make_unique<Button>(sendWidgetData, sendButtonData));
    Widget::AddToGroup("EditButtons", "SendMessageButton");

    struct InFlightMessage {
        std::vector<uint16_t> path;
        std::string content;
        size_t hopIndex{0};
        float nextHopDelay{0.0f};
        bool active{false};
    } inFlight;

    auto GetRouterById = [](uint16_t id) -> Router* {
        for (INode* node : Engine::GetAllNodes()) {
            if (node && node->GetData().Id == id) {
                return dynamic_cast<Router*>(node);
            }
        }
        return nullptr;
    };

    Engine::RegisterUpdateFunction([&]() {
        if (!inFlight.active) {
            auto selection = Engine::ConsumeMessageSelection();
            if (!selection.has_value()) return;

            const uint16_t sourceId = selection->NodeA;
            const uint16_t destinationId = selection->NodeB;
            Router* sourceRouter = GetRouterById(sourceId);
            Router* destinationRouter = GetRouterById(destinationId);
            if (sourceRouter == nullptr || destinationRouter == nullptr) {
                std::cout << "❌ Source or destination router does not exist.\n";
                return;
            }

            bool anyUpdates = true;
            while (anyUpdates) {
                anyUpdates = false;
                for (INode* node : Engine::GetAllNodes()) {
                    if (auto* router = dynamic_cast<Router*>(node)) {
                        if (router->SyncWithNetwork() > 0) anyUpdates = true;
                    }
                }
            }

            const auto path = sourceRouter->GetLSDB().GetShortestPath(sourceId, destinationId);
            if (path.size() < 2) {
                std::cout << "❌ No route from Router " << sourceId << " to Router " << destinationId << ".\n";
                return;
            }

            std::cout << "Enter message from Router " << sourceId << " to Router " << destinationId << ": ";
            std::string content;
            std::getline(std::cin >> std::ws, content);
            if (content.empty()) {
                std::cout << "❌ Empty message ignored.\n";
                return;
            }

            inFlight.path = path;
            inFlight.content = content;
            inFlight.hopIndex = 0;
            inFlight.nextHopDelay = 0.0f;
            inFlight.active = true;
            sourceRouter->GetData().FlashTimer = 0.25f;
            std::cout << "🚀 Message queued on path: ";
            for (auto id : path) std::cout << id << " ";
            std::cout << "\n";
            return;
        }

        inFlight.nextHopDelay -= env.DeltaTime;
        if (inFlight.nextHopDelay > 0.0f) return;
        if (inFlight.hopIndex + 1 >= inFlight.path.size()) {
            inFlight.active = false;
            return;
        }

        const uint16_t fromId = inFlight.path[inFlight.hopIndex];
        const uint16_t toId = inFlight.path[inFlight.hopIndex + 1];
        Router* fromRouter = GetRouterById(fromId);
        Router* toRouter = GetRouterById(toId);
        if (fromRouter == nullptr || toRouter == nullptr) {
            std::cout << "❌ Message aborted: missing router on path.\n";
            inFlight.active = false;
            return;
        }

        Message msg{
            .SenderAddress = fromRouter->GetData().Address,
            .ReceiverAddress = toRouter->GetData().Address,
            .DestinationId = inFlight.path.back(),
            .OriginId = inFlight.path.front(),
            .Content = inFlight.content
        };
        fromRouter->SendMessage(msg);
        Engine::FlashEdgeBetween(fromId, toId, 0.45f);

        if (!toRouter->GetData().MessageQueue.empty()) {
            toRouter->GetData().currentMessage = toRouter->GetData().MessageQueue.front();
            toRouter->GetData().MessageQueue.pop();
        }

        inFlight.hopIndex++;
        inFlight.nextHopDelay = 0.55f;

        if (inFlight.hopIndex == inFlight.path.size() - 1) {
            auto& destinationData = toRouter->GetData();
            destinationData.LastDeliveredMessage = inFlight.content;
            destinationData.DeliveredMessageTimer = 6.0f;
            destinationData.FlashTimer = 0.4f;
            std::cout << "✅ Message arrived at Router " << toId << ": " << inFlight.content << "\n";
            inFlight.active = false;
        }
    });

    Engine::Loop();
    return 0;
}
