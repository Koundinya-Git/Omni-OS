use anyhow::{Context, Result};
use chrono::{Duration, Utc};
use image_hasher::HasherConfig;
use rusqlite::{params, Connection};
use serde::Deserialize;
use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;
use std::process::Command;
use std::sync::{Arc, Mutex};
use std::time::Duration as StdDuration;
use tar::Builder;
use tokio::time;
use xz2::write::XzEncoder;

#[derive(Debug, Deserialize)]
struct HyprlandWindow {
    class: Option<String>,
    title: Option<String>,
}

fn get_tier() -> u32 {
    if let Ok(content) = fs::read_to_string("/etc/omni/hw-tier") {
        if let Ok(tier) = content.trim().parse::<u32>() {
            return tier;
        }
    }
    0
}

fn get_interval_from_tier(tier: u32) -> StdDuration {
    if tier >= 15 {
        StdDuration::from_secs(2)
    } else if tier >= 10 {
        StdDuration::from_secs(10)
    } else {
        StdDuration::from_secs(60)
    }
}

fn get_omni_dir() -> PathBuf {
    if let Ok(home) = std::env::var("HOME") {
        let mut path = PathBuf::from(home);
        path.push(".local");
        path.push("share");
        path.push("omni");
        path
    } else if let Some(mut dir) = dirs::data_local_dir() {
        // Fallback for Windows/macOS if HOME is not set
        dir.push("omni");
        dir
    } else {
        PathBuf::from(".local/share/omni")
    }
}

fn init_db(conn: &Connection) -> Result<()> {
    conn.execute(
        "CREATE TABLE IF NOT EXISTS recall_frames (
            id INTEGER PRIMARY KEY,
            timestamp INTEGER,
            window_class TEXT,
            window_title TEXT,
            image_path TEXT
        )",
        [],
    )?;
    
    let _ = conn.execute("ALTER TABLE recall_frames ADD COLUMN ocr_text TEXT", []);
    let _ = conn.execute("ALTER TABLE recall_frames ADD COLUMN end_timestamp INTEGER", []);
    
    conn.execute(
        "CREATE TABLE IF NOT EXISTS starred_spans (
            id INTEGER PRIMARY KEY,
            start_time INTEGER,
            end_time INTEGER,
            title TEXT
        )",
        [],
    )?;
    
    Ok(())
}

async fn capture_loop(db_conn: Arc<Mutex<Connection>>, frames_dir: PathBuf, interval: StdDuration) {
    let mut ticker = time::interval(interval);
    let hasher = HasherConfig::new().to_hasher();
    let mut last_hash: Option<image_hasher::ImageHash> = None;
    let mut last_id: Option<i64> = None;
    
    loop {
        ticker.tick().await;
        
        let now = Utc::now().timestamp();
        
        // 1. Get active window metadata
        let mut w_class = String::new();
        let mut w_title = String::new();
        
        if let Ok(output) = Command::new("hyprctl")
            .arg("activewindow")
            .arg("-j")
            .output()
        {
            if output.status.success() {
                if let Ok(window_info) = serde_json::from_slice::<HyprlandWindow>(&output.stdout) {
                    w_class = window_info.class.unwrap_or_default();
                    w_title = window_info.title.unwrap_or_default();
                }
            }
        }
        
        // 2. Capture screenshot
        let mut image_path = frames_dir.clone();
        image_path.push(format!("{}.png", now));
        
        if let Some(path_str) = image_path.to_str() {
            let capture_status = Command::new("grim")
                .arg("-c")
                .arg(path_str)
                .status();
                
            if let Ok(status) = capture_status {
                if status.success() {
                    let is_duplicate = if let Ok(img) = image::open(path_str) {
                        let current_hash = hasher.hash_image(&img);
                        let duplicate = if let Some(last) = &last_hash {
                            current_hash.dist(last) <= 1
                        } else {
                            false
                        };
                        
                        if !duplicate {
                            last_hash = Some(current_hash);
                        }
                        duplicate
                    } else {
                        false
                    };

                    if is_duplicate {
                        let _ = fs::remove_file(path_str);
                        if let Some(id) = last_id {
                            if let Ok(conn) = db_conn.lock() {
                                let _ = conn.execute(
                                    "UPDATE recall_frames SET end_timestamp = ?1 WHERE id = ?2",
                                    params![now, id],
                                );
                            }
                        }
                    } else {
                        if let Ok(conn) = db_conn.lock() {
                            if let Ok(_) = conn.execute(
                                "INSERT INTO recall_frames (timestamp, window_class, window_title, image_path, end_timestamp) VALUES (?1, ?2, ?3, ?4, ?5)",
                                params![now, w_class, w_title, path_str, now],
                            ) {
                                last_id = Some(conn.last_insert_rowid());
                            }
                        }
                    }
                }
            }
        }
    }
}

