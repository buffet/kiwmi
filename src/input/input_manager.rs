use crate::input::Keyboard;

use wlroots::input;

pub fn input_manager() -> input::manager::Builder {
    input::manager::Builder::default()
        .keyboard_added(|_compositor_handle, _keyboard_handle| Some(Box::new(Keyboard)))
}
