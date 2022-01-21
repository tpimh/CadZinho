#include "gui_use.h"
#include "dxf_dim.h"

int gui_dim_interactive(gui_obj *gui){
	
	if (gui->modal != DIMENSION) return 0;
	
	if (gui->step == 0){
		
	}
	else if (gui->step == 1){
		
	}
	else{
		
	}
	
	return 1;
}

int gui_dim_info (gui_obj *gui){
	if (gui->modal != DIMENSION) return 0;
	static int fix_angle = 0;
	static double angle_fixed = 0.0;
	static double dist_fixed = 3.0;
	static int fix_dist = 0;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Place a dim", NK_TEXT_LEFT);
	
	if (gui->step == 0) nk_label(gui->ctx, "Enter start point", NK_TEXT_LEFT);
	else if (gui->step == 1) nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
	else {
		if (fix_dist) nk_label(gui->ctx, "Confirm", NK_TEXT_LEFT);
		else nk_label(gui->ctx, "Enter distance", NK_TEXT_LEFT);
	}
	
	nk_checkbox_label(gui->ctx, "Fixed angle", &fix_angle);
	if (fix_angle)
		angle_fixed = nk_propertyd(gui->ctx, "Angle", -180.0, angle_fixed, 180.0, 1.0, 1.0);
	
	nk_checkbox_label(gui->ctx, "Fixed distance", &fix_dist);
	if (fix_dist)
		dist_fixed = nk_propertyd(gui->ctx, "dist_fixed", -1e9, dist_fixed, 1.0e9, SMART_STEP(dist_fixed), SMART_STEP(dist_fixed));
	
	if (gui->step == 0){
		/* frst point */
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){
			/* to next point*/
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	} 
	else {
		char tmp_str[DXF_MAX_CHARS + 1];
		
		/* create a temporary DIMENSION entity */
		dxf_node *new_dim = dxf_new_dim (gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0, FRAME_LIFE); /* paper space */
		
		/* dimension angle */
		double angle;
		if (fix_angle) angle = angle_fixed * M_PI / 180.0; /* user entered angle or */
		else angle = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0])); /* calculed from points */
		double cosine = cos(angle);
		double sine = sin(angle);
		
		/* calcule dimension linear length (measure). It is a polarized distance in base line direction */
		double length = cosine*(gui->step_x[1]  - gui->step_x[0]) + sine*(gui->step_y[1] - gui->step_y[0]);
		
		/* update dimension entity parameters */
		dxf_attr_change(new_dim, 13, &gui->step_x[0]);
		dxf_attr_change(new_dim, 23, &gui->step_y[0]);
		dxf_attr_change(new_dim, 14, &gui->step_x[1]);
		dxf_attr_change(new_dim, 24, &gui->step_y[1]);
		dxf_attr_change(new_dim, 42, &length);
		angle *= 180.0/M_PI;
		dxf_attr_change(new_dim, 50, &angle);
		dxf_attr_change(new_dim, 1, gui->drawing->dimpost);
		
		/* distance of dimension from measure points */
		double dist = 3.0 * gui->drawing->dimscale;
		double dir = 1.0;
		if(fix_dist) dist = dist_fixed; /* user entered distance */
		else if (gui->step == 2){ /* or calcule from points */
			/* It is a polarized distance in perpenticular direction from dimension base line*/
			dist = -sine*(gui->step_x[2]  - gui->step_x[1])+ cosine*(gui->step_y[2] - gui->step_y[1]);
		}
		dir = (dist  > 0.0) ? 1.0 : -1.0;
		
		/* obtain dimension base point from distance */
		double base_x = gui->step_x[1] - sine * dist;
		double base_y = gui->step_y[1] + cosine * dist;
		dxf_attr_change(new_dim, 10, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 20, &base_y);
		
		/* calcule dimension annotation (text) placement point (outer from perpenticular direction)*/
		base_x += -length/2.0 * cosine - sine * 1.0 * dir * gui->drawing->dimscale;
		base_y += -length/2.0 * sine + cosine * 1.0 * dir * gui->drawing->dimscale;
		dxf_attr_change(new_dim, 11, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 21, &base_y);
		
		/* create dimension block contents as a list of entities ("render" the dimension "picture") */
		list_node *list = dxf_dim_linear_make(gui->drawing, new_dim, 2, 0, 0, 0.0, 0.0);
		
		/* draw phantom */
		gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
		gui->element = NULL;
		gui->draw_phanton = 1;
		
		if (gui->step == 1 && gui->ev & EV_ENTER){
			/* second point - go to confirm */
			gui->step = 2;
			gui_next_step(gui);
		}
		else if (gui->step == 2 && gui->ev & EV_ENTER){
			/* confirm - create a new DXF dim */
			
			/* first, create the "picture" block */ 
			int last_dim = dxf_find_last_dim (gui->drawing); /* get last dim number available*/
			snprintf(tmp_str, DXF_MAX_CHARS, "*D%d", last_dim); /* block name - "*D" + sequential number*/
			dxf_node *blkrec = NULL, *blk = NULL;
			if (dxf_new_block (gui->drawing, tmp_str, "",
				(double []){0.0, 0.0, 0.0},
				0, "", "", "", "",
				"0", list, &blkrec, &blk, DWG_LIFE))
			{	
				dxf_attr_change(blk, 70, (void*)(int[]){1}); /* set block to annonimous */
				/* atach block to dimension ent */
				dxf_attr_change(new_dim, 2, (void*)tmp_str);
				/* copy temporary dimension to drawing buffer */
				dxf_node *new_ent = dxf_ent_copy(new_dim, DWG_LIFE);
				drawing_ent_append(gui->drawing, new_ent);
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
				
				/* undo/redo list*/
				do_add_entry(&gui->list_do, "Linear DIMENSION");
				do_add_item(gui->list_do.current, NULL, blkrec);
				do_add_item(gui->list_do.current, NULL, blk);
				do_add_item(gui->list_do.current, NULL, new_ent);
			}
			gui->draw_phanton = 0;
			gui_first_step(gui);
			
		}
		else if (gui->ev & EV_CANCEL){ /* back to first point, if user cancel operation */
			gui_first_step(gui);
		}
	}
	
	return 1;
}

