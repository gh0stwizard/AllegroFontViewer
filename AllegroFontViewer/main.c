#include "main.h"

#include <assert.h>


extern int
main(int argc, const char * const argv[])
{
	init();
	loop();
	assert(config_save(CFG));
	tini();
	return 0;
}
