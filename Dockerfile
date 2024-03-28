FROM kalilinux/kali-rolling
RUN apt update && apt install -y make gcc tcpdump tshark netbase iproute2 less man
WORKDIR /shared
COPY inetutils-2.0/ping/ping /bin/ping
