#include "gui_file.h"


const char *filter_types[] = {
	[FILE_ALL] = "*",
	[FILE_DXF] = "DXF",
	[FILE_TXT] = "TXT",
	[FILE_PAT] = "PAT",
	[FILE_LIN] = "LIN",
	[FILE_SHP] = "SHP",
	[FILE_SHX] = "SHX",
	[FILE_TTF] = "TTF",
	[FILE_OTF] = "OTF",
	[FILE_PDF] = "PDF",
};
const char *filter_descr[] = {
	[FILE_ALL] = "All files (*)",
	[FILE_DXF] = "Drawing files (.dxf)",
	[FILE_TXT] = "Text files (.txt)",
	[FILE_PAT] = "Patterns files (.pat)",
	[FILE_LIN] = "Line style files (.lin)",
	[FILE_SHP] = "Shapes files (.shp)",
	[FILE_SHX] = "Binary shapes file (.shx)",
	[FILE_TTF] = "True type font file (.ttf)",
	[FILE_OTF] = "Open font file (.otf)",
	[FILE_PDF] = "PDF (.pdf)",
};

struct file_info{
	char name[DXF_MAX_CHARS];
	time_t date;
	off_t size;
};

/* auxiliary functions for sorting files (qsort) */
/* compare by file/dir name*/
int cmp_file_name(const void * a, const void * b) {
	struct file_info *info1 = ((struct sort_by_idx *)a)->data;
	struct file_info *info2 = ((struct sort_by_idx *)b)->data;
	
	return (strcmp(info1->name, info2->name));
}

int cmp_file_name_rev(const void * a, const void * b) { /* reverse */
	return -cmp_file_name(a, b);
}

/* compare by file/dir modification time*/
int cmp_file_date(const void * a, const void * b) {
	struct file_info *info1 = ((struct sort_by_idx *)a)->data;
	struct file_info *info2 = ((struct sort_by_idx *)b)->data;
	
	double diff = difftime(info1->date, info2->date);
	if (diff < 0.0) return -1;
	else return 1;
}

int cmp_file_date_rev(const void * a, const void * b) { /* reverse */
	return -cmp_file_date(a, b);
}

/* compare by file/dir byte size*/
int cmp_file_size(const void * a, const void * b) {
	struct file_info *info1 = ((struct sort_by_idx *)a)->data;
	struct file_info *info2 = ((struct sort_by_idx *)b)->data;
	
	return (int)(info1->size - info2->size);
}

int cmp_file_size_rev(const void * a, const void * b) { /* reverse */
	return -cmp_file_size(a, b);
}