int gui_dim_mng (gui_obj *gui){
	int show_config = 1;
	int i = 0;
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 200;
	gui->next_win_h = 300;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "config", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, "Dimension Config", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		static char dimscale_str[64] = "1.0";
		static char dimlfac_str[64] = "1.0";
		nk_flags res;
		
		/* edit global dim scale */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Global Scale Factor:", NK_TEXT_LEFT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimscale_str, 63, nk_filter_float);
		if (!(res & NK_EDIT_ACTIVE)){
			snprintf(dimscale_str, 63, "%.9g", gui->drawing->dimscale);
		}
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
			nk_edit_unfocus(gui->ctx);
			if (strlen(dimscale_str)) /* update parameter value */
				gui->drawing->dimscale = atof(dimscale_str);
			snprintf(dimscale_str, 63, "%.9g", gui->drawing->dimscale);
			/* change in DXF main struct */
			dxf_node *start = NULL, *end = NULL, *part = NULL;
			if(dxf_find_head_var(gui->drawing->head, "$DIMSCALE", &start, &end)){
				/* variable exists */
				part = dxf_find_attr_i2(start, end, 40, 0);
				if (part != NULL){
					part->value.d_data = gui->drawing->dimscale;
				}
			}
			else{
				dxf_attr_append(gui->drawing->head, 9, "$DIMSCALE", DWG_LIFE);
				dxf_attr_append(gui->drawing->head, 40, &gui->drawing->dimscale, DWG_LIFE);
			}
		}
		
		/* edit measure dim scale */
		nk_label(gui->ctx, "Measure Scale Factor:", NK_TEXT_LEFT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimlfac_str, 63, nk_filter_float);
		if (!(res & NK_EDIT_ACTIVE)){
			snprintf(dimlfac_str, 63, "%.9g", gui->drawing->dimlfac);
		}
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
			nk_edit_unfocus(gui->ctx);
			if (strlen(dimlfac_str)) /* update parameter value */
				gui->drawing->dimlfac = atof(dimlfac_str);
			snprintf(dimlfac_str, 63, "%.9g", gui->drawing->dimlfac);
			/* change in DXF main struct */
			dxf_node *start = NULL, *end = NULL, *part = NULL;
			if(dxf_find_head_var(gui->drawing->head, "$DIMLFAC", &start, &end)){
				/* variable exists */
				part = dxf_find_attr_i2(start, end, 40, 0);
				if (part != NULL){
					part->value.d_data = gui->drawing->dimlfac;
				}
			}
			else{
				dxf_attr_append(gui->drawing->head, 9, "$DIMLFAC", DWG_LIFE);
				dxf_attr_append(gui->drawing->head, 40, &gui->drawing->dimlfac, DWG_LIFE);
			}
		}
		
		nk_label(gui->ctx, "Annotation text:", NK_TEXT_LEFT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, gui->drawing->dimpost, DXF_MAX_CHARS, nk_filter_default);
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
			nk_edit_unfocus(gui->ctx);
			/* change in DXF main struct */
			dxf_node *start = NULL, *end = NULL, *part = NULL;
			if(dxf_find_head_var(gui->drawing->head, "$DIMPOST", &start, &end)){
				/* variable exists */
				part = dxf_find_attr_i2(start, end, 1, 0);
				if (part != NULL){
					strncpy(part->value.s_data, gui->drawing->dimpost, DXF_MAX_CHARS);
				}
			}
			else{
				dxf_attr_append(gui->drawing->head, 9, "$DIMPOST", DWG_LIFE);
				dxf_attr_append(gui->drawing->head, 1, &gui->drawing->dimpost, DWG_LIFE);
			}
		}
		
	} else show_config = 0;
	nk_end(gui->ctx);
	
	return show_config;
}