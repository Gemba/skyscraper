# Consider alpine
FROM ubuntu:20.04

# Confifuring tadata, see https://dev.to/setevoy/docker-configure-tzdata-and-timezone-during-build-20bk
ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Installing Qt5
RUN apt-get update
RUN apt-get install -y --no-install-recommends build-essential qt5-default wget sudo
RUN apt-get install ca-certificates -y
RUN rm -rf /var/lib/apt/lists/*

# Workaround for sudo in update_skyscraper.sh
RUN useradd -m docker && echo "docker:docker" | chpasswd && adduser docker sudo

# Building skyscraper
COPY ./* ./
RUN rm ./VERSION.ini
RUN chmod +x update_skyscraper.sh  && ./update_skyscraper.sh

ENTRYPOINT ["Skyscraper"]