/* file explorer window */
int file_win (gui_obj *gui, const char *ext_type[], const char *ext_descr[], int num_ext, char *init_dir){
	if ((ext_type == NULL) || (ext_descr == NULL) || num_ext == 0) return 0;
	
	static char full_path[PATH_MAX_CHARS];
	static char sel_file[PATH_MAX_CHARS];
	
	/* change to initial directory, if it has passed */
	if (init_dir) chdir(init_dir);

	struct dirent *entry;
	static DIR *work = NULL; /* working directory */
	DIR *subdir;
	char ext[4], *suffix, *end;
	char str_tmp[20];
	
	if (!work){ /* if not working dir previously open */
		work = opendir(".");
		full_path[0] = 0;
		sel_file[0] = 0;
	}
	if (!work) return 0;
	
	int show_browser = 1;
	int i = 0;
	int idx = 0;
	
	struct stat filestat;
	struct tm *info;
	
	static struct sort_by_idx sort_dirs[10000];
	static struct sort_by_idx sort_files[10000];
	static struct file_info dirs[10000];
	static struct file_info files[10000];
	int num_files = 0, num_dirs = 0;
	
	/* customized buttons for files and directories */
	struct nk_style_button b_dir, b_file;
	b_dir = gui->ctx->style.button;
	b_file = gui->ctx->style.button;
	b_file.text_alignment = NK_TEXT_LEFT;
	b_dir.text_alignment = NK_TEXT_LEFT;
	b_dir.text_normal = nk_rgb(255,255,0); /* text in directory buttons are yellow */
	b_dir.text_hover = nk_rgb(255,255,0);
	b_dir.text_active = nk_rgb(255,255,0);
	
	if (gui->filter_idx >= num_ext) gui->filter_idx = 0;
	
	
	if (nk_begin(gui->ctx, "File explorer", nk_rect(550, 100, 600, 510),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		/* read the workind directory */
		rewinddir(work);
		while((entry = readdir(work))){ /*sweep the directory */
			/* verify if current item is a subdir */
			subdir = opendir(entry->d_name);
			if (subdir != NULL){ /* a subdir */
				if (!(strcmp(entry->d_name, ".") == 0) &&
				    !(strcmp(entry->d_name, "..") == 0)){ /* don't show current and parent dir information */
					if (num_dirs < 10000){
						/* update current dir information in list */
						sort_dirs[num_dirs].idx = num_dirs; /* index*/
						strncpy (dirs[num_dirs].name, entry->d_name, DXF_MAX_CHARS); /* name */
						stat(entry->d_name, &filestat); /* get storage information */
						dirs[num_dirs].date = filestat.st_mtime; /* modification time */
						dirs[num_dirs].size = filestat.st_size; /* file size */
						sort_dirs[num_dirs].data = &(dirs[num_dirs]); /* pointer to structure */
						
						num_dirs++; 
					}
				}
				
				closedir(subdir);
			}
			else{/* a file */
				/* get file extension */
				suffix = get_ext(entry->d_name);
				strncpy(ext, suffix, 4);
				ext[3] = 0; /*terminate string */
				str_upp(ext); /* upper case extension*/
				/* verify if the current file extension is in filter criteria */
				if ((strcmp(ext_type[gui->filter_idx], "*") == 0) || /* no filter criteria (all files) */
				    (strcmp(ext, ext_type[gui->filter_idx]) == 0)) {
					if (num_files < 10000){
						/* update current file information in list */
						sort_files[num_files].idx = num_files; /* index*/
						strncpy (files[num_files].name, entry->d_name, DXF_MAX_CHARS); /* name */
						stat(entry->d_name, &filestat); /* get storage information */
						files[num_files].date = filestat.st_mtime; /* modification time */
						files[num_files].size = filestat.st_size; /* file size */
						sort_files[num_files].data = &(files[num_files]); /* pointer to structure */
						
						num_files++;
					}
				}
				
			}
		}
		
		static int sorted = 0;
		enum sort {
			UNSORTED,
			BY_NAME,
			BY_DATE,
			BY_SIZE
		};
		static int sort_reverse = 0;
		
		/* show current directory */
		char curr_path[PATH_MAX_CHARS+1];
		getcwd(curr_path, PATH_MAX_CHARS);
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label_colored(gui->ctx, "Current directory:", NK_TEXT_LEFT, nk_rgb(255,255,0));
		
		/* dynamic width for directory path and fixed width for "up" button */
		nk_layout_row_template_begin(gui->ctx, 22);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_push_static(gui->ctx, 30);
		nk_layout_row_template_end(gui->ctx);
		
		nk_label(gui->ctx, curr_path, NK_TEXT_LEFT); /* show current directory */
		
		if (nk_button_label(gui->ctx,  "Up")){
			/* up in directory structure */
			closedir(work);
			chdir(".."); /* change working dir */
			work = opendir(".");
			if (!work) return 0;
		}
		
		/* list header */
		nk_layout_row_dynamic(gui->ctx, 32, 1);
		if (nk_group_begin(gui->ctx, "file_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			
			/* dynamic width for dir/file name and fixed width for other informations */
			nk_layout_row_template_begin(gui->ctx, 22);
			nk_layout_row_template_push_dynamic(gui->ctx);
			nk_layout_row_template_push_static(gui->ctx, 80);
			nk_layout_row_template_push_static(gui->ctx, 145);
			nk_layout_row_template_push_static(gui->ctx, 8);
			nk_layout_row_template_end(gui->ctx);
			
			/* sort option - by dir/file name */
			if (sorted == BY_NAME){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Name", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Name", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Name")){
				sorted = BY_NAME;
				sort_reverse = 0;
			}
			
			/* sort option - by file/dir byte size */
			if (sorted == BY_SIZE){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Size", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Size", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Size")){
				sorted = BY_SIZE;
				sort_reverse = 0;
			}
			
			/* sort option -  by file/dir modification time */
			if (sorted == BY_DATE){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Date", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Date", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Date")){
				sorted = BY_DATE;
				sort_reverse = 0;
			}
			
			nk_group_end(gui->ctx);
		}
		nk_layout_row_dynamic(gui->ctx, 300, 1);
		if (nk_group_begin(gui->ctx, "File_view", NK_WINDOW_BORDER)) {
			
			/* dynamic width for dir/file name and fixed width for other informations */
			nk_layout_row_template_begin(gui->ctx, 20);
			nk_layout_row_template_push_dynamic(gui->ctx);
			nk_layout_row_template_push_static(gui->ctx, 80);
			nk_layout_row_template_push_static(gui->ctx, 145);
			nk_layout_row_template_end(gui->ctx);
			
			/* sort list, according sorting criteria */
			if (sorted == BY_NAME){
				if(!sort_reverse){
					qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_name);
					qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_name);
				}
				else{
					qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_name_rev);
					qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_name_rev);
				}
			}
			else if (sorted == BY_DATE){
				if(!sort_reverse){
					qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_date);
					qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_date);
				}
				else{
					qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_date_rev);
					qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_date_rev);
				}
			}
			else if (sorted == BY_SIZE){
				qsort(sort_dirs, num_dirs, sizeof(struct sort_by_idx), cmp_file_name);
				if(!sort_reverse){
					qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_size);
				}
				else{
					qsort(sort_files, num_files, sizeof(struct sort_by_idx), cmp_file_size_rev);
				}
			}
			
			/* first, show the subdirs */
			for (i = 0; i < num_dirs; i++){
				idx = sort_dirs[i].idx;
				if (nk_button_label_styled(gui->ctx, &b_dir,  dirs[idx].name)){
					/* enter in the subdir */
					closedir(work);
					chdir(dirs[idx].name); /* change working dir */
					work = opendir(".");
					full_path[0] = 0;
					sel_file[0] = 0;
					if (!work) return 0;
				}
				
				/*show byte size - NOT applicable in dir*/
				//snprintf(str_tmp, 20, "%d", dirs[idx].size);
				//nk_label_colored(gui->ctx, str_tmp, NK_TEXT_RIGHT, nk_rgb(255,255,0));
				nk_label_colored(gui->ctx, "-", NK_TEXT_CENTERED, nk_rgb(255,255,0));
				
				/* show modification date/time */
				info = localtime(&(dirs[idx].date));
				snprintf(str_tmp, 20, "%02d/%02d/%04d-%02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
				nk_label_colored(gui->ctx, str_tmp, NK_TEXT_RIGHT, nk_rgb(255,255,0));
				
			}
			/* then, show the files */
			for (i = 0; i < num_files; i++){
				idx = sort_files[i].idx;
				if (nk_button_label_styled(gui->ctx, &b_file,  files[idx].name)){
					/* select file */
					strncpy(sel_file, files[idx].name, PATH_MAX_CHARS);
				}
				
				/*show byte size */
				if (files[idx].size > 1048576) snprintf(str_tmp, 20, "%0.2fM", (float) files[idx].size / 1048576);
				else if (files[idx].size > 1024) snprintf(str_tmp, 20, "%0.2fK", (float) files[idx].size / 1024);
				else snprintf(str_tmp, 20, "%d", files[idx].size);
				nk_label(gui->ctx, str_tmp, NK_TEXT_RIGHT);
				
				/* show modification date/time */
				info = localtime(&(files[idx].date));
				snprintf(str_tmp, 20, "%02d/%02d/%04d-%02d:%02d:%02d", info->tm_mday, info->tm_mon+1, info->tm_year+1900, info->tm_hour, info->tm_min, info->tm_sec);
				nk_label(gui->ctx, str_tmp, NK_TEXT_RIGHT);
				
			}
			nk_group_end(gui->ctx);
			
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			nk_label_colored(gui->ctx, "Selected:", NK_TEXT_LEFT, nk_rgb(0,0,255));
			
			/* file extension filter option */
			int h = num_ext * 22 + 5;
			h = (h < 300)? h : 300;
			if (nk_combo_begin_label(gui->ctx, ext_descr[gui->filter_idx], nk_vec2(270, h))){
				/* change type of file extension */
				nk_layout_row_dynamic(gui->ctx, 17, 1);
				for (i = 0; i < num_ext; i++){
					if (nk_button_label(gui->ctx, ext_descr[i])){
						gui->filter_idx = i;
						nk_combo_close(gui->ctx);
					}
				}
				nk_combo_end(gui->ctx);
			}
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			
			/* show the selected file*/
			nk_flags res; /* the user will can edit the file name */
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, sel_file, PATH_MAX_CHARS, nk_filter_default);
			if (res & NK_EDIT_COMMITED){ /* user hit enter */
				nk_edit_unfocus(gui->ctx);
				/* the user also will can enter a new path */
				subdir = opendir(sel_file); /* try to open  the path entered by user */
				if (subdir != NULL){
					closedir(work);
					chdir(sel_file); /* change the current path */
					work = subdir;
					full_path[0] = 0;
					sel_file[0] = 0;
				}
			}
			
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			if (nk_button_label(gui->ctx,  "OK")){
				/* return the full path of selected file and close window*/
				if (strlen(sel_file) > 0){
					snprintf(full_path, PATH_MAX_CHARS, "%s%c%s", curr_path, DIR_SEPARATOR, sel_file);
				}
				closedir(work);
				work = NULL;
				strncpy(gui->curr_path, full_path, PATH_MAX_CHARS);
				full_path[0] = 0;
				sel_file[0] = 0;
				show_browser = 2;
			}
			if (nk_button_label(gui->ctx,  "Cancel")){
				/* close window */
				closedir(work);
				work = NULL;
				sel_file[0] = 0;
				show_browser = 0;
				nk_window_show(gui->ctx, "File explorer", NK_HIDDEN);
			}
			
		}
		
	} else { /* user close window by press "x" button*/
		closedir(work);
		work = NULL;
		sel_file[0] = 0;
		show_browser = 0;
	}
	nk_end(gui->ctx);
	
	return show_browser;
}

