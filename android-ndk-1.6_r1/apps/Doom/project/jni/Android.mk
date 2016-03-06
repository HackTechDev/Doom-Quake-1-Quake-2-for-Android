LOCAL_PATH := $(call my-dir)
# clear vars
include $(CLEAR_VARS)
# module name
LOCAL_MODULE := doom
LP := $(LOCAL_PATH)
# doom folder
DOOM := apps/Doom/project/jni
# includes
INC := -I$(DOOM) -I$(DOOM)/include
DOOM_FLAGS := -DNORMALUNIX -DLINUX -DHAVE_CONFIG_H
OPTS := -O3 -ffast-math -fexpensive-optimizations
LOCAL_CFLAGS := $(DOOM_FLAGS) $(OPTS) $(INC)
# sources
LOCAL_SRC_FILES	:= \
am_map.c \
m_cheat.c \
p_lights.c p_user.c \
sounds.c \
hu_lib.c \
md5.c \
p_map.c \
r_bsp.c \
s_sound.c \
d_deh.c \
hu_stuff.c \
m_menu.c \
p_maputl.c r_data.c \
st_lib.c \
d_items.c \
m_misc.c \
p_mobj.c \
r_demo.c \
st_stuff.c \
d_main.c \
info.c \
p_plats.c \
r_draw.c \
tables.c \
doomdef.c \
m_random.c \
p_pspr.c \
r_filter.c version.c \
doomstat.c \
p_ceilng.c \
p_saveg.c \
r_fps.c \
v_video.c \
p_checksum.c \
p_setup.c \
r_main.c \
wi_stuff.c \
dstrings.c \
p_doors.c \
p_sight.c \
r_patch.c \
w_memcache.c \
f_finale.c \
p_enemy.c \
p_spec.c \
r_plane.c \
w_mmap.c \
f_wipe.c \
lprintf.c \
p_floor.c \
p_switch.c r_segs.c\
w_wad.c  \
g_game.c \
m_argv.c \
p_genlin.c \
p_telept.c r_sky.c \
z_bmalloc.c \
m_bbox.c \
p_inter.c \
p_tick.c \
r_things.c z_zone.c \
d_client.c \
d_server.c \
i_video.c i_network.c \
i_system.c i_main.c i_sound.c \
jni_doom.c
# Build libdoom.so
include $(BUILD_SHARED_LIBRARY)

