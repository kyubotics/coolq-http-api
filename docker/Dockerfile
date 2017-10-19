FROM coolq/wine-coolq:latest

ARG GIT_TAG

ADD https://github.com/richardchien/coolq-http-api/releases/download/${GIT_TAG}/io.github.richardchien.coolqhttpapi.cpk /tmp/io.github.richardchien.coolqhttpapi.cpk
ADD bootstrap.py /tmp/bootstrap.py
RUN echo "\n\n/usr/bin/python3 /tmp/bootstrap.py" >> /etc/cont-init.d/110-get-coolq

EXPOSE 5700

VOLUME ["/home/user/coolq"]