async fn ocr_loop(db_conn: Arc<Mutex<Connection>>) {
    let mut ticker = time::interval(StdDuration::from_secs(10));
    let client = reqwest::Client::new();
    
    loop {
        ticker.tick().await;
        
        let row_to_process = {
            if let Ok(conn) = db_conn.lock() {
                if let Ok(mut stmt) = conn.prepare("SELECT id, image_path FROM recall_frames WHERE ocr_text IS NULL LIMIT 1") {
                    if let Ok(mut rows) = stmt.query([]) {
                        if let Ok(Some(row)) = rows.next() {
                            let id: i64 = row.get(0).unwrap_or(0);
                            let path: String = row.get(1).unwrap_or_default();
                            Some((id, path))
                        } else {
                            None
                        }
                    } else { None }
                } else { None }
            } else { None }
        };
        
        if let Some((id, path)) = row_to_process {
            if path.ends_with(".png") && std::path::Path::new(&path).exists() {
                if let Ok(output) = Command::new("tesseract").arg(&path).arg("stdout").output() {
                    if output.status.success() {
                        let text = String::from_utf8_lossy(&output.stdout).trim().to_string();
                        if !text.is_empty() {
                            let mut map = HashMap::new();
                            map.insert("text", &text);
                            let _ = client.post("http://localhost:8000/api/embed")
                                .json(&map)
                                .send()
                                .await;
                                
                            if let Ok(conn) = db_conn.lock() {
                                let _ = conn.execute(
                                    "UPDATE recall_frames SET ocr_text = ?1 WHERE id = ?2",
                                    params![text, id],
                                );
                            }
                        } else {
                             if let Ok(conn) = db_conn.lock() {
                                let _ = conn.execute(
                                    "UPDATE recall_frames SET ocr_text = '' WHERE id = ?1",
                                    params![id],
                                );
                            }
                        }
                    } else {
                        if let Ok(conn) = db_conn.lock() {
                            let _ = conn.execute(
                                "UPDATE recall_frames SET ocr_text = '' WHERE id = ?1",
                                params![id],
                            );
                        }
                    }
                } else {
                    if let Ok(conn) = db_conn.lock() {
                        let _ = conn.execute(
                            "UPDATE recall_frames SET ocr_text = '' WHERE id = ?1",
                            params![id],
                        );
                    }
                }
            } else {
                if let Ok(conn) = db_conn.lock() {
                    let _ = conn.execute(
                        "UPDATE recall_frames SET ocr_text = '' WHERE id = ?1",
                        params![id],
                    );
                }
            }
        }
    }
}

