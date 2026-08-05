use std::process::{Command, Stdio};
use std::io::Write;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Recording for 5 seconds...");
    // Record audio (16kHz, mono, 16-bit little-endian) suitable for whisper
    let record_status = Command::new("arecord")
        .arg("-d").arg("5")
        .arg("-f").arg("S16_LE")
        .arg("-r").arg("16000")
        .arg("-c").arg("1")
        .arg("/tmp/voice.wav")
        .status()?;

    if !record_status.success() {
        eprintln!("arecord failed to capture audio.");
        return Ok(());
    }

    println!("Transcribing audio...");
    // Transcribe with whisper-cli
    let whisper_output = Command::new("whisper-cli")
        .arg("-m").arg("/usr/share/whisper.cpp/models/ggml-base.en.bin")
        .arg("-f").arg("/tmp/voice.wav")
        .arg("-nt") // no timestamps
        .output()?;

    if !whisper_output.status.success() {
        eprintln!("whisper-cli failed to transcribe audio.");
        return Ok(());
    }

    let transcript = String::from_utf8_lossy(&whisper_output.stdout).to_string();
    let transcript = transcript.trim();
    println!("Transcript: {}", transcript);

    if transcript.is_empty() {
        println!("No speech detected.");
        return Ok(());
    }

    println!("Synthesizing speech...");
    // Use piper to synthesize the TTS output
    let mut piper_cmd = Command::new("piper")
        .arg("--model").arg("/usr/share/piper-voices/en_US-lessac-medium.onnx")
        .arg("--output_file").arg("/tmp/voice_out.wav")
        .stdin(Stdio::piped())
        .spawn()?;

    if let Some(mut stdin) = piper_cmd.stdin.take() {
        stdin.write_all(transcript.as_bytes())?;
    }
    
    piper_cmd.wait()?;

    println!("Playing response...");
    // Play the generated TTS output
    Command::new("aplay")
        .arg("/tmp/voice_out.wav")
        .status()?;

    Ok(())
}
