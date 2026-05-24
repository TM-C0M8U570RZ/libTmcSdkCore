#include "tmc_application.h"
#include "strextra.h"

tmc::Application* globalInstance;

namespace tmc {

Application::Application(int argc,
                         char** argv,
                         const std::filesystem::path& exefsDir,
                         const std::filesystem::path& romfsDir,
                         const std::string& packageName,
                         AgeRating ar)
{
    globalInstance = this;
    args = std::vector<std::string>(argc);
    for (s32 i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }
    u64 fileStart = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < args.size(); i++)
    {
        if (args[i].size() >= 2 && args[i][0] == '-' && args[i][1] != '-')
        {
            std::vector<char> temp = processShortArgs(args[i]);
            for (u64 j = 0; j < temp.size(); j++)
            {
                shortArgs.push_back({temp[j], std::vector<std::string>({""})});
            }
            u64 j = 1;
            while (args.size() > (i + j) && args[i + j][0] != '-')
            {
                if (j == 1) shortArgs.back().second.clear();
                shortArgs.back().second.push_back(args[i + j]);
                j++;
            }
        }
        else if (args[i].size() > 2 && args[i][0] == '-' && args[i][1] == '-')
        {
            longArgs.push_back({args[i].substr(2, args[i].size() - 2), std::vector<std::string>({""})});
            u64 j = 1;
            if (args.size() > (i + 1) && args[i + 1][0] != '-')
            {
                if (j == 1) longArgs.back().second.clear();
                longArgs.back().second.push_back(args[i + 1]);
                j++;
            }
        }
        else if (args[i] == "--" && args.size() > (i + 1))
        {
            fileStart = i + 1;
            break;
        }
    }
    if (fileStart != std::numeric_limits<u64>::max())
    {
        for (u64 i = fileStart; i < args.size(); i++)
        {
            filepaths.push_back(std::filesystem::path(args[i]));
        }
    }
    this->exefsDir = exefsDir;
    this->romfsDir = romfsDir;
    this->packageName = packageName;
    this->ar = ar;
}

Application* Application::getGlobalInstance(int argc,
                                            char** argv,
                                            const std::filesystem::path& exefsDir,
                                            const std::filesystem::path& romfsDir,
                                            const std::string& packageName,
                                            AgeRating ar)
{
    if (argc != 0 && argv != nullptr)
    {
        Application* a = new Application(argc, argv, exefsDir, romfsDir, packageName, ar);
        if (a->hasArg("TMC-QUERY-EXEFS-DIR"))
        {
            std::cout << exefsDir << "\n";
            exit(0);
        }
        else if (a->hasArg("TMC-QUERY-ROMFS-DIR"))
        {
            std::cout << romfsDir << "\n";
            exit(0);
        }
        else if (a->hasArg("TMC-QUERY-PACKAGE-NAME"))
        {
            std::cout << packageName << "\n";
            exit(0);
        }
        else if (a->hasArg("TMC-QUERY-AGE-RATING"))
        {
            switch (ar)
            {
            case AgeRating::EC: // The software is educational software for preschool-age kids
                std::cout << "EC\n";
                exit(0);
            case AgeRating::E: // The software is safe for all ages
                std::cout << "E\n";
                exit(0);
            case AgeRating::E10: // The software is for ages 10 and up (contains content such as mild crude humor and simulated gambling)
                std::cout << "E10\n";
                exit(0);
            case AgeRating::T: // The software is for teens and adults (Contains content such as mild blood and some swearing)
                std::cout << "T\n";
                exit(0);
            case AgeRating::M: // The software is for ages 17 and up (Contains content such as nudity without sexual acts and extreme violence)
                std::cout << "M\n";
                exit(0);
            case AgeRating::AO: // The software is only for adults (contains content such as pornography and real gambling)
                std::cout << "AO\n";
                exit(0);
            }
        }
    }
    return globalInstance;
}

template <>
std::vector<bool> Application::getAs(const std::string& longArg)
{
    std::vector<bool> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(toAllUpper(longArgs[idx].second[i]) == "TRUE");
    }
    return result;
}

std::filesystem::path Application::resolveResource(const std::string& resourcePath)
{
    std::size_t slashPos = resourcePath.find("/");
    if (slashPos != std::string::npos && slashPos != 0)
    {
        std::string topLevel = resourcePath.substr(0, slashPos);
        std::string relativeResource = resourcePath.substr(slashPos + 1);
        if (!ignCaseStrComp(topLevel, "@romfs") || !ignCaseStrComp(topLevel, "romfs:"))
        {
            return getRomfsDir() / relativeResource;
        }
        else if (!ignCaseStrComp(topLevel, "@exefs") || !ignCaseStrComp(topLevel, "exefs:"))
        {
            return getExefsDir() / relativeResource;
        }
        return resourcePath;
    }
    return resourcePath;
}

template <>
std::vector<char> Application::getAs(const std::string& longArg)
{
    std::vector<char> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(longArgs[idx].second[i][0]);
    }
    return result;
}

