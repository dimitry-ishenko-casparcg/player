////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include <asio.hpp>
#include <exception>
#include <iostream>
#include <pgm/args.hpp>

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    pgm::args args
    {
        { "-v", "--version", "Show version and exit." },
        { "-h", "--help",    "Show this help screen and exit." },
    };
    args.parse(argc, argv);

    if(args["--version"])
    {
        std::cout << argv[0] << " " << VERSION << std::endl;
        return 0;
    }
    if(args["--help"])
    {
        std::cout << args.usage(argv[0]) << std::endl;
        return 0;
    }

    asio::io_context io;

    //

    io.run();
    return 0;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
