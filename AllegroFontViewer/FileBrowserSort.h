#ifndef _AFV_FILEBROWSERSORT_H__
#define _AFV_FILEBROWSERSORT_H__

#include "FileBrowser.h"

bool fbsort_init(void);
void fbsort_dirs(FILEBROWSER *fb);
void fbsort_file(FILEBROWSER *fb);
void fbsort_cleanup(void);


#endif /* _AFV_FILEBROWSERSORT_H__ */
