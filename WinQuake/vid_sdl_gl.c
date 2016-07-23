/*
Copyright (C) 1996-1997 Id Software, Inc.
20016 Atröm "Panzerschrek" Kunç.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#include "winquake.h"

cvar_t	gl_ztrick = {"gl_ztrick","0"};
qboolean isPermedia = false;

float		gldepthmin = -1.0f, gldepthmax = 1.0f;
qboolean gl_mtexable = true;

int		texture_extension_number = 1;
int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

unsigned char d_15to8table[65536];

BINDTEXFUNCPTR bindTexFunc;

qboolean VID_Is8bit(void)
{
	return false;
}

void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
}

void GL_EndRendering (void)
{
}