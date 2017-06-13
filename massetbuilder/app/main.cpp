#include "massetbuilder_app.h"
#include "tclap/CmdLine.h"
#include "assetbuilder.h"

INITIALIZE_EASYLOGGINGPP

namespace MAssetBuilder
{

bool ReadCommandLine(int argc, char** argv, int& exitCode)
{
    try
    {
        TCLAP::CmdLine cmd("massetbuilder - asset convertor/compiler", ' ', APPLICATION_VERSION);
        TCLAP::SwitchArg verboseArg("l", "log", "Log Output", cmd, false);
        TCLAP::ValueArg<std::string> inputDir("i", "input", "Asset source directory", true, "", "string");
        TCLAP::ValueArg<std::string> outputDir("o", "output", "Asset target directory", true, "", "string");

        cmd.add(inputDir);
        cmd.add(outputDir);
        cmd.setExceptionHandling(false);

        cmd.ignoreUnmatched(false);

        if (argc != 0)
        {
            cmd.parse(argc, argv);
        }

        MAssetBuilderSettings::Instance().SetVerbose(verboseArg.getValue());
        MAssetBuilderSettings::Instance().SetInputPath(inputDir.getValue());
        MAssetBuilderSettings::Instance().SetOutputPath(outputDir.getValue());
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
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

} // MAssetBuilder

using namespace MAssetBuilder;

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

    LOG(INFO) << "Running asset builder...";
    AssetBuilder builder;
    builder.GatherAssets();
    if (!builder.CompileAssets())
    {
        exitCode = 1;
    }

    el::Loggers::flushAll();

    LOG(INFO) << "Done! (exit: " << exitCode << ")";
    return exitCode;
}

