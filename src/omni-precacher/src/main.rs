use rusqlite::Connection;
use chrono::{Local, Datelike, Timelike, TimeZone};
use std::{collections::HashMap, process::Command, fs, path::Path, thread, time::Duration};

fn get_total_ram_gb() -> f64 {
    if let Ok(content) = fs::read_to_string("/proc/meminfo") {
        for line in content.lines() {
            if line.starts_with("MemTotal:") {
                let parts: Vec<&str> = line.split_whitespace().collect();
                if parts.len() >= 2 {
                    if let Ok(kb) = parts[1].parse::<f64>() {
                        return kb / (1024.0 * 1024.0);
                    }
                }
            }
        }
    }
    8.0
}

fn get_predicted_apps(db_path: &str, limit: usize) -> Vec<String> {
    let now = Local::now();
    let current_dow = now.weekday().number_from_monday();
    let current_hour = now.hour();

    let conn = match Connection::open(db_path) {
        Ok(c) => c,
        Err(_) => return vec![],
    };

    let mut stmt = match conn.prepare("SELECT timestamp, app_name FROM telemetry_log") {
        Ok(s) => s,
        Err(_) => return vec![],
    };

    let rows = stmt.query_map([], |row| {
        Ok((row.get::<_, i64>(0)?, row.get::<_, String>(1)?))
    });

    let mut freq = HashMap::new();

    if let Ok(iter) = rows {
        for row in iter.flatten() {
            let (ts, app) = row;
            if app.is_empty() || app == "null" {
                continue;
            }
            if let Some(dt) = Local.timestamp_opt(ts, 0).single() {
                if dt.weekday().number_from_monday() == current_dow && (dt.hour() as i32 - current_hour as i32).abs() <= 1 {
                    *freq.entry(app).or_insert(0) += 1;
                }
            }
        }
    }

    let mut sorted_apps: Vec<_> = freq.into_iter().collect();
    sorted_apps.sort_by(|a, b| b.1.cmp(&a.1));
    
    sorted_apps.into_iter().take(limit).map(|(app, _)| app).collect()
}

fn vmtouch_target(path: &str) {
    if Path::new(path).exists() {
        let _ = Command::new("vmtouch").args(["-t", path]).output();
    }
}

fn precache_app(app_name: &str, deep_cache: bool) {
    if let Ok(out) = Command::new("which").arg(&app_name.to_lowercase()).output() {
        if out.status.success() {
            let bin = String::from_utf8_lossy(&out.stdout).trim().to_string();
            vmtouch_target(&bin);
        }
    }

    if deep_cache {
        if let Some(home) = std::env::var_os("HOME") {
            let home_str = home.to_string_lossy();
            let app_lower = app_name.to_lowercase();
            
            let conf_dir = format!("{}/.config/{}", home_str, app_lower);
            if Path::new(&conf_dir).is_dir() { vmtouch_target(&conf_dir); }
            
            let share_dir = format!("{}/.local/share/{}", home_str, app_lower);
            if Path::new(&share_dir).is_dir() { vmtouch_target(&share_dir); }
        }
    }
}

fn main() {
    let home = std::env::var("HOME").unwrap_or_else(|_| "/root".to_string());
    let db_path = format!("{}/.local/share/omni/telemetry.db", home);
    
    let ram_gb = get_total_ram_gb();
    
    let (limit, deep) = if ram_gb < 8.0 {
        (2, false)
    } else if ram_gb < 32.0 {
        (5, false)
    } else {
        (5, true)
    };

    println!("[omni-precacher] Starting up... (RAM: {:.1}GB, Limit: {}, Deep Cache: {})", ram_gb, limit, deep);

    loop {
        if Path::new(&db_path).exists() {
            for app in get_predicted_apps(&db_path, limit) {
                precache_app(&app, deep);
            }
        }
        thread::sleep(Duration::from_secs(1800));
    }
}
