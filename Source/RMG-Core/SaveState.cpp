/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "SaveState.hpp"
#include "Directories.hpp"
#include "RomSettings.hpp"
#include "RomHeader.hpp"
#include "Error.hpp"

#include "m64p/Api.hpp"

#include <algorithm>

//
// Local Functions
//

// replaces all occurences of any chars in replace with c inside str
static void str_replace_chars(std::string& str, const std::string replace, const char c)
{
    for (int i = 0; i < str.size(); i++)
    {
        char str_char = str.at(i);
        if (replace.find(str_char) != std::string::npos)
        {
            str.at(i) = c;
        }
    }
}

//
// Exported Functions
//

bool CoreSetSaveStateSlot(int slot)
{
    std::string error;
    m64p_error ret;

    if (!m64p::Core.IsHooked())
    {
        return false;
    }

    ret = m64p::Core.DoCommand(M64CMD_CORE_STATE_SET, M64CORE_SAVESTATE_SLOT, &slot);
    if (ret != M64ERR_SUCCESS)
    {
        error = "CoreSetSaveStateSlot m64p::Core.DoCommand(M64CMD_CORE_STATE_SET, M64CORE_SAVESTATE_SLOT) Failed: ";
        error += m64p::Core.ErrorMessage(ret);
        CoreSetError(error);
    }

    return ret == M64ERR_SUCCESS;
}

int CoreGetSaveStateSlot(void)
{
    std::string error;
    m64p_error ret;
    int slot = -1;

    if (!m64p::Core.IsHooked())
    {
        return -1;
    }

    ret = m64p::Core.DoCommand(M64CMD_CORE_STATE_QUERY, M64CORE_SAVESTATE_SLOT, &slot);
    if (ret != M64ERR_SUCCESS)
    {
        error = "CoreGetSaveStateSlot m64p::Core.DoCommand(M64CMD_CORE_STATE_QUERY, M64CORE_SAVESTATE_SLOT) Failed: ";
        error += m64p::Core.ErrorMessage(ret);
        CoreSetError(error);
    }

    return slot;
}

bool CoreGetSaveStatePath(int slot, std::filesystem::path& path)
{
    // TODO: this should probably be an API function
    // in mupen64plus-core instead

    std::filesystem::path saveStatePath;
    std::filesystem::path oldSaveStatePath;
    std::string saveStateFileName;
    std::filesystem::path saveStateExtension;
    CoreRomHeader romHeader;
    CoreRomSettings romSettings;

    // attempt to retrieve the current
    // rom header and settings
    if (!CoreGetCurrentRomHeader(romHeader) ||
        !CoreGetCurrentRomSettings(romSettings))
    {
        return false;
    }

    // retrieve save state directory
    saveStatePath = CoreGetSaveStateDirectory();
    saveStatePath += "/";

    // construct extension
    saveStateExtension = ".st";
    saveStateExtension += std::to_string(slot);

    // construct old save state path
    oldSaveStatePath = saveStatePath;
    oldSaveStatePath += romSettings.GoodName;
    oldSaveStatePath += saveStateExtension;

    // use old filename if it exists
    if (std::filesystem::is_regular_file(oldSaveStatePath))
    {
        path = oldSaveStatePath;
        return true;
    }

    // else use new filename
    if (romSettings.GoodName.find("(unknown rom)") == std::string::npos)
    {
        if (romSettings.GoodName.size() < 32)
        {
            saveStatePath += romSettings.GoodName;
        }
        else
        {
            saveStatePath += romSettings.GoodName.substr(0, 32);
        }
    }
    else if (!romHeader.Name.empty())
    {
        saveStatePath += romHeader.Name;
    }
    else
    {
        saveStatePath += "unknown";
    }
    saveStatePath += "-";
    saveStatePath += romSettings.MD5.substr(0, 8);
    saveStatePath += saveStateExtension;

    // retrieve filename from path
    saveStateFileName = saveStatePath.filename().string();

    // sanitize filename
    str_replace_chars(saveStateFileName, ":<>\"/\\|?*", '_');

    // replace filename in path
    saveStatePath.replace_filename(saveStateFileName);

    path = saveStatePath;
    return true;
}

bool CoreSaveState(void)
{
    std::string error;
    m64p_error ret;

    if (!m64p::Core.IsHooked())
    {
        return false;
    }

    ret = m64p::Core.DoCommand(M64CMD_STATE_SAVE, 0, nullptr);
    if (ret != M64ERR_SUCCESS)
    {
        error = "CoreSaveState: m64p::Core.DoCommand(M64CMD_STATE_SAVE) Failed: ";
        error += m64p::Core.ErrorMessage(ret);
        CoreSetError(error);
    }

    return ret == M64ERR_SUCCESS;
}

bool CoreSaveState(std::filesystem::path file)
{
    std::string error;
    m64p_error ret;

    if (!m64p::Core.IsHooked())
    {
        return false;
    }

    ret = m64p::Core.DoCommand(M64CMD_STATE_SAVE, 1, (void*)file.string().c_str());
    if (ret != M64ERR_SUCCESS)
    {
        error = "CoreSaveState: m64p::Core.DoCommand(M64CMD_STATE_SAVE) Failed: ";
        error += m64p::Core.ErrorMessage(ret);
        CoreSetError(error);
    }

    return ret == M64ERR_SUCCESS;
}

bool CoreLoadSaveState(void)
{
    std::string error;
    m64p_error ret;

    if (!m64p::Core.IsHooked())
    {
        return false;
    }

    ret = m64p::Core.DoCommand(M64CMD_STATE_LOAD, 0, nullptr);
    if (ret != M64ERR_SUCCESS)
    {
        error = "CoreLoadSaveState: m64p::Core.DoCommand(M64CMD_STATE_LOAD) Failed: ";
        error += m64p::Core.ErrorMessage(ret);
        CoreSetError(error);
    }

    return ret == M64ERR_SUCCESS;
}

bool CoreLoadSaveState(std::filesystem::path file)
{
    std::string error;
    m64p_error ret;

    if (!m64p::Core.IsHooked())
    {
        return false;
    }

    ret = m64p::Core.DoCommand(M64CMD_STATE_LOAD, 0, (void*)file.string().c_str());
    if (ret != M64ERR_SUCCESS)
    {
        error = "CoreLoadSaveState: m64p::Core.DoCommand(M64CMD_STATE_LOAD) Failed: ";
        error += m64p::Core.ErrorMessage(ret);
        CoreSetError(error);
    }

    return ret == M64ERR_SUCCESS;
}
