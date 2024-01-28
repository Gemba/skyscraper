# syntax=docker/dockerfile:1

# Dev Container Template (https://containers.dev/templates) based on latest Debian "bookworm" release or use vanilla Debian from Docker Hub instead
FROM mcr.microsoft.com/devcontainers/base:bookworm
# FROM debian:bookworm

# Update, upgrade, and install necessary packages
RUN sudo apt update && sudo apt upgrade && sudo apt install -y qtbase5-dev

# Make (compile) and install Skyscraper
RUN QT_SELECT=5
# These can not be done in a Dev Container, as workspace doesn't exist until after Docker container is created from image, so commands are unaware of source files. Instead, do in `postCreateCommand` of `devcontainer.json`.
# RUN qmake && make -j$(nproc) && sudo make install 
