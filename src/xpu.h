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

#ifndef __XPU_H
#define __XPU_H

#define ERROR(fmt, ...)  fprintf(stderr, "XPU Error %s (%d): " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__)
#define DEBUG(fmt, ...)  printf("XPU %s (%d): " fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__)

void
xpu_prefered_server(char *rdp_server, int *rdp_port, const char *user, int xpu_port);

#endif
