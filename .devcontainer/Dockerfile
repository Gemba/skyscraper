# syntax=docker/dockerfile:1

# Dev Container Template (https://containers.dev/templates) based on latest Debian "bookworm" release or use vanilla Debian image from Docker Hub instead
FROM mcr.microsoft.com/devcontainers/base:bookworm
# FROM debian:bookworm

# Update, upgrade, and install necessary packages
RUN sudo apt update && sudo apt upgrade -y && sudo apt install -y qtbase5-dev

# Set QT version for qmake/make (compilation and installation) of Skyscraper
RUN QT_SELECT=5

# Install apps
# `neovim` for editing
# `gdb` for debugging
# `file` for checking that `Skyscraper` binary has debugging info
RUN sudo apt install neovim gdb file -y