int file_pop (gui_obj *gui, enum files_types filters[], int num_filters, char *init_dir){
	/* simple popup for entering file name/path */
	
	int show_app_file = 1;
	static struct nk_rect s = {20, 200, 400, 150};
	if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "File", NK_WINDOW_CLOSABLE, s)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "File to Open:", NK_TEXT_CENTERED);
		
		/* user can type the file name/path, or paste text, or drop from system navigator */
		nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->curr_path, PATH_MAX_CHARS, nk_filter_default);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if ((nk_button_label(gui->ctx, "OK")) && (gui->show_file_br != 1)) {
			nk_popup_close(gui->ctx);
			show_app_file = 2;
			gui->show_file_br = 0;
		}
		if (nk_button_label(gui->ctx, "Explore")) {
			/* option for internal file explorer */
			int i;
			
			/* update file extension filter */
			for (i = 0; i < num_filters; i++){
				gui->file_filter_types[i] = filter_types[filters[i]];
				gui->file_filter_descr[i] = filter_descr[filters[i]];
			}
			gui->file_filter_count = num_filters;
			gui->show_file_br = 1;
		}
		
		nk_popup_end(gui->ctx);
	} else {
		show_app_file = 0;
		gui->show_file_br = 0;
	}
	return show_app_file;
}

