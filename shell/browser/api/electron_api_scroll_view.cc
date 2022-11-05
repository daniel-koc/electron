// Copyright (c) 2022 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/browser/api/electron_api_scroll_view.h"

#include "base/cxx17_backports.h"
#include "base/no_destructor.h"
#include "cc/layers/layer.h"
#include "shell/browser/ui/views/scroll/scroll_bar_views.h"
#include "shell/common/gin_converters/gfx_converter.h"
#include "shell/common/gin_helper/constructor.h"
#include "shell/common/gin_helper/dictionary.h"
#include "shell/common/gin_helper/object_template_builder.h"
#include "shell/common/gin_helper/promise.h"
#include "shell/common/node_includes.h"
#include "ui/base/ui_base_features.h"
#include "ui/compositor/compositor.h"
#include "ui/compositor/layer.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/widget/widget.h"

#if !BUILDFLAG(IS_MAC)
#include "shell/browser/ui/views/scroll/scroll_view.h"
#include "shell/browser/ui/views/scroll/scroll_view_scroll_with_layers.h"
#endif

namespace electron::api {

namespace {

void UpdateScrollBars(views::ScrollView* scroll_view, bool is_smooth_scroll) {
  if (!scroll_view)
    return;

  if (is_smooth_scroll) {
    scroll_view->SetHorizontalScrollBar(std::make_unique<ScrollBarViews>(true));
    scroll_view->SetVerticalScrollBar(std::make_unique<ScrollBarViews>(false));
  } else {
    scroll_view->SetHorizontalScrollBar(
        std::make_unique<views::ScrollBarViews>(true));
    scroll_view->SetVerticalScrollBar(
        std::make_unique<views::ScrollBarViews>(false));
  }
}

}  // namespace

ScrollView::CompositorObserver::CompositorObserver(ScrollView* scroll_view)
    : scroll_view_(scroll_view), is_inside_set_scroll_position_(false) {
  CHECK(scroll_view_);
  CHECK(scroll_view_->view());

  if (!scroll_view_->view()->GetWidget())
    return;

  if (!scroll_view_->view()->GetWidget()->GetCompositor())
    return;

  scroll_view_->view()->GetWidget()->GetCompositor()->AddObserver(this);
}

ScrollView::CompositorObserver::~CompositorObserver() {
  CHECK(scroll_view_->view());

  if (!scroll_view_->view()->GetWidget())
    return;

  if (!scroll_view_->view()->GetWidget()->GetCompositor())
    return;

  scroll_view_->view()->GetWidget()->GetCompositor()->RemoveObserver(this);
}

void ScrollView::CompositorObserver::SetScrollPosition(
    gfx::Point point,
    base::OnceCallback<void(std::string)> callback) {
  point_ = std::make_unique<gfx::Point>(point);
  completion_callback_ = std::move(callback);

  auto* scroll = static_cast<views::ScrollView*>(scroll_view_->view());

  if (!scroll)
    return;

  views::View* contents_view = scroll->contents();
  if (!contents_view)
    return;

  ui::Layer* contents_layer = contents_view->layer();
  if (!contents_layer)
    return;

  cc::Layer* contents_cc_layer = contents_layer->cc_layer_for_testing();
  if (!contents_cc_layer)
    return;

  contents_cc_layer->SetNeedsCommit();
}

void ScrollView::CompositorObserver::OnCompositingDidCommit(
    ui::Compositor* compositor) {
  if (!point_)
    return;

  is_inside_set_scroll_position_ = true;
  scroll_view_->SetScrollPositionImpl(*point_, std::move(completion_callback_));
  is_inside_set_scroll_position_ = false;
  point_.reset();
}

ScrollView::ScrollView(v8::Isolate* isolate,
                       const gin_helper::Dictionary& options)
#if BUILDFLAG(IS_MAC)
    : View(new views::ScrollView())
#else
    : View(base::FeatureList::IsEnabled(
               ::features::kUiCompositorScrollWithLayers)
               ? new ScrollViewScrollWithLayers()
               : new electron::ScrollView())
#endif
{
  if (base::FeatureList::IsEnabled(::features::kUiCompositorScrollWithLayers))
    set_scroll_position_after_commit_ = true;

  auto* scroll_view = static_cast<views::ScrollView*>(view());
  scroll_view->SetBackgroundColor(absl::optional<SkColor>());

  on_contents_scrolled_subscription_ = scroll_view->AddContentsScrolledCallback(
      base::BindRepeating(&ScrollView::OnDidScroll, base::Unretained(this)));

  bool smooth_scroll = false;
  if (options.Get("smoothScroll", &smooth_scroll))
    SetSmoothScroll(smooth_scroll);
}

ScrollView::~ScrollView() = default;

void ScrollView::SetBackgroundColor(absl::optional<WrappedSkColor> color) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  scroll->SetBackgroundColor(color);
}

