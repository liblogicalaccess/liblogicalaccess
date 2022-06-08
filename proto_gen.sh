#!/bin/bash


PROTOC_PATH=/home/xaqq/.conan/data/protobuf/3.17.1/_/_/package/f45ead7d52ff58ecb8744a1f730cb62a47f8ae8b/bin/protoc
GRPC_CPP_PLUGIN=/home/xaqq/.conan/data/grpc/1.39.1/_/_/package/1cdc29d11178cbf0d8ab95b20ce23470cd47d152/bin/grpc_cpp_plugin

## Generate both hpp and cpp into proto folder.
#protoc -I proto --cpp_out=dllexport_decl=LIBLOGICALACCESS_API:proto proto/iks.proto
${PROTOC_PATH} -I proto --cpp_out=proto proto/iks.proto
${PROTOC_PATH} -I proto --grpc_out=proto --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} proto/iks.proto

mv proto/*.cc plugins/logicalaccess/plugins/iks
mv proto/*.h  plugins/logicalaccess/plugins/iks
