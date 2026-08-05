use std::env;
use std::io::{BufRead, BufReader};
use std::os::unix::net::UnixStream;
use std::process::Command;
use serde::Deserialize;

#[derive(Deserialize, Debug)]
struct Client {
    workspace: Workspace,
    class: String,
}

#[derive(Deserialize, Debug)]
struct Workspace {
    id: i32,
}

fn main() {
    let sig = env::var("HYPRLAND_INSTANCE_SIGNATURE").expect("HYPRLAND_INSTANCE_SIGNATURE not set");
    let socket_path = format!("/tmp/hypr/{}/.socket2.sock", sig);

    let stream = match UnixStream::connect(&socket_path) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Failed to connect to socket {}: {}", socket_path, e);
            return;
        }
    };
    let reader = BufReader::new(stream);

    for line in reader.lines() {
        let line = match line {
            Ok(l) => l,
            Err(_) => continue,
        };
        // Listen for window open or move events to re-check the layout
        if line.starts_with("openwindow>>") || line.starts_with("movewindow>>") {
            check_and_snap();
        }
    }
}

fn check_and_snap() {
    let output = match Command::new("hyprctl")
        .args(&["clients", "-j"])
        .output() {
        Ok(out) => out,
        Err(_) => return,
    };

    let clients: Vec<Client> = serde_json::from_slice(&output.stdout).unwrap_or_else(|_| vec![]);

    // Group clients by workspace id
    let mut workspaces = std::collections::HashMap::new();
    for client in clients {
        workspaces.entry(client.workspace.id).or_insert_with(Vec::new).push(client.class.to_lowercase());
    }

    for (_ws_id, classes) in workspaces {
        let has_ide = classes.iter().any(|c| c.contains("vscode") || c.contains("code") || c.contains("kitty") || c.contains("alacritty"));
        let has_browser = classes.iter().any(|c| c.contains("firefox") || c.contains("chrome") || c.contains("chromium") || c.contains("brave"));

        if has_ide && has_browser {
            let _ = Command::new("hyprctl")
                .args(&["dispatch", "layoutmsg", "mfact 0.6"])
                .output();
        }
    }
}
