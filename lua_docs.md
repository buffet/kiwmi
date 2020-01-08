# Lua API Documentation

kiwmi is configured completely in Lua.
All types kiwmi offers are actually reference types, pointing to the actual internal types.
This means Lua's garbage collection has no effect on the lifetime of the object.

kiwmi offers the following classes to work with:

## kiwmi_server

This is the type of the global `kiwmi` singleton, representing the compositor.
This is the entry point to the API.

### Methods

#### kiwmi:cursor()

Returns a reference to the cursor object.

#### kiwmi:focused_view()

Returns the currently focused view.

#### kiwmi:on(type, callback)

Used to register event listeners.

#### kiwmi:quit()

Quit kiwmi.

#### kiwmi:spawn(command)

Spawn a new process.
`command` is passed to `/bin/sh`.

#### kiwmi:view_at(x, y)

Get the view at a specified position.

### Events

#### keyboard

A new keyboard got attached.
Callback receives a reference to the keyboard.

#### output

A new output got attached.
Callback receives a reference to the output.

#### view

A new view got created (actually mapped).
Callback receives a reference to the view.

## kiwmi_cursor

A reference to the cursor object.

### Methods

#### cursor:on(type, callback)

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

#### button_up

A mouse button got released.
Callback receives the ID of the button (i.e. LMB is 1, RMB is 2, ...).

#### motion

The cursor got moved.
Callback receives a table containing `oldx`, `oldy`, `newx`, and `newy`.

## kiwmi_keyboard

A handle to a keyboard.

### Methods

#### keyboard:modifiers()

Returns a table with the state of all modifiers.
These are: `shift`, `caps`, `ctrl`, `alt`, `mod2`, `mod3`, `super`, and `mod5`.

#### keyboard:on(type, callback)

Used to register event listeners.

### Events

#### key_down

A key got pressed.
Callback receives a table containing the `key` and the `keyboard`).

#### key_up

A key got released.
Callback receives a table containing the `key` and the `keyboard`).

## kiwmi_lua_callback

A handle to a registered callback.

### Methods

#### callback:cancel()

Unregisters the callback.

## kiwmi_output

Represents an output (most often a display).

### Methods

#### output:auto()

Tells the compositor to start automatically positioning the output (this is on per default).

#### output:move(x, y)

Moves the output to a specified position.
This is referring to the top-left corner.

#### output:name()

The name of the output.

#### output:on(type, callbacks)

Used to register event listeners.

#### output:pos()

Get the position of the output.
Returns two parameters: `x` and `y`.

#### output:size()

Get the size of the output.
Returns two parameters: `width` and `height`.

### Events

#### destroy

The output is getting destroyed.
Callback receives the output.

#### resize

The output is being resized.
Callback receives a table containing the `output`, the new `width`, and the new `height`.

## kiwmi_view

Represents a view (a window in kiwmi terms).

### Methods

#### view:close()

Closes the view.

#### view:focus()

Focuses the view.

#### view:hidden()

Returns `true` if the view is hidden, `false` otherwise.

#### view:hide()

Hides the view.

#### view:move(x, y)

Moves the view to the specified position.

#### view:on(type, callback)

Used to register event listeners.

#### view:pos()

Returns the position of the view (top-left corner).

#### view:resize(width, height)

Resizes the view.

#### view:show()

Unhides the view.

#### view:size()

Returns the size of the view.

#### view:tiles(edges)

Takes a table containing all edges that are tiled, or a bool to indicate all 4 edges.

### Events

#### destroy

The view is being destroyed.
Callback receives the view.
