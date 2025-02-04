FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    build-essential \
    libsdl2-dev \
    libsdl2-image-dev\
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /cpp_cource

COPY . .

RUN g++ -o my_app main.cpp sdl2-config --cflags --libs

CMD ["./cpp_cource"]
