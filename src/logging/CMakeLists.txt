include (util-cmake/add_macros)

extended_add_library (
  NAME Logging
  NAMESPACE Util
  TYPE STATIC
  SOURCES "logging.cpp"
          "logging_options.cpp"
          "performance_record.cpp"
          "command_line_args_printer.cpp"
          "format.cpp"
          "statistics/counter_dump_reader.cpp"
  LIBRARIES GPISpace::workflow_development
            GPISpace::execution
            Util::Generic
  POSITION_INDEPENDENT
)

extended_add_executable (NAME generate_statistics DONT_APPEND_EXE_SUFFIX
  SOURCES "statistics/main.cpp"
  LIBRARIES Util::Generic
            Util::Logging
  INSTALL
)
