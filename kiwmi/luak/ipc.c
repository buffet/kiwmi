/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "luak/ipc.h"

#include <lauxlib.h>
#include <wlr/util/log.h>

#include "kiwmi-ipc-protocol.h"
#include "luak/luak.h"

static void
ipc_eval(
    struct wl_client *client,
    struct wl_resource *resource,
    uint32_t id,
    const char *message)
{
    struct kiwmi_server *server = wl_resource_get_user_data(resource);
    struct wl_resource *command_resource =
        wl_resource_create(client, &kiwmi_command_interface, 1, id);
    lua_State *L = server->lua->L;

    int top = lua_gettop(L);

    lua_pushboolean(L, true);
    lua_setglobal(L, "FROM_KIWMIC");

    if (luaL_dostring(L, message)) {
        const char *error = lua_tostring(L, -1);
        wlr_log(WLR_ERROR, "Error running IPC command: %s", error);
        kiwmi_command_send_done(
            command_resource, KIWMI_COMMAND_ERROR_FAILURE, error);
        lua_pop(L, 1);

        lua_pushboolean(L, false);
        lua_setglobal(L, "FROM_KIWMIC");

        return;
    }

    lua_pushboolean(L, false);
    lua_setglobal(L, "FROM_KIWMIC");

    int results = top - lua_gettop(L);

    if (results == 0) {
        kiwmi_command_send_done(
            command_resource, KIWMI_COMMAND_ERROR_SUCCESS, "");
    } else {
        lua_getglobal(L, "tostring");
        lua_insert(L, -2);

        if (lua_pcall(L, 1, 1, 0)) {
            const char *error = lua_tostring(L, -1);
            wlr_log(WLR_ERROR, "Error running IPC command: %s", error);
            kiwmi_command_send_done(
                command_resource, KIWMI_COMMAND_ERROR_FAILURE, error);
            lua_pop(L, 1);
            return;
        }

        kiwmi_command_send_done(
            command_resource, KIWMI_COMMAND_ERROR_SUCCESS, lua_tostring(L, -1));
    }

    lua_pop(L, results);
}

static const struct kiwmi_ipc_interface kiwmi_ipc_implementation = {
    .eval = ipc_eval,
};

static void
kiwmi_server_resource_destroy(struct wl_resource *UNUSED(resource))
{
    // EMPTY
}

static void
ipc_server_bind(
    struct wl_client *client,
    void *data,
    uint32_t version,
    uint32_t id)
{
    struct kiwmi_server *server = data;
    struct wl_resource *resource =
        wl_resource_create(client, &kiwmi_ipc_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(
        resource,
        &kiwmi_ipc_implementation,
        server,
        kiwmi_server_resource_destroy);
}

bool
luaK_ipc_init(struct kiwmi_server *server, struct kiwmi_lua *lua)
{
    lua->global = wl_global_create(
        server->wl_display, &kiwmi_ipc_interface, 1, server, ipc_server_bind);
    if (!lua->global) {
        wlr_log(WLR_ERROR, "Failed to create IPC global");
        return false;
    }

    return true;
}
