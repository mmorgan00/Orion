
#!/bin/bash

# Define source and destination directories
SOURCE_DIR="./docs/html"
DEST_DIR="../mmorgan00.github.io"

rm -rf $SOURCE_DIR #clean first

doxygen

# Check if the source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
  echo "Source directory $SOURCE_DIR does not exist."
  exit 1
fi

# Create the destination directory if it does not exist
if [ ! -d "$DEST_DIR" ]; then
  mkdir -p "$DEST_DIR"
fi

# Copy files recursively from source to destination
cp -r "$SOURCE_DIR"/* "$DEST_DIR"
rm -rf "./docs"

# Check if the copy operation was successful
if [ $? -eq 0 ]; then
  echo "Files moved successfully from $SOURCE_DIR to $DEST_DIR."
else
  echo "Failed to copy files from $SOURCE_DIR to $DEST_DIR."
  exit 1
fi
