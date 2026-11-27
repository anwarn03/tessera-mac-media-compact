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

// An extra one-pixel place under the visible cover gives each track a fresh
// atlas key. Tessera currently offers no public picture revision parameter.
class CoverRevision {
 public:
  void create(lv_obj_t *parent, int x, int y) {
    marker_ = ui::picture(parent);
    lv_obj_set_size(marker_, 1, 1);
    lv_obj_set_pos(marker_, x, y);
  }

  void update(const std::string &entity, JsonObjectConst data) {
    if (!marker_ || entity.empty()) return;
    lv_obj_remove_flag(marker_, LV_OBJ_FLAG_HIDDEN);
    JsonObjectConst a = data["attributes"];
    std::string next = std::string(a["media_content_id"] | "") + "\n" +
                       std::string(a["media_title"] | "") + "\n" +
                       std::string(a["media_artist"] | "") + "\n" +
                       std::string(a["media_album_name"] | "") + "\n" +
                       std::string(a["entity_picture"] | "") + "\n" +
                       std::string(a["entity_picture_local"] | "");
    if (!initialized_ || next != signature_) {
      initialized_ = true;
      signature_ = next;
      bump();
      ticks_ = 0;
      retry_ = true;
    }
    ui::set_picture(marker_, cover_address(entity, data), tessera::Fit::COVER);
  }

  void clear() {
    if (marker_) {
      ui::set_picture(marker_, "");
      lv_obj_add_flag(marker_, LV_OBJ_FLAG_HIDDEN);
    }
    initialized_ = false;
    retry_ = false;
  }

  // Metadata can reach Home Assistant before the new cover. Ask again after
  // a short delay, even if its long picture URL was truncated in plugin state.
  void tick() {
    if (!marker_ || !retry_) return;
    ++ticks_;
    if (ticks_ == 3 || ticks_ == 8) bump();
    if (ticks_ >= 8) retry_ = false;
  }

 private:
  void bump() {
    revision_ = revision_ % 30000 + 1;
    lv_obj_set_style_radius(marker_, revision_, 0);
  }
  lv_obj_t *marker_ = nullptr;
  std::string signature_;
  int revision_ = 0, ticks_ = 0;
  bool initialized_ = false, retry_ = false;
};

void set_playback_action(const std::string &entity, int x, int width, int tap_x) {
  if (entity.empty() || width <= 0) return;
  const int segment = std::max(0, std::min(2, (tap_x - x) * 3 / width));
  switch (segment) {
    case 0: tessera::action("media_player.media_previous_track", entity); break;
    case 1: tessera::action("media_player.media_play_pause", entity); break;
    default: tessera::action("media_player.media_next_track", entity); break;
  }
}

class PlaybackKeys {
 public:
  void create(lv_obj_t *parent, int x, int y, int width) {
    for (int i = 0; i < 3; ++i) {
      labels_[i] = ui::label(parent, Font::ICON_SMALL, theme::INK);
      const int left = width * i / 3, right = width * (i + 1) / 3;
      lv_obj_set_pos(labels_[i], x + left, y);
      lv_obj_set_size(labels_[i], right - left, ui::line_height(Font::ICON_SMALL));
      lv_obj_set_style_text_align(labels_[i], LV_TEXT_ALIGN_CENTER, 0);
    }
  }
  void show(bool playing) {
    ui::set_text(labels_[0], ui::icon(0xF04AE));
    ui::set_text(labels_[1], ui::icon(playing ? 0xF03E4 : 0xF040A));
    ui::set_text(labels_[2], ui::icon(0xF04AD));
  }
  void clear() { for (auto *label : labels_) ui::set_text(label, ""); }
  void theme() { for (auto *label : labels_) ui::set_color(label, theme::INK); }
 private:
  lv_obj_t *labels_[3] = {};
};

}  // namespace

class CompactTile : public tessera::Tile {
 public:
  explicit CompactTile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    entity_ = c.entity;
    tile_ = c.tile;
    const int w = c.width;
    const int h = c.height;
    width_ = w;
    height_ = h;
    const int inset = ui::px(8);
    const int gap = ui::px(10);
    cover_side_ = std::max(32, std::min(h - 2 * inset, (w - 2 * inset - gap) * 2 / 5));
    right_x_ = inset + cover_side_ + gap;
    right_w_ = std::max(1, w - right_x_ - inset);
    const int source_h = ui::line_height(Font::BODY);
    const int title_h = ui::line_height(Font::TITLE);
    const int artist_h = ui::line_height(Font::BODY);
    const int keys_h = ui::line_height(Font::ICON_SMALL);
    const int group_h = source_h + title_h + artist_h + keys_h + ui::px(10);
    const int group_y = std::max(0, (h - group_h) / 2);
    controls_y_ = group_y + source_h + title_h + artist_h + ui::px(10);

    if (ui::pictures()) {
      cover_y_ = (h - cover_side_) / 2;
      revision_.create(c.parent, inset, cover_y_);
      cover_ = ui::picture(c.parent);
      lv_obj_set_size(cover_, cover_side_, cover_side_);
      lv_obj_set_pos(cover_, inset, cover_y_);
    }

    source_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    title_ = ui::label(c.parent, Font::TITLE, theme::INK);
    artist_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    for (lv_obj_t *p : {source_, title_, artist_}) {
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
    keys_.create(c.parent, right_x_, controls_y_, right_w_);
  }

