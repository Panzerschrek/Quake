// USAGE - define PROCESS_OGL_FUNCTION, include this file.

#ifndef PROCESS_GL_FUNC
#error
#endif

// Multitexturing
PROCESS_GL_FUNC( PFNGLACTIVETEXTUREPROC, glActiveTexture );
PROCESS_GL_FUNC( PFNGLMULTITEXCOORD2FPROC, glMultiTexCoord2f );