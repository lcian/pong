#pragma once

#include <string>

#include "vendor/raylib-5.0_linux_amd64/include/raylib.h"
#include "vendor/raylib-cpp-5.0.2/include/raylib-cpp.hpp"

void DrawTextCenter(const raylib::Rectangle& rectangle, const char* text,
                    int font_size, Color color = RAYWHITE) {
  DrawText(text,
           static_cast<int>(rectangle.GetX()) +
               static_cast<int>(rectangle.GetWidth()) / 2 -
               raylib::MeasureText(text, font_size) / 2,
           static_cast<int>(rectangle.GetY()) +
               static_cast<int>(rectangle.GetHeight()) / 2 - font_size / 2,
           font_size, color);
}

class Button {
 public:
  Button(float x, float y, float width, float height, Color primary_color,
         Color secondary_color, float border_width, std::string text,
         bool selected = false, int font_size = 20) {
    inner_rect_ =
        raylib::Rectangle(x + border_width, y + border_width,
                          width - border_width * 2, height - border_width * 2);
    outer_rect_ = raylib::Rectangle(x, y, width, height);
    text_ = text;
    primary_color_ = primary_color;
    secondary_color_ = secondary_color;
    font_size_ = font_size;
    selected_ = selected;
  }
  void Draw() {
    outer_rect_.Draw(secondary_color_);
    inner_rect_.Draw((selected_) ? secondary_color_ : primary_color_);
    DrawTextCenter(inner_rect_, text_.c_str(), font_size_,
                   (selected_) ? primary_color_ : secondary_color_);
  }
  raylib::Rectangle Rect() { return inner_rect_; }
  void Selected(bool selected) { selected_ = selected; }
  void Text(std::string text) { text_ = text; }

 private:
  std::string text_;
  raylib::Rectangle outer_rect_;
  raylib::Rectangle inner_rect_;
  Color primary_color_;
  Color secondary_color_;
  int font_size_;
  bool selected_;
};
