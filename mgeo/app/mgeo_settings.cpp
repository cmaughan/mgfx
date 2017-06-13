#include "mgeo_app.h"

MGeoSettings& MGeoSettings::Instance()
{
    static MGeoSettings settings;
    return settings;
}

