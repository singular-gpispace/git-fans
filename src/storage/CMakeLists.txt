extended_add_library (NAME storage
  NAMESPACE GITFAN
  SOURCES "service_provider.cpp"
          "options.cpp"
          "conestorage.cpp"
          "rpc/server.cpp"
          "filesystem/string_container.cpp"
          "filesystem/linear_conestorage.cpp"
          "filesystem/constant_conestorage.cpp"
  COMPILE_DEFINITIONS
    BOOST_COROUTINES_NO_DEPRECATION_WARNING
  LIBRARIES Util::Generic
            Util::Logging
            Util::RPC
  POSITION_INDEPENDENT
)
