FROM smtrat/carl:latest
RUN apt-get update \
&& apt-get install -y \
openjdk-8-jre \
uuid-dev \
pkg-config \
libboost-dev
RUN git clone https://github.com/hypro/hypro.git
RUN cd hypro
RUN git checkout c++14-support
RUN mkdir build && cd build && cmake .. \
&& make resources \
&& make