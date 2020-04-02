## 编译Docker版本的ChainSQL

### 安装工具
- Docker

### 编译ChainSQL

- Dockerfile文件
```
# ChainSQL 

# Use the alpine baseimage
FROM alpine:3.7
MAINTAINER RUANCHENG <ruancheng@peersafe.cn>

# make sure the package repository is up to date
#RUN apk update && \
#	apk upgrade --update-cache 

# install the dependencies
RUN apk add build-base  \
			protobuf-dev  \
			boost-dev  \
			llvm5-dev  \
			llvm5-static  \
			clang-dev  \
			cmake  \
			mariadb-dev  \
			libressl-dev


#ENV CC /usr/bin/gcc
#ENV CXX /usr/bin/g++ 

WORKDIR /opt/chainsql

COPY ./src ./src
COPY ./Builds/CMake ./Builds/CMake
COPY ./doc/chainsqld-example.cfg chainsqld.cfg
COPY ./Builds/Docker/mysqlclient.pc /usr/lib/pkgconfig/
COPY ./CMakeLists.txt CMakeLists.txt

RUN cd Builds; cmake -Dtarget=clang.release.unity  ../; make chainsqld -j2


# move to root directory and strip
RUN cp ./build/clang.release.unity/chainsqld chainsqld
RUN strip chainsqld

# clean source
#RUN rm -r src

# launch chainsqld when launching the container
ENTRYPOINT ./chainsqld
#ENTRYPOINT sleep 1000

```
- 编译命令

```
# 例如编译v0.30.6版本的ChainsSQL
docker build -t chainsql:v0.30.6 .
```

## Docker版本的ChainSQL的发布

- 从Docker镜像中拷贝依赖库

```
# 其中 ecf69045b65d 为编译ChainSQL的容器ID
docker cp ecf69045b65d:/opt/chainsql/chainsqld ./build/
docker cp ecf69045b65d:/lib/ld-musl-x86_64.so.1 ./build/
docker cp ecf69045b65d:/lib/libz.so.1.2.11 ./build/
docker cp ecf69045b65d:/usr/lib/libgcc_s.so.1 ./build/
```

- 编译命令

```
docker build -t peersafes/chainsql:v0.30.6 -f Dockerfile.release ./
```

- 将镜像推送到 [Docker hub](https://hub.docker.com/)

```
docker push peersafes/chainsql:v0.30.6
```