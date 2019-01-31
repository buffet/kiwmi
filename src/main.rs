mod input;
mod output;
mod shells;

use log::{info, warn, LevelFilter};

use wlroots::{
    compositor,
    cursor::{self, xcursor, Cursor},
    input::keyboard,
    output::layout,
    seat::{self, Seat},
    shell::xdg_shell_v6,
    utils::log::Logger,
};

struct ExCursor;
impl cursor::Handler for ExCursor {}

struct ExOutputLayout;
impl layout::Handler for ExOutputLayout {}

struct CompositorState {
    xcursor_manager: xcursor::Manager,
    cursor_handle: cursor::Handle,
    layout_handle: layout::Handle,
    keyboards: Vec<keyboard::Handle>,
    shells: Vec<xdg_shell_v6::Handle>,
    seat_handle: Option<seat::Handle>,
}

impl CompositorState {
    pub fn new() -> Self {
        let mut xcursor_manager = xcursor::Manager::create(String::from("default"), 24)
            .expect("Could not create xcursor manager");

        if xcursor_manager.load(1.0) {
            warn!("Cursor did not load");
        }

        let cursor_handle = Cursor::create(Box::new(ExCursor));
        cursor_handle
            .run(|c| xcursor_manager.set_cursor_image(String::from("left_ptr"), c))
            .unwrap();

        let layout_handle = layout::Layout::create(Box::new(ExOutputLayout));

        CompositorState {
            xcursor_manager: xcursor_manager,
            cursor_handle: cursor_handle,
            layout_handle: layout_handle,
            keyboards: vec![],
            shells: vec![],
            seat_handle: None,
        }
    }
}

fn main() {
    Logger::init(LevelFilter::Debug, None);
    info!("Logger initialized");

    build_compositor().run();
}

struct SeatHandler;
impl seat::Handler for SeatHandler {}

fn build_compositor() -> compositor::Compositor {
    let mut compositor = compositor::Builder::new()
        .gles2(true)
        .wl_shm(true)
        .data_device(true)
        .input_manager(input::build())
        .output_manager(output::build())
        .xdg_shell_v6_manager(shells::xdg_v6::build())
        .build_auto(CompositorState::new());

    let seat_handle = Seat::create(
        &mut compositor,
        String::from("seat0"),
        Box::new(SeatHandler),
    );

    seat_handle
        .run(|seat| seat.set_capabilities(seat::Capability::all()))
        .unwrap();

    let state: &mut CompositorState = compositor.downcast();
    state.seat_handle = Some(seat_handle);

    compositor
}
