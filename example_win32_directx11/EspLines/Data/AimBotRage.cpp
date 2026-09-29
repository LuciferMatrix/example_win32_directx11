#include "Aim.hpp"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>
#include <shared_mutex>
#include <examples/example_win32_directx11/EspLines/Math/Quaternion.hpp>

static std::atomic<bool> g_AimBotRageRunning{ false };
static std::thread g_AimBotRageThread;

void FrameWork::AimBotRage::Start() {
    if (g_AimBotRageRunning) return;
    g_AimBotRageRunning = true;
    g_AimBotRageThread = std::thread([]() {
        Work();
    });
    g_AimBotRageThread.detach();
}

void FrameWork::AimBotRage::Stop() {
    g_AimBotRageRunning = false;
}

bool FrameWork::AimBotRage::IsRunning() {
    return g_AimBotRageRunning;
}

void FrameWork::AimBotRage::Work() {
    static uint32_t s_lastLockedRageTarget = 0;

    auto ClearLockedRageTarget = []() {
        if (s_lastLockedRageTarget != 0) {
            FrameWork::Mem.Write<uint32_t>(s_lastLockedRageTarget + Offsets::LockedAimingCollider, 0);
            s_lastLockedRageTarget = 0;
        }
    };

    while (!g_Options.General.ShutDown) {
        if (!g_AimBotRageRunning) {
            ClearLockedRageTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!g_Options.LegitBot.AimBot.ExtremeEnabled) {
            ClearLockedRageTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        bool isAimKeyPressed = false;
        if (g_Options.LegitBot.AimBot.WhenShooting) {
            isAimKeyPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        }
        else {
            if (g_Options.LegitBot.AimBot.LMouse) {
                isAimKeyPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            }
            else {
                isAimKeyPressed = (GetAsyncKeyState(g_Options.LegitBot.AimBot.AimKey) & 0x8000) != 0;
            }
        }

        if (!isAimKeyPressed) {
            ClearLockedRageTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (Context::WindowWidth == 0 || Context::WindowHeight == 0 || !Context::HasMatrix || Context::LocalPlayerAddress == 0) {
            ClearLockedRageTarget();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        Entity* bestTarget = nullptr;
        float bestMetric = FLT_MAX;
        Vector2 screenCenter(Context::WindowWidth / 2.0f, Context::WindowHeight / 2.0f);

        {
            std::shared_lock<std::shared_mutex> lock(g_Globals.EspConfig.EntitiesMutex);
            for (auto& pair : Context::Entities) {
                Entity& entity = pair.second;

                if (!entity.IsKnown || entity.IsDead || (g_Options.LegitBot.AimBot.IgnoreDowned && (entity.Pose == XPose::Knocked || entity.IsKnocked)))
                    continue;

                if (g_Options.LegitBot.AimBot.IgnoreTrainingBots)
                {
                    if (entity.Name.empty() && entity.IsBot) continue;
                }

                Vector3 targetBone = entity.Head;
                switch (g_Options.LegitBot.AimBot.Hitbox) {
                case 0: targetBone = entity.Head; break;
                case 1: targetBone = entity.Neck; break;
                }

                auto screenPos = W2S::WorldToScreen(Context::ViewMatrix, targetBone, Context::WindowWidth, Context::WindowHeight);
                if (screenPos.X < 0 || screenPos.Y < 0) continue;

                float playerDistance = Vector3::Distance(Context::LocalMainCamera, targetBone);
                if (playerDistance > g_Options.LegitBot.AimBot.DistanceExtreme) continue;

                float x = screenPos.X - screenCenter.X;
                float y = screenPos.Y - screenCenter.Y;
                float crosshairDist = std::sqrt(x * x + y * y);

                if (g_Options.LegitBot.AimBot.UseFOV && crosshairDist > g_Options.LegitBot.AimBot.FOVRange)
                    continue;

                bool better = false;

                if (g_Options.LegitBot.AimBot.MultiComboTargetPriority[0]) {
                    better = crosshairDist < bestMetric;
                    if (better) bestMetric = crosshairDist;
                }

                if (!better && g_Options.LegitBot.AimBot.MultiComboTargetPriority[1]) {
                    better = entity.Health < bestMetric;
                    if (better) bestMetric = entity.Health;
                }

                if (!better && g_Options.LegitBot.AimBot.MultiComboTargetPriority[2]) {
                    better = playerDistance < bestMetric;
                    if (better) bestMetric = playerDistance;
                }
                if (better)
                    bestTarget = &entity;
            }

            if (bestTarget) {
                if (s_lastLockedRageTarget != 0 && s_lastLockedRageTarget != bestTarget->Address) {
                    FrameWork::Mem.Write<uint32_t>(s_lastLockedRageTarget + Offsets::LockedAimingCollider, 0);
                    s_lastLockedRageTarget = 0;
                }

                uint32_t headCollider = 0;
                if (FrameWork::Mem.Read<uint32_t>(bestTarget->Address + Offsets::Collider, headCollider) && headCollider != 0) {
                    uint32_t currentCollider = 0;
                    if (!FrameWork::Mem.Read<uint32_t>(bestTarget->Address + Offsets::LockedAimingCollider, currentCollider) || currentCollider != headCollider) {
                        FrameWork::Mem.Write(bestTarget->Address + Offsets::LockedAimingCollider, headCollider);
                    }
                    s_lastLockedRageTarget = bestTarget->Address;
                }

                Vector3 selectedBone;
                switch (g_Options.LegitBot.AimBot.Hitbox) {
                case 0: selectedBone = bestTarget->Head; break;
                case 1: selectedBone = bestTarget->Neck; break;
                }

                Quaternion newRot = Quaternion::GetRotationToLocation(selectedBone, 0.1f, Context::LocalMainCamera);
                FrameWork::Mem.Write(Context::LocalPlayerAddress + Offsets::AimRotation, newRot);
            }
            else {
                ClearLockedRageTarget();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ClearLockedRageTarget();
}
