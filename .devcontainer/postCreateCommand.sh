#!/bin/bash

echo "Running $0 script as a part of the 'postCreateCommand' in 'devcontainer.json'"

# Configure git
echo "Configuring git"
git config --global --add safe.directory /workspaces/skyscraper
# git config --global user.email "<ADD_USER_EMAIL_HERE>"
# git config --global user.name "<ADD_USER_NAME_HERE>"

# Set aliases
echo "Setting aliases (in ~/.bash_aliases)"
echo 'alias ll="ls -lah"' >> ~/.bash_aliases

# Create ROMs directory
mkdir -p ~/RetroPie/roms/nes
