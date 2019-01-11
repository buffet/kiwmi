mod input;

use log::info;

use wlroots::{
    compositor,
    utils::log::{init_logging, WLR_INFO},
};

fn main() {
    init_logs();

    build_compositor().run();
}

fn init_logs() {
    init_logging(WLR_INFO, None);

    env_logger::Builder::from_env(env_logger::Env::default().filter("KIWMI_LOG")).init();

    info!("Logger initialized!");
}

fn build_compositor() -> compositor::Compositor {
    compositor::Builder::new()
        .gles2(true)
        .data_device(true)
        .input_manager(input::manager())
        .build_auto(())
}
