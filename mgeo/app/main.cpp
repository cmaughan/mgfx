#include "mgeo_app.h"
#include "tclap/CmdLine.h"

#include "load_assimp.h"

INITIALIZE_EASYLOGGINGPP
    
bool ReadCommandLine(int argc, char** argv, int& exitCode)
{
    try
    {
        TCLAP::CmdLine cmd("mgeo - geometry to flatbuffer mmesh conversion tool", ' ', APPLICATION_VERSION);
        TCLAP::SwitchArg verboseArg("l", "log", "Log Output", cmd, false);
        TCLAP::ValueArg<bool> generateTangentsArg("t", "tangents", "Generate Tangents", false, true, "bool");
        TCLAP::ValueArg<std::string> nameArg("i", "input", "File to convert", true, "", "string");

        cmd.add(nameArg);
        cmd.add(generateTangentsArg);
        cmd.setExceptionHandling(false);

        cmd.ignoreUnmatched(false);

        if (argc != 0)
        {
            cmd.parse(argc, argv);
        }
    
        MGeoSettings::Instance().SetVerbose(verboseArg.getValue());
        if (generateTangentsArg.isSet())
        {
            MGeoSettings::Instance().SetGenerateTangents(generateTangentsArg.getValue());
        }
    
        fs::path inputPath(SDL_GetBasePath());
        MGeoSettings::Instance().SetInputPath(inputPath / nameArg.getValue());
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        std::ostringstream strError;
        strError << e.argId() << " : " << e.error();
        LOG(ERROR) << strError.str();
        exitCode = 1;
        return false;
    }
    catch (TCLAP::ExitException& e)
    {
        exitCode = e.getExitStatus();
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    fs::path basePath = SDL_GetBasePath();
    el::Configurations conf((basePath / "logger.conf").string().c_str());
    el::Loggers::reconfigureAllLoggers(conf);

    int exitCode = 0;
    if (!ReadCommandLine(argc, argv, exitCode))
    {
        return exitCode;
    }

    LOG(INFO) << "Loading : " << MGeoSettings::Instance().GetInputPath().string();

    exitCode = 0;
    flatbuffers::FlatBufferBuilder builder;
    if (MGeo::LoadAssimp(MGeoSettings::Instance().GetInputPath(), builder))
    {
        fs::path outPath = MGeoSettings::Instance().GetInputPath();
        outPath.replace_extension(".mmesh");

        if (FileUtils::WriteFile(outPath, builder.GetBufferPointer(), builder.GetSize()))
        {
            LOG(INFO) << "Wrote: " << outPath.string() << ", size: " << builder.GetSize();
        }
        else
        {
            LOG(ERROR) << "Failed to write to: " << outPath.string();
            exitCode = 1;
        }
    }
    else
    {
        LOG(INFO) << "Failed to load file: " << MGeoSettings::Instance().GetInputPath().string();
        exitCode = 1;
    }

    el::Loggers::flushAll();
    return exitCode;
}

