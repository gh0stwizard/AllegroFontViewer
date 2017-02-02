#include "main.h"

#include <assert.h>


extern int
main(int argc, const char * const argv[])
{
    (void)argc;
    (void)argv;
	init();
	loop();
	assert(config_save(CFG));
	tini();
	return 0;
}
