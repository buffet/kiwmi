mod input;
mod output;

use log::{info, warn, LevelFilter};

use wlroots::{
    compositor,
    cursor::{self, xcursor, Cursor},
    input::keyboard,
    output::layout,
    utils::log::Logger,
};

struct ExCursor;
impl cursor::Handler for ExCursor {}

struct ExOutputLayout;
impl layout::Handler for ExOutputLayout {}

struct CompositorState {
    pub xcursor_manager: xcursor::Manager,
    pub cursor_handle: cursor::Handle,
    pub layout_handle: layout::Handle,
    pub keyboards: Vec<keyboard::Handle>,
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
            keyboards: Vec::new(),
        }
    }
}

fn main() {
    Logger::init(LevelFilter::Debug, None);
    info!("Logger initialized");

    build_compositor().run();
}

fn build_compositor() -> compositor::Compositor {
    compositor::Builder::new()
        .gles2(true)
        .data_device(true)
        .input_manager(input::manager())
        .output_manager(output::manager())
        .build_auto(CompositorState::new())
}
