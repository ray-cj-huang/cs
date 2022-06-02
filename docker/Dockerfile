### Build/test container ###
# Define builder stage
FROM pushin-p:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make -j 4
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:focal as deploy

COPY --from=builder /usr/src/project/build/bin/server .
COPY --from=builder /usr/src/project/configs/server_config .
COPY --from=builder /usr/src/project/static ./static
COPY --from=builder /usr/src/project/tests/static_files ./tests/static_files

RUN mkdir build
RUN cd build

EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["../server"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["../server_config"]
