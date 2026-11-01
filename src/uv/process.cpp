////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "error.hpp"
#include "process.hpp"

#include <csignal>
#include <iterator> // std::size

////////////////////////////////////////////////////////////////////////////////
namespace
{

constexpr auto operator|(uv_stdio_flags x, uv_stdio_flags y) {
    return static_cast<uv_stdio_flags>(static_cast<int>(x) | static_cast<int>(y));
}

}

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

process::process(loop& loop, std::vector<std::string> args, std::vector<std::string> env) :
    timer_{loop}, stdin_{loop}, stdout_{loop}, stderr_{loop}
{
    std::vector<char*> argv;
    for (auto& r : args) argv.push_back(r.data());
    argv.push_back(nullptr);

    std::vector<char*> envp;
    for (auto& e : env) envp.push_back(e.data());
    envp.push_back(nullptr);

    uv_stdio_container_t io[3] = {};
    io[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
    io[0].data.stream = reinterpret_cast<uv_stream_t*>(stdin_.get());

    io[1].flags = UV_IGNORE; // TODO
    io[1].data.stream = reinterpret_cast<uv_stream_t*>(stdout_.get());

    io[2].flags = UV_IGNORE; // TODO
    io[2].data.stream = reinterpret_cast<uv_stream_t*>(stderr_.get());

    uv_process_options_t opts{};
    opts.file  = argv.at(0);
    opts.args  = argv.data();
    opts.env   = env.empty() ? nullptr : envp.data();
    opts.stdio = io;
    opts.stdio_count = std::size(io);
    opts.exit_cb = [](uv_process_t* p, auto status, auto signal)
    {
        auto& cb = *static_cast<exit_callback*>(p->data);
        if (cb) cb(status, signal);
    };
    get()->data = exit_cb_.get();

    if (auto rc = uv_spawn(loop.get(), get(), &opts); rc < 0) throw error{rc, "uv_spawn"};
}

process& process::operator=(process&& rhs) noexcept
{
    if (this != &rhs)
    {
        teardown();
        handle::operator=(std::move(rhs));

        exit_cb_= std::move(rhs.exit_cb_);
        stdin_  = std::move(rhs.stdin_  );
        stdout_ = std::move(rhs.stdout_ );
        stderr_ = std::move(rhs.stderr_ );
        timer_  = std::move(rhs.timer_  );
    }
    return *this;
}

void process::stop(msec timeout)
{
    if (is_active())
    {
        signal(SIGTERM);
        timer_.start(timeout, [this]{ if (is_active()) signal(SIGKILL); });
    }
}

void process::teardown() noexcept
{
    if (is_active())
    {
        *exit_cb_ = nullptr;

        try { stop(); }
        catch (...) { signal(SIGKILL); }

        while (is_active()) uv_run(get()->loop, UV_RUN_ONCE);
    }
}

}
