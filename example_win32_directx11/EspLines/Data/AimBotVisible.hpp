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
#include <examples/example_win32_directx11/EspLines/Math/WordToScreen.hpp>

namespace FrameWork {
    class AimBotVisible {
    public:
        static void Work();
        static void Start();
        static void Stop();
        static bool IsRunning();
    };
}