async fn retention_loop(db_conn: Arc<Mutex<Connection>>, omni_dir: PathBuf) {
    let mut ticker = time::interval(StdDuration::from_secs(3600)); // Every hour
    
    loop {
        ticker.tick().await;
        
        let cutoff = (Utc::now() - Duration::days(7)).timestamp();
        let mut frames_to_delete = Vec::new();
        let mut frames_to_compress = Vec::new(); // (id, image_path, span_id)
        
        if let Ok(conn) = db_conn.lock() {
            let query = "
                SELECT r.id, r.image_path,
                (SELECT s.id FROM starred_spans s WHERE r.timestamp >= s.start_time AND r.timestamp <= s.end_time LIMIT 1) as span_id
                FROM recall_frames r
                WHERE r.timestamp < ?1
            ";
            
            if let Ok(mut stmt) = conn.prepare(query) {
                if let Ok(mut rows) = stmt.query(params![cutoff]) {
                    while let Ok(Some(row)) = rows.next() {
                        let id: i64 = row.get(0).unwrap_or(0);
                        let path: String = row.get(1).unwrap_or_default();
                        
                        let span_id_val: rusqlite::Result<i64> = row.get(2);
                        
                        if let Ok(s_id) = span_id_val {
                            if path.ends_with(".png") {
                                frames_to_compress.push((id, path, s_id));
                            }
                        } else {
                            frames_to_delete.push((id, path));
                        }
                    }
                }
            }
        }
        
        for (id, path) in frames_to_delete {
            let _ = fs::remove_file(&path);
            if let Ok(conn) = db_conn.lock() {
                let _ = conn.execute("DELETE FROM recall_frames WHERE id = ?1", params![id]);
            }
        }
        
        let mut compress_groups: HashMap<i64, Vec<(i64, String)>> = HashMap::new();
        for (id, path, span_id) in frames_to_compress {
            compress_groups.entry(span_id).or_insert_with(Vec::new).push((id, path));
        }
        
        for (span_id, files) in compress_groups {
            let mut archive_dir = omni_dir.clone();
            archive_dir.push("starred");
            let _ = fs::create_dir_all(&archive_dir);
            archive_dir.push(format!("{}.tar.xz", span_id));
            let archive_path_str = archive_dir.to_str().unwrap().to_string();
            
            let mut should_update_db = false;
            
            if let Ok(tar_xz_file) = fs::OpenOptions::new().create(true).append(true).open(&archive_path_str) {
                let enc = XzEncoder::new(tar_xz_file, 9);
                let mut builder = Builder::new(enc);
                
                let mut success = true;
                for (_, path) in &files {
                    if let Ok(mut f) = std::fs::File::open(path) {
                        let filename = std::path::Path::new(path).file_name().unwrap();
                        if let Err(_) = builder.append_file(filename, &mut f) {
                            success = false;
                        }
                    }
                }
                if builder.into_inner().is_ok() {
                    should_update_db = success;
                } else {
                    should_update_db = false;
                }
            }
            
            if should_update_db {
                if let Ok(conn) = db_conn.lock() {
                    for (id, path) in files {
                        let _ = conn.execute("UPDATE recall_frames SET image_path = ?1 WHERE id = ?2", params![archive_path_str, id]);
                        let _ = fs::remove_file(path);
                    }
                }
            }
        }
    }
}

#[tokio::main]
async fn main() -> Result<()> {
    let tier = get_tier();
    let interval = get_interval_from_tier(tier);
    
    let omni_dir = get_omni_dir();
    fs::create_dir_all(&omni_dir).context("Failed to create omni directory")?;
    
    let mut frames_dir = omni_dir.clone();
    frames_dir.push("recall-frames");
    fs::create_dir_all(&frames_dir).context("Failed to create recall-frames directory")?;
    
    let mut db_path = omni_dir.clone();
    db_path.push("recall.db");
    
    let conn = Connection::open(&db_path).context("Failed to open SQLite database")?;
    init_db(&conn).context("Failed to initialize database schemas")?;
    
    let db_conn = Arc::new(Mutex::new(conn));
    
    let capture_db = Arc::clone(&db_conn);
    let capture_task = tokio::spawn(async move {
        capture_loop(capture_db, frames_dir, interval).await;
    });
    
    let retention_db = Arc::clone(&db_conn);
    let retention_omni_dir = omni_dir.clone();
    let retention_task = tokio::spawn(async move {
        retention_loop(retention_db, retention_omni_dir).await;
    });
    
    let ocr_db = Arc::clone(&db_conn);
    let ocr_task = tokio::spawn(async move {
        ocr_loop(ocr_db).await;
    });
    
    // Run all tasks concurrently
    let _ = tokio::try_join!(capture_task, retention_task, ocr_task);
    
    Ok(())
}
