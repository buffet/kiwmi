use log::debug;

use wlroots::{
    compositor,
    input::{self, keyboard},
    xkbcommon::xkb::{keysyms, keysym_get_name},
    WLR_KEY_PRESSED,
};

pub struct Keyboard;

impl input::keyboard::Handler for Keyboard {
    fn on_key(
        &mut self,
        compositor_handle: compositor::Handle,
        _keyboard_handle: keyboard::Handle,
        key_event: &keyboard::event::Key,
    ) {
        for key in key_event.pressed_keys() {
            if key_event.key_state() == WLR_KEY_PRESSED {
                debug!("Key down: {}", keysym_get_name(key));
                match key {
                    keysyms::KEY_Escape => compositor::terminate(),
                    keysyms::KEY_XF86Switch_VT_1..=keysyms::KEY_XF86Switch_VT_12 => {
                        compositor_handle
                            .run(|compositor| {
                                let backend = compositor.backend_mut();
                                if let Some(mut session) = backend.get_session() {
                                    session.change_vt(key - keysyms::KEY_XF86Switch_VT_1 + 1);
                                }
                            })
                            .unwrap();
                    }
                    _ => {}
                }
            } else {
                debug!("Key up: {}", keysym_get_name(key));
            }
        }
    }
}
