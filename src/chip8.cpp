#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "chip8.h"

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const unsigned int START_ADDRESS = 0x200;

Chip8::Chip8() {
  pc = START_ADDRESS;

  for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    memory[FONTSET_START_ADDRESS + i] = fontset[i];

  randByte = std::uniform_int_distribution<uint8_t>(0, 255u);

  table[0x0] = &Chip8::Table0;
  table[0x1] = &Chip8::OP_1nnn;
  table[0x2] = &Chip8::OP_2nnn;
  table[0x3] = &Chip8::OP_3xkk;
  table[0x4] = &Chip8::OP_4xkk;
  table[0x5] = &Chip8::OP_5xy0;
  table[0x6] = &Chip8::OP_6xkk;
  table[0x7] = &Chip8::OP_7xkk;
  table[0x8] = &Chip8::Table8;
  table[0x9] = &Chip8::OP_9xy0;
  table[0xA] = &Chip8::OP_Annn;
  table[0xB] = &Chip8::OP_Bnnn;
  table[0xC] = &Chip8::OP_Cxkk;
  table[0xD] = &Chip8::OP_Dxyn;
  table[0xE] = &Chip8::TableE;
  table[0xF] = &Chip8::TableF;

  for (size_t i = 0; i <= 0xE; i++) {
    table0[i] = &Chip8::OP_NULL;
    table8[i] = &Chip8::OP_NULL;
    tableE[i] = &Chip8::OP_NULL;
  }

  table0[0x0] = &Chip8::OP_00E0;
  table0[0xE] = &Chip8::OP_00EE;

  table8[0x0] = &Chip8::OP_8xy0;
  table8[0x1] = &Chip8::OP_8xy1;
  table8[0x2] = &Chip8::OP_8xy2;
  table8[0x3] = &Chip8::OP_8xy3;
  table8[0x4] = &Chip8::OP_8xy4;
  table8[0x5] = &Chip8::OP_8xy5;
  table8[0x6] = &Chip8::OP_8xy6;
  table8[0x7] = &Chip8::OP_8xy7;
  table8[0xE] = &Chip8::OP_8xyE;

  tableE[0x1] = &Chip8::OP_ExA1;
  tableE[0xE] = &Chip8::OP_Ex9E;

  for (size_t i = 0; i <= 0x65; i++) {
    tableF[i] = &Chip8::OP_NULL;
  }

  tableF[0x07] = &Chip8::OP_Fx07;
  tableF[0x0A] = &Chip8::OP_Fx0A;
  tableF[0x15] = &Chip8::OP_Fx15;
  tableF[0x18] = &Chip8::OP_Fx18;
  tableF[0x1E] = &Chip8::OP_Fx1E;
  tableF[0x29] = &Chip8::OP_Fx29;
  tableF[0x33] = &Chip8::OP_Fx33;
  tableF[0x55] = &Chip8::OP_Fx55;
  tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::reset() {}

bool Chip8::load(const char *file_path) {
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    std::streampos size = file.tellg();
    char *buffer = new char[size];

    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    for (long i = 0; i < size; ++i) {
      memory[START_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;
  }

  return true;
}

void Chip8::cycle() {
  opcode = (memory[pc] << 8u) | memory[pc + 1];
  pc += 2;

  ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

  if (delayTimer > 0)
    --delayTimer;
  if (soundTimer > 0)
    --soundTimer;
}

void Chip8::Table0() { ((*this).*(table0[opcode & 0x000Fu]))(); }

void Chip8::Table8() { ((*this).*(table8[opcode & 0x000Fu]))(); }

void Chip8::TableE() { ((*this).*(tableE[opcode & 0x000Fu]))(); }

void Chip8::TableF() { ((*this).*(tableF[opcode & 0x00FFu]))(); }

void Chip8::OP_NULL() {}

void Chip8::OP_00E0() { memset(video, 0, sizeof(video)); }
void Chip8::OP_00EE() {
  --sp;
  pc = stack[sp];
}
void Chip8::OP_1nnn() {
  uint16_t nnn = opcode & 0x0FFFu;
  pc = nnn;
}
void Chip8::OP_2nnn() {
  uint16_t nnn = opcode & 0x0FFFu;
  stack[sp] = pc;
  ++sp;
  pc = nnn;
}
void Chip8::OP_3xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = opcode & 0x00FFu;

  if (registers[x] == kk)
    pc += 2;
}
void Chip8::OP_4xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = opcode & 0x00FFu;

  if (registers[x] != kk)
    pc += 2;
}
void Chip8::OP_5xy0() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  if (registers[x] == registers[y])
    pc += 2;
}
void Chip8::OP_6xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = opcode & 0x00FFu;

  registers[x] = kk;
}
void Chip8::OP_7xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = opcode & 0x00FFu;

  registers[x] += kk;
}
void Chip8::OP_8xy0() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  registers[x] = registers[y];
}
void Chip8::OP_8xy1() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  registers[x] |= registers[y];
}
void Chip8::OP_8xy2() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  registers[x] &= registers[y];
}
void Chip8::OP_8xy3() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  registers[x] ^= registers[y];
}
void Chip8::OP_8xy4() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  uint16_t sum = registers[x] + registers[y];
  if (sum > 255u)
    registers[0xF] = 1;
  else
    registers[0xF] = 0;

  registers[x] = sum & 0xFFu;
}
void Chip8::OP_8xy5() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  if (registers[x] > registers[y])
    registers[0xF] = 1;
  else
    registers[0xF] = 0;

  registers[x] -= registers[y];
}
void Chip8::OP_8xy6() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  registers[0xF] = (registers[x] & 0x1u);

  registers[x] >>= 1;
}
void Chip8::OP_8xy7() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  if (registers[y] > registers[x])
    registers[0xF] = 1;
  else
    registers[0xF] = 0;

  registers[x] = registers[y] - registers[x];
}
void Chip8::OP_8xyE() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  registers[0xF] = (registers[x] & 0x80u);

  registers[x] <<= 1;
}
void Chip8::OP_9xy0() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  if (registers[x] != registers[y])
    pc += 2;
}
void Chip8::OP_Annn() {
  uint16_t nnn = opcode & 0x0FFFu;
  index = nnn;
}
void Chip8::OP_Bnnn() {
  uint16_t nnn = opcode & 0x0FFFu;
  pc = nnn + registers[0];
}
void Chip8::OP_Cxkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = opcode & 0x00FFu;

  registers[x] = randByte(randGen) & kk;
}
void Chip8::OP_Dxyn() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  uint8_t n = opcode & 0x000Fu;

  uint8_t xPos = registers[x] % VIDEO_WIDTH;
  uint8_t yPos = registers[y] % VIDEO_HEIGHT;

  registers[0xF] = 0;

  for (unsigned int row = 0; row < n; ++row) {
    uint8_t spriteByte = memory[index + row];
    for (unsigned int col = 0; col < 8; ++col) {
      uint8_t spritePixel = spriteByte & (0x80u >> col);
      uint32_t *screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];
      if (spritePixel) {
        if (*screenPixel == 0xFFFFFFFF)
          registers[0xF] = 1;
        *screenPixel ^= 0xFFFFFFFF;
      }
    }
  }
}
void Chip8::OP_Ex9E() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t key = registers[x];
  if (keypad[key])
    pc += 2;
}
void Chip8::OP_ExA1() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t key = registers[x];
  if (!keypad[key])
    pc += 2;
}
void Chip8::OP_Fx07() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  registers[x] = delayTimer;
}
void Chip8::OP_Fx0A() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  if (keypad[0])
    registers[x] = 0;
  else if (keypad[1])
    registers[x] = 1;
  else if (keypad[2])
    registers[x] = 2;
  else if (keypad[3])
    registers[x] = 3;
  else if (keypad[4])
    registers[x] = 4;
  else if (keypad[5])
    registers[x] = 5;
  else if (keypad[6])
    registers[x] = 6;
  else if (keypad[7])
    registers[x] = 7;
  else if (keypad[8])
    registers[x] = 8;
  else if (keypad[9])
    registers[x] = 9;
  else if (keypad[10])
    registers[x] = 10;
  else if (keypad[11])
    registers[x] = 11;
  else if (keypad[12])
    registers[x] = 12;
  else if (keypad[13])
    registers[x] = 13;
  else if (keypad[14])
    registers[x] = 14;
  else if (keypad[15])
    registers[x] = 15;
  else
    pc -= 2;
}
void Chip8::OP_Fx15() {
  uint8_t x = (opcode & 0x0F00) >> 8u;
  delayTimer = registers[x];
}
void Chip8::OP_Fx18() {
  uint8_t x = (opcode & 0x0F00) >> 8u;
  soundTimer = registers[x];
}
void Chip8::OP_Fx1E() {
  uint8_t x = (opcode & 0x0F00) >> 8u;
  index += registers[x];
}
void Chip8::OP_Fx29() {
  uint8_t x = (opcode & 0x0F00) >> 8u;
  uint8_t digit = registers[x];

  index = FONTSET_START_ADDRESS + (5 * digit);
}
void Chip8::OP_Fx33() {
  uint8_t x = (opcode & 0x0F00) >> 8u;
  uint8_t value = registers[x];

  memory[index + 2] = value % 10;
  value /= 10;
  memory[index + 1] = value % 10;
  value /= 10;
  memory[index] = value % 10;
}
void Chip8::OP_Fx55() {
  uint8_t x = (opcode & 0x0F00) >> 8u;

  for (unsigned int reg = 0; reg <= x; ++reg) {
    memory[index + reg] = registers[reg];
  }
}
void Chip8::OP_Fx65() {
  uint8_t x = (opcode & 0x0F00) >> 8u;

  for (unsigned int reg = 0; reg <= x; ++reg) {
    registers[reg] = memory[index + reg];
  }
}
