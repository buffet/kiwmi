use crate::CompositorState;

use wlroots::{compositor, input::pointer, wlroots_dehandle, shell::xdg_shell_v6};

pub struct Pointer;

impl pointer::Handler for Pointer {
    #[wlroots_dehandle(compositor, cursor)]
    fn on_motion_absolute(
        &mut self,
        compositor_handle: compositor::Handle,
        _pointer_handle: pointer::Handle,
        absolute_motion_event: &pointer::event::AbsoluteMotion,
    ) {
        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.downcast();
        let (x, y) = absolute_motion_event.pos();

        let cursor_handle = &state.cursor_handle;
        use cursor_handle as cursor;
        cursor.warp_absolute(absolute_motion_event.device(), x, y);
    }

    #[wlroots_dehandle(compositor, cursor)]
    fn on_motion(
        &mut self,
        compositor_handle: compositor::Handle,
        _pointer_handle: pointer::Handle,
        motion_event: &pointer::event::Motion,
    ) {
        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.downcast();
        let (dx, dy) = motion_event.delta();

        let cursor_handle = &state.cursor_handle;
        use cursor_handle as cursor;
        cursor.move_to(None, dx, dy);
    }

    #[wlroots_dehandle(compositor, shell, keyboard, seat, surface)]
    fn on_button(
        &mut self,
        compositor_handle: compositor::Handle,
        _pointer_handle: pointer::Handle,
        _: &pointer::event::Button,
    ) {
        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.downcast();

        let shell_handle = &state.shells[0];
        let seat_handle = state.seat_handle.clone().unwrap();
        let keyboard_handle = &state.keyboards[0];

        use shell_handle as shell;
        match shell.state() {
            Some(&mut xdg_shell_v6::ShellState::TopLevel(ref mut toplevel)) => {
                toplevel.set_activated(true);
            }
            _ => {}
        };

        let surface_handle = shell.surface();
        use keyboard_handle as keyboard;
        use seat_handle as seat;
        use surface_handle as surface;

        seat.set_keyboard(keyboard.input_device());
        seat.keyboard_notify_enter(
            surface,
            &mut keyboard.keycodes(),
            &mut keyboard.get_modifier_masks(),
        );
    }
}
