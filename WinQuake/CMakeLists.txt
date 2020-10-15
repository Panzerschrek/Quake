cmake_minimum_required(VERSION 2.8)

project( PanzerQuake )

find_package( SDL2 REQUIRED )

set( SOURCES_COMMON
	adivtab.h
	anorm_dots.h
	anorms.h
	bspfile.h
	cdaudio.h
	chase.c
	cl_demo.c
	client.h
	cl_input.c
	cl_main.c
	cl_parse.c
	cl_tent.c
	cmd.c
	cmd.h
	common.c
	common.h
	console.c
	console.h
	crc.c
	crc.h
	cvar.c
	cvar.h
	host.c
	host_cmd.c
	input.h
	in_sdl.c
	keys.c
	keys.h
	mathlib.c
	mathlib.h
	menu.c
	menu.h
	model.h
	modelgen.h
	mpdosock.h
	net.h
	net_dgrm.c
	net_dgrm.h
	net_loop.c
	net_loop.h
	net_main.c
	net_vcr.c
	pr_cmds.c
	pr_comp.h
	pr_edict.c
	pr_exec.c
	progdefs.h
	progs.h
	protocol.h
	quakedef.h
	r_part.c
	r_shared.h
	sbar.c
	sbar.h
	server.h
	snd_dma.c
	snd_mem.c
	snd_mix.c
	snd_sdl.c
	sound.h
	spritegn.h
	sv_main.c
	sv_move.c
	sv_phys.c
	sv_user.c
	sys.h
	sys_sdl.c
	vid.h
	vid_common.c
	vid_common.h
	view.c
	view.h
	wad.c
	wad.h
	winquake.h
	world.c
	world.h
	zone.c
	zone.h
	)

if( WIN32 )
	list( APPEND SOURCES_COMMON
		net_win.c
		net_wins.c
		net_wipx.c
		)
else()
	list( APPEND SOURCES_COMMON
		net_none.c
		net_udp.c
		net_udp.h
		)
endif()

set( SOURCES_SOFT
	d_edge.c
	d_iface.h
	d_init.c
	d_local.h
	d_modech.c
	d_part.c
	d_polyse.c
	draw.c
	draw.h
	d_scan.c
	d_sky.c
	d_sprite.c
	d_surf.c
	d_vars.c
	model.c
	r_aclip.c
	r_alias.c
	r_bsp.c
	r_draw.c
	r_edge.c
	r_efrag.c
	render.h
	r_light.c
	r_local.h
	r_main.c
	r_misc.c
	r_sky.c
	r_sprite.c
	r_surf.c
	r_vars.c
	screen.c
	screen.h
	vid_sdl.c
	)

set( SOURCES_GL
	gl_draw.c
	gl_hatch.c
	gl_hatch.h
	gl_funcs_list.h
	gl_mesh.c
	gl_model.c
	gl_model.h
	glquake.h
	gl_refrag.c
	gl_rlight.c
	gl_rmain.c
	gl_rmisc.c
	gl_rsurf.c
	gl_screen.c
	gl_shader.c
	gl_shader.h
	gl_warp.c
	vid_sdl_gl.c
	)

add_executable( PanzerQuake ${SOURCES_COMMON} ${SOURCES_SOFT} )
add_executable( PanzerQuakeGL ${SOURCES_COMMON} ${SOURCES_GL} )

target_include_directories( PanzerQuake PRIVATE ${SDL2_INCLUDE_DIRS} )
target_include_directories( PanzerQuakeGL PRIVATE ${SDL2_INCLUDE_DIRS} )
target_compile_definitions( PanzerQuakeGL PRIVATE GLQUAKE )

target_link_libraries( PanzerQuake PRIVATE ${SDL2_LIBRARIES} )
target_link_libraries( PanzerQuakeGL PRIVATE ${SDL2_LIBRARIES} )

if( WIN32 )
	target_link_libraries( PanzerQuake PRIVATE wsock32 )
	target_link_libraries( PanzerQuakeGL PRIVATE wsock32 opengl32 )
else()
	target_link_libraries( PanzerQuake PRIVATE m )
	target_link_libraries( PanzerQuakeGL PRIVATE m GL )

endif()
