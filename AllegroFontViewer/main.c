#include "main.h"
#include <assert.h>


extern int
main()
{
	init();
	loop();
	assert(config_save(CFG));
	tini();
	return 0;
}
