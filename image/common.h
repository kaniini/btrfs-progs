/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#ifndef __BTRFS_IMAGE_COMMON_H__
#define __BTRFS_IMAGE_COMMON_H__

#include "kerncompat.h"
#include <stdio.h>

void csum_block(u8 *buf, size_t len);
int detect_version(FILE *in);
int update_disk_super_on_device(struct btrfs_fs_info *info,
				const char *other_dev, u64 cur_devid);
void write_backup_supers(int fd, u8 *buf);

#endif
