FROM ubuntu:latest
RUN apt-get update && apt-get install -y curl sudo
COPY get.sh /get.sh
RUN chmod +x /get.sh
CMD ["/get.sh"]
