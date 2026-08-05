import sys
# pyrefly: ignore [missing-import]
from PIL import Image

def process_logo(input_path, output_path):
    img = Image.open(input_path).convert('RGBA')
    datas = img.getdata()
    
    # Get top-left pixel as background color
    bg_color = img.getpixel((0, 0))
    print(f"Background color detected as: {bg_color}")
    
    # A simple flood-fill or exact match for background
    new_data = []
    # If the background is blue as the user said, we can just replace anything close to bg_color
    for item in datas:
        # Check if the pixel matches the background color (within a small tolerance)
        if abs(item[0] - bg_color[0]) < 30 and abs(item[1] - bg_color[1]) < 30 and abs(item[2] - bg_color[2]) < 30:
            new_data.append((255, 255, 255, 0)) # Transparent
        else:
            new_data.append(item)
            
    img.putdata(new_data)
    
    # The logo should be small (like the windows/mac/linux logo)
    # Let's crop it to the bounding box first, then resize
    bbox = img.getbbox()
    if bbox:
        img = img.crop(bbox)
        
    # Resize keeping aspect ratio, max width/height 150
    img.thumbnail((150, 150), Image.Resampling.LANCZOS)
    
    img.save(output_path)
    print("Saved to", output_path)

if __name__ == "__main__":
    process_logo('archiso/airootfs/usr/share/plymouth/themes/omni-os/logo.png', 'archiso/airootfs/usr/share/plymouth/themes/omni-os/logo.png')
