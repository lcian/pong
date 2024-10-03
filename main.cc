#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>

#include "./com_strategy.hpp"
#include "./utils.hpp"
#include "vendor/raylib-5.0_linux_amd64/include/raylib.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int FPS = 60;
const int FONT_SIZE = 20;

enum SceneKind {
  SPLASH,
  MENU,
  OPTIONS,
  SCORE,
  GAMEPLAY,
  GAME_OVER,
};

using SceneKind = enum SceneKind;

class Scene;

class ISceneSubscriber {
 public:
  virtual void Update(Scene*) {}
};

class Scene {
 public:
  virtual ~Scene() {}
  virtual void Render() {}
  SceneKind Kind() { return kind_; }
  void Subscribe(ISceneSubscriber* subscriber) {
    subscribers_.push_back(subscriber);
  }
  void notifySubscribers() {
    for (auto& subscriber : subscribers_) {
      subscriber->Update(this);
    }
  }

 protected:
  Scene(SceneKind kind, int frames, raylib::Window* window)
      : kind_{kind}, frames_{frames}, window_{window}, subscribers_{} {}
  SceneKind kind_;
  int frames_;
  raylib::Window* window_;
  std::vector<ISceneSubscriber*> subscribers_;
};

class SplashScene : public Scene {
 public:
  explicit SplashScene(raylib::Window* window)
      : Scene{SceneKind::SPLASH, 0, window}, over_{false} {}
  void Render() override {
    window_->BeginDrawing();
    window_->ClearBackground(BLACK);
    raylib::Rectangle splash{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTextCenter(splash, "PONG", 50, RAYWHITE);
    window_->EndDrawing();
    ++frames_;
    if (frames_ >= 120) {
      over_ = true;
      notifySubscribers();
      return;
    }
  }
  bool Over() { return over_; }

 private:
  bool over_;
};

class MenuScene : public Scene {
 public:
  explicit MenuScene(raylib::Window* window)
      : Scene{SceneKind::MENU, 0, window},
        buttons_{
            Button{100, 150, 600, 80, BLACK, RAYWHITE, 5, "PLAY", true},
            Button{100, 270, 600, 80, BLACK, RAYWHITE, 5, "OPTIONS"},
            Button{100, 390, 600, 80, BLACK, RAYWHITE, 5, "QUIT"},
        },
        selected_button_{0},
        selected_{false} {}
  size_t SelectedButton() { return selected_button_; }
  bool Selected() { return selected_; }
  void Render() override {
    if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_J)) {
      selected_button_ = (selected_button_ + 1) % buttons_.size();
      frames_ = 0;
    } else if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_K)) {
      if (selected_button_ == 0) {
        selected_button_ = buttons_.size() - 1;
      } else {
        selected_button_ -= 1;
      }
      frames_ = 0;
    } else if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_SPACE)) {
      selected_ = true;
      notifySubscribers();
      return;
    }

    window_->BeginDrawing();
    window_->ClearBackground(BLACK);
    raylib::Rectangle title{0, 0, SCREEN_WIDTH, 150};
    title.Draw(BLACK);
    DrawTextCenter(title, "PONG", 30, RAYWHITE);
    for (size_t i = 0; i < buttons_.size(); ++i) {
      buttons_[i].Selected(i == selected_button_);
      buttons_[i].Draw();
    }
    raylib::Rectangle help{0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 80};
    help.Draw(BLACK);
    DrawTextCenter(help, "use H J K L to move, SPACEBAR to select", FONT_SIZE,
                   RAYWHITE);
    window_->EndDrawing();
    ++frames_;
  }

 private:
  std::vector<Button> buttons_;
  size_t selected_button_;
  bool selected_;
};

enum Difficulty { EASY, MEDIUM, HARD, IMPOSSIBLE };
using Difficulty = enum Difficulty;

Difficulty operator++(Difficulty& difficulty) {
  if (difficulty == IMPOSSIBLE) {
    difficulty = EASY;
  } else {
    difficulty = static_cast<Difficulty>((difficulty + 1) % 4);
  }
  return difficulty;
}

Difficulty operator--(Difficulty& difficulty) {
  if (difficulty == EASY) {
    difficulty = IMPOSSIBLE;
  } else {
    difficulty = static_cast<Difficulty>((difficulty - 1) % 4);
  }
  return difficulty;
}

