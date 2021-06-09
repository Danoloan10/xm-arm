FROM ubuntu:14.04

RUN apt-get update && apt-get install --yes \
	git \
	gcc-multilib \
	binutils \
	make \
	makeself \
	ncurses-dev \
	libxml2-dev \
	libxml2-utils

COPY . /opt/xtratum

CMD [ "sh", "-c", "(cd /opt/xtratum; bash)" ]
