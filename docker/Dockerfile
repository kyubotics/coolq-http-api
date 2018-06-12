FROM coolq/wine-coolq:latest

ARG GIT_TAG

ADD https://github.com/richardchien/coolq-http-api/releases/download/${GIT_TAG}/io.github.richardchien.coolqhttpapi.cpk /home/user/io.github.richardchien.coolqhttpapi.cpk
ADD bootstrap.py /home/user/bootstrap.py
RUN chown user:user /home/user/io.github.richardchien.coolqhttpapi.cpk /home/user/bootstrap.py
RUN echo "\n\nsudo -E -Hu user /usr/bin/python3 /home/user/bootstrap.py" >> /etc/cont-init.d/110-get-coolq

EXPOSE 5700

VOLUME ["/home/user/coolq"]