ARG OPENVDS_IMAGE=openvds
ARG ONESEISMIC_BASEIMAGE=docker.io/library/golang:1.22-alpine
FROM ${ONESEISMIC_BASEIMAGE} AS openvds
RUN apk --no-cache add \
    boost-dev \
    boost-dev \
    cmake \
    cmake \
    curl \
    eigen \
    eigen-dev \
    g++ \
    gcc \
    git \
    libuv-dev \
    make \
    openssl-dev \
    perl \
    util-linux-dev \
    zlib-dev


WORKDIR /
RUN mkdir fftw3
RUN curl https://www.fftw.org/fftw-3.3.10.tar.gz -o fftw3/fftw-3.3.10.tar.gz
WORKDIR /fftw3
RUN tar -xvzf fftw-3.3.10.tar.gz
WORKDIR /fftw3/fftw-3.3.10

RUN cmake -S . \
    -B build \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build --config Release --target install --verbose

ARG OPENVDS_VERSION=3.4.5
WORKDIR /
RUN git clone --depth 1 --branch ${OPENVDS_VERSION} https://community.opengroup.org/osdu/platform/domain-data-mgmt-services/seismic/open-vds.git
WORKDIR /open-vds

# for performance reasons open-vds should be build and run with new openssl
# libssl is installed on alpine by default and would be used during runtime

# it is unclear if curl version makes a difference and which one is better
ARG BUILD_TYPE=Release
RUN cmake -S . \
    -B build \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DBUILD_JAVA=OFF \
    -DBUILD_PYTHON=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_DOCS=OFF \
    -DDISABLE_STRICT_WARNINGS=OFF \
    -DDISABLE_AZURESDKFORCPP_IOMANAGER=OFF \
    -DDISABLE_AWS_IOMANAGER=ON \
    -DDISABLE_GCP_IOMANAGER=ON \
    -DDISABLE_DMS_IOMANAGER=ON \
    -DDISABLE_AZURE_PRESIGNED_IOMANAGER=ON \
    -DDISABLE_CURL_IOMANAGER=ON \
    -DBUILD_UV=OFF \
    -DBUILD_ZLIB=OFF \
    -DBUILD_OPENSSL=OFF \
    -DBUILD_CURL=ON

RUN cmake --build build --config ${BUILD_TYPE} --target install -j 8 --verbose

WORKDIR /fftw3_build
ARG FFTW_VERSION=3.3.10
ARG FFTW_MD5=8ccbf6a5ea78a16dbc3e1306e234cc5c

RUN curl -o fftw.tar.gz http://www.fftw.org/fftw-${FFTW_VERSION}.tar.gz
RUN echo "${FFTW_MD5}  fftw.tar.gz" > fftw.tar.gz.md5
RUN md5sum -c fftw.tar.gz.md5
RUN tar -xzvf fftw.tar.gz
WORKDIR /fftw3_build/fftw-${FFTW_VERSION}
RUN mkdir build
WORKDIR /fftw3_build/fftw-${FFTW_VERSION}/build
# RUN ./configure
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/fftw3_install
RUN make
RUN make install
RUN rm -rf fftw.tar.gz fftw.tar.gz.md5 fftw-${FFTW_VERSION}


FROM $OPENVDS_IMAGE as openvds_snyk_monitor
WORKDIR /
RUN apk --no-cache add curl
RUN curl --compressed https://downloads.snyk.io/cli/stable/snyk-alpine -o snyk
RUN chmod +x ./snyk
WORKDIR /open-vds
ENTRYPOINT /snyk monitor --unmanaged --remote-repo-url=${SNYK_PROJECT_TAG}


FROM $OPENVDS_IMAGE as builder
WORKDIR /src
COPY go.mod go.sum ./
RUN go mod download && go mod verify
COPY . .
ARG CGO_CPPFLAGS="-I/open-vds/Dist/OpenVDS/include -I/fftw3/include"
ARG CGO_LDFLAGS="-L/open-vds/Dist/OpenVDS/lib -L/fftw3/lib"
RUN go build -a ./...
RUN GOBIN=/tools go install github.com/swaggo/swag/cmd/swag@v1.16.3
RUN /tools/swag init --dir cmd/query,api/handlers,api/middleware,internal/core -g main.go --md docs


FROM builder as tester
ARG CGO_CPPFLAGS="-I/open-vds/Dist/OpenVDS/include -I/fftw3/include"
ARG CGO_LDFLAGS="-L/open-vds/Dist/OpenVDS/lib -L/fftw3/lib"
ARG LD_LIBRARY_PATH=/open-vds/Dist/OpenVDS/lib:$LD_LIBRARY_PATH

ENV CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/open-vds/Dist/OpenVDS
RUN cmake -S . -B build
RUN cmake --build build
RUN ctest --test-dir build

ARG OPENVDS_AZURESDKFORCPP=1
RUN go test -race ./...


FROM builder as static_analyzer
ARG CGO_CPPFLAGS="-I/open-vds/Dist/OpenVDS/include -I/fftw3/include"
ARG CGO_LDFLAGS="-L/open-vds/Dist/OpenVDS/lib -L/fftw3/lib"
ARG STATICCHECK_VERSION="2023.1.7"
ARG LD_LIBRARY_PATH=/open-vds/Dist/OpenVDS/lib:$LD_LIBRARY_PATH
RUN apk --no-cache add curl
RUN curl \
    -L https://github.com/dominikh/go-tools/releases/download/${STATICCHECK_VERSION}/staticcheck_linux_amd64.tar.gz \
    -o staticcheck-${STATICCHECK_VERSION}.tar.gz
RUN ls && tar xf staticcheck-${STATICCHECK_VERSION}.tar.gz
RUN ./staticcheck/staticcheck ./...


FROM builder as installer
ARG CGO_CPPFLAGS="-I/open-vds/Dist/OpenVDS/include -I/fftw3/include"
ARG CGO_LDFLAGS="-L/open-vds/Dist/OpenVDS/lib -L/fftw3/lib"
ARG LD_LIBRARY_PATH=/open-vds/Dist/OpenVDS/lib
RUN GOBIN=/server go install -a ./...

FROM ${ONESEISMIC_BASEIMAGE} as runner
RUN apk --no-cache add \
    jemalloc-dev

WORKDIR /server
COPY --from=installer /open-vds/Dist/OpenVDS/lib/* /open-vds/
COPY --from=installer /server /server
COPY --from=installer /src/docs/index.html /server/docs/
COPY --from=installer /fftw3/* /fftw3/

RUN addgroup -S -g 1001 radix-non-root-group
RUN adduser -S -u 1001 -G radix-non-root-group radix-non-root-user
USER 1001

ENV LD_LIBRARY_PATH=/open-vds:/fftw3
ENV OPENVDS_AZURESDKFORCPP=1
ENV LD_PRELOAD=/usr/lib/libjemalloc.so
ENTRYPOINT [ "/server/query" ]
