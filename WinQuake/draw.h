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

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

extern	qpic_t		*draw_disc;	// also used on sbar

extern void (*Draw_Init) (void);
extern void (*Draw_Character) (int x, int y, int num);
extern void (*Draw_DebugChar) (char num);
extern void (*Draw_Pic )(int x, int y, qpic_t *pic);
extern void (*Draw_TransPic) (int x, int y, qpic_t *pic);
extern void (*Draw_TransPicTranslate) (int x, int y, qpic_t *pic, byte *translation);
extern void (*Draw_ConsoleBackground) (int lines);
extern void (*Draw_BeginDisc) (void);
extern void (*Draw_EndDisc) (void);
extern void (*Draw_TileClear) (int x, int y, int w, int h);
extern void (*Draw_Fill) (int x, int y, int w, int h, int c);
extern void (*Draw_FadeScreen) (void);
extern void (*Draw_String) (int x, int y, char *str);
extern qpic_t *(*Draw_PicFromWad) (char *name);
extern qpic_t *(*Draw_CachePic) (char *path);

void Draw_S_Init (void);
void Draw_S_Character (int x, int y, int num);
void Draw_S_DebugChar (char num);
void Draw_S_Pic (int x, int y, qpic_t *pic);
void Draw_S_TransPic (int x, int y, qpic_t *pic);
void Draw_S_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation);
void Draw_S_ConsoleBackground (int lines);
void Draw_S_BeginDisc (void);
void Draw_S_EndDisc (void);
void Draw_S_TileClear (int x, int y, int w, int h);
void Draw_S_Fill (int x, int y, int w, int h, int c);
void Draw_S_FadeScreen (void);
void Draw_S_String (int x, int y, char *str);
qpic_t *Draw_S_PicFromWad (char *name);
qpic_t *Draw_S_CachePic (char *path);

void Draw_GL_Init (void);
void Draw_GL_Character (int x, int y, int num);
void Draw_GL_DebugChar (char num);
void Draw_GL_Pic (int x, int y, qpic_t *pic);
void Draw_GL_TransPic (int x, int y, qpic_t *pic);
void Draw_GL_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation);
void Draw_GL_ConsoleBackground (int lines);
void Draw_GL_BeginDisc (void);
void Draw_GL_EndDisc (void);
void Draw_GL_TileClear (int x, int y, int w, int h);
void Draw_GL_Fill (int x, int y, int w, int h, int c);
void Draw_GL_FadeScreen (void);
void Draw_GL_String (int x, int y, char *str);
qpic_t *Draw_GL_PicFromWad (char *name);
qpic_t *Draw_GL_CachePic (char *path);