const std::unordered_map<Difficulty, ComStrategy*> DIFFICULTY_TO_STRATEGY{
    {Difficulty::EASY, new RandomComStrategy{}},
    {Difficulty::MEDIUM, new ImperfectComStrategy{10, 10}},
    {Difficulty::HARD, new ImperfectComStrategy{20, 5}},
    {Difficulty::IMPOSSIBLE, new PerfectComStrategy{}},
};

const std::unordered_map<Difficulty, std::string> DIFFICULTY_TO_STRING{
    {Difficulty::EASY, "DIFFICULTY: EASY"},
    {Difficulty::MEDIUM, "DIFFICULTY: MEDIUM"},
    {Difficulty::HARD, "DIFFICULTY: HARD"},
    {Difficulty::IMPOSSIBLE, "DIFFICULTY: IMPOSSIBLE"},
};

class OptionsScene : public Scene {
 public:
  explicit OptionsScene(raylib::Window* window, Difficulty difficulty)
      : Scene{SceneKind::OPTIONS, 0, window},
        difficulty_{difficulty},
        buttons_{Button{100, 150, 600, 80, BLACK, RAYWHITE, 5,
                        DIFFICULTY_TO_STRING.at(difficulty_), true},
                 Button{100, 390, 600, 80, BLACK, RAYWHITE, 5, "BACK"}},
        selected_button_{0},
        over_{false} {}

  void Render() override {
    if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_J)) {
      selected_button_ = (selected_button_ + 1) % buttons_.size();
      frames_ = 0;
    } else if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_K)) {
      if (selected_button_ == 0) {
        selected_button_ = buttons_.size() - 1;
      } else {
        --selected_button_;
      }
      frames_ = 0;
    } else if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_SPACE)) {
      if (selected_button_ == 1) {
        over_ = true;
        frames_ = 0;
        notifySubscribers();
        return;
      }
    } else if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_H)) {
      if (selected_button_ == 0) {
        --difficulty_;
        buttons_[0].Text(DIFFICULTY_TO_STRING.at(difficulty_));
        frames_ = 0;
      }
    } else if (frames_ >= 8 && raylib::Keyboard::IsKeyDown(KEY_L)) {
      if (selected_button_ == 0) {
        ++difficulty_;
        buttons_[0].Text(DIFFICULTY_TO_STRING.at(difficulty_));
        frames_ = 0;
      }
    }
    for (size_t i = 0; i < buttons_.size(); ++i) {
      buttons_[i].Selected(i == selected_button_);
      buttons_[i].Draw();
    }
    window_->BeginDrawing();
    window_->ClearBackground(BLACK);
    window_->EndDrawing();
    ++frames_;
  }
  Difficulty GetDifficulty() { return difficulty_; }
  bool Over() { return over_; }

 private:
  Difficulty difficulty_;
  std::vector<Button> buttons_;
  size_t selected_button_;
  bool over_;
};

class GameplayScene;

