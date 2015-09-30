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

#include "internal.xpe_group.h"
#include "../res/c4d_symbols.h"

#include <c4d.h>
#include <c4d_graphview.h>

static String* name = NULL;

const String* XPE_GetName() {
    return name;
}

BaseBitmap* XPE_GetIcon() {
    return NULL;
}

Bool RegisterXPEGroup() {
    static GV_OPGROUP_HANDLER handler;
    ClearMem(&handler, sizeof(handler));

    name = gNew(String);
    if (name) *name = GeLoadString(IDC_XPE_NAME);

    handler.group_id = ID_GV_OPGROUP_TYPE_XPRESSOEFFECTOR;
    handler.GetName = XPE_GetName;
    handler.GetIcon = XPE_GetIcon;

    Bool ok = GvRegisterOpGroupType(&handler, sizeof(handler));
    if (!ok) {
        GePrint(GeLoadString(IDC_XPE_NOTREGISTERED));
    }

    return ok;
}

void FreeXPEGroup() {
    if (name) gDelete(name);
}
