use crate::CompositorState;

use wlroots::{compositor, input::pointer, wlroots_dehandle};

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

    // TODO: implement on_button
}