int gui_file_open (gui_obj *gui, char *init_dir){
	/* window for open drawing files */
	
	int show_app_file = 1, i;
	static int discard_changes = 0, init = 0;
	static char file[DXF_MAX_CHARS+1], dir[PATH_MAX_CHARS+1];
	
	if (!init){
		/* change to initial directory, if it has passed */
		//if (init_dir) dir_change(init_dir);
		
		/* change directory to current drawing */
		dir_change(gui->dwg_dir);
		strncpy (dir, gui->dwg_dir, PATH_MAX_CHARS);
		//strncpy (file, gui->dwg_file, DXF_MAX_CHARS);
		file[0] = 0;
		init = 1;
	}
	if (gui->show_file_br == 2){
		/* update dir and file with returned path from browser window */
		strncpy (dir, get_dir(gui->curr_path) , PATH_MAX_CHARS);
		strncpy (file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
		gui->show_file_br = 0;
	}
	
	if (nk_begin(gui->ctx, "Open Drawing", nk_rect(200, 150, 400, 350),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_flags res;
		
		/* show current directory */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label_colored(gui->ctx, "Current directory:", NK_TEXT_LEFT, nk_rgb(255,255,0));
		nk_label(gui->ctx, dir, NK_TEXT_LEFT); /* show current directory */
		
		nk_label(gui->ctx, "File to Open:", NK_TEXT_CENTERED);
		
		/* user can type the file name/path, or paste text, or drop from system navigator */
		nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, file, PATH_MAX_CHARS, nk_filter_default);
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
			/* probably, user change parameter string */
			/* then verify if entered a full path and adjust the strings */
			char *curr_dir = get_dir(file);
			if (strlen(curr_dir)){
				char no_dir[3];
				no_dir[0] = '.';
				no_dir[1] = DIR_SEPARATOR;
				no_dir[2] = 0;
				if (strcmp(curr_dir, no_dir) != 0){ /* verify if change in directory */
					strncpy (dir, curr_dir, DXF_MAX_CHARS);
					strncpy (file, get_filename(file) , DXF_MAX_CHARS);
				}
			}
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if ((nk_button_label(gui->ctx, "OK")) && (gui->show_file_br != 1)) {
			if (strlen(file)){
				if (gui->changed){
					discard_changes = 1;
				}
				else{
					/* put "dir" and "file" strings in current path */
					char full_path[PATH_MAX_CHARS+1];
					strncpy (full_path, dir, PATH_MAX_CHARS);
					strncat (full_path, file, PATH_MAX_CHARS - strlen(dir));
					strncpy (gui->curr_path, full_path, PATH_MAX_CHARS);
					/* close window */
					show_app_file = 2;
					gui->show_file_br = 0;
					discard_changes = 0;
					init = 0;
				}
			}
		}
		if (nk_button_label(gui->ctx, "Explore")) {
			/* option for internal file explorer */
			
			/* update file extension filter */
			gui->file_filter_types[0] = filter_types[FILE_DXF];
			gui->file_filter_descr[0] = filter_descr[FILE_DXF];
			gui->file_filter_types[1] = filter_types[FILE_ALL];
			gui->file_filter_descr[1] = filter_descr[FILE_ALL];
			gui->file_filter_count = 2;
			
			gui->show_file_br = 1;
		}
		
		/* recent files */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Recent:", NK_TEXT_CENTERED);
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		i = 0;
		list_node *rcnt_curr = gui->recent_drwg->next;
		while (rcnt_curr != NULL && i < DRWG_RECENT_MAX){ /* sweep the list */
			if (rcnt_curr->data){
				STRPOOL_U64 str_a = (STRPOOL_U64) rcnt_curr->data; /* str key in pool */
				char *rcnt_file = (char *) strpool_cstr( &gui->file_pool, str_a); /* get string */
				if (nk_button_label(gui->ctx, get_filename(rcnt_file))) {
					/* use selected file path */
					//strncpy(gui->curr_path, rcnt_file, PATH_MAX_CHARS);
					strncpy (dir, get_dir(rcnt_file) , DXF_MAX_CHARS);
					strncpy (file, get_filename(rcnt_file) , DXF_MAX_CHARS);
				}
			}
			rcnt_curr = rcnt_curr->next;
			i++;
		}
		
		if (discard_changes){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Discard changes", NK_WINDOW_CLOSABLE|NK_WINDOW_NO_SCROLLBAR, nk_rect(30, 100, 300, 100))){
				//nk_layout_row_dynamic(gui->ctx, 20, 2);
				nk_layout_row_template_begin(gui->ctx, 23);
				nk_layout_row_template_push_static(gui->ctx, 24);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_end(gui->ctx);
				nk_image(gui->ctx, nk_image_ptr(gui->svg_bmp[SVG_WARNING]));
				nk_label(gui->ctx, "Changes in drawing will be lost", NK_TEXT_LEFT);
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if ((nk_button_label(gui->ctx, "Discard")) && (gui->show_file_br != 1)) {
					/* put "dir" and "file" strings in current path */
					char full_path[PATH_MAX_CHARS+1];
					strncpy (full_path, dir, PATH_MAX_CHARS);
					strncat (full_path, file, PATH_MAX_CHARS - strlen(dir));
					strncpy (gui->curr_path, full_path, PATH_MAX_CHARS);
					/* close window */
					show_app_file = 2;
					gui->show_file_br = 0;
					init = 0;
					discard_changes = 0;
					nk_popup_close(gui->ctx);
				}
				if (nk_button_label(gui->ctx, "Cancel")) {
					discard_changes = 0;
					nk_popup_close(gui->ctx);
				}
				
				nk_popup_end(gui->ctx);
			}
			else {
				discard_changes = 0;
			}
		}
		
	} else {
		show_app_file = 0;
		gui->show_file_br = 0;
		discard_changes = 0;
		init = 0;
	}
	nk_end(gui->ctx);
	return show_app_file;
}

