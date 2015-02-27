#include <linux/list.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <core.h>
#include "cli.h"
#include "console.h"
#include "com32.h"
#include "menu.h"
#include "config.h"
#include "syslinux/adv.h"
#include "syslinux/boot.h"
#include "syslinux/config.h"

//#include <sys/module.h>

struct file_ext {
	const char *name;
	enum kernel_type type;
};

static const struct file_ext file_extensions[] = {
	{ ".c32", IMAGE_TYPE_COM32 },
	{ ".img", IMAGE_TYPE_FDIMAGE },
	{ ".bss", IMAGE_TYPE_BSS },
	{ ".bin", IMAGE_TYPE_BOOT },
	{ ".bs", IMAGE_TYPE_BOOT },
	{ ".0", IMAGE_TYPE_PXE },
	{ NULL, 0 },
};

/*
 * Return a pointer to one byte after the last character of the
 * command.
 */
static inline const char *find_command(const char *str)
{
	const char *p;

	p = str;
	while (*p && !my_isspace(*p))
		p++;
	return p;
}

__export uint32_t parse_image_type(const char *kernel)
{
	const struct file_ext *ext;
	const char *p;
	int len;

	/* Find the end of the command */
	p = find_command(kernel);
	len = p - kernel;

	for (ext = file_extensions; ext->name; ext++) {
		int elen = strlen(ext->name);

		if (!strncmp(kernel + len - elen, ext->name, elen))
			return ext->type;
	}

	/* use IMAGE_TYPE_KERNEL as default */
	return IMAGE_TYPE_KERNEL;
}

