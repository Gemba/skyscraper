FROM ubuntu:24.04

# Confifuring tadata, see https://dev.to/setevoy/docker-configure-tzdata-and-timezone-during-build-20bk
ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Installing Qt and toolchain
RUN apt update
RUN apt install -y --no-install-recommends \
  build-essential \
  qt6-base-dev \
  qmake6 \
  qt6-base-dev-tools \
  libqt6sql6-sqlite \
  p7zip-full \
  wget \
  sudo \
  python3
RUN apt install -y adduser ca-certificates
RUN rm -rf /var/lib/apt/lists/*

# Workaround for sudo in update_skyscraper.sh
RUN useradd -m docker && echo "docker:docker" | chpasswd && adduser docker sudo

# Building skyscraper
COPY ./* ./
RUN rm -f ./VERSION.ini
RUN chmod +x update_skyscraper.sh  && ./update_skyscraper.sh

ENTRYPOINT ["Skyscraper"]