void ScrollView::SetContentView(
    absl::optional<gin::Handle<View>> content_view) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());

  if (content_view) {
    UpdateScrollBars(scroll, smooth_scroll_);
    (*content_view)->set_delete_view(false);
    auto contents = std::unique_ptr<views::View>((*content_view)->view());
    scroll->SetContents(std::move(contents));

    content_view_.Reset(isolate(), (*content_view).ToV8());
    api_content_view_ = (*content_view).get();
  }

  if (!api_content_view_)
    return;
  DCHECK_EQ(scroll->contents(), api_content_view_->view());
  scroll->SetContents(nullptr);
  api_content_view_->set_delete_view(true);
  content_view_.Reset();
  api_content_view_ = nullptr;
}

gin::Handle<View> ScrollView::GetContentView(v8::Isolate* isolate) {
  if (api_content_view_)
    return gin::CreateHandle(isolate, api_content_view_);
  else
    return gin::Handle<View>();
}

void ScrollView::SetContentSize(const gfx::Size& size) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  if (!scroll->contents())
    return;
  scroll->contents()->SetSize(size);
}

gfx::Size ScrollView::GetContentSize() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  if (!scroll->contents())
    return gfx::Size();
  return scroll->contents()->bounds().size();
}

void ScrollView::SetHorizontalScrollBarMode(std::string mode) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  views::ScrollView::ScrollBarMode scroll_bar_mode =
      views::ScrollView::ScrollBarMode::kEnabled;
  if (mode == "disabled")
    scroll_bar_mode = views::ScrollView::ScrollBarMode::kDisabled;
  else if (mode == "enabled-but-hidden")
    scroll_bar_mode = views::ScrollView::ScrollBarMode::kHiddenButEnabled;
  scroll->SetHorizontalScrollBarMode(scroll_bar_mode);
}

std::string ScrollView::GetHorizontalScrollBarMode() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  auto mode = scroll->GetHorizontalScrollBarMode();
  if (mode == views::ScrollView::ScrollBarMode::kDisabled)
    return "disabled";
  else if (mode == views::ScrollView::ScrollBarMode::kHiddenButEnabled)
    return "enabled-but-hidden";
  return "enabled";
}

void ScrollView::SetVerticalScrollBarMode(std::string mode) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  views::ScrollView::ScrollBarMode scroll_bar_mode =
      views::ScrollView::ScrollBarMode::kEnabled;
  if (mode == "disabled")
    scroll_bar_mode = views::ScrollView::ScrollBarMode::kDisabled;
  else if (mode == "enabled-but-hidden")
    scroll_bar_mode = views::ScrollView::ScrollBarMode::kHiddenButEnabled;
  scroll->SetVerticalScrollBarMode(scroll_bar_mode);
}

std::string ScrollView::GetVerticalScrollBarMode() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  auto mode = scroll->GetVerticalScrollBarMode();
  if (mode == views::ScrollView::ScrollBarMode::kDisabled)
    return "disabled";
  else if (mode == views::ScrollView::ScrollBarMode::kHiddenButEnabled)
    return "enabled-but-hidden";
  return "enabled";
}

void ScrollView::SetTreatAllScrollEventsAsHorizontal(
    bool events_as_horizontal) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  scroll->SetTreatAllScrollEventsAsHorizontal(events_as_horizontal);
}

bool ScrollView::GetTreatAllScrollEventsAsHorizontal() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetTreatAllScrollEventsAsHorizontal();
}

void ScrollView::SetScrollEventsEnabled(bool enable) {
  scroll_events_ = enable;
}

bool ScrollView::IsScrollEventsEnabled() const {
  return scroll_events_;
}

v8::Local<v8::Promise> ScrollView::SetScrollPosition(gfx::Point point) {
  gin_helper::Promise<void> promise(isolate());
  auto handle = promise.GetHandle();
  SetScrollPositionImpl(
      point, base::BindOnce(
                 [](gin_helper::Promise<void> promise, std::string error) {
                   if (error.empty()) {
                     promise.Resolve();
                   } else {
                     promise.RejectWithErrorMessage(error);
                   }
                 },
                 std::move(promise)));
  return handle;
}

