#include "mac_media_compact.h"
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <string>

namespace esphome::mac_media_compact {
namespace ui = tessera::ui;
using tessera::Font;

namespace {

// Tessera accepts `cover:<entity>` without query parameters.
std::string cover_address(const std::string &entity, JsonObjectConst data) {
  (void) data;
  return "cover:" + entity;
}

void set_playback_action(const std::string &entity, int x, int width, int tap_x) {
  if (entity.empty() || width <= 0) return;
  const int segment = std::max(0, std::min(2, (tap_x - x) * 3 / width));
  switch (segment) {
    case 0: tessera::action("media_player.media_previous_track", entity); break;
    case 1: tessera::action("media_player.media_play_pause", entity); break;
    default: tessera::action("media_player.media_next_track", entity); break;
  }
}

void set_playback_text(lv_obj_t *label, bool playing) {
  ui::set_text(label, playing ? "<       II       >" : "<       >       >");
}

}  // namespace

class CompactTile : public tessera::Tile {
 public:
  explicit CompactTile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    entity_ = c.entity;
    tile_ = c.tile;
    const int w = c.width;
    const int h = c.height;
    const int inset = ui::px(8);
    const int gap = ui::px(10);
    cover_side_ = std::max(32, std::min(h - 2 * inset, (w - 2 * inset - gap) * 2 / 5));
    right_x_ = inset + cover_side_ + gap;
    right_w_ = std::max(1, w - right_x_ - inset);
    const int source_h = ui::line_height(Font::BODY);
    const int title_h = ui::line_height(Font::TITLE);
    const int artist_h = ui::line_height(Font::BODY);
    const int keys_h = ui::line_height(Font::BODY_LARGE);
    const int group_h = source_h + title_h + artist_h + keys_h + ui::px(10);
    const int group_y = std::max(0, (h - group_h) / 2);
    controls_y_ = group_y + source_h + title_h + artist_h + ui::px(10);

    if (ui::pictures()) {
      cover_ = ui::picture(c.parent);
      lv_obj_set_size(cover_, cover_side_, cover_side_);
      cover_y_ = (h - cover_side_) / 2;
      lv_obj_set_pos(cover_, inset, cover_y_);
    }

    source_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    title_ = ui::label(c.parent, Font::TITLE, theme::INK);
    artist_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    keys_ = ui::label(c.parent, Font::BODY_LARGE, theme::INK);
    for (lv_obj_t *p : {source_, title_, artist_, keys_}) {
      lv_obj_set_width(p, right_w_);
      lv_obj_set_style_text_align(p, LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_obj_set_pos(source_, right_x_, group_y);
    lv_obj_set_height(source_, source_h);
    lv_obj_set_pos(title_, right_x_, group_y + source_h);
    lv_obj_set_height(title_, title_h);
    lv_label_set_long_mode(title_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(artist_, right_x_, group_y + source_h + title_h);
    lv_obj_set_height(artist_, artist_h);
    lv_label_set_long_mode(artist_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(keys_, right_x_, controls_y_);
    lv_obj_set_height(keys_, keys_h);
    set_playback_text(keys_, false);
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
    set_playback_text(keys_, playing_);
    if (cover_ && !entity_.empty())
      ui::set_picture(cover_, cover_address(entity_, data), tessera::Fit::COVER);
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
    if (p.x >= ui::px(8) && p.x < ui::px(8) + cover_side_ && p.y >= cover_y_ && p.y < cover_y_ + cover_side_) {
      tessera::open_card(plugin_->plugin_id(), "cover", entity_, tile_, "Lecture en cours");
      return;
    }
    if (p.x >= right_x_ && p.x < right_x_ + right_w_ && p.y >= controls_y_ - ui::px(8))
      set_playback_action(entity_, right_x_, right_w_, p.x);
  }

 private:
  const tessera::Plugin *plugin_;
  std::string entity_;
  lv_obj_t *cover_ = nullptr, *source_ = nullptr, *title_ = nullptr, *artist_ = nullptr, *keys_ = nullptr;
  int cover_side_ = 0, cover_y_ = 0, right_x_ = 0, right_w_ = 1, controls_y_ = 0, tile_ = -1;
  bool playing_ = false;
};

class CoverCard : public tessera::Card {
 public:
  void open(const tessera::CardContext &c) override {
    entity_ = c.entity;
    width_ = c.width;
    height_ = c.height;
    const int inset = ui::px(12);
    const int controls_h = ui::line_height(Font::BODY_LARGE);
    const int title_h = ui::line_height(Font::TITLE);
    const int artist_h = ui::line_height(Font::BODY);
    const int text_h = title_h + artist_h + controls_h + ui::px(16);
    cover_side_ = std::max(48, std::min(width_ - 2 * inset, height_ - text_h - 2 * inset));
    const int cover_x = std::max(0, (width_ - cover_side_) / 2);
    cover_ = ui::picture(c.parent);
    lv_obj_set_size(cover_, cover_side_, cover_side_);
    lv_obj_set_pos(cover_, cover_x, inset);
    title_ = ui::label(c.parent, Font::TITLE, theme::INK);
    artist_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    keys_ = ui::label(c.parent, Font::BODY_LARGE, theme::INK);
    controls_y_ = inset + cover_side_ + ui::px(8) + title_h + artist_h + ui::px(8);
    for (lv_obj_t *p : {title_, artist_, keys_}) {
      lv_obj_set_width(p, std::max(1, width_ - 2 * inset));
      lv_obj_set_style_text_align(p, LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_obj_set_pos(title_, inset, inset + cover_side_ + ui::px(8));
    lv_obj_set_height(title_, title_h);
    lv_label_set_long_mode(title_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(artist_, inset, inset + cover_side_ + ui::px(8) + title_h);
    lv_obj_set_height(artist_, artist_h);
    lv_label_set_long_mode(artist_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(keys_, inset, controls_y_);
    lv_obj_set_height(keys_, controls_h);
    set_playback_text(keys_, false);
  }

  void on_state(JsonObjectConst data) override {
    const std::string state = data["state"] | "";
    const std::string title = data["attributes"]["media_title"] | "";
    const std::string artist = data["attributes"]["media_artist"] | "";
    ui::set_text(title_, title.empty() ? (state == "unavailable" ? "Lecteur indisponible" : "Aucune lecture") : title);
    ui::set_text(artist_, artist);
    set_playback_text(keys_, state == "playing");
    if (cover_ && !entity_.empty()) ui::set_picture(cover_, cover_address(entity_, data), tessera::Fit::COVER);
  }

  void on_theme() override {
    ui::set_color(title_, theme::INK);
    ui::set_color(artist_, theme::MUTED);
    ui::set_color(keys_, theme::INK);
  }

  void on_tap() override {
    const tessera::Point p = tessera::tap_point();
    const int inset = ui::px(12);
    if (p.x >= inset && p.x < width_ - inset && p.y >= controls_y_ - ui::px(8))
      set_playback_action(entity_, inset, width_ - 2 * inset, p.x);
  }

 private:
  std::string entity_;
  lv_obj_t *cover_ = nullptr, *title_ = nullptr, *artist_ = nullptr, *keys_ = nullptr;
  int width_ = 0, height_ = 0, cover_side_ = 0, controls_y_ = 0;
};

void MacMediaCompact::setup() {
  add_tile("compact", [this]() { return new CompactTile(this); });
  add_card("cover", []() { return new CoverCard(); }, true);
}
}  // namespace esphome::mac_media_compact
