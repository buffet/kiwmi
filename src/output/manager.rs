use crate::{output::Output, CompositorState};

use log::debug;
use wlroots::{compositor, output, wlroots_dehandle};

pub fn build() -> output::manager::Builder {
    output::manager::Builder::default().output_added(output_added)
}

#[wlroots_dehandle(compositor, layout, cursor, output)]
fn output_added<'output>(
    compositor_handle: compositor::Handle,
    output_builder: output::Builder<'output>,
) -> Option<output::BuilderResult<'output>> {
    debug!("Output added");

    let result = output_builder.build_best_mode(Output);

    {
        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.data.downcast_mut().unwrap();

        let xcursor_manager = &mut state.xcursor_manager;
        let layout_handle = &state.layout_handle;
        let cursor_handle = &state.cursor_handle;
        let output_handle = &result.output;

        use cursor_handle as cursor;
        use layout_handle as layout;
        use output_handle as output;

        layout.add_auto(output);
        cursor.attach_output_layout(layout);
        xcursor_manager.load(output.scale());
        xcursor_manager.set_cursor_image("left_ptr".to_string(), cursor);
        let (x, y) = cursor.coords();
        cursor.warp(None, x, y);
    }

    Some(result)
}
