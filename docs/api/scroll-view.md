# ScrollView

Show a part of view with scrollbar.
The `ScrollView` can show an arbitrary content view inside it. It is used to make
any View scrollable. When the content is larger than the `ScrollView`,
scrollbars will be optionally showed. When the content view is smaller
then the `ScrollView`, the content view will be resized to the size of the
`ScrollView`.

Process: [Main](../glossary.md#main-process)

This module cannot be used until the `ready` event of the `app`
module is emitted.

```javascript
const win = new BaseWindow({ width: 800, height: 400 })

const scroll = new ScrollView()
win.contentView.addChildView(scroll)
scroll.setBounds({ x: 0, y: 0, width: 800, height: 400 })

const scrollContent = new View()
scroll.setContentView(scrollContent)
scrollContent.setBounds({ x: 0, y: 0, width: 1200, height: 600 })

const view1 = new WebContentsView()
scrollContent.addChildView(view1)
view1.webContents.loadURL('https://my.app/one')
view1.setBounds({ x: 0, y: 0, width: 600, height: 600 })

const view2 = new WebContentsView()
scrollContent.addChildView(view2)
view2.webContents.loadURL('https://my.app/two')
view2.setBounds({ x: 600, y: 0, width: 600, height: 600 })
```

## Class: ScrollView extends `View`

> Create and control scroll views.

Process: [Main](../glossary.md#main-process)

`ScrollView` is an [EventEmitter][event-emitter].

### `new ScrollView([options])` _Experimental_

* `options` Object (optional)
  * `smoothScroll` boolean (optional) - When is `true` enables smooth scroll in ScrollView. Default is `false`.

Creates the new scroll view.

### Instance Events

Objects created with `new ScrollView` emit the following events:

#### Event: 'did-scroll' _Experimental_

Returns:

* `event` Event

Emitted when the content view is being scrolled.

### Instance Methods

Objects created with `new ScrollView` have the following instance methods:

#### `view.setContentView(contents)`

* `contents` [View](view.md) | null - The view that needs to scroll.

Set the contents. if `null` is passing as argument then currently contents is removed from the scroll.

#### `view.getContentView()`

Returns [`View`](view.md) - The contents of the `view`.

#### `view.setContentSize(size)`

* `size` [Size](structures/size.md)

Set the size of the contents.

#### `view.getContentSize()`

Returns [`Size`](structures/size.md) - The `size` of the contents.

#### `view.setHorizontalScrollBarMode(mode)`

* `mode` string - Can be `disabled`, `enabled-but-hidden`, `enabled`. Default is `enabled`.

Controls how the horizontal scroll bar appears and functions.
* `disabled` - The scrollbar is hidden, and the pane will not respond to e.g. mousewheel events even if the contents are larger than the viewport.
* `enabled-but-hidden` - The scrollbar is hidden whether or not the contents are larger than the viewport, but the pane will respond to scroll events.
*`enabled` - The scrollbar will be visible if the contents are larger than the viewport and the pane will respond to scroll events.

#### `view.getHorizontalScrollBarMode()`

Returns `string` - horizontal scrollbar mode.

#### `view.setVerticalScrollBarMode(mode)`

* `mode` string - Can be `disabled`, `enabled-but-hidden`, `enabled`. Default is `enabled`.

Controls how the vertical scroll bar appears and functions.
* `disabled` - The scrollbar is hidden, and the pane will not respond to e.g. mousewheel events even if the contents are larger than the viewport.
* `enabled-but-hidden` - The scrollbar is hidden whether or not the contents are larger than the viewport, but the pane will respond to scroll events.
*`enabled` - The scrollbar will be visible if the contents are larger than the viewport and the pane will respond to scroll events.

#### `view.getVerticalScrollBarMode()`

Returns `string` - vertical scrollbar mode.

#### `view.setScrollPosition(point)`

* `point` [Point](structures/point.md) - The point in the `contentView` to scroll to.

Returns `Promise<void>` - Promis is resolved when scroll position is acual set. Scroll to the horizontal (`point.x`) and vertical (`point.y`) position.

#### `view.getScrollPosition()`

Returns [`Point`](structures/point.md) - The horizontal and vertical scroll position.

#### `view.getMaximumScrollPosition()`

Returns [`Point`](structures/point.md) - The maximum horizontal and vertical scroll position.

#### `view.setScrollEventsEnabled(enable)`

* `enable` boolean - Whether the scroll events are enabled. Default is `false`.

#### `view.isScrollEventsEnabled()`

Returns `boolean` - Whether the scroll events are enabled.

#### `view.setTreatAllScrollEventsAsHorizontal(eventsAsHorizontal)`

* `eventsAsHorizontal` boolean - Whether the mousewheel should scroll horizontally.

Causes vertical scroll events (e.g. scrolling with the mousewheel) as
horizontal events, to make scrolling in horizontal-only scroll situations
easier for the user.

#### `view.getTreatAllScrollEventsAsHorizontal()`

Returns `boolean` - Whether the mousewheel scrolls horizontally. Default is `false`.

#### `view.clipHeightTo(minHeight, maxHeight)`

* `minHeight` Integer - The min height for the bounded scroll view.
* `maxHeight` Integer - The max height for the bounded scroll view.

Turns this scroll view into a bounded scroll view, with a fixed height.
By default, a ScrollView will stretch to fill its outer container.

#### `view.getMinHeight()`

Returns `Integer` - The min height for the bounded scroll view.

This is negative value if the view is not bounded.

#### `view.getMaxHeight()`

Returns `Integer` - The max height for the bounded scroll view.

This is negative value if the view is not bounded.

#### `view.scrollRectToVisible(rect)`

* `rect` [`Rectangle`](structures/rectangle.md) - The region in the `contentView` to scroll to.

Scrolls the specified region, in this View's coordinate system, to be visible.

#### `view.getVisibleRect()`

Returns [`Rectangle`](structures/rectangle.md) - The visible region of the `contentView`.

#### `view.setAllowKeyboardScrolling(allow)`

* `allow` boolean - Whether the left/right/up/down arrow keys attempt to scroll the view.

#### `view.getAllowKeyboardScrolling()`

Returns `boolean` - Gets whether the keyboard arrow keys attempt to scroll the view. Default is `true`.

#### `view.SetDrawOverflowIndicator(indicator)`

* `indicator` boolean - Whether to draw a white separator on the four sides of the scroll view when it overflows.

#### `view.GetDrawOverflowIndicator()`

Returns `boolean` - Gets whether to draw a white separator on the four sides of the scroll view when it overflows. Default is `true`.
