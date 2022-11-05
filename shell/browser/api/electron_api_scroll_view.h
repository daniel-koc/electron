// Copyright (c) 2022 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ELECTRON_SHELL_BROWSER_API_ELECTRON_API_SCROLL_VIEW_H_
#define ELECTRON_SHELL_BROWSER_API_ELECTRON_API_SCROLL_VIEW_H_

#include "base/callback_list.h"
#include "shell/browser/api/electron_api_view.h"
#include "ui/compositor/compositor_observer.h"

namespace gin_helper {
class Dictionary;
}

namespace electron::api {

class ScrollView : public View {
 public:
  // Create a new instance of ScrollView.
  static gin::Handle<ScrollView> Create(v8::Isolate* isolate,
                                        const gin_helper::Dictionary& options);

  // Return the cached constructor function.
  static v8::Local<v8::Function> GetConstructor(v8::Isolate* isolate);

  // gin_helper::Wrappable
  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::FunctionTemplate> prototype);

 protected:
  ScrollView(v8::Isolate* isolate, const gin_helper::Dictionary& options);
  ~ScrollView() override;

  // View:
  void SetBackgroundColor(absl::optional<WrappedSkColor> color) override;

  // ScrollView APIs.
  void SetContentView(absl::optional<gin::Handle<View>> content_view);
  gin::Handle<View> GetContentView(v8::Isolate* isolate);
  void SetContentSize(const gfx::Size& size);
  gfx::Size GetContentSize() const;
  void SetHorizontalScrollBarMode(std::string mode);
  std::string GetHorizontalScrollBarMode() const;
  void SetVerticalScrollBarMode(std::string mode);
  std::string GetVerticalScrollBarMode() const;
  void SetTreatAllScrollEventsAsHorizontal(bool events_as_horizontal);
  bool GetTreatAllScrollEventsAsHorizontal() const;
  void SetScrollEventsEnabled(bool enable);
  bool IsScrollEventsEnabled() const;
  v8::Local<v8::Promise> SetScrollPosition(gfx::Point point);
  gfx::Point GetScrollPosition() const;
  gfx::Point GetMaximumScrollPosition() const;
  void ClipHeightTo(int min_height, int max_height);
  int GetMinHeight() const;
  int GetMaxHeight() const;
  void ScrollRectToVisible(const gfx::Rect& rect);
  gfx::Rect GetVisibleRect() const;
  void SetAllowKeyboardScrolling(bool allow);
  bool GetAllowKeyboardScrolling() const;
  void SetDrawOverflowIndicator(bool indicator);
  bool GetDrawOverflowIndicator() const;

  void SetSmoothScroll(bool enable);
  void OnDidScroll();

  void SetScrollPositionImpl(gfx::Point point,
                             base::OnceCallback<void(std::string)> callback);

 private:
  static gin_helper::WrappableBase* New(gin_helper::Arguments* args);

  class CompositorObserver : public ui::CompositorObserver {
   public:
    CompositorObserver(ScrollView* scroll_view);
    ~CompositorObserver() override;

    bool is_inside_set_scroll_position() const {
      return is_inside_set_scroll_position_;
    }

    gfx::Point* point() { return point_.get(); }

    void SetScrollPosition(gfx::Point point,
                           base::OnceCallback<void(std::string)> callback);

    // ui::CompositorObserver:
    void OnCompositingDidCommit(ui::Compositor* compositor) override;

   private:
    ScrollView* scroll_view_;
    std::unique_ptr<gfx::Point> point_;
    base::OnceCallback<void(std::string)> completion_callback_;
    bool is_inside_set_scroll_position_;
  };

  std::unique_ptr<CompositorObserver> compositor_observer_;
  bool set_scroll_position_after_commit_ = false;
  bool smooth_scroll_ = false;
  bool scroll_events_ = false;
  base::CallbackListSubscription on_contents_scrolled_subscription_;

  v8::Global<v8::Value> content_view_;
  View* api_content_view_ = nullptr;
};

}  // namespace electron::api

#endif  // ELECTRON_SHELL_BROWSER_API_ELECTRON_API_SCROLL_VIEW_H_
