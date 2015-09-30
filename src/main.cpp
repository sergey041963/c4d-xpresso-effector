/**
 * Copyright (C) 2013-2015 Niklas Rosenstein
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include <c4d.h>

#include "../res/c4d_symbols.h"
#include <CSVEffector.h>
#include <CSVNode.h>
#include <XPressoEffector.h>
#include <MoDataNode.h>
#include "internal.xpe_group.h"

extern Bool RegisterXPressoEffector();
extern void UnloadXPressoEffector();
extern Bool RegisterMoDataNode();
extern void FreeMoDataNode();
extern Bool RegisterCSVEffector();
extern Bool RegisterCSVNode();
extern Bool RegisterHooks();

BaseContainer* FindSubmenu(BaseContainer* menu, const String& ident) {
    if (!menu) return NULL;
    LONG i = 0;
    while (TRUE) {
        LONG idx = menu->GetIndexId(i);
        if (idx == NOTOK) break;

        GeData* data = menu->GetIndexData(i++);
        if (!data || data->GetType() != DA_CONTAINER) continue;

        BaseContainer* sub = data->GetContainer();
        if (!sub) continue;
        if (sub->GetString(MENURESOURCE_SUBTITLE) == ident) return sub;
    }
    return NULL;
}

void EnhanceMenu() {
    BaseContainer* menu = GetMenuResource("M_EDITOR");
    BaseContainer* mograph = FindSubmenu(menu, "IDS_MENU_MOGRAPH");
    BaseContainer* effectors = FindSubmenu(mograph, "IDS_MENU_EFFECTOR");
    if (effectors) {
        effectors->InsData(MENURESOURCE_COMMAND, "PLUGIN_CMD_" + LongToString(ID_CSVEFFECTOR));
        effectors->InsData(MENURESOURCE_COMMAND, "PLUGIN_CMD_" + LongToString(ID_XPRESSOEFFECTOR));
    }
}


Bool PluginStart() {
    // This plugin only works with Cinema 4D R15 and higher.
    /*LONG version = GetC4DVersion();
    if (version < 15000) {
        GePrint(GeLoadString(IDC_C4DVERSIONTOOLD));
        return FALSE;
    }*/

    Bool ok = TRUE;
    ok = RegisterXPEGroup() && ok;
    ok = RegisterXPressoEffector() && ok;
    ok = RegisterMoDataNode() && ok;
    ok = RegisterCSVEffector() && ok;
    ok = RegisterCSVNode() && ok;
    ok = RegisterHooks() && ok;

    if (ok) {
        GePrint(GeLoadString(IDC_REGISTERED));
    }
    return ok;
}

Bool PluginMessage(LONG type, void* pData) {
    switch (type) {
        case C4DPL_INIT_SYS:
            return resource.Init();
        case C4DPL_BUILDMENU:
            EnhanceMenu();
            break;
    };
    return TRUE;
}

void PluginEnd() {
    FreeXPEGroup();
    FreeMoDataNode();
    UnloadXPressoEffector();
}