int gui_file_save (gui_obj *gui, char *init_dir){
	/* window for save drawing files */	
	int show_app_file = 1, i;
	static int init = 0, overwrite = 0;
	static char file[DXF_MAX_CHARS+1], dir[PATH_MAX_CHARS+1];
	
	if (!init){
		/* change to initial directory, if it has passed */
		//if (init_dir) dir_change(init_dir);
		
		/* change directory to current drawing */
		dir_change(gui->dwg_dir);
		strncpy (dir, gui->dwg_dir, PATH_MAX_CHARS);
		strncpy (file, gui->dwg_file, DXF_MAX_CHARS);
		init = 1;
	}
	
	if (gui->show_file_br == 2){
		/* update dir and file with returned path from browser window */
		strncpy (dir, get_dir(gui->curr_path) , PATH_MAX_CHARS);
		strncpy (file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
		gui->show_file_br = 0;
	}
	
	if (nk_begin(gui->ctx, "Save Drawing", nk_rect(200, 150, 400, 180),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_flags res;
		
		/* show current directory */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label_colored(gui->ctx, "Current directory:", NK_TEXT_LEFT, nk_rgb(255,255,0));
		nk_label(gui->ctx, dir, NK_TEXT_LEFT); /* show current directory */
		
		nk_label(gui->ctx, "File to Save on:", NK_TEXT_CENTERED);
		
		/* user can type the file name/path, or paste text, or drop from system navigator */
		nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, file, PATH_MAX_CHARS, nk_filter_default);
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
			/* probably, user change parameter string */
			/* then verify if entered a full path and adjust the strings */
			char *curr_dir = get_dir(file);
			if (strlen(curr_dir)){
				char no_dir[3];
				no_dir[0] = '.';
				no_dir[1] = DIR_SEPARATOR;
				no_dir[2] = 0;
				if (strcmp(curr_dir, no_dir) != 0){ /* verify if change in directory */
					strncpy (dir, curr_dir, DXF_MAX_CHARS);
					strncpy (file, get_filename(file) , DXF_MAX_CHARS);
				}
			}
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if ((nk_button_label(gui->ctx, "OK")) && (gui->show_file_br != 1)) {
			if (strlen(file)){
				/* put "dir" and "file" strings in current path */
				char full_path[PATH_MAX_CHARS+1];
				strncpy (full_path, dir, PATH_MAX_CHARS);
				strncat (full_path, file, PATH_MAX_CHARS - strlen(dir));
				
				if (file_exists(full_path)){
					overwrite = 1;
				}
				else {
					strncpy (gui->curr_path, full_path, PATH_MAX_CHARS);
					/* close window */
					show_app_file = 2;
					gui->show_file_br = 0;
					init = 0;
					overwrite = 0;
				}
			}
		}
		if (nk_button_label(gui->ctx, "Explore")) {
			/* option for internal file explorer */
			
			/* update file extension filter */
			gui->file_filter_types[0] = filter_types[FILE_DXF];
			gui->file_filter_descr[0] = filter_descr[FILE_DXF];
			gui->file_filter_types[1] = filter_types[FILE_ALL];
			gui->file_filter_descr[1] = filter_descr[FILE_ALL];
			gui->file_filter_count = 2;
			
			gui->show_file_br = 1;
		}
		
		if (overwrite){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Existing File", NK_WINDOW_CLOSABLE|NK_WINDOW_NO_SCROLLBAR, nk_rect(30, 100, 300, 100))){
				//nk_layout_row_dynamic(gui->ctx, 20, 2);
				nk_layout_row_template_begin(gui->ctx, 23);
				nk_layout_row_template_push_static(gui->ctx, 24);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_end(gui->ctx);
				nk_image(gui->ctx, nk_image_ptr(gui->svg_bmp[SVG_WARNING]));
				nk_label(gui->ctx, "Over write existing file?", NK_TEXT_LEFT);
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if ((nk_button_label(gui->ctx, "OK")) && (gui->show_file_br != 1)) {
					/* put "dir" and "file" strings in current path */
					char full_path[PATH_MAX_CHARS+1];
					strncpy (full_path, dir, PATH_MAX_CHARS);
					strncat (full_path, file, PATH_MAX_CHARS - strlen(dir));
					strncpy (gui->curr_path, full_path, PATH_MAX_CHARS);
					/* close window */
					show_app_file = 2;
					gui->show_file_br = 0;
					init = 0;
					overwrite = 0;
					nk_popup_close(gui->ctx);
				}
				if (nk_button_label(gui->ctx, "Cancel")) {
					overwrite = 0;
					nk_popup_close(gui->ctx);
				}
				
				nk_popup_end(gui->ctx);
			}
			else {
				overwrite = 0;
			}
		}
	} else {
		/* close window */
		show_app_file = 0;
		gui->show_file_br = 0;
		init = 0;
		overwrite = 0;
	}
	nk_end(gui->ctx);
	return show_app_file;
}

