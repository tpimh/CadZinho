#ifndef _CZ_SCRIPT_LIB
#define _CZ_SCRIPT_LIB

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "dxf.h"
#include <time.h>

struct script_obj{
	lua_State *L; /* main lua state */
	lua_State *T; /* thread for execution */
	int status;
	int active;
	int dynamic;
	int do_init;
	char path[DXF_MAX_CHARS];
	clock_t time;
	double timeout;
	
	char win[DXF_MAX_CHARS];
	char win_title[DXF_MAX_CHARS];
	int win_x, win_y, win_w, win_h;
	
	char dyn_func[DXF_MAX_CHARS];
};

struct ent_lua { /* DXF entity object, in Lua userdata */
	dxf_node *curr_ent;
	dxf_node *orig_ent;
	int sel;
	dxf_drawing *drawing;
};

#include "gui.h"

int set_timeout (lua_State *L);
int debug_print (lua_State *L);
int script_get_sel (lua_State *L);
int script_ent_write (lua_State *L);

int script_get_ent_typ (lua_State *L);
int script_get_blk_name (lua_State *L);
int script_get_ins_data (lua_State *L);
int script_count_attrib (lua_State *L);
int script_get_attrib_i (lua_State *L);
int script_get_attribs (lua_State *L);
int script_get_points (lua_State *L);
int script_get_bound (lua_State *L);
int script_get_ext (lua_State *L);
int script_get_blk_ents (lua_State *L);
int script_get_all (lua_State *L);
int script_get_text_data (lua_State *L);

int script_edit_attr (lua_State *L);
int script_add_ext (lua_State *L);
int script_edit_ext_i (lua_State *L);
int script_del_ext_i (lua_State *L);
int script_del_ext_all (lua_State *L);

//int script_ent_append (lua_State *L);
int script_new_line (lua_State *L);
int script_new_pline (lua_State *L);
int script_pline_append (lua_State *L);
int script_pline_close (lua_State *L);
int script_new_circle (lua_State *L);
int script_new_hatch (lua_State *L);
int script_new_text (lua_State *L);
int script_new_block (lua_State *L);
int script_new_block_file (lua_State *L) ;

int script_get_dwg_appids (lua_State *L);

int script_set_layer (lua_State *L);
int script_set_color (lua_State *L);
int script_set_ltype (lua_State *L);
int script_set_style (lua_State *L);
int script_set_lw (lua_State *L);
int script_set_modal (lua_State *L) ;
int script_new_appid (lua_State *L);

int script_start_dynamic (lua_State *L);
int script_stop_dynamic (lua_State *L);
int script_ent_draw (lua_State *L);

int script_win_show (lua_State *L);
int script_win_close (lua_State *L);
int script_nk_layout (lua_State *L);
int script_nk_button (lua_State *L);
int script_nk_label (lua_State *L);
int script_nk_edit (lua_State *L);

int script_miniz_open (lua_State *L);
int script_miniz_close (lua_State *L);
int script_miniz_read (lua_State *L);
int script_miniz_write(lua_State *L);

int script_yxml_new (lua_State *L);
int script_yxml_close (lua_State *L);
int script_yxml_read (lua_State *L);

int script_fs_dir (lua_State *L);
int script_fs_chdir (lua_State *L);
int script_fs_cwd (lua_State *L);
int script_fs_script_path(lua_State *L);

int script_unique_id (lua_State *L);
int script_last_blk (lua_State *L);

#endif