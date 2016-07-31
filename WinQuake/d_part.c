/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// d_part.c: software driver module for drawing particles

#include "quakedef.h"
#include "d_local.h"


/*
==============
D_EndParticles
==============
*/
void D_EndParticles (void)
{
// not used by software driver
}


/*
==============
D_StartParticles
==============
*/
void D_StartParticles (void)
{
// not used by software driver
}

static int		part_u;
static int		part_v;
static int		part_izi;
static int		part_size;
static int		part_size;
static byte		part_color;

void D_DrawParticlePixels8(void)
{
	byte	*pdest;
	short	*pz;
	int		i, count;

	pz = d_pzbuffer + (d_zwidth * part_v) + part_u;
	pdest = d_viewbuffer + d_scantable[part_v] + part_u;

	switch (part_size)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = part_color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = part_color;
			}

			if (pz[1] <= part_izi)
			{
				pz[1] = part_izi;
				pdest[1] = part_color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = part_color;
			}

			if (pz[1] <= part_izi)
			{
				pz[1] = part_izi;
				pdest[1] = part_color;
			}

			if (pz[2] <= part_izi)
			{
				pz[2] = part_izi;
				pdest[2] = part_color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = part_color;
			}

			if (pz[1] <= part_izi)
			{
				pz[1] = part_izi;
				pdest[1] = part_color;
			}

			if (pz[2] <= part_izi)
			{
				pz[2] = part_izi;
				pdest[2] = part_color;
			}

			if (pz[3] <= part_izi)
			{
				pz[3] = part_izi;
				pdest[3] = part_color;
			}
		}
		break;

	default:
		count = part_size << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<part_size ; i++)
			{
				if (pz[i] <= part_izi)
				{
					pz[i] = part_izi;
					pdest[i] = part_color;
				}
			}
		}
		break;
	}
}

void D_DrawParticlePixels32(void)
{
	int	*pdest;
	short	*pz;
	int		i, count;
	int		color;

	color = d_8to24table[part_color];

	pz = d_pzbuffer + (d_zwidth * part_v) + part_u;
	pdest = (int*)d_viewbuffer + d_scantable[part_v] + part_u;

	switch (part_size)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = color;
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = color;
			}

			if (pz[1] <= part_izi)
			{
				pz[1] = part_izi;
				pdest[1] = color;
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = color;
			}

			if (pz[1] <= part_izi)
			{
				pz[1] = part_izi;
				pdest[1] = color;
			}

			if (pz[2] <= part_izi)
			{
				pz[2] = part_izi;
				pdest[2] = color;
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= part_izi)
			{
				pz[0] = part_izi;
				pdest[0] = color;
			}

			if (pz[1] <= part_izi)
			{
				pz[1] = part_izi;
				pdest[1] = color;
			}

			if (pz[2] <= part_izi)
			{
				pz[2] = part_izi;
				pdest[2] = color;
			}

			if (pz[3] <= part_izi)
			{
				pz[3] = part_izi;
				pdest[3] = color;
			}
		}
		break;

	default:
		count = part_size << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i=0 ; i<part_size ; i++)
			{
				if (pz[i] <= part_izi)
				{
					pz[i] = part_izi;
					pdest[i] = color;
				}
			}
		}
		break;
	}
}

/*
==============
D_DrawParticle
==============
*/
void D_DrawParticle (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;

// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);		

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	part_u = (int)(xcenter + zi * transformed[0] + 0.5);
	part_v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((part_v > d_vrectbottom_particle) || 
		(part_u > d_vrectright_particle) ||
		(part_v < d_vrecty) ||
		(part_u < d_vrectx))
	{
		return;
	}

	part_izi = (int)(zi * 0x8000);

	part_size = part_izi >> d_pix_shift;

	if (part_size < d_pix_min)
		part_size = d_pix_min;
	else if (part_size > d_pix_max)
		part_size = d_pix_max;

	part_color = pparticle->color;
	d_drawparticlepixels();
}

