use crate::CompositorState;

use wlroots::{compositor, input::pointer, with_handles};

pub struct Pointer;

impl pointer::Handler for Pointer {
    fn on_motion_absolute(
        &mut self,
        compositor_handle: compositor::Handle,
        _pointer_handle: pointer::Handle,
        absolute_motion_event: &pointer::event::AbsoluteMotion,
    ) {
        with_handles!([(compositor: {compositor_handle})] => {
            let compositor_state: &mut CompositorState = compositor.downcast();
            let (x, y) = absolute_motion_event.pos();

            compositor_state.cursor_handle
                .run(|cursor| cursor.warp_absolute(absolute_motion_event.device(), x, y))
                .unwrap();
        })
        .unwrap();
    }

    fn on_motion(
        &mut self,
        compositor_handle: compositor::Handle,
        _pointer_handle: pointer::Handle,
        motion_event: &pointer::event::Motion,
    ) {
        with_handles!([(compositor: {compositor_handle})] => {
            let compositor_state: &mut CompositorState = compositor.downcast();
            let (dx, dy) = motion_event.delta();

            compositor_state.cursor_handle
                .run(|cursor| cursor.move_to(None, dx, dy))
                .unwrap();
        })
        .unwrap();
    }
}
