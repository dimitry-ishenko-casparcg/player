////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "process.hpp"

#include <array>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

process::process(loop& loop, std::vector<std::string> args, std::vector<std::string> env)
{
    std::vector<char*> argv;
    for (auto& r : args) argv.push_back(r.data());
    argv.push_back(nullptr);

    std::vector<char*> envp;
    for (auto& e : env) envp.push_back(e.data());
    envp.push_back(nullptr);

    std::array<uv_stdio_container_t, 3> io{};
    for (int fd = 0; fd < io.size(); ++fd)
    {
        io[fd].flags = UV_INHERIT_FD;
        io[fd].data.fd = fd;
    }

    uv_process_options_t opts{};
    opts.file  = argv.at(0);
    opts.args  = argv.data();
    opts.env   = env.empty() ? nullptr : envp.data();
    opts.stdio = io.data();
    opts.stdio_count = io.size();
    opts.exit_cb = [](uv_process_t* p, auto status, auto signal)
    {
        auto& cb = *static_cast<exit_callback*>(p->data);
        if (cb) cb(status, signal);
    };
    get()->data = exit_cb_.get();

    if (auto rc = uv_spawn(loop.get(), get(), &opts); rc < 0) throw error{rc, "uv_spawn"};
}

}
