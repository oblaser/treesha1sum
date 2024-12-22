/*
author          Oliver Blaser
date            22.12.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <fstream>
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

// const char* const changeDir = "--cd";
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

bool isOption(const std::string& arg) { return (/*(arg == changeDir) ||*/ (arg == noColor) || (arg == help) || (arg == version)); }

} // namespace argstr

namespace {

enum EXITCODE // https://tldp.org/LDP/abs/html/exitcodes.html / on MSW are no preserved codes
{
    EC_OK = 0,
    EC_ERROR = 1,

    EC__begin_ = 79,

    // EC_asdf = EC__begin_,
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
    // cout << std::left << setw(lw) << std::string("  ") + argstr::changeDir << "change to DIRECTORY before executing" << endl;
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

} // namespace



static bool checkArgs(const std::vector<std::string>& args);
static void process(const fs::path& dir, size_t& depth);
static std::string pathStr(const fs::path& path);
static std::string toString(const fs::file_type& type);



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
        auto args = std::vector<std::string>(rawArgs.begin() + 1, rawArgs.end());

#else // OMW_PLAT_WIN

int main(int argc, char** argv)
{
    std::vector<std::string> rawArgs(argc);

    for (int i = 0; i < argc; ++i) { rawArgs[i] = argv[i]; }

#ifndef PRJ_DEBUG
    const
#endif // PRJ_DEBUG
        auto args = std::vector<std::string>(rawArgs.begin() + 1, rawArgs.end());

#endif // OMW_PLAT_WIN

#if defined(PRJ_DEBUG) && 1
    if (args.size() == 0)
    {
        // args.push_back("--no-color");
        // args.push_back("--help");
        // args.push_back("--version");

        // args.push_back("../../test/system/input");
        // args.push_back("../../test/system/input/empty.txt");
        args.push_back("../../test/system/input/\xC3\xA4/just another text file.txt");
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

    if (/*args.isValid()*/ checkArgs(args))
    {
        r = EC_OK;

        if (argstr::contains(args, argstr::help)) printHelp();
        else if (argstr::contains(args, argstr::version)) printVersion();
        else
        {
            std::string dir = args.back();
            size_t depth = 0;

            if (argstr::isOption(dir)) { dir = "."; }

            const fs::path dirPath =
#ifdef OMW_PLAT_WIN
                dir; // omw::windows::u8tows(dir);
#else
                dir;
#endif

            process(dirPath, depth);
            r = EC_OK;
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



bool checkArgs(const std::vector<std::string>& args)
{
    bool ok = true;

    for (size_t i = 0; ok && (i < (args.size() - 1)); ++i)
    {
        const auto& arg = args[i];

        if (!argstr::isOption(arg))
        {
            ok = false;

            cout << "unknown option: \"" << arg << "\"" << endl;
        }
    }

    if (!ok)
    {
        cout << endl;
        printUsageAndTryHelp();
    }

    return ok;
}

void process(const fs::path& path, size_t& depth)
{
    ++depth;

    const fs::file_status stat = fs::symlink_status(path);

    if (fs::is_regular_file(stat))
    {
        SHA1 sha1;
        std::ifstream fstream(path, std::ios::binary);

        sha1.update(fstream);

        cout << sha1.digest() << " *" << pathStr(path) << endl;
    }
    else
    {
        if (fs::is_directory(stat))
        {
            for (const auto& entry : std::filesystem::directory_iterator(path)) { process(entry.path(), depth); }
        }
        else
        {
            cout << std::left << setw(SHA1::digestSize * 2) << ("[" + toString(stat.type()) + "]") << std::right;

            if (fs::is_symlink(stat))
            {
                const fs::path target = fs::weakly_canonical(fs::read_symlink(path));

                cout << "  " << pathStr(path) << " -> " << pathStr(target);
            }
            else { cout << "  " << pathStr(path); }

            cout << endl;
        }
    }

    --depth;
}

std::string pathStr(const fs::path& path)
{
#ifdef OMW_PLAT_WIN
    std::string r = path.lexically_normal().u8string();
    return omw::replaceAll(r, '\\', ' /');
#else
    return path.lexically_normal().u8string();
#endif
}

std::string toString(const fs::file_type& type)
{
    std::string str;

    switch (type)
    {
    case fs::file_type::none:
        str = "none";
        break;

    case fs::file_type::not_found:
        str = "not found";
        break;

    case fs::file_type::regular:
        str = "regular file";
        break;

    case fs::file_type::directory:
        str = "directory";
        break;

    case fs::file_type::symlink:
        str = "symlink";
        break;

    case fs::file_type::block:
        str = "block device";
        break;

    case fs::file_type::character:
        str = "character device";
        break;

    case fs::file_type::fifo:
        str = "fifo/pipe";
        break;

    case fs::file_type::socket:
        str = "socket";
        break;

    case fs::file_type::unknown:
        str = "unknown";
        break;

    default:
        str = "implementation-defined";
        break;
    }

    return str;
}