gfx::Point ScrollView::GetScrollPosition() const {
  CHECK(view());

  if (set_scroll_position_after_commit_ && compositor_observer_ &&
      compositor_observer_->point()) {
    return *compositor_observer_->point();
  }

  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetVisibleRect().origin();
}

gfx::Point ScrollView::GetMaximumScrollPosition() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  gfx::Size content_size = scroll->contents()->bounds().size();
  gfx::Size viewport_size = scroll->GetVisibleRect().size();
  return gfx::Point(
      std::max(0, content_size.width() - viewport_size.width()),
      std::max(0, content_size.height() - viewport_size.height()));
}

void ScrollView::ClipHeightTo(int min_height, int max_height) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  scroll->ClipHeightTo(min_height, max_height);
}

int ScrollView::GetMinHeight() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetMinHeight();
}

int ScrollView::GetMaxHeight() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetMaxHeight();
}

void ScrollView::ScrollRectToVisible(const gfx::Rect& rect) {
  if (!api_content_view_)
    return;
  api_content_view_->view()->ScrollRectToVisible(rect);
}

gfx::Rect ScrollView::GetVisibleRect() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetVisibleRect();
}

void ScrollView::SetAllowKeyboardScrolling(bool allow) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  scroll->SetAllowKeyboardScrolling(allow);
}

bool ScrollView::GetAllowKeyboardScrolling() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetAllowKeyboardScrolling();
}

void ScrollView::SetDrawOverflowIndicator(bool indicator) {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  scroll->SetDrawOverflowIndicator(indicator);
}

bool ScrollView::GetDrawOverflowIndicator() const {
  CHECK(view());
  auto* scroll = static_cast<views::ScrollView*>(view());
  return scroll->GetDrawOverflowIndicator();
}

void ScrollView::SetSmoothScroll(bool enable) {
  CHECK(view());
  if (smooth_scroll_ != enable) {
    auto* scroll = static_cast<views::ScrollView*>(view());
    UpdateScrollBars(scroll, enable);
  }
  smooth_scroll_ = enable;
}

void ScrollView::OnDidScroll() {
  if (IsScrollEventsEnabled())
    Emit("did-scroll");
}

void ScrollView::SetScrollPositionImpl(
    gfx::Point point,
    base::OnceCallback<void(std::string)> callback) {
  CHECK(view());
  if (!api_content_view_) {
    std::move(callback).Run(std::string("Error"));
    return;
  }

  auto* scroll = static_cast<views::ScrollView*>(view());
  gfx::Size content_size = scroll->contents()->bounds().size();
  gfx::Rect visible_rect = scroll->GetVisibleRect();
  int max_x_position = std::max(0, content_size.width() - visible_rect.width());
  int max_y_position =
      std::max(0, content_size.height() - visible_rect.height());
  point.set_x(base::clamp(point.x(), 0, max_x_position));
  point.set_y(base::clamp(point.y(), 0, max_y_position));

  if (set_scroll_position_after_commit_) {
    if (!compositor_observer_) {
      compositor_observer_ =
          std::make_unique<ScrollView::CompositorObserver>(this);
    }

    if (!compositor_observer_->is_inside_set_scroll_position()) {
      compositor_observer_->SetScrollPosition(point, std::move(callback));
      return;
    }
  }

  // If a scrollBar is disabled, then we need to enable it when performing
  // ScrollView::ScrollToOffset, otherwise updating scrollBar positions is not
  // executed.
  auto horiz_mode = scroll->GetHorizontalScrollBarMode();
  if (horiz_mode == views::ScrollView::ScrollBarMode::kDisabled)
    scroll->SetHorizontalScrollBarMode(
        views::ScrollView::ScrollBarMode::kHiddenButEnabled);
  auto vert_mode = scroll->GetHorizontalScrollBarMode();
  if (vert_mode == views::ScrollView::ScrollBarMode::kDisabled)
    scroll->SetVerticalScrollBarMode(
        views::ScrollView::ScrollBarMode::kHiddenButEnabled);

  api_content_view_->view()->ScrollRectToVisible(gfx::Rect(
      point.x(), point.y(), visible_rect.width(), visible_rect.height()));

  if (horiz_mode == views::ScrollView::ScrollBarMode::kDisabled)
    scroll->SetHorizontalScrollBarMode(horiz_mode);
  if (vert_mode == views::ScrollView::ScrollBarMode::kDisabled)
    scroll->SetVerticalScrollBarMode(vert_mode);

  std::move(callback).Run(std::string());

  scroll->Layout();
}

