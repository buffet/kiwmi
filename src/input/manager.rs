use crate::input::Keyboard;

use wlroots::{
    compositor,
    input::{self, keyboard},
};

pub fn manager() -> input::manager::Builder {
    input::manager::Builder::default().keyboard_added(keyboard_added)
}

fn keyboard_added(
    _compositor_handle: compositor::Handle,
    _keyboard_handle: keyboard::Handle,
) -> Option<Box<keyboard::Handler>> {
    Some(Box::new(Keyboard))
}
