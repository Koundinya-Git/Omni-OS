#!/usr/bin/env bash
set -e

# Build the Docker image
echo "=> Building Docker container for Archiso..."
docker build -t omni-os-builder .

# Run the container to generate the ISO
echo "=> Generating ISO inside container..."
docker run --rm --privileged \
    -v "$(pwd)/work:/build/work" \
    -v "$(pwd)/out:/build/out" \
    omni-os-builder

echo "=> Build complete! Check the 'out' directory."
