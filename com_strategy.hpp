#pragma once

#include <deque>
#include <random>

#include "Rectangle.hpp"
#include "vendor/raylib-5.0_linux_amd64/include/raylib.h"
#include "vendor/raylib-cpp-5.0.2/include/raylib-cpp.hpp"

class GameplayScene;

class ComStrategy {
 public:
  virtual char NextMove(raylib::Rectangle&, raylib::Rectangle&) = 0;
};

class RandomComStrategy : public ComStrategy {
 public:
  char NextMove(raylib::Rectangle&, raylib::Rectangle&) override {
    return std::rand() % 2 == 0 ? 'H' : 'L';
  }
};

class ImperfectComStrategy : public ComStrategy {
 public:
  explicit ImperfectComStrategy(int imperfection_rate, int imperfection_frames)
      : pending_moves_{},
        imperfection_rate_{imperfection_rate},
        imperfection_frames_{imperfection_frames} {};
  char NextMove(raylib::Rectangle& player, raylib::Rectangle& ball) override {
    if (!pending_moves_.empty()) {
      char res = pending_moves_.front();
      pending_moves_.pop_front();
      return res;
    }
    if (std::rand() % 100 <= imperfection_rate_) {
      for (int i = 0; i < imperfection_frames_; ++i) {
        char move = std::rand() % 2 == 0 ? 'H' : 'L';
        pending_moves_.push_back(move);
      }
      return 0;
    }
    if (ball.x < player.x + player.width / 2) {
      return 'H';
    } else if (ball.x > player.x + player.width / 2) {
      return 'L';
    }
    return 0;
  }

 private:
  std::deque<char> pending_moves_;
  int imperfection_rate_;
  int imperfection_frames_;
};

class PerfectComStrategy : public ComStrategy {
 public:
  char NextMove(raylib::Rectangle& player, raylib::Rectangle& ball) override {
    if (ball.x < player.x + player.width / 2) {
      return 'H';
    } else if (ball.x > player.x + player.width / 2) {
      return 'L';
    }
    return 0;
  }
};