template <>
std::vector<u8> Application::getAs(const std::string& longArg)
{
    std::vector<u8> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<u8>(std::stoull(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<s8> Application::getAs(const std::string& longArg)
{
    std::vector<s8> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<s8>(std::stoll(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<u16> Application::getAs(const std::string& longArg)
{
    std::vector<u16> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<u16>(std::stoull(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<s16> Application::getAs(const std::string& longArg)
{
    std::vector<s16> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<s16>(std::stoll(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<u32> Application::getAs(const std::string& longArg)
{
    std::vector<u32> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<u32>(std::stoull(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<s32> Application::getAs(const std::string& longArg)
{
    std::vector<s32> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<s32>(std::stoll(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<u64> Application::getAs(const std::string& longArg)
{
    std::vector<u64> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(std::stoull(longArgs[idx].second[i]));
    }
    return result;
}

template <>
std::vector<s64> Application::getAs(const std::string& longArg)
{
    std::vector<s64> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(std::stoll(longArgs[idx].second[i]));
    }
    return result;
}

template <>
std::vector<f32> Application::getAs(const std::string& longArg)
{
    std::vector<f32> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<f32>(std::stod(longArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<f64> Application::getAs(const std::string& longArg)
{
    std::vector<f64> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(std::stod(longArgs[idx].second[i]));
    }
    return result;
}

template <>
std::vector<std::string> Application::getAs(const std::string& longArg)
{
    std::vector<std::string> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(longArgs[idx].second[i]);
    }
    return result;
}

template <>
std::vector<std::filesystem::path> Application::getAs(const std::string& longArg)
{
    std::vector<std::filesystem::path> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < longArgs[idx].second.size(); i++)
    {
        result.push_back(std::filesystem::path(longArgs[idx].second[i]));
    }
    return result;
}

template <>
std::string Application::getAs(const std::string& longArg)
{
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    return longArgs[idx].second[0];
}

template <>
bool Application::getAs(const std::string& longArg)
{
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    return (toAllUpper(longArgs[idx].second[0]) == "TRUE");
}

template <>
std::filesystem::path Application::getAs(const std::string& longArg)
{
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    return std::filesystem::path(longArgs[idx].second[0]);
}

template <>
std::vector<bool> Application::getAs(char shortArg)
{
    std::vector<bool> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(toAllUpper(shortArgs[idx].second[i]) == "TRUE");
    }
    return result;
}

template <>
std::vector<char> Application::getAs(char shortArg)
{
    std::vector<char> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(shortArgs[idx].second[i][0]);
    }
    return result;
}

template <>
std::vector<u8> Application::getAs(char shortArg)
{
    std::vector<u8> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<u8>(std::stoull(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<s8> Application::getAs(char shortArg)
{
    std::vector<s8> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<s8>(std::stoll(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<u16> Application::getAs(char shortArg)
{
    std::vector<u16> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<u16>(std::stoull(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<s16> Application::getAs(char shortArg)
{
    std::vector<s16> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<s16>(std::stoll(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<u32> Application::getAs(char shortArg)
{
    std::vector<u32> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<u32>(std::stoull(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<s32> Application::getAs(char shortArg)
{
    std::vector<s32> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<s32>(std::stoll(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<u64> Application::getAs(char shortArg)
{
    std::vector<u64> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(std::stoull(shortArgs[idx].second[i]));
    }
    return result;
}

template <>
std::vector<s64> Application::getAs(char shortArg)
{
    std::vector<s64> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(std::stoll(shortArgs[idx].second[i]));
    }
    return result;
}

template <>
std::vector<f32> Application::getAs(char shortArg)
{
    std::vector<f32> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(static_cast<f32>(std::stod(shortArgs[idx].second[i])));
    }
    return result;
}

template <>
std::vector<f64> Application::getAs(char shortArg)
{
    std::vector<f64> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(std::stod(shortArgs[idx].second[i]));
    }
    return result;
}

template <>
std::vector<std::string> Application::getAs(char shortArg)
{
    std::vector<std::string> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(shortArgs[idx].second[i]);
    }
    return result;
}

template <>
std::vector<std::filesystem::path> Application::getAs(char shortArg)
{
    std::vector<std::filesystem::path> result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    for (u64 i = 0; i < shortArgs[idx].second.size(); i++)
    {
        result.push_back(std::filesystem::path(shortArgs[idx].second[i]));
    }
    return result;
}

template <>
std::string Application::getAs(char shortArg)
{
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    return shortArgs[idx].second[0];
}

template <>
bool Application::getAs(char shortArg)
{
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    return (toAllUpper(shortArgs[idx].second[0]) == "TRUE");
}

template <>
std::filesystem::path Application::getAs(char shortArg)
{
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    return std::filesystem::path(shortArgs[idx].second[0]);
}

template <>
u8 Application::getAs(const std::string& longArg)
{
    u8 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u8>(std::stoull(longArgs[idx].second[0]));
    return result;
}

template <>
u16 Application::getAs(const std::string& longArg)
{
    u16 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u16>(std::stoull(longArgs[idx].second[0]));
    return result;
}

template <>
u32 Application::getAs(const std::string& longArg)
{
    u32 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u32>(std::stoull(longArgs[idx].second[0]));
    return result;
}

template <>
u64 Application::getAs(const std::string& longArg)
{
    u64 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u64>(std::stoull(longArgs[idx].second[0]));
    return result;
}

template <>
s8 Application::getAs(const std::string& longArg)
{
    s8 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s8>(std::stoll(longArgs[idx].second[0]));
    return result;
}

template <>
s16 Application::getAs(const std::string& longArg)
{
    s16 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s16>(std::stoll(longArgs[idx].second[0]));
    return result;
}

template <>
s32 Application::getAs(const std::string& longArg)
{
    s32 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s32>(std::stoll(longArgs[idx].second[0]));
    return result;
}

template <>
s64 Application::getAs(const std::string& longArg)
{
    u64 result;
    u64 idx = std::numeric_limits<s64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s64>(std::stoll(longArgs[idx].second[0]));
    return result;
}

template <>
f32 Application::getAs(const std::string& longArg)
{
    f32 result;
    u64 idx = std::numeric_limits<s64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<f32>(std::stod(longArgs[idx].second[0]));
    return result;
}

template <>
f64 Application::getAs(const std::string& longArg)
{
    f64 result;
    u64 idx = std::numeric_limits<s64>::max();
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<f64>(std::stod(longArgs[idx].second[0]));
    return result;
}

template <>
u8 Application::getAs(char shortArg)
{
    u8 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u8>(std::stoull(shortArgs[idx].second[0]));
    return result;
}

template <>
u16 Application::getAs(char shortArg)
{
    u16 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u16>(std::stoull(shortArgs[idx].second[0]));
    return result;
}

template <>
u32 Application::getAs(char shortArg)
{
    u32 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u32>(std::stoull(shortArgs[idx].second[0]));
    return result;
}

template <>
u64 Application::getAs(char shortArg)
{
    u64 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<u64>(std::stoull(shortArgs[idx].second[0]));
    return result;
}

template <>
s8 Application::getAs(char shortArg)
{
    s8 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s8>(std::stoll(shortArgs[idx].second[0]));
    return result;
}

template <>
s16 Application::getAs(char shortArg)
{
    s16 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s16>(std::stoll(shortArgs[idx].second[0]));
    return result;
}

template <>
s32 Application::getAs(char shortArg)
{
    s32 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s32>(std::stoll(shortArgs[idx].second[0]));
    return result;
}

template <>
s64 Application::getAs(char shortArg)
{
    s64 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<s64>(std::stoll(shortArgs[idx].second[0]));
    return result;
}

template <>
f32 Application::getAs(char shortArg)
{
    f32 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<f32>(std::stod(shortArgs[idx].second[0]));
    return result;
}

template <>
f64 Application::getAs(char shortArg)
{
    f64 result;
    u64 idx = std::numeric_limits<u64>::max();
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg)
        {
            idx = i;
            break;
        }
    }
    if (idx == std::numeric_limits<u64>::max())
    {
        throw(std::runtime_error("Argument not found!"));
    }
    result = static_cast<f64>(std::stod(shortArgs[idx].second[0]));
    return result;
}

std::vector<char> Application::processShortArgs(std::string shortArgs)
{
    std::vector<char> result;
    for (u64 i = 1; i < shortArgs.size(); i++)
    {
        result.push_back(shortArgs[i]);
    }
    return result;
}

bool Application::hasArg(const std::string& longArg)
{
    for (u64 i = 0; i < longArgs.size(); i++)
    {
        if (longArgs[i].first == longArg) return true;
    }
    return false;
}

bool Application::hasArg(char shortArg)
{
    for (u64 i = 0; i < shortArgs.size(); i++)
    {
        if (shortArgs[i].first == shortArg) return true;
    }
    return false;
}

std::filesystem::path Application::getExefsDir()
{
    if (exefsDir.empty())
    {
        std::filesystem::path result(args[0]);
        result = result.parent_path();
        return result;
    }
    return exefsDir;
}

std::filesystem::path Application::getRomfsDir()
{
    if (romfsDir.empty())
    {
        std::filesystem::path result(args[0]);
        result = result.parent_path();
        result /= packageName + "_romfs";
        return result;
    }
    return romfsDir;
}

std::string Application::getPackageName()
{
    return packageName;
}

std::filesystem::path Application::getLocalConfigDir()
{
#ifdef ID_JUST_LIKE_TO_INTERJECT
    std::filesystem::path result = getenv("HOME");
    result /= ".local/share";
    result /= packageName;
#elif defined ITS_WINDOWS_FROM_MICROSOFT
    std::filesystem::path result = getenv("APPDATA");
    result /= packageName;
#else
    std::filesystem::path result = getenv("HOME");
    result /= "Library/Application Support";
    result /= packageName;
#endif
    return result;
}

} // namespace tmc
