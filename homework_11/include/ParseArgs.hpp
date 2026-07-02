#pragma once

#include <string>
#include <iostream>

#include <boost/program_options.hpp>

struct Args {
  std::string uart = "/tmp/ttyA";
  std::string gpiochip = "gpiochip0";
  uint32_t startLine = 24;
  uint32_t dropLine = 23;
  std::string dataFolder{"./"};
};

inline Args ParseArgs(int argc, char** argv)
{
  namespace po = boost::program_options;

  Args args;

  po::options_description desc("Allowed options");

  desc.add_options()("help,h", "show help message")(
    "uart,u", po::value<std::string>(&args.uart)->default_value("/tmp/ttyA"), "UART device, e.g. /tmp/ttyA, /dev/ttyAMA3")(
    "gpiochip,g", po::value<std::string>(&args.gpiochip)->default_value("gpiochip0"), "GPIO chip, e.g. gpiochip0")(
    "start-line", po::value<uint32_t>(&args.startLine)->default_value(24), "GPIO line used as START output")(
    "drop-line", po::value<uint32_t>(&args.dropLine)->default_value(23), "GPIO line used as DROP output")(
    "dataFolder", po::value<std::string>(&args.dataFolder)->default_value("./"), "Folder for storing data");

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      std::exit(0);
    }

    po::notify(vm);
  }
  catch (const po::error& e) {
    std::cerr << "Argument error: " << e.what() << "\n\n";
    std::cerr << desc << '\n';
    std::exit(1);
  }

  return args;
}