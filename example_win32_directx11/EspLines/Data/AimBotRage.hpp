#pragma once
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <examples/example_win32_directx11/src/Globals.hpp>
#include <examples/example_win32_directx11/EspLines/Memory/Memory.hpp>
#include <examples/example_win32_directx11/EspLines/Offsets.hpp>
#include <examples/example_win32_directx11/EspLines/Player.h>
#include <examples/example_win32_directx11/EspLines/Math/Vector/Vector2.hpp>
#include <examples/example_win32_directx11/EspLines/Math/Vector/Vector3.hpp>
#include <examples/example_win32_directx11/EspLines/Math/Quaternion.hpp>
#include <examples/example_win32_directx11/EspLines/Math/WordToScreen.hpp>
#include <examples/example_win32_directx11/EspLines/Math/AimB.hpp>

namespace FrameWork {
    inline MemoryUtils& Mem = ::Mem;

    namespace Context {
        inline auto& WindowWidth = g_Globals.EspConfig.Width;
        inline auto& WindowHeight = g_Globals.EspConfig.Height;
        inline auto& HasMatrix = g_Globals.EspConfig.Matrix;
        inline auto& ViewMatrix = g_Globals.EspConfig.ViewMatrix;
        inline auto& Entities = g_Globals.EspConfig.Entities;
        inline auto& LocalMainCamera = g_Globals.EspConfig.MainCamera;
        inline auto& LocalPlayerAddress = g_Globals.EspConfig.LocalPlayer;
    }

    using Entity = Player;

    class AimBotRage {
    public:
        static void Work();
        static void Start();
        static void Stop();
        static bool IsRunning();
    };
}
