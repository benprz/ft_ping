FROM ubuntu:latest

RUN apt update && apt install -y netbase iproute2 openssh-server

WORKDIR /shared

RUN echo "service ssh start && ip a && tail -f" > /script.sh

COPY inetutils-2.0/ping/ping /bin/ping

EXPOSE 22

CMD ["sh", "/script.sh"]