// static
gin::Handle<ScrollView> ScrollView::Create(
    v8::Isolate* isolate,
    const gin_helper::Dictionary& options) {
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> arg = gin::ConvertToV8(isolate, options);
  v8::Local<v8::Object> obj;
  if (GetConstructor(isolate)->NewInstance(context, 1, &arg).ToLocal(&obj)) {
    gin::Handle<ScrollView> scroll_view;
    if (gin::ConvertFromV8(isolate, obj, &scroll_view))
      return scroll_view;
  }
  return gin::Handle<ScrollView>();
}

// static
v8::Local<v8::Function> ScrollView::GetConstructor(v8::Isolate* isolate) {
  static base::NoDestructor<v8::Global<v8::Function>> constructor;
  if (constructor.get()->IsEmpty()) {
    constructor->Reset(isolate,
                       gin_helper::CreateConstructor<ScrollView>(
                           isolate, base::BindRepeating(&ScrollView::New)));
  }
  return v8::Local<v8::Function>::New(isolate, *constructor.get());
}

// static
gin_helper::WrappableBase* ScrollView::New(gin_helper::Arguments* args) {
  gin_helper::Dictionary options =
      gin::Dictionary::CreateEmpty(args->isolate());
  args->GetNext(&options);

  // Constructor call.
  auto* scroll_view = new ScrollView(args->isolate(), options);
  scroll_view->InitWithArgs(args);
  return scroll_view;
}

// static
void ScrollView::BuildPrototype(v8::Isolate* isolate,
                                v8::Local<v8::FunctionTemplate> prototype) {
  prototype->SetClassName(gin::StringToV8(isolate, "ScrollView"));
  gin_helper::ObjectTemplateBuilder(isolate, prototype->PrototypeTemplate())
      .SetMethod("setContentView", &ScrollView::SetContentView)
      .SetMethod("getContentView", &ScrollView::GetContentView)
      .SetMethod("setContentSize", &ScrollView::SetContentSize)
      .SetMethod("getContentSize", &ScrollView::GetContentSize)
      .SetMethod("setHorizontalScrollBarMode",
                 &ScrollView::SetHorizontalScrollBarMode)
      .SetMethod("getHorizontalScrollBarMode",
                 &ScrollView::GetHorizontalScrollBarMode)
      .SetMethod("setVerticalScrollBarMode",
                 &ScrollView::SetVerticalScrollBarMode)
      .SetMethod("getVerticalScrollBarMode",
                 &ScrollView::GetVerticalScrollBarMode)
      .SetMethod("setTreatAllScrollEventsAsHorizontal",
                 &ScrollView::SetTreatAllScrollEventsAsHorizontal)
      .SetMethod("getTreatAllScrollEventsAsHorizontal",
                 &ScrollView::GetTreatAllScrollEventsAsHorizontal)
      .SetMethod("setScrollEventsEnabled", &ScrollView::SetScrollEventsEnabled)
      .SetMethod("isScrollEventsEnabled", &ScrollView::IsScrollEventsEnabled)
      .SetMethod("setScrollPosition", &ScrollView::SetScrollPosition)
      .SetMethod("getScrollPosition", &ScrollView::GetScrollPosition)
      .SetMethod("getMaximumScrollPosition",
                 &ScrollView::GetMaximumScrollPosition)
      .SetMethod("clipHeightTo", &ScrollView::ClipHeightTo)
      .SetMethod("getMinHeight", &ScrollView::GetMinHeight)
      .SetMethod("getMaxHeight", &ScrollView::GetMaxHeight)
      .SetMethod("scrollRectToVisible", &ScrollView::ScrollRectToVisible)
      .SetMethod("getVisibleRect", &ScrollView::GetVisibleRect)
      .SetMethod("setAllowKeyboardScrolling",
                 &ScrollView::SetAllowKeyboardScrolling)
      .SetMethod("getAllowKeyboardScrolling",
                 &ScrollView::GetAllowKeyboardScrolling)
      .SetMethod("setDrawOverflowIndicator",
                 &ScrollView::SetDrawOverflowIndicator)
      .SetMethod("getDrawOverflowIndicator",
                 &ScrollView::GetDrawOverflowIndicator);
}

}  // namespace electron::api

namespace {

using electron::api::ScrollView;

void Initialize(v8::Local<v8::Object> exports,
                v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context,
                void* priv) {
  v8::Isolate* isolate = context->GetIsolate();
  gin_helper::Dictionary dict(isolate, exports);
  dict.Set("ScrollView", ScrollView::GetConstructor(isolate));
}

}  // namespace

NODE_LINKED_MODULE_CONTEXT_AWARE(electron_browser_scroll_view, Initialize)
