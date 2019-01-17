use log::{info, max_level, set_boxed_logger, set_max_level, Level, LevelFilter, Metadata, Record};

use wlroots::{
    utils::log::{init_logging, WLR_DEBUG, WLR_ERROR, WLR_INFO, WLR_SILENT},
    wlroots_sys::_wlr_log,
};

use std::ffi::CString;

pub fn init(level: LevelFilter) {
    init_logging(
        match level {
            LevelFilter::Off => WLR_SILENT,
            LevelFilter::Warn | LevelFilter::Error => WLR_ERROR,
            LevelFilter::Info => WLR_INFO,
            LevelFilter::Debug | LevelFilter::Trace => WLR_DEBUG,
        },
        None,
    );

    let _ = set_boxed_logger(Box::new(Logger)).map(|_| set_max_level(level));

    info!("Logger initialized!");
}

struct Logger;

impl log::Log for Logger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        metadata.level() <= max_level()
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            let wlr_level = match record.level() {
                Level::Warn | Level::Error => WLR_ERROR,
                Level::Info => WLR_INFO,
                Level::Debug | Level::Trace => WLR_DEBUG,
            };

            let msg = CString::new(if let Some(file) = record.file() {
                if let Some(line) = record.line() {
                    format!("[{}:{}] {}", file, line, record.args())
                } else {
                    format!("[{}] {}", file, record.args())
                }
            } else {
                format!("{}", record.args())
            })
            .expect("Could not convert log message to CString");

            unsafe {
                _wlr_log(wlr_level, msg.as_ptr());
            }
        }
    }

    fn flush(&self) {}
}
