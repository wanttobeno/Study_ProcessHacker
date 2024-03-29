/*
 * Process Hacker Extra Plugins -
 *   Hexadecimal PID Plugin
 *
 * Copyright (C) 2011 wj32
 *
 * This file is part of Process Hacker.
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <phdk.h>
#include "resource.h"
//#include "phplug.h"
#include "phapppub.h"

#define PIDHEX_COLUMN_ID 1

typedef struct _PROCESS_EXTENSION
{
    WCHAR PidHexText[PH_PTR_STR_LEN_1];
} PROCESS_EXTENSION, *PPROCESS_EXTENSION;

PPH_PLUGIN PluginInstance;
PH_CALLBACK_REGISTRATION TreeNewMessageCallbackRegistration;
PH_CALLBACK_REGISTRATION ProcessTreeNewInitializingCallbackRegistration;

VOID TreeNewMessageCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
    )
{
    PPH_PLUGIN_TREENEW_MESSAGE message = Parameter;

    switch (message->Message)
    {
    case TreeNewGetCellText:
        {
            PPH_TREENEW_GET_CELL_TEXT getCellText = message->Parameter1;
            PPH_PROCESS_NODE node;

            node = (PPH_PROCESS_NODE)getCellText->Node;

            switch (message->SubId)
            {
            case PIDHEX_COLUMN_ID:
                {
                    if (!PH_IS_FAKE_PROCESS_ID(node->ProcessId))
                    {
                        PPROCESS_EXTENSION extension;

                        extension = PhPluginGetObjectExtension(PluginInstance, node->ProcessItem, EmProcessItemType);
                        PhInitializeStringRef(&getCellText->Text, extension->PidHexText);
                    }
                }
                break;
            }
        }
        break;
    }
}

LONG NTAPI PidHexSortFunction(
    _In_ PVOID Node1,
    _In_ PVOID Node2,
    _In_ ULONG SubId,
    _In_ PH_SORT_ORDER SortOrder,
    _In_ PVOID Context
    )
{
    PPH_PROCESS_NODE node1 = Node1;
    PPH_PROCESS_NODE node2 = Node2;

    return intptrcmp((LONG_PTR)node1->ProcessId, (LONG_PTR)node2->ProcessId);
}

VOID ProcessTreeNewInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
    )
{
    PPH_PLUGIN_TREENEW_INFORMATION info = Parameter;
    PH_TREENEW_COLUMN column;

    memset(&column, 0, sizeof(PH_TREENEW_COLUMN));
    column.Text = L"PID (Hex)";
    column.Width = 50;
    column.Alignment = PH_ALIGN_RIGHT;
    column.TextFlags = DT_RIGHT;

    PhPluginAddTreeNewColumn(PluginInstance, info->CmData, &column, PIDHEX_COLUMN_ID, NULL, PidHexSortFunction);
}

VOID ProcessItemCreateCallback(
    _In_ PVOID Object,
    _In_ PH_EM_OBJECT_TYPE ObjectType,
    _In_ PVOID Extension
    )
{
    PPH_PROCESS_ITEM processItem = Object;
    PPROCESS_EXTENSION extension = Extension;

    _ultow(HandleToUlong(processItem->ProcessId), extension->PidHexText, 16);
}

LOGICAL DllMain(
    _In_ HINSTANCE Instance,
    _In_ ULONG Reason,
    _Reserved_ PVOID Reserved
    )
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        PPH_PLUGIN_INFORMATION info;

        PluginInstance = PhRegisterPlugin(L"wj32.HexPidPlugin", Instance, &info);

        if (!PluginInstance)
            return FALSE;

        info->DisplayName = L"Hexadecimal PID";
        info->Description = L"Adds a column to display PIDs in hexadecimal.";
        info->Author = L"wj32";

        PhRegisterCallback(PhGetPluginCallback(PluginInstance, PluginCallbackTreeNewMessage),
            TreeNewMessageCallback, NULL, &TreeNewMessageCallbackRegistration);
        PhRegisterCallback(PhGetGeneralCallback(GeneralCallbackProcessTreeNewInitializing),
            ProcessTreeNewInitializingCallback, NULL, &ProcessTreeNewInitializingCallbackRegistration);

        PhPluginSetObjectExtension(PluginInstance, EmProcessItemType, sizeof(PROCESS_EXTENSION),
            ProcessItemCreateCallback, NULL);
    }

    return TRUE;
}
