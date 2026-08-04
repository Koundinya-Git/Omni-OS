use chrono::Local;
use http_body_util::{BodyExt, Full};
use hyper::body::{Bytes, Incoming};
use hyper::server::conn::http1;
use hyper::service::service_fn;
use hyper::{Method, Request, Response, StatusCode};
use hyper_util::rt::TokioIo;
use serde::Deserialize;
use serde_json::json;
use std::env;
use std::fs::{self, OpenOptions};
use std::io::Write;
use std::net::SocketAddr;
use std::path::PathBuf;
use std::process::Stdio;
use tokio::io::{AsyncBufReadExt, BufReader};
use tokio::net::TcpListener;
use tokio::process::Command;

#[derive(Deserialize)]
#[serde(tag = "type", rename_all = "snake_case")]
enum Action {
    Install { packages: Vec<String> },
    Persona { style: String },
    Block { sites: Vec<String> },
}

async fn handle_request(
    req: Request<Incoming>,
) -> Result<Response<Full<Bytes>>, hyper::Error> {
    if req.method() != Method::POST || req.uri().path() != "/action" {
        return Ok(not_found());
    }

    let body_bytes = match req.into_body().collect().await {
        Ok(b) => b.to_bytes(),
        Err(_) => return Ok(bad_request("Failed to read body")),
    };

    let action: Action = match serde_json::from_slice(&body_bytes) {
        Ok(a) => a,
        Err(e) => return Ok(bad_request(&format!("Invalid JSON: {}", e))),
    };

    match action {
        Action::Install { packages } => handle_install(packages).await,
        Action::Persona { style } => handle_persona(style).await,
        Action::Block { sites } => handle_block(sites).await,
    }
}

async fn handle_install(packages: Vec<String>) -> Result<Response<Full<Bytes>>, hyper::Error> {
    log_action(&format!("Installing packages: {:?}", packages));

    if packages.is_empty() {
        return Ok(json_response("error", "No packages provided"));
    }

    let mut cmd = Command::new("paru");
    cmd.arg("-S").arg("--noconfirm").args(&packages);
    cmd.stdout(Stdio::piped()).stderr(Stdio::piped());

    let mut child = match cmd.spawn() {
        Ok(c) => c,
        Err(e) => {
            log_action(&format!("Failed to spawn paru: {}", e));
            return Ok(json_response("error", &format!("Failed to spawn paru: {}", e)));
        }
    };

    if let Some(stdout) = child.stdout.take() {
        let mut reader = BufReader::new(stdout).lines();
        tokio::spawn(async move {
            while let Ok(Some(line)) = reader.next_line().await {
                println!("[paru stdout] {}", line);
            }
        });
    }

    if let Some(stderr) = child.stderr.take() {
        let mut reader = BufReader::new(stderr).lines();
        tokio::spawn(async move {
            while let Ok(Some(line)) = reader.next_line().await {
                eprintln!("[paru stderr] {}", line);
            }
        });
    }

    match child.wait().await {
        Ok(status) if status.success() => {
            log_action("Install successful");
            Ok(json_response("ok", "Packages installed successfully"))
        }
        Ok(status) => {
            log_action(&format!("Install failed with status: {}", status));
            Ok(json_response("error", &format!("Install failed with status: {}", status)))
        }
        Err(e) => {
            log_action(&format!("Failed to wait for paru: {}", e));
            Ok(json_response("error", &format!("Wait error: {}", e)))
        }
    }
}

async fn handle_persona(style: String) -> Result<Response<Full<Bytes>>, hyper::Error> {
    log_action(&format!("Setting persona to: {}", style));

    let user = env::var("SUDO_USER")
        .or_else(|_| env::var("USER"))
        .unwrap_or_else(|_| get_first_real_user());

    if user.is_empty() {
        log_action("Could not determine target user for persona");
        return Ok(json_response("error", "Could not determine target user"));
    }

    let home = if user == "root" {
        PathBuf::from("/root")
    } else {
        PathBuf::from(format!("/home/{}", user))
    };

    let config_dir = home.join(".config").join("omni");
    if let Err(e) = fs::create_dir_all(&config_dir) {
        log_action(&format!("Failed to create config dir: {}", e));
        return Ok(json_response("error", &format!("Failed to create config dir: {}", e)));
    }

    let conf_path = config_dir.join("persona.conf");
    match fs::write(&conf_path, &style) {
        Ok(_) => {
            log_action("Persona set successfully");
            Ok(json_response("ok", "Persona configured"))
        }
        Err(e) => {
            log_action(&format!("Failed to write persona config: {}", e));
            Ok(json_response("error", &format!("Failed to write persona config: {}", e)))
        }
    }
}

async fn handle_block(sites: Vec<String>) -> Result<Response<Full<Bytes>>, hyper::Error> {
    log_action(&format!("Blocking sites: {:?}", sites));

    let mut file = match OpenOptions::new().append(true).create(true).open("/etc/hosts") {
        Ok(f) => f,
        Err(e) => {
            log_action(&format!("Failed to open /etc/hosts: {}", e));
            return Ok(json_response("error", &format!("Failed to open /etc/hosts: {}", e)));
        }
    };

    for site in sites {
        if let Err(e) = writeln!(file, "0.0.0.0 {}", site) {
            log_action(&format!("Failed to write to /etc/hosts: {}", e));
            return Ok(json_response("error", &format!("Failed to write to /etc/hosts: {}", e)));
        }
    }

    log_action("Sites blocked successfully");
    Ok(json_response("ok", "Sites blocked successfully"))
}

fn get_first_real_user() -> String {
    if let Ok(content) = fs::read_to_string("/etc/passwd") {
        for line in content.lines() {
            let parts: Vec<&str> = line.split(':').collect();
            if parts.len() >= 3 {
                if let Ok(uid) = parts[2].parse::<u32>() {
                    if uid >= 1000 && uid < 65534 {
                        return parts[0].to_string();
                    }
                }
            }
        }
    }
    String::new()
}

fn json_response(status: &str, message: &str) -> Response<Full<Bytes>> {
    let body = json!({
        "status": status,
        "message": message
    });
    Response::builder()
        .status(StatusCode::OK)
        .header("content-type", "application/json")
        .body(Full::new(Bytes::from(body.to_string())))
        .unwrap()
}

fn bad_request(msg: &str) -> Response<Full<Bytes>> {
    let body = json!({
        "status": "error",
        "message": msg
    });
    Response::builder()
        .status(StatusCode::BAD_REQUEST)
        .header("content-type", "application/json")
        .body(Full::new(Bytes::from(body.to_string())))
        .unwrap()
}

fn not_found() -> Response<Full<Bytes>> {
    let body = json!({
        "status": "error",
        "message": "Not Found"
    });
    Response::builder()
        .status(StatusCode::NOT_FOUND)
        .header("content-type", "application/json")
        .body(Full::new(Bytes::from(body.to_string())))
        .unwrap()
}

fn log_action(msg: &str) {
    let timestamp = Local::now().format("%Y-%m-%d %H:%M:%S");
    println!("[{}] {}", timestamp, msg);
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let addr = SocketAddr::from(([0, 0, 0, 0], 9191));
    let listener = TcpListener::bind(addr).await?;

    log_action(&format!("omni-action-bridge listening on {}", addr));

    loop {
        let (stream, _) = listener.accept().await?;
        let io = TokioIo::new(stream);

        tokio::task::spawn(async move {
            if let Err(err) = http1::Builder::new()
                .serve_connection(io, service_fn(handle_request))
                .await
            {
                eprintln!("Error serving connection: {:?}", err);
            }
        });
    }
}
