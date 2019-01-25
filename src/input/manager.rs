use crate::{
    input::{Keyboard, Pointer},
    CompositorState,
};

use log::debug;

use wlroots::{
    compositor,
    input::{self, keyboard, pointer},
    with_handles,
};

pub fn manager() -> input::manager::Builder {
    input::manager::Builder::default()
        .keyboard_added(keyboard_added)
        .pointer_added(pointer_added)
}

fn keyboard_added(
    compositor_handle: compositor::Handle,
    keyboard_handle: keyboard::Handle,
) -> Option<Box<keyboard::Handler>> {
    debug!("Keyboard added");

    with_handles!([(compositor: {compositor_handle})] => {
        let compositor_state: &mut CompositorState = compositor.data.downcast_mut().unwrap();
        compositor_state.keyboards.push(keyboard_handle);
    })
    .unwrap();
    Some(Box::new(Keyboard))
}

fn pointer_added(
    compositor_handle: compositor::Handle,
    pointer_handle: pointer::Handle,
) -> Option<Box<pointer::Handler>> {
    debug!("Pointer added");

    with_handles!([(compositor: {compositor_handle}), (pointer: {pointer_handle})] => {
        let compositor_state: &mut CompositorState = compositor.downcast();
        compositor_state.cursor_handle
            .run(|cursor| cursor.attach_input_device(pointer.input_device()))
            .unwrap();
    })
    .unwrap();
    Some(Box::new(Pointer))
}