class ScoreScene : public Scene {
 public:
  explicit ScoreScene(raylib::Window* window, int p1_score, int p2_score)
      : Scene{SceneKind::SCORE, 0, window},
        p1_score_{p1_score},
        p2_score_{p2_score},
        over_{false} {}
  void Render() override {
    window_->BeginDrawing();
    window_->ClearBackground(BLACK);
    raylib::Rectangle title_rectangle{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    title_rectangle.Draw(BLACK);
    DrawTextCenter(title_rectangle,
                   std::format("{} : {}", p1_score_, p2_score_).c_str(), 50,
                   RAYWHITE);
    window_->EndDrawing();
    ++frames_;
    if (frames_ >= 90) {
      over_ = true;
      notifySubscribers();
    }
  }
  int P1Score() { return p1_score_; }
  int P2Score() { return p2_score_; }
  bool Over() { return over_; }

 private:
  int p1_score_;
  int p2_score_;
  bool over_;
};

class GameplayScene : public Scene {
 public:
  static const int bar_width_{100};
  static const int bar_height_{10};
  static const int ball_width_{10};
  static const int ball_height_{10};
  constexpr static const float vy_inc_{1.1f};
  explicit GameplayScene(raylib::Window* window, int p1_score, int p2_score,
                         ComStrategy* com_strategy)
      : Scene{SceneKind::GAMEPLAY, 0, window},
        p1_score_{p1_score},
        p2_score_{p2_score},
        p1_{
            static_cast<int>(SCREEN_WIDTH / 2 - bar_width_ / 2),
            SCREEN_HEIGHT - bar_height_ * 2,
            bar_width_,
            bar_height_,
        },
        p2_{
            static_cast<int>(SCREEN_WIDTH / 2 - bar_width_ / 2),
            bar_height_,
            bar_width_,
            bar_height_,
        },
        ball_{
            SCREEN_WIDTH / 2.0 - 10 / 2.0,
            SCREEN_HEIGHT / 2.0 - 10 / 2.0,
            10,
            10,
        },
        ball_vx_{0.0},
        ball_vy_{(std::rand() % 2 == 0 ? 1 : -1) * 5.0f},
        com_strategy_{com_strategy},
        over_{false} {}
  void Render() override {
    // handle out of bounds
    if (ball_.y <= 0 || ball_.y >= SCREEN_HEIGHT) {
      if (ball_.y <= 0) {
        ++p1_score_;
      } else if (ball_.y == SCREEN_HEIGHT) {
        ++p2_score_;
      }
      over_ = true;
      notifySubscribers();
      return;
    }

    // handle collisions
    if (p1_.CheckCollision(ball_) || p2_.CheckCollision(ball_)) {
      ball_vy_ = -ball_vy_ * vy_inc_;
      ball_vy_ = std::min(ball_vy_, ball_vy_max_);
      ball_vy_ = std::max(ball_vy_, -ball_vy_max_);
      raylib::Rectangle collision;
      float offset;
      if (p1_.CheckCollision(ball_)) {
        collision = p1_.GetCollision(ball_);
        offset = (p1_.x + p1_.width / 2) - collision.x;
        ball_vx_ = -(offset * 5 / p1_.width);
      } else if (p2_.CheckCollision(ball_)) {
        collision = p2_.GetCollision(ball_);
        offset = (p2_.x + p2_.width / 2) - collision.x;
      }
    } else if (ball_.x == 0 || ball_.x + ball_.width >= SCREEN_WIDTH) {
      ball_vx_ = -ball_vx_;
    }

    // handle input
    if (raylib::Keyboard::IsKeyDown(KEY_H)) {
      p1_.SetX(std::max(p1_.GetX() - 8, .0f));
      frames_ = 0;
    } else if (raylib::Keyboard::IsKeyDown(KEY_L)) {
      p1_.SetX(std::min(p1_.GetX() + 8, SCREEN_WIDTH - p1_.GetWidth()));
      frames_ = 0;
    }

    // COM movement
    char move{com_strategy_->NextMove(p2_, ball_)};
    switch (move) {
      case 'H': {
        p2_.x = std::max(p2_.x - 8, .0f);
        break;
      }
      case 'L': {
        p2_.x = std::min(p2_.x + 8, SCREEN_WIDTH - p2_.width);
        break;
      }
      default:
        break;
    }

    // handle movement
    ball_.x += ball_vx_;
    ball_.x = std::max(ball_.x, .0f);
    ball_.x = std::min(ball_.x, static_cast<float>(SCREEN_WIDTH));
    ball_.y += ball_vy_;
    ball_.y = std::max(ball_.y, .0f);
    ball_.y = std::min(ball_.y, static_cast<float>(SCREEN_HEIGHT));

    // display stuff
    window_->BeginDrawing();
    window_->ClearBackground(BLACK);
    p1_.Draw(RAYWHITE);
    p2_.Draw(RAYWHITE);
    ball_.Draw(RAYWHITE);
    window_->EndDrawing();

    ++frames_;
  }
  int P1Score() { return p1_score_; }
  int P2Score() { return p2_score_; }
  raylib::Rectangle& P1() { return p1_; }
  raylib::Rectangle& P2() { return p2_; }
  raylib::Rectangle& Ball() { return ball_; }
  float BallVx() { return ball_vx_; }
  float BallVy() { return ball_vy_; }
  bool Over() { return over_; }

