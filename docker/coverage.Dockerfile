### Generate coverage tests ###
# Define builder stage
FROM pushin-p:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Generate coverage test files
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage