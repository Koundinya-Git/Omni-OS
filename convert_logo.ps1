Add-Type -AssemblyName System.Drawing

$logoPath = "d:\Omni-OS\archiso\airootfs\usr\share\plymouth\themes\omni-os\logo.png"

# Load the original image
$originalImage = [System.Drawing.Image]::FromFile($logoPath)
$bmp = New-Object System.Drawing.Bitmap($originalImage)
$originalImage.Dispose()

# Get the background color from top-left pixel
$bgColor = $bmp.GetPixel(0,0)
Write-Output "Background color detected: $bgColor"

# Make the background color transparent
$bmp.MakeTransparent($bgColor)

# Create a new resized bitmap
$newWidth = 150
$newHeight = 150
$resizedBmp = New-Object System.Drawing.Bitmap($newWidth, $newHeight)
$graphics = [System.Drawing.Graphics]::FromImage($resizedBmp)
$graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic

# Maintain aspect ratio?
# Let's just fit it in the 150x150 box
$graphics.DrawImage($bmp, 0, 0, $newWidth, $newHeight)
$graphics.Dispose()

# Save over the original logo
$resizedBmp.Save($logoPath, [System.Drawing.Imaging.ImageFormat]::Png)

$bmp.Dispose()
$resizedBmp.Dispose()
Write-Output "Image processed successfully!"
