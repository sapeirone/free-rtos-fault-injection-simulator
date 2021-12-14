FROM gcc:11.2 AS build-env

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y cmake

WORKDIR /home
COPY ./FreeRTOS/ ./FreeRTOS/
COPY ./FreeRTOS-Plus/ ./FreeRTOS-Plus/
COPY ./simulator/ ./simulator/
COPY ./CMakeLists.txt .

RUN ls -lah

RUN cmake . && make

FROM debian:11-slim AS run-env
WORKDIR /home/
COPY --from=build-env /home/build/sim sim.out
COPY --from=build-env /home/simulator/input_data/input_small.dat ./simulator/input_data/input_small.dat
COPY input.csv ./input.csv

# generate a golden execution
RUN chmod +x ./sim.out
RUN ["./sim.out", "--golden"]
RUN ls -lah
ENTRYPOINT [ "./sim.out", "--campaign", "input.csv", "-y", "-j=4"]

