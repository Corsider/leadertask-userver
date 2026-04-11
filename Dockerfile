FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest

WORKDIR /app

COPY . .

RUN apt-get update && apt-get install -y python3-venv python3-pip libpq-dev \
    && python3 -m pip install --upgrade pip setuptools wheel

RUN cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build-debug -j$(nproc)

EXPOSE 8080

CMD ["./build-debug/jwt_auth", "--config", "configs/static_config.yaml"]