/* -*- c-basic-offset: 8 -*-
   freerdp: A Remote Desktop Protocol client.
   XP Unlimited protocol - Load balancing client extension

   Copyright (C) 2010 Eduardo Beloni <beloni@ossystems.com.br>
   Copyright (C) 2010 O.S. Systems Software LTDA.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freerdp/freerdp.h>

#include "xpu.h"

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

