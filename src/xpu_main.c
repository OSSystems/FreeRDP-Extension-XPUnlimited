/* -*- c-basic-offset: 8 -*-
   freerdp: A Remote Desktop Protocol client.
   XP Unlimited protocol - Load balancing client extension

   Copyright (C) 2010 Eduardo Beloni <beloni@ossystems.com.br>
   Copyright (C) 2010 O.S. Systems Software LTDA.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freerdp/freerdp.h>

#include "xpu.h"

#if !defined(RDPEXT_API)
# error FreeRDP needs extension support
#else
# if RDPEXT_API < 1
#  error FreeRDP extension API version 1 or later is required
# endif
#endif

struct xpu_plugin
{
	rdpExtPlugin plugin;

	int port;
};
typedef struct xpu_plugin xpuPlugin;

static int
xpu_init(rdpExtPlugin * plugin, rdpInst * inst)
{
	xpuPlugin * xpu = (xpuPlugin *) plugin;

	DEBUG("port = %d\n", xpu->port);

	return 0;
}

static int
xpu_uninit(rdpExtPlugin * plugin, rdpInst * inst)
{
	DEBUG("xpu extension uninit\n");
	free(plugin);

	return 0;
}

static uint32 RDPEXT_CC
xpu_pre_connect(rdpExtPlugin * plugin, rdpInst * inst)
{
	xpuPlugin * xpu = (xpuPlugin *) plugin;

	xpu_prefered_server(inst->settings->server, &inst->settings->tcp_port_rdp,
		inst->settings->username, xpu->port);

	DEBUG("xpu port: %d, %s:%d\n", xpu->port,
		inst->settings->server, inst->settings->tcp_port_rdp);
	return 0;
}

static uint32 RDPEXT_CC
xpu_post_connect(rdpExtPlugin * plugin, rdpInst * inst)
{
	xpuPlugin * xpu = (xpuPlugin *) plugin;

	DEBUG("xpu port: %d, %s:%d\n", xpu->port,
		inst->settings->server, inst->settings->tcp_port_rdp);
	return 0;
}

int RDPEXT_CC
FreeRDPExtensionEntry(PFREERDP_EXTENSION_ENTRY_POINTS pEntryPoints)
{
	xpuPlugin * xpu;

	xpu = (xpuPlugin *) malloc(sizeof(xpuPlugin));
	memset(xpu, 0, sizeof(xpuPlugin));

	xpu->plugin.ext = pEntryPoints->ext;
	xpu->plugin.init = xpu_init;
	xpu->plugin.uninit = xpu_uninit;

	if (pEntryPoints->data)
	{
		xpu->port = atoi(pEntryPoints->data);
	}
	else
	{
		xpu->port = 3390; /* default XPU port */
	}

	pEntryPoints->pRegisterExtension((rdpExtPlugin *) xpu);
	pEntryPoints->pRegisterPreConnectHook((rdpExtPlugin *) xpu, xpu_pre_connect);
	pEntryPoints->pRegisterPostConnectHook((rdpExtPlugin *) xpu, xpu_post_connect);

	return 0;
}