int gui_hist_add (gui_obj *gui){ /* add file to history */
	
	/* get position in array, considering as circular buffer */
	int pos = (gui->drwg_hist_wr + gui->drwg_hist_head) % DRWG_HIST_MAX;
	/* put file path in history */
	strncpy (gui->drwg_hist[pos], gui->curr_path , DXF_MAX_CHARS);
	/* history buffer is full -> change the head of buffer to overwrite first entry */
	if(gui->drwg_hist_wr >= DRWG_HIST_MAX) gui->drwg_hist_head++;
	/* adjust circular buffer head */
	gui->drwg_hist_head %= DRWG_HIST_MAX;
	
	/* adjust buffer parameters to next entries */
	if (gui->drwg_hist_pos < gui->drwg_hist_size && gui->drwg_hist_pos < DRWG_HIST_MAX - 1)
		gui->drwg_hist_pos ++; /* position */
	if (gui->drwg_hist_wr < DRWG_HIST_MAX){
		gui->drwg_hist_wr ++; /* size */
		gui->drwg_hist_size = gui->drwg_hist_wr; /* next write position */
	}
	gui->hist_new = 0;
	
	/* put file path in recent file list */
	STRPOOL_U64 str_a = strpool_inject( &gui->file_pool, gui->curr_path , (int) strlen(gui->curr_path ) );
	/* verify if file was previously loaded */
	list_node *prev = list_find_data(gui->recent_drwg, (void *)str_a);
	if (prev){
		/* remove repeated file form list */
		list_remove(gui->recent_drwg, prev);
	}
	/* put newest file in head of list */
	list_insert(gui->recent_drwg, list_new((void *)str_a, PRG_LIFE));
	
	return 1;
}
