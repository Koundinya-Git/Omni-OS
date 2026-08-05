use clap::Parser;
use crossterm::{
    execute,
    style::{Color, Print, ResetColor, SetForegroundColor},
};
use std::io::stdout;
use std::process::{Command, Stdio};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// The search term or URL for yt-dlp
    query: String,
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args = Args::parse();
    let mut stdout = stdout();

    execute!(
        stdout,
        SetForegroundColor(Color::Cyan),
        Print(format!("Searching and fetching stream for: {}\n", args.query)),
        ResetColor
    )?;

    // Prepare yt-dlp query: if it doesn't look like a URL, prepend ytsearch1:
    let query_arg = if args.query.starts_with("http") {
        args.query.clone()
    } else {
        format!("ytsearch1:{}", args.query)
    };

    // Use yt-dlp to get the direct audio stream URL
    let ytdl_output = Command::new("yt-dlp")
        .arg("-g")
        .arg("-f")
        .arg("bestaudio")
        .arg(&query_arg)
        .stdout(Stdio::piped())
        .output()?;

    if !ytdl_output.status.success() {
        execute!(
            stdout,
            SetForegroundColor(Color::Red),
            Print("Failed to retrieve audio stream URL.\n"),
            ResetColor
        )?;
        return Ok(());
    }

    let url = String::from_utf8_lossy(&ytdl_output.stdout).trim().to_string();

    execute!(
        stdout,
        SetForegroundColor(Color::Green),
        Print("Playing music with mpv...\n"),
        ResetColor
    )?;

    // Play stream using mpv without a video window
    Command::new("mpv")
        .arg("--no-video")
        .arg(&url)
        .status()?;

    execute!(
        stdout,
        SetForegroundColor(Color::Green),
        Print("Playback finished.\n"),
        ResetColor
    )?;

    Ok(())
}