 private:
  constexpr static const float ball_vy_max_ = 20.0f;
  int p1_score_;
  int p2_score_;
  raylib::Rectangle p1_;
  raylib::Rectangle p2_;
  raylib::Rectangle ball_;
  float ball_vx_;
  float ball_vy_;
  ComStrategy* com_strategy_;
  bool over_;
};

class GameOverScene : public Scene {
 public:
  explicit GameOverScene(raylib::Window* window, std::string winner)
      : Scene{SceneKind::GAME_OVER, 0, window} {
    over_ = false;
    winner_ = std::move(winner);
  }
  void Render() override {
    window_->BeginDrawing();
    window_->ClearBackground(BLACK);
    raylib::Rectangle message_rectangle{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    message_rectangle.Draw(BLACK);
    DrawTextCenter(message_rectangle, std::format("{} WINS!", winner_).c_str(),
                   50, RAYWHITE);
    window_->EndDrawing();
    ++frames_;
    if (frames_ >= 120) {
      over_ = true;
      notifySubscribers();
    }
  }
  bool Over() { return over_; }

 private:
  std::string winner_;
  bool over_;
};

class Game : public ISceneSubscriber {
 public:
  explicit Game(raylib::Window* window)
      : scene_{new SplashScene{window}},
        window_{window},
        difficulty_{Difficulty::MEDIUM} {
    scene_->Subscribe(this);
  }
  void Render() { scene_->Render(); }
  void Update(Scene* scene) override {
    switch (scene->Kind()) {
      case SPLASH: {
        SplashScene* splash_scene = static_cast<SplashScene*>(scene);
        if (!splash_scene->Over()) {
          return;
        }
        delete scene_;
        scene_ = new MenuScene{window_};
        scene_->Subscribe(this);
        break;
      }
      case MENU: {
        MenuScene* menu_scene = static_cast<MenuScene*>(scene);
        if (!menu_scene->Selected()) {
          return;
        }
        size_t i = menu_scene->SelectedButton();
        switch (i) {
          case 0: {
            delete scene_;
            scene_ = new ScoreScene{window_, 0, 0};
            scene_->Subscribe(this);
            break;
          }
          case 1: {
            delete scene_;
            scene_ = new OptionsScene{window_, difficulty_};
            scene_->Subscribe(this);
            break;
          }
          default: {
            window_->Close();
            std::exit(0);
            break;
          }
        }
        break;
      }
      case OPTIONS: {
        OptionsScene* options_scene = static_cast<OptionsScene*>(scene);
        if (!options_scene->Over()) {
          return;
        }
        difficulty_ = options_scene->GetDifficulty();
        delete scene_;
        scene_ = new MenuScene{window_};
        scene_->Subscribe(this);
        break;
      }
      case SCORE: {
        ScoreScene* score_scene = static_cast<ScoreScene*>(scene);
        if (!score_scene->Over()) {
          return;
        }
        delete scene_;
        scene_ = new GameplayScene{window_, score_scene->P1Score(),
                                   score_scene->P2Score(),
                                   DIFFICULTY_TO_STRATEGY.at(difficulty_)};
        scene_->Subscribe(this);
        break;
      }
      case GAMEPLAY: {
        GameplayScene* gameplay_scene = static_cast<GameplayScene*>(scene);
        if (!gameplay_scene->Over()) {
          return;
        }
        delete scene_;
        if (gameplay_scene->P1Score() == max_score_ ||
            gameplay_scene->P2Score() == max_score_) {
          scene_ = new GameOverScene{
              window_,
              gameplay_scene->P1Score() == max_score_ ? "PLAYER 1" : "COM"};
        } else {
          scene_ = new ScoreScene{window_, gameplay_scene->P1Score(),
                                  gameplay_scene->P2Score()};
        }
        scene_->Subscribe(this);
        break;
      }
      case GAME_OVER: {
        GameOverScene* game_over_scene = static_cast<GameOverScene*>(scene);
        if (!game_over_scene->Over()) {
          return;
        }
        delete scene_;
        scene_ = new MenuScene{window_};
        scene_->Subscribe(this);
        break;
      }
      default: {
        assert(false && "unreachable");
        break;
      }
    }
  }

 private:
  static const int max_score_ = 5;
  Scene* scene_;
  raylib::Window* window_;
  Difficulty difficulty_;
};

int main() {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  raylib::Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "PONG");
  SetTargetFPS(FPS);
  Game game{&window};
  while (!window.ShouldClose()) {
    game.Render();
  }
  return 0;
}
