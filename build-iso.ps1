param([switch]$Clean)
$ErrorActionPreference = "Stop"
$IMAGE_NAME = "omni-os-builder"
$CONTAINER_NAME = "omni-os-build"
$OUTPUT_DIR = Join-Path $PSScriptRoot "out"

if ($Clean) {
    docker rm -f $CONTAINER_NAME 2>$null
    docker rmi -f $IMAGE_NAME 2>$null
    if (Test-Path $OUTPUT_DIR) { Remove-Item -Recurse -Force $OUTPUT_DIR }
}
if (!(Test-Path $OUTPUT_DIR)) { New-Item -ItemType Directory -Path $OUTPUT_DIR | Out-Null }

Write-Host "[1/3] Building Docker image..." -ForegroundColor Green
docker build -t $IMAGE_NAME .
Write-Host "[2/3] Building Omni-OS ISO inside container..." -ForegroundColor Green
docker run --privileged --name $CONTAINER_NAME $IMAGE_NAME
Write-Host "[3/3] Extracting ISO from container..." -ForegroundColor Green
docker cp "${CONTAINER_NAME}:/build/out/." $OUTPUT_DIR
docker rm $CONTAINER_NAME | Out-Null
Write-Host "Done!" -ForegroundColor Green
