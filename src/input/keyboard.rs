use crate::CompositorState;

use log::debug;

use wlroots::{
    compositor,
    input::{self, keyboard},
    wlroots_dehandle,
    xkbcommon::xkb::{keysym_get_name, keysyms},
    WLR_KEY_PRESSED,
};

use std::{process::Command, thread};

pub struct Keyboard;

impl input::keyboard::Handler for Keyboard {
    #[wlroots_dehandle(compositor, seat, keyboard)]
    fn on_key(
        &mut self,
        compositor_handle: compositor::Handle,
        keyboard_handle: keyboard::Handle,
        key_event: &keyboard::event::Key,
    ) {
        use compositor_handle as compositor;

        if key_event.key_state() == WLR_KEY_PRESSED {
            for key in key_event.pressed_keys() {
                debug!("Key down: {}", keysym_get_name(key));

                match key {
                    keysyms::KEY_Escape => compositor::terminate(),
                    keysyms::KEY_F1 => {
                        thread::spawn(move || {
                            Command::new("weston-terminal").output().unwrap();
                        });
                    }
                    keysyms::KEY_XF86Switch_VT_1..=keysyms::KEY_XF86Switch_VT_12 => {
                        let backend = compositor.backend_mut();
                        if let Some(mut session) = backend.get_session() {
                            session.change_vt(key - keysyms::KEY_XF86Switch_VT_1 + 1);
                        }
                    }
                    _ => {}
                }
            }
        } else {
            for key in key_event.pressed_keys() {
                debug!("Key up: {}", keysym_get_name(key));
            }
        }

        let state: &mut CompositorState = compositor.downcast();
        let seat_handle = state.seat_handle.clone().unwrap();
        use seat_handle as seat;
        use keyboard_handle as keyboard;
        seat.keyboard_send_modifiers(&mut keyboard.get_modifier_masks());
        seat.keyboard_notify_key(
            key_event.time_msec(),
            key_event.keycode(),
            key_event.key_state() as u32,
        );
    }
}
