use std::fs;
use std::process::Command;
use std::time::{Duration, SystemTime};
use tokio::time::sleep;

const PACMAN_LOCK: &str = "/var/lib/pacman/db.lck";
const LCK_MAX_AGE_SECS: u64 = 900; // 15 mins

async fn check_pacman_lock() {
    if let Ok(metadata) = fs::metadata(PACMAN_LOCK) {
        if let Ok(modified) = metadata.modified() {
            if let Ok(age) = SystemTime::now().duration_since(modified) {
                if age.as_secs() > LCK_MAX_AGE_SECS {
                    println!(
                        "Pacman lock file is too old ({} secs). Removing it.",
                        age.as_secs()
                    );
                    let _ = fs::remove_file(PACMAN_LOCK);
                }
            }
        }
    }
}

async fn check_services() -> bool {
    let services = vec!["NetworkManager", "display-manager"];
    let mut critical_failure = false;

    for service in services {
        let output = Command::new("systemctl")
            .arg("is-failed")
            .arg(service)
            .output();

        if let Ok(output) = output {
            let status = String::from_utf8_lossy(&output.stdout).trim().to_string();
            if status == "failed" {
                println!("Service {} has failed. Attempting restart...", service);
                // Try restarting
                let _ = Command::new("systemctl")
                    .arg("restart")
                    .arg(service)
                    .status();

                // Check again after a short delay
                sleep(Duration::from_secs(5)).await;
                let output2 = Command::new("systemctl")
                    .arg("is-failed")
                    .arg(service)
                    .output();

                if let Ok(output2) = output2 {
                    let status2 = String::from_utf8_lossy(&output2.stdout).trim().to_string();
                    if status2 == "failed" {
                        println!(
                            "Service {} failed again after restart! Critical failure.",
                            service
                        );
                        critical_failure = true;
                    }
                }
            }
        }
    }
    critical_failure
}

async fn rollback_and_reboot() {
    println!("Initiating snapper rollback...");
    let _ = Command::new("snapper").arg("rollback").status();

    println!("Rebooting system...");
    let _ = Command::new("reboot").status();
}

#[tokio::main]
async fn main() {
    println!("Starting omni-medic-daemon...");
    loop {
        check_pacman_lock().await;

        if check_services().await {
            rollback_and_reboot().await;
        }

        sleep(Duration::from_secs(30)).await;
    }
}