  void on_state(JsonObjectConst data) override {
    std::string state = data["state"] | "";
    active_ = state == "playing" || state == "paused";
    if (!active_) {
      for (lv_obj_t *p : {source_, title_, artist_}) ui::set_text(p, "");
      keys_.clear();
      revision_.clear();
      if (cover_) { ui::set_picture(cover_, ""); lv_obj_add_flag(cover_, LV_OBJ_FLAG_HIDDEN); }
      return;
    }
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
    keys_.show(playing_);
    revision_.update(entity_, data);
    if (cover_ && !entity_.empty()) {
      lv_obj_remove_flag(cover_, LV_OBJ_FLAG_HIDDEN);
      ui::set_picture(cover_, cover_address(entity_, data), tessera::Fit::COVER);
    }
  }

  void on_theme() override {
    ui::set_color(source_, theme::MUTED);
    ui::set_color(title_, theme::INK);
    ui::set_color(artist_, theme::MUTED);
    keys_.theme();
  }

  void on_tick(uint32_t) override { if (active_) revision_.tick(); }

  void on_tap() override {
    if (entity_.empty() || !active_) return;
    const tessera::Point p = tessera::tap_point();
    if (p.x < 0 || p.y < 0 || p.x >= width_ || p.y >= height_) return;
    if (p.x >= right_x_ && p.x < right_x_ + right_w_ && p.y >= controls_y_ - ui::px(8)) {
      set_playback_action(entity_, right_x_, right_w_, p.x);
      return;
    }
    tessera::open_card(plugin_->plugin_id(), "cover", entity_, tile_, "Lecture en cours");
  }

 private:
  const tessera::Plugin *plugin_;
  CoverRevision revision_;
  PlaybackKeys keys_;
  std::string entity_;
  lv_obj_t *cover_ = nullptr, *source_ = nullptr, *title_ = nullptr, *artist_ = nullptr;
  int cover_side_ = 0, cover_y_ = 0, right_x_ = 0, right_w_ = 1, controls_y_ = 0, tile_ = -1, width_ = 0, height_ = 0;
  bool playing_ = false;
  bool active_ = false;
};

class CoverCard : public tessera::Card {
 public:
  void open(const tessera::CardContext &c) override {
    entity_ = c.entity;
    width_ = c.width;
    height_ = c.height;
    const int inset = ui::px(12);
    const int controls_h = ui::line_height(Font::ICON_SMALL);
    const int title_h = ui::line_height(Font::TITLE);
    const int artist_h = ui::line_height(Font::BODY);
    const int text_h = title_h + artist_h + controls_h + ui::px(16);
    cover_side_ = std::max(48, std::min(width_ - 2 * inset, height_ - text_h - 2 * inset));
    const int cover_x = std::max(0, (width_ - cover_side_) / 2);
    revision_.create(c.parent, cover_x, inset);
    cover_ = ui::picture(c.parent);
    lv_obj_set_size(cover_, cover_side_, cover_side_);
    lv_obj_set_pos(cover_, cover_x, inset);
    title_ = ui::label(c.parent, Font::TITLE, theme::INK);
    artist_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    controls_y_ = inset + cover_side_ + ui::px(8) + title_h + artist_h + ui::px(8);
    for (lv_obj_t *p : {title_, artist_}) {
      lv_obj_set_width(p, std::max(1, width_ - 2 * inset));
      lv_obj_set_style_text_align(p, LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_obj_set_pos(title_, inset, inset + cover_side_ + ui::px(8));
    lv_obj_set_height(title_, title_h);
    lv_label_set_long_mode(title_, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(artist_, inset, inset + cover_side_ + ui::px(8) + title_h);
    lv_obj_set_height(artist_, artist_h);
    lv_label_set_long_mode(artist_, LV_LABEL_LONG_DOT);
    keys_.create(c.parent, inset, controls_y_, width_ - 2 * inset);
  }

  void on_state(JsonObjectConst data) override {
    const std::string state = data["state"] | "";
    active_ = state == "playing" || state == "paused";
    if (!active_) {
      for (lv_obj_t *p : {title_, artist_}) ui::set_text(p, "");
      keys_.clear();
      revision_.clear();
      if (cover_) { ui::set_picture(cover_, ""); lv_obj_add_flag(cover_, LV_OBJ_FLAG_HIDDEN); }
      return;
    }
    const std::string title = data["attributes"]["media_title"] | "";
    const std::string artist = data["attributes"]["media_artist"] | "";
    ui::set_text(title_, title.empty() ? (state == "unavailable" ? "Lecteur indisponible" : "Aucune lecture") : title);
    ui::set_text(artist_, artist);
    keys_.show(state == "playing");
    revision_.update(entity_, data);
    if (cover_ && !entity_.empty()) {
      lv_obj_remove_flag(cover_, LV_OBJ_FLAG_HIDDEN);
      ui::set_picture(cover_, cover_address(entity_, data), tessera::Fit::COVER);
    }
  }

  void on_theme() override {
    ui::set_color(title_, theme::INK);
    ui::set_color(artist_, theme::MUTED);
    keys_.theme();
  }

  void on_tick(uint32_t) override { if (active_) revision_.tick(); }

  void on_tap() override {
    if (!active_) return;
    const tessera::Point p = tessera::tap_point();
    const int inset = ui::px(12);
    if (p.x >= inset && p.x < width_ - inset && p.y >= controls_y_ - ui::px(8))
      set_playback_action(entity_, inset, width_ - 2 * inset, p.x);
  }

 private:
  CoverRevision revision_;
  PlaybackKeys keys_;
  std::string entity_;
  lv_obj_t *cover_ = nullptr, *title_ = nullptr, *artist_ = nullptr;
  int width_ = 0, height_ = 0, cover_side_ = 0, controls_y_ = 0;
  bool active_ = false;
};

void MacMediaCompact::setup() {
  add_tile("compact", [this]() { return new CompactTile(this); });
  add_card("cover", []() { return new CoverCard(); }, true);
}
}  // namespace esphome::mac_media_compact
