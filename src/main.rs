mod input;
mod logger;

use log::LevelFilter;

use wlroots::compositor;

fn main() {
    logger::init(LevelFilter::Info);

    build_compositor().run();
}

fn build_compositor() -> compositor::Compositor {
    compositor::Builder::new()
        .gles2(true)
        .data_device(true)
        .input_manager(input::manager())
        .build_auto(())
}
