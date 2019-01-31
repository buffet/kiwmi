use crate::CompositorState;

use wlroots::{
    area::{Area, Origin, Size},
    compositor, output,
    render::{matrix, Renderer},
    utils::current_time,
    wlroots_dehandle,
};

pub struct Output;

impl output::Handler for Output {
    #[wlroots_dehandle(compositor, output)]
    fn on_frame(&mut self, compositor_handle: compositor::Handle, output_handle: output::Handle) {
        use compositor_handle as compositor;
        use output_handle as output;
        let state: &mut CompositorState = compositor.data.downcast_mut().unwrap();
        let renderer = compositor
            .renderer
            .as_mut()
            .expect("Compositor was not loaded with a renderer");
        let mut render_context = renderer.render(output, None);
        render_context.clear([0.45, 0.25, 0.25, 1.0]);
        render_shells(state, &mut render_context)
    }
}

#[wlroots_dehandle(shell, surface, layout)]
fn render_shells(state: &mut CompositorState, renderer: &mut Renderer) {
    let shells = state.shells.clone();
    let layout_handle = &state.layout_handle;
    for shell_handle in shells {
        use shell_handle as shell;
        let surface_handle = shell.surface();
        use layout_handle as layout;
        use surface_handle as surface;
        let (width, height) = surface.current_state().size();
        let (render_width, render_height) = (
            width * renderer.output.scale() as i32,
            height * renderer.output.scale() as i32,
        );
        let (lx, ly) = (0.0, 0.0);
        let render_box = Area::new(
            Origin::new(lx as i32, ly as i32),
            Size::new(render_width, render_height),
        );
        if layout.intersects(renderer.output, render_box) {
            let transform = renderer.output.get_transform().invert();
            let matrix = matrix::project_box(
                render_box,
                transform,
                0.0,
                renderer.output.transform_matrix(),
            );
            if let Some(texture) = surface.texture().as_ref() {
                renderer.render_texture_with_matrix(texture, matrix);
            }
            surface.send_frame_done(current_time());
        }
    }
}
