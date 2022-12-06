#include "Clock.h"

INT WINAPI wWinMain(_In_ [[maybe_unused]] HINSTANCE instance,
    _In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
    _In_ [[maybe_unused]] PWSTR cmd_line,
    _In_ [[maybe_unused]] INT cmd_show) {
    Clock clock;

    clock.InitializeWindow(instance, cmd_show);
    clock.RunMessageLoop();

    return 0;
}