use crate::CompositorState;

use log::debug;

use wlroots::{compositor, shell::xdg_shell_v6, surface, utils::Handleable, wlroots_dehandle};

pub fn build() -> xdg_shell_v6::manager::Builder {
    xdg_shell_v6::manager::Builder::default().surface_added(surface_added)
}

struct XdgShellV6Handler;

impl xdg_shell_v6::Handler for XdgShellV6Handler {
    #[wlroots_dehandle(compositor)]
    fn destroyed(
        &mut self,
        compositor_handle: compositor::Handle,
        shell_handle: xdg_shell_v6::Handle,
    ) {
        use compositor_handle as compositor;
        let state: &mut CompositorState = compositor.downcast();
        let weak = shell_handle;

        if let Some(index) = state.shells.iter().position(|s| *s == weak) {
            state.shells.remove(index);
        }
    }
}

struct Surface;

impl surface::Handler for Surface {
    fn on_commit(&mut self, _: compositor::Handle, surface_handle: surface::Handle) {
        debug!("Commiting for surface {:?}", surface_handle);
    }
}

#[wlroots_dehandle(compositor, shell, layout, output)]
fn surface_added(
    compositor_handle: compositor::Handle,
    shell_handle: xdg_shell_v6::Handle,
) -> (
    Option<Box<xdg_shell_v6::Handler>>,
    Option<Box<surface::Handler>>,
) {
    {
        use compositor_handle as compositor;
        use shell_handle as shell;
        shell.ping();
        let state: &mut CompositorState = compositor.downcast();
        state.shells.push(shell.weak_reference());
        log::warn!("shells.len(): {}", state.shells.len());
        let layout_handle = &state.layout_handle;
        use layout_handle as layout;
        for (output_handle, _) in layout.outputs() {
            use output_handle as output;
            output.schedule_frame()
        }
    }

    (Some(Box::new(XdgShellV6Handler)), Some(Box::new(Surface)))
}
