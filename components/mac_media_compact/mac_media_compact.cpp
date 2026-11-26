#include "mac_media_compact.h"
#include <algorithm>
#include <string>

namespace esphome::mac_media_compact {
namespace ui = tessera::ui;
using tessera::Font;

class CompactTile : public tessera::Tile {
 public:
  explicit CompactTile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    entity_ = c.entity;
    const int w = c.width;
    const int h = c.height;
    const int gap = ui::px(10);
    cover_side_ = std::max(38, std::min(h - ui::px(12), w / 3));
    cover_side_ = std::min(cover_side_, std::max(32, w / 2 - gap));
    right_x_ = cover_side_ + gap;
    right_w_ = std::max(1, w - right_x_ - ui::px(4));
    controls_y_ = std::max(0, h - ui::line_height(Font::BODY_LARGE) - ui::px(5));

    if (ui::pictures()) {
      cover_ = ui::picture(c.parent);
      lv_obj_set_size(cover_, cover_side_, cover_side_);
      lv_obj_set_pos(cover_, 0, (h - cover_side_) / 2);
      ui::set_picture(cover_, std::string("cover:") + entity_, tessera::Fit::COVER);
    }

    source_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    title_ = ui::label(c.parent, Font::TITLE, theme::INK);
    artist_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    keys_ = ui::label(c.parent, Font::BODY_LARGE, theme::INK);
    for (lv_obj_t *p : {source_, title_, artist_, keys_}) {
      lv_obj_set_width(p, right_w_);
      lv_obj_set_style_text_align(p, LV_TEXT_ALIGN_LEFT, 0);
    }
    lv_obj_set_pos(source_, right_x_, ui::px(2));
    lv_obj_set_height(source_, ui::line_height(Font::BODY));
    lv_obj_set_pos(title_, right_x_, ui::line_height(Font::BODY) + ui::px(3));
    lv_obj_set_height(title_, ui::line_height(Font::TITLE));
    lv_label_set_long_mode(title_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(artist_, right_x_, ui::line_height(Font::BODY) + ui::line_height(Font::TITLE) + ui::px(4));
    lv_obj_set_height(artist_, ui::line_height(Font::BODY));
    lv_label_set_long_mode(artist_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(keys_, right_x_, controls_y_);
    lv_obj_set_height(keys_, ui::line_height(Font::BODY_LARGE));
    ui::set_text(keys_, "<       II       >");
  }

  void on_state(JsonObjectConst data) override {
    std::string state = data["state"] | "";
    playing_ = state == "playing";
    std::string name = data["name"] | "MacBook Air";
    std::string title = data["attributes"]["media_title"] | "";
    std::string artist = data["attributes"]["media_artist"] | "";
    std::string app = data["attributes"]["app_name"] | "";
    if (app.empty()) app = name;
    if (title.empty()) title = state == "unavailable" ? "Lecteur indisponible" : "Aucune lecture";
    ui::set_text(source_, app);
    ui::set_text(title_, title);
    ui::set_text(artist_, artist);
    ui::set_text(keys_, playing_ ? "<       II       >" : "<        >        >");
    if (cover_ && !entity_.empty())
      ui::set_picture(cover_, std::string("cover:") + entity_, tessera::Fit::COVER);
  }

  void on_theme() override {
    ui::set_color(source_, theme::MUTED);
    ui::set_color(title_, theme::INK);
    ui::set_color(artist_, theme::MUTED);
    ui::set_color(keys_, theme::INK);
  }

  void on_tap() override {
    if (entity_.empty()) return;
    const tessera::Point p = tessera::tap_point();
    if (p.x < right_x_ || p.x >= right_x_ + right_w_ || p.y < controls_y_ - ui::px(8)) return;
    const int segment = (p.x - right_x_) * 3 / right_w_;
    switch (segment) {
      case 0: tessera::action("media_player.media_previous_track", entity_); break;
      case 1: tessera::action("media_player.media_play_pause", entity_); break;
      default: tessera::action("media_player.media_next_track", entity_); break;
    }
  }

 private:
  const tessera::Plugin *plugin_;
  std::string entity_;
  lv_obj_t *cover_ = nullptr, *source_ = nullptr, *title_ = nullptr, *artist_ = nullptr, *keys_ = nullptr;
  int cover_side_ = 0, right_x_ = 0, right_w_ = 1, controls_y_ = 0;
  bool playing_ = false;
};

void MacMediaCompact::setup() {
  add_tile("compact", [this]() { return new CompactTile(this); });
}
}  // namespace esphome::mac_media_compact
