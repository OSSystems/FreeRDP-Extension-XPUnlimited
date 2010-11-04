# FreeRDP - XPUnlimited Extension
# Copyright (C) 2010 O.S. Systems Software LTDA.

find_package(FreeRDP REQUIRED)

include_directories(${FREERDP_INCLUDES})

freerdp_add_extension(xpu MODULE xpu.c xpu_main.c)
