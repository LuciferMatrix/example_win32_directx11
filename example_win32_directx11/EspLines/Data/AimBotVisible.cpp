#include "AimBotVisible.hpp"
#include "Aim.hpp"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>
#include <shared_mutex>

static std::atomic<bool> g_AimBotVisibleRunning{ false };
static std::thread g_AimBotVisibleThread;

void FrameWork::AimBotVisible::Start() {
    if (g_AimBotVisibleRunning) return;
    g_AimBotVisibleRunning = true;
    g_AimBotVisibleThread = std::thread([]() {
        Work();
    });
    g_AimBotVisibleThread.detach();
}

void FrameWork::AimBotVisible::Stop() {
    g_AimBotVisibleRunning = false;
}

bool FrameWork::AimBotVisible::IsRunning() {
    return g_AimBotVisibleRunning;
}

void FrameWork::AimBotVisible::Work() {
    static uint32_t s_lastLockedTarget = 0;

    auto ClearLockedTarget = []() {
        if (s_lastLockedTarget != 0) {
            FrameWork::Mem.Write<uint32_t>(s_lastLockedTarget + Offsets::ReplaceCollider, 0);
            s_lastLockedTarget = 0;
        }
    };

    while (!g_Options.General.ShutDown) {
        if (!g_AimBotVisibleRunning) {
            ClearLockedTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!g_Options.LegitBot.AimBot.VisibleEnabled) {
            ClearLockedTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        bool isAimKeyPressed = false;
        if (g_Options.LegitBot.AimBot.WhenShooting) {
            isAimKeyPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        }
        else {
            isAimKeyPressed = (GetAsyncKeyState(g_Options.LegitBot.AimBot.AimKey) & 0x8000) != 0;
        }

        if (!isAimKeyPressed) {
            ClearLockedTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (Context::WindowWidth == 0 || Context::WindowHeight == 0 || !Context::HasMatrix) {
            ClearLockedTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        Entity* target = nullptr;
        float closestDistance = FLT_MAX;
        Vector2 screenCenter(Context::WindowWidth / 2.0f, Context::WindowHeight / 2.0f);

        {
            std::shared_lock<std::shared_mutex> lock(g_Globals.EspConfig.EntitiesMutex);
            for (auto& pair : Context::Entities) {
                Entity& entity = pair.second;

                if (!entity.IsKnown || entity.IsDead || (g_Options.LegitBot.AimBot.IgnoreDowned && (entity.Pose == XPose::Knocked || entity.IsKnocked)))
                    continue;

                auto Head = W2S::WorldToScreen(Context::ViewMatrix, entity.Head, Context::WindowWidth, Context::WindowHeight);
                if (Head.X < 0 || Head.Y < 0) continue;

                float playerDistance = Vector3::Distance(Context::LocalMainCamera, entity.Head);
                if (playerDistance > g_Options.LegitBot.AimBot.DistanceVisible) continue;

                float x = Head.X - screenCenter.X;
                float y = Head.Y - screenCenter.Y;
                float crosshairDist = std::sqrt(x * x + y * y);

                if (crosshairDist < closestDistance && crosshairDist < g_Options.LegitBot.AimBot.FOVRange) {
                    closestDistance = crosshairDist;
                    target = &entity;
                }
            }

            if (target != nullptr) {
                if (s_lastLockedTarget != 0 && s_lastLockedTarget != target->Address) {
                    FrameWork::Mem.Write<uint32_t>(s_lastLockedTarget + Offsets::ReplaceCollider, 0);
                    s_lastLockedTarget = 0;
                }

                uint32_t m_HeadCollider = 0;
                bool readSuccess = FrameWork::Mem.Read<uint32_t>(target->Address + Offsets::HeadCollider, m_HeadCollider);
                if (readSuccess && m_HeadCollider != 0) {
                    uint32_t currentCollider = 0;
                    if (!FrameWork::Mem.Read<uint32_t>(target->Address + Offsets::ReplaceCollider, currentCollider) || currentCollider != m_HeadCollider) {
                        FrameWork::Mem.Write(target->Address + Offsets::ReplaceCollider, m_HeadCollider);
                    }
                    s_lastLockedTarget = target->Address;
                }
            }
            else {
                ClearLockedTarget();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ClearLockedTarget();
}
