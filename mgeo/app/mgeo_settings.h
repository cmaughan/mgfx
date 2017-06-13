#pragma once

class MGeoSettings
{
public:
    static MGeoSettings& Instance();

    bool GetVerbose() const { return m_verbose; }
    void SetVerbose(bool verbose) { m_verbose = verbose; }

    bool GetGenerateTangents() const { return m_tangents; }
    void SetGenerateTangents(bool gen) { m_tangents = gen; }

    const fs::path& GetInputPath() const { return m_inputPath; }
    void SetInputPath(const fs::path& path) { m_inputPath = path; }

private:
    bool m_verbose = false;
    bool m_tangents = true;
    fs::path m_inputPath;
};
