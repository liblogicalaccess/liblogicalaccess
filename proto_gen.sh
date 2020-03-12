#!/bin/bash


PROTOC_PATH=/home/xaqq/.conan/data/protoc_installer/3.9.1/bincrafters/stable/package/c0c1ef10e3d0ded44179e28b669d6aed0277ca6a/bin/protoc
GRPC_CPP_PLUGIN=/home/xaqq/.conan/data/grpc/1.25.0/_/_/package/197eec0f2b2a3f88b8f98aa876bdaf970c308033/bin/grpc_cpp_plugin

## Generate both hpp and cpp into proto folder.
#protoc -I proto --cpp_out=dllexport_decl=LIBLOGICALACCESS_API:proto proto/iks.proto
${PROTOC_PATH} -I proto --cpp_out=proto proto/iks.proto
${PROTOC_PATH} -I proto --grpc_out=proto --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} proto/iks.proto

mv proto/*.cc plugins/logicalaccess/plugins/iks
mv proto/*.h  plugins/logicalaccess/plugins/iks
