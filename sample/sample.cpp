/*
 *  Copyright 2020-present Daniel Trugman
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "format.hpp"
#include "log.hpp"

#include "pfs/procfs.hpp"
#include "pfs/version.hpp"

void print_task(const pfs::task& task)
{
    try
    {
        LOG("=========================================================");
        LOG("Task ID[" << task.id() << "]");
        LOG("=========================================================");

        auto status = task.get_status();
        print(status);

        auto stat = task.get_stat();
        print(stat);

        auto mem_stat = task.get_statm();
        print(mem_stat);

        auto comm = task.get_comm();
        print(comm);

        if (!task.is_kernel_thread(stat))
        {
            auto exe = task.get_exe();
            print(exe);
        }

        auto cmdline = task.get_cmdline();
        print(cmdline);

        auto cwd = task.get_cwd();
        print(cwd);

        auto environ = task.get_environ();
        print(environ);

        auto maps = task.get_maps();
        print(maps);

        if (!maps.empty())
        {
            static const size_t BYTES = 8;
            auto mem                  = task.get_mem();
            auto first_map            = *maps.begin();
            auto header_bytes = mem.read(first_map.start_address, BYTES);
            auto header       = hexlify(header_bytes);
            print(header);
        }

        auto fds = task.get_fds();
        print(fds);

        auto mountinfo = task.get_mountinfo();
        print(mountinfo);

        auto ns = task.get_ns();
        print(ns);
    }
    catch (const std::runtime_error& ex)
    {
        LOG("Error when printing task[" << task.id() << "]:");
        LOG(TAB << ex.what());
    }
}

void print_net(const pfs::net& net)
{
    try
    {
        LOG("=========================================================");
        LOG("Net");
        LOG("=========================================================");

        auto icmp = net.get_icmp();
        print(icmp);

        auto icmp6 = net.get_icmp6();
        print(icmp6);

        auto raw = net.get_raw();
        print(raw);

        auto raw6 = net.get_raw6();
        print(raw6);

        auto tcp = net.get_tcp();
        print(tcp);

        auto tcp6 = net.get_tcp6();
        print(tcp6);

        auto udp = net.get_udp();
        print(udp);

        auto udp6 = net.get_udp6();
        print(udp6);

        auto udplite = net.get_udplite();
        print(udplite);

        auto udplite6 = net.get_udplite6();
        print(udplite6);

        auto unix = net.get_unix();
        print(unix);
    }
    catch (const std::runtime_error& ex)
    {
        LOG("Error when printing net info:");
        LOG(TAB << ex.what());
    }
}

void print_system(const pfs::procfs& pfs)
{
    try
    {
        LOG("=========================================================");
        LOG("System");
        LOG("=========================================================");

        auto buddyinfo = pfs.get_buddyinfo();
        print(buddyinfo);

        auto cmdline = pfs.get_cmdline();
        print(cmdline);

        auto modules = pfs.get_modules();
        print(modules);

        auto filesystems = pfs.get_filesystems();
        print(filesystems);

        auto loadavg = pfs.get_loadavg();
        print(loadavg);

        auto meminfo = pfs.get_meminfo();
        print(meminfo);

        auto version = pfs.get_version();
        print(version);

        auto version_signature = pfs.get_version_signature();
        print(version_signature);
    }
    catch (const std::runtime_error& ex)
    {
        LOG("Error when printing system info:");
        LOG(TAB << ex.what());
    }
}

// =========================================================================
// Main
// =========================================================================

void usage(char* argv0)
{
    LOG("Sample application using pfs v"
        << PFS_VER_MAJOR << "." << PFS_VER_MINOR << "." << PFS_VER_PATCH);
    LOG("Usage: " << argv0 << " [args]...");
    LOG("");
    LOG("   system         Enumerate system-wide information");
    LOG("   net            Enumerate network information");
    LOG("   tasks          Enumerate all running processes and threads");
    LOG("   [task-id]...   Enumerate the specified tasks");
    LOG("");
}

void safe_main(int argc, char** argv)
{
    static const std::string CMD_ENUM_SYSTEM("system");
    static const std::string CMD_ENUM_TASKS("tasks");
    static const std::string CMD_ENUM_NET("net");

    if (argc < 2)
    {
        usage(argv[0]);
        return;
    }

    pfs::procfs pfs;

    if (CMD_ENUM_SYSTEM.compare(argv[1]) == 0)
    {
        print_system(pfs);
    }
    else if (CMD_ENUM_NET.compare(argv[1]) == 0)
    {
        print_net(pfs.get_net());
    }
    else if (CMD_ENUM_TASKS.compare(argv[1]) == 0)
    {
        for (const auto& process : pfs.get_processes())
        {
            for (const auto& thread : process.get_tasks())
            {
                print_task(thread);
            }
        }
    }
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            int id = std::stoi(argv[i]);
            print_task(pfs.get_task(id));
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        safe_main(argc, argv);
    }
    catch (const std::invalid_argument& ex)
    {
        usage(argv[0]);
    }
    catch (const std::exception& ex)
    {
        LOG(ex.what());
    }

    return 0;
}