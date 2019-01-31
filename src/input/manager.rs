use crate::{
    input::{Keyboard, Pointer},
    CompositorState,
};

use log::debug;

use wlroots::{
    compositor,
    input::{self, keyboard, pointer},
    wlroots_dehandle,
};

pub fn build() -> input::manager::Builder {
    input::manager::Builder::default()
        .keyboard_added(keyboard_added)
        .pointer_added(pointer_added)
}

#[wlroots_dehandle(compositor, keyboard, seat)]
fn keyboard_added(
    compositor_handle: compositor::Handle,
    keyboard_handle: keyboard::Handle,
) -> Option<Box<keyboard::Handler>> {
    {
        debug!("Keyboard added");

        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.data.downcast_mut().unwrap();

        {
            let seat_handle = state.seat_handle.as_ref().unwrap();
            use keyboard_handle as keyboard;
            use seat_handle as seat;
            seat.set_keyboard(keyboard.input_device());
        }

        state.keyboards.push(keyboard_handle);
    }

    Some(Box::new(Keyboard))
}

#[wlroots_dehandle(compositor, pointer, cursor)]
fn pointer_added(
    compositor_handle: compositor::Handle,
    pointer_handle: pointer::Handle,
) -> Option<Box<pointer::Handler>> {
    {
        debug!("Pointer added");

        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.downcast();

        let cursor_handle = &state.cursor_handle;
        use cursor_handle as cursor;
        use pointer_handle as pointer;
        cursor.attach_input_device(pointer.input_device());
    }

    Some(Box::new(Pointer))
}
