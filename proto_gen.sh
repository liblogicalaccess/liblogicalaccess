#!/bin/bash


PROTOC_PATH=/home/xaqq/.conan/data/protobuf/3.5.2/bincrafters/stable/package/d0ec62fc032e5a10524fa454546aa1bdbb22baf8/bin/protoc
GRPC_CPP_PLUGIN=/home/xaqq/.conan/data/grpc/1.14.1/xaqq/stable/package/7fe3ee14efb96cc10391b3be256e452fbbcf641d/bin/grpc_cpp_plugin

## Generate both hpp and cpp into proto folder.
#protoc -I proto --cpp_out=dllexport_decl=LIBLOGICALACCESS_API:proto proto/iks.proto
${PROTOC_PATH} -I proto --cpp_out=proto proto/iks.proto
${PROTOC_PATH} -I proto --grpc_out=proto --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} proto/iks.proto

mv proto/*.cc plugins/logicalaccess/plugins/iks
mv proto/*.h  plugins/logicalaccess/plugins/iks
