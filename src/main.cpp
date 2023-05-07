#include "chip8.h"
#include "platform.h"
#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
    std::exit(EXIT_FAILURE);
  }

  int videoScale = std::stoi(argv[1]);
  int cycleDelay = std::stoi(argv[2]);
  char const *romFileName = argv[3];

  Platform platform("Chip-8 Emulator", VIDEO_WIDTH * videoScale,
                    VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

  Chip8 emulator;
  emulator.load(romFileName);

  int videoPitch = sizeof(emulator.video[0]) * VIDEO_WIDTH;
  auto lastCycleTime = std::chrono::high_resolution_clock::now();
  bool quit = false;

  while (!quit) {
    quit = platform.ProcessInput(emulator.keypad);

    auto currentTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(
                   currentTime - lastCycleTime)
                   .count();

    if (dt > cycleDelay) {
      lastCycleTime = currentTime;
      emulator.cycle();

      platform.Update(emulator.video, videoPitch);
    }
  }
  return 0;
}
