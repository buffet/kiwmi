use crate::CompositorState;

use log::debug;
use wlroots::{compositor, output, with_handles};

pub fn build() -> output::manager::Builder {
    output::manager::Builder::default().output_added(output_added)
}

struct Output;

impl output::Handler for Output {}

fn output_added<'output>(
    compositor_handle: compositor::Handle,
    output_builder: output::Builder<'output>,
) -> Option<output::BuilderResult<'output>> {
    debug!("Output added");

    let mut result = output_builder.build_best_mode(Output);
    with_handles!([(compositor: {compositor_handle})] => {
        let compositor_state: &mut CompositorState = compositor.data.downcast_mut().unwrap();
        let layout_handle = &mut compositor_state.layout_handle;
        let cursor_handle = &mut compositor_state.cursor_handle;
        let xcursor_manager = &mut compositor_state.xcursor_manager;
        // TODO use output config if present instead of auto
        with_handles!([
            (layout: {layout_handle}),
            (cursor: {cursor_handle}),
            (output: {&mut result.output})
        ] => {
            layout.add_auto(output);
            cursor.attach_output_layout(layout);
            xcursor_manager.load(output.scale());
            xcursor_manager.set_cursor_image("left_ptr".to_string(), cursor);
            let (x, y) = cursor.coords();
            cursor.warp(None, x, y);
        }).unwrap();
        Some(result)
    })
    .unwrap()
}
