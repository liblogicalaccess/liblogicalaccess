#!/bin/bash


## Generate both hpp and cpp into proto folder.
#protoc -I proto --cpp_out=dllexport_decl=LIBLOGICALACCESS_API:proto proto/iks.proto
protoc -I proto --cpp_out=proto proto/iks.proto
protoc -I proto --grpc_out=proto --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) proto/iks.proto

mv proto/*.cc plugins/logicalaccess/plugins/iks
mv proto/*.h  plugins/logicalaccess/plugins/iks
