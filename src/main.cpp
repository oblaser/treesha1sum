/*
author          Oliver Blaser
date            19.12.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "middleware/sha1.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/defs.h>
#include <omw/vector.h>
#include <omw/windows/windows.h>


using std::cout;
using std::endl;
using std::setw;

namespace fs = std::filesystem;

namespace argstr {

const char* const noColor = "--no-color";
const char* const help = "--help";
const char* const version = "--version";

bool contains(const std::vector<std::string>& rawArgs, const char* arg)
{
    bool r = false;

    for (size_t i = 0; i < rawArgs.size(); ++i)
    {
        if (rawArgs[i] == arg)
        {
            r = true;
            break;
        }
    }

    return r;
}

} // namespace argstr

namespace {

enum EXITCODE // https://tldp.org/LDP/abs/html/exitcodes.html / on MSW are no preserved codes
{
    EC_OK = 0,
    EC_ERROR = 1,

    EC__begin_ = 79,

    EC_NOT_A_DIR = EC__begin_,
    // EC_..,

    EC__end_,

    EC__max_ = 113
};
static_assert(EC__end_ <= EC__max_, "too many error codes defined");

const std::string usageString = std::string(prj::exeName) + " [options] [DIRECTORY]";

void printHelp()
{
    constexpr int lw = 18;

    cout << prj::appName << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "  " << usageString << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << std::left << setw(lw) << std::string("  ") + argstr::noColor << "monochrome console output" << endl;
    cout << std::left << setw(lw) << std::string("  ") + argstr::help << "prints this help text" << endl;
    cout << std::left << setw(lw) << std::string("  ") + argstr::version << "prints version info" << endl;
    cout << endl;
    cout << "Website: <" << prj::website << ">" << endl;
}

void printUsageAndTryHelp()
{
    cout << "Usage: " << usageString << "\n\n";
    cout << "Try '" << prj::exeName << " --help' for more options." << endl;
}

void printVersion()
{
    const omw::Version& v = prj::version;

    cout << prj::appName << "   ";
    if (v.isPreRelease()) cout << omw::fgBrightMagenta;
    cout << v.toString();
    if (v.isPreRelease()) cout << omw::defaultForeColor;
#ifdef PRJ_DEBUG
    cout << "   " << omw::fgBrightRed << "DEBUG" << omw::defaultForeColor << "   " << __DATE__ << " " << __TIME__;
#endif
    cout << endl;

    cout << endl;
    cout << "project page: " << prj::website << endl;
    cout << endl;
    cout << "Copyright (c) 2024 Oliver Blaser." << endl;
    cout << "License: GNU GPLv3 <http://gnu.org/licenses/>." << endl;
    cout << "This is free software. There is NO WARRANTY." << endl;
}

std::string toString(const fs::file_type& type)
{
    std::string str;

    switch (type)
    {
    case fs::file_type::none:
        std::cout << "none";
        break;

    case fs::file_type::not_found:
        std::cout << "not found";
        break;

    case fs::file_type::regular:
        std::cout << "regular file";
        break;

    case fs::file_type::directory:
        std::cout << "directory";
        break;

    case fs::file_type::symlink:
        std::cout << "symlink";
        break;

    case fs::file_type::block:
        std::cout << "block device";
        break;

    case fs::file_type::character:
        std::cout << "character device";
        break;

    case fs::file_type::fifo:
        std::cout << "fifo/pipe";
        break;

    case fs::file_type::socket:
        std::cout << "socket";
        break;

    case fs::file_type::unknown:
        std::cout << "unknown";
        break;

    default:
        std::cout << "implementation-defined";
        break;
    }

    return str;
}

} // namespace



static int process(const fs::path& dir, size_t& depth);



#ifdef OMW_PLAT_WIN
int wmain(int argc, wchar_t** argv)
{
#if defined(PRJ_DEBUG) && 0 // print argv
    for (int iarg = 0; iarg < argc; ++iarg)
    {
        bool utf = false;

        const size_t len = wcslen(argv[iarg]);
        for (size_t i = 0; i < len; ++i)
        {
            const auto& c = argv[iarg][i];

            if (c > 127)
            {
                if (!utf) cout << '[';
                cout << omw::toHexStr((uint16_t)c);
                utf = true;
            }
            else
            {
                if (utf) cout << ']';
                cout << (char)c;
                utf = false;
            }
        }

        cout << endl;
    }
#endif // print argv

    std::vector<std::string> rawArgs(argc);

    for (int i = 0; i < argc; ++i)
    {
        try
        {
            rawArgs[i] = omw::windows::wstou8(argv[i]);
        }
        catch (const std::exception& ex)
        {
            cout << "faild to convert argv[" << i << "] (" << ex.what() << ")" << endl;
            std::wcout << L"  argv[" << i << L"] " << argv[i] << endl; // console code page is not yet set to UTF-8
            return EC_ERROR;
        }
    }

#ifndef PRJ_DEBUG
    const
#endif // PRJ_DEBUG
        std::vector<std::string>& args = rawArgs;

#else // OMW_PLAT_WIN

int main(int argc, char** argv)
{
    std::vector<std::string> rawArgs(argc);

    for (int i = 0; i < argc; ++i) { rawArgs[i] = argv[i]; }

#ifndef PRJ_DEBUG
    const
#endif // PRJ_DEBUG
        std::vector<std::string>& args = rawArgs;

#endif // OMW_PLAT_WIN

#if defined(PRJ_DEBUG) && 1
    if (args.size() == 0)
    {
        args.push_back("--help");
        args.push_back("--version");
    }
#endif

    if (argstr::contains(args, argstr::noColor)) omw::ansiesc::disable();
    else
    {
        bool envt = true;
#ifdef OMW_PLAT_WIN
        envt = omw::windows::consoleEnVirtualTermProc();
#endif
        omw::ansiesc::enable(envt);
    }

#ifdef OMW_PLAT_WIN
    const auto winOutCodePage = omw::windows::consoleGetOutCodePage();
    bool winOutCodePageRes = omw::windows::consoleSetOutCodePage(65001);
#endif

#ifndef PRJ_DEBUG
    if (prj::version.isPreRelease()) cout << omw::fgBrightMagenta << "pre-release v" << prj::version.toString() << omw::defaultForeColor << endl;
#endif

#if defined(PRJ_DEBUG) && 1
    cout << omw::foreColor(26) << "--======# args #======--\n";
    for (size_t i = 0; i < args.size(); ++i) cout << " " << args[i] << endl;
    cout << "--======# end args #======--" << endl << omw::defaultForeColor;
#endif

    int r = EC_ERROR;

    // if (args.isValid())
    {
        r = EC_OK;

        if (argstr::contains(args, argstr::help)) printHelp();
        else if (argstr::contains(args, argstr::version)) printVersion();
        else
        {
            fs::path dir;
            size_t depth = 0;

            if (args.size() > 1) { dir = argv[1]; }
            else { dir = "."; }

            r = process(dir, depth);
        }
    }
    // else
    //{
    //     r = EC_ERROR;
    //
    //    if (args.count() == 0)
    //    {
    //        cout << "No arguments." << endl;
    //        printUsageAndTryHelp();
    //    }
    //    else if (!args.options().isValid())
    //    {
    //        cout << prj::exeName << ": unrecognized option: '" << args.options().unrecognized() << "'" << endl;
    //        printUsageAndTryHelp();
    //    }
    //    else
    //    {
    //        cout << "Error" << endl;
    //        printUsageAndTryHelp();
    //    }
    //}

#if defined(PRJ_DEBUG) && 1
    cout << omw::foreColor(26) << "===============\nreturn " << r << "\npress enter..." << omw::normal << endl;
#ifdef OMW_PLAT_WIN
    int dbg___getc_ = getc(stdin);
#endif
#endif

    cout << omw::normal << std::flush;

#ifdef OMW_PLAT_WIN
    winOutCodePageRes = omw::windows::consoleSetOutCodePage(winOutCodePage);
#endif

    return r;
}



int process(const fs::path& path, size_t& depth)
{
    if ((depth == 0) && !fs::is_directory(path)) { return EC_NOT_A_DIR; }

    ++depth;

    const fs::file_status stat = fs::symlink_status(path);

    if (fs::is_regular_file(stat))
    {
        const char* const sha1str = "TODO SHA1";

        cout << sha1str << "  " << path << endl;
    }
    else
    {
        if (fs::is_directory(stat))
        {
            for (const auto& entry : std::filesystem::directory_iterator(path)) { process(entry.path(), depth); }
        }
        else if (fs::is_symlink(stat))
        {
            const fs::path target = fs::weakly_canonical(fs::read_symlink(path));

            cout << target.u8string() << " <-  " << path << endl;
        }
        else { cout << "[" << toString(stat.type()) << "]  " << path << endl; }
    }

    --depth;
    return EC_OK;
}