extended_add_library (NAME storage
  NAMESPACE GITFAN
  SOURCES "service_provider.cpp"
          "options.cpp"
          "conestorage.cpp"
          "rpc/server.cpp"
          "filesystem/string_container.cpp"
          "filesystem/linear_conestorage.cpp"
          "filesystem/constant_conestorage.cpp"
  LIBRARIES Util::Generic
            Util::Logging
            RPC
  POSITION_INDEPENDENT
)
