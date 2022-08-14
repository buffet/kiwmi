# Lua API Documentation

kiwmi is configured completely in Lua.
All types kiwmi offers are actually reference types, pointing to the actual internal types.
This means Lua's garbage collection has no effect on the lifetime of the object.

kiwmi offers the following classes to work with:

## Globals

### `FROM_KIWMIC`

`true` when invoked from kiwmic, `false` otherwise.

## kiwmi_server

This is the type of the global `kiwmi` singleton, representing the compositor.
This is the entry point to the API.

### Methods

#### kiwmi:active_output()

Returns the active `kiwmi_output`.

See `request_active_output`.

#### kiwmi:bg_color(color)

Sets the background color (shown behind all views) to `color` (in the format #rrggbb).

#### kiwmi:cursor()

Returns a reference to the cursor object.

#### kiwmi:focused_view()

Returns the currently focused view.

#### kiwmi:output_at(lx, ly)

Returns the output at a specified position

#### kiwmi:on(event, callback)

Used to register event listeners.

#### kiwmi:quit()

Quit kiwmi.

#### kiwmi:schedule(delay, callback)

Call `callback` after `delay` ms.
Callback get passed itself, so that it can easily reregister itself.

#### kiwmi:set_verbosity(level)

Sets verbosity of kiwmi to the level specified with a number (see `kiwmi:verbosity()`).

#### kiwmi:spawn(command)

Spawn a new process.
`command` is passed to `/bin/sh`.

#### kiwmi:stop_interactive()

Stops an interactive move or resize.

#### kiwmi:unfocus()

Unfocus the currently focused view.

#### kiwmi:verbosity()

Returns the numerical verbosity level of kiwmi (value of one of `wlr_log_importance`, silent = 0, error = 1, info = 2, debug = 3).

#### kiwmi:view_at(lx, ly)

Get the view at a specified position.

### Events

#### keyboard

A new keyboard got attached.
Callback receives a reference to the keyboard.

#### request_active_output

Called when the active output needs to be requested (for example because a layer-shell surface needs to be positioned).
Callback receives nothing and optionally returns a kiwmi_output.

If this isn't set or returns `nil`, the compositor defaults to the output the focused view is on, and if there is no view, the output the mouse is on.

#### output

A new output got attached.
Callback receives a reference to the output.

#### view

A new view got created (actually mapped).
Callback receives a reference to the view.

## kiwmi_cursor

A reference to the cursor object.

### Methods

#### cursor:output_at_pos()

 Returns the output at the cursor position or `nil` if there is none.

#### cursor:on(event, callback)

Used to register event listeners.

#### cursor:pos()

Get the current position of the cursor.
Returns two parameters: `x` and `y`.

#### cursor:view_at_pos()

Returns the view at the cursor position, or `nil` if there is none.

### Events

#### button_down

A mouse button got pressed.
Callback receives the ID of the button (i.e. LMB is 1, RMB is 2, ...).

The callback is supposed to return `true` if the event was handled.
The compositor will not forward it to the view under the cursor.

#### button_up

A mouse button got released.
Callback receives the ID of the button (i.e. LMB is 1, RMB is 2, ...).

The callback is supposed to return `true` if the event was handled.
The compositor will not forward it to the view under the cursor.

#### motion

The cursor got moved.
Callback receives a table containing `oldx`, `oldy`, `newx`, and `newy`.

#### scroll

Something was scrolled.
The callback receives a table containing `device` with the device name, `vertical` indicating whether it was a vertical or horizontal scroll, and `length` with the length of the vector (negative for left of up scrolls).

The callback is supposed to return `true` if the event was handled.
The compositor will not forward it to the view under the cursor.

#### touch

A touchpad or toushcreen was interacted with in some way. The callback
receives a table containing

* `name`: one of `down`, `up`, `motion` or `frame`
* `id`: a number used to indicate which finger has dome something, used on devices that support "multitouch". For example, the first finger to touch the pad may be assigned id 0 and a second finger added may be assigned 1. The id may or may not be reassigned if a finger is lifted and replaced.
* `x`, `y`: for `down` or `motion` events, the location where the screen was touched. These each range between 0.0 and 1.0

All touch events are sent to the same callback, because the
appropriate mode of processing them is to collect messages until
there's a `frame` event and group them together. For more information see [the Wayland book](https://wayland-book.com/seat/touch.html)

The callback is supposed to return `true` if the event was handled.
The compositor will not forward it to the view under the cursor.
_FIXME: should use gesture cancellation instead_

## kiwmi_keyboard

A handle to a keyboard.

### Methods

#### keyboard:keymap(keymap)

The function takes a table as parameter.
The possible table indexes are "rules, model, layout, variant, options".
All the table parameters are optional and set to the system default if not set.
For the values to set have a look at the xkbcommon library.
<https://xkbcommon.org/doc/current/structxkb__rule__names.html>

#### keyboard:modifiers()

Returns a table with the state of all modifiers.
These are: `shift`, `caps`, `ctrl`, `alt`, `mod2`, `mod3`, `super`, and `mod5`.

#### keyboard:on(event, callback)

Used to register event listeners.

### Events

#### destroy

The keyboard is getting destroyed.
Callback receives the keyboard.

#### key_down

A key got pressed.

Callback receives a table containing the `key`, `keycode`, `raw`, and the `keyboard`.

This event gets triggered twice, once with mods applied (i.e. `Shift+3` is `#`) and `raw` set to `false`, and then again with no mods applied and `raw` set to `true`.

The callback is supposed to return `true` if the event was handled.
The compositor will not forward it to the focused view in that case.

#### key_up

A key got released.
Callback receives a table containing the `key`, `keycode`, `raw`, and the `keyboard`.

This event gets triggered twice, once with mods applied (i.e. `Shift+3` is `#`) and `raw` set to `false`, and then again with no mods applied and `raw` set to `true`.

The callback is supposed to return `true` if the event was handled.
The compositor will not forward it to the focused view in that case.

## kiwmi_lua_callback

A handle to a registered callback.

## kiwmi_output

Represents an output (most often a display).

### Methods

#### output:auto()

Tells the compositor to start automatically positioning the output (this is on per default).

#### output:move(lx, ly)

Moves the output to a specified position.
This is referring to the top-left corner.

#### output:name()

The name of the output.

#### output:on(event, callbacks)

Used to register event listeners.

#### output:pos()

Get the position of the output.
Returns two parameters: `x` and `y`.

#### output:size()

Get the size of the output.
Returns two parameters: `width` and `height`.

#### output:usable_area()

Returns a table containing the `x`, `y`, `width` and `height` of the output's usable area, relative to the output's top left corner.

### Events

#### destroy

The output is getting destroyed.
Callback receives the output.

#### resize

The output is being resized.
Callback receives a table containing the `output`, the new `width`, and the new `height`.

#### usable_area_change

The usable area of this output has changed, e.g. because the output was resized or the bars around it changed.
Callback receives a table containing the `output` and the new `x`, `y`, `width` and `height`.

## kiwmi_view

Represents a view (a window in kiwmi terms).

### Methods

#### view:app_id()

Returns the app id of the view.
This is comparable to the window class of X windows.

#### view:close()

Closes the view.

#### view:csd(client_draws)

Set whether the client is supposed to draw their own client decoration.

#### view:focus()

Focuses the view.

#### view:hidden()

Returns `true` if the view is hidden, `false` otherwise.

#### view:hide()

Hides the view.

#### view:id()

Returns an ID unique to the view.

#### view:imove()

Starts an interactive move.

#### view:iresize(edges)

Starts an interactive resize.
Takes a table containing the name of the edges, that the resize is happening on.
So for example to resize pulling on the bottom right corner you would pass `{"b", "r"}`.

#### view:move(lx, ly)

Moves the view to the specified position.

#### view:on(event, callback)

Used to register event listeners.

#### view:pid()

Returns the process ID of the client associated with the view.

#### view:pos()

Returns the position of the view (top-left corner).

#### view:resize(width, height)

Resizes the view.

#### view:show()

Unhides the view.

#### view:size()

Returns the size of the view.

**NOTE**: Used directly after `view:resize()`, this still returns the old size.

#### view:tiled(edges)

Takes a table containing all edges that are tiled, or a bool to indicate all 4 edges.

#### view:title()

Returns the title of the view.

### Events

#### destroy

The view is being destroyed.
Callback receives the view.

#### post_render

This is a no-op event. Temporarily preserved only to make config migration easier.

#### pre_render

This is a no-op event. Temporarily preserved only to make config migration easier.

#### request_move

The view wants to start an interactive move.
Callback receives the view.

#### request_resize

The view wants to start an interactive resize.
Callback receives a table containing the `view`, and `edges`, containing the edges.
