file(REMOVE_RECURSE
  "CMakeFiles/generate_proto_sources"
  "nanopb/generator/nanopb_generator.py"
  "nanopb/generator/proto/nanopb.proto"
  "nanopb/generator/proto/nanopb_pb2.py"
  "protobuf/authentication.pb.c"
  "protobuf/authentication.pb.h"
  "protobuf/keyexchange.pb.c"
  "protobuf/keyexchange.pb.h"
  "protobuf/mercury.pb.c"
  "protobuf/mercury.pb.h"
  "protobuf/metadata.pb.c"
  "protobuf/metadata.pb.h"
  "protobuf/spirc.pb.c"
  "protobuf/spirc.pb.h"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/generate_proto_sources.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
