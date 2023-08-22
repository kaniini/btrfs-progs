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

#include "kerncompat.h"
#include "crypto/hash.h"
#include "crypto/crc32c.h"
#include "crypto/sha.h"
#include "crypto/blake2.h"
#include "common/messages.h"
#include "common/cpu-utils.h"

struct hash_testvec {
	const char *plaintext;
	size_t psize;
	const char *digest;
};

struct hash_testspec {
	const char *name;
	int digest_size;
	const struct hash_testvec *testvec;
	size_t count;
	unsigned long cpu_flag;
	int (*hash)(const u8 *buf, size_t length, u8 *out);
	int backend;
};

static const struct hash_testvec crc32c_tv[] = {
	{
		.psize = 0,
		.digest = "\x00\x00\x00\x00",
	},
	{
		.plaintext = "abcdefg",
		.psize = 7,
		.digest = "\x41\xf4\x27\xe6",
	},
};

static const struct hash_testvec xxhash64_tv[] = {
	{
		.psize = 0,
		.digest = "\x99\xe9\xd8\x51\x37\xdb\x46\xef",
	},
	{
		.plaintext = "\x40",
		.psize = 1,
		.digest = "\x20\x5c\x91\xaa\x88\xeb\x59\xd0",
	},
	{
		.plaintext = "\x40\x8b\xb8\x41\xe4\x42\x15\x2d"
			     "\x88\xc7\x9a\x09\x1a\x9b",
		.psize = 14,
		.digest = "\xa8\xe8\x2b\xa9\x92\xa1\x37\x4a",
	},
	{
		.plaintext = "\x40\x8b\xb8\x41\xe4\x42\x15\x2d"
		             "\x88\xc7\x9a\x09\x1a\x9b\x42\xe0"
			     "\xd4\x38\xa5\x2a\x26\xa5\x19\x4b"
			     "\x57\x65\x7f\xad\xc3\x7d\xca\x40"
			     "\x31\x65\x05\xbb\x31\xae\x51\x11"
			     "\xa8\xc0\xb3\x28\x42\xeb\x3c\x46"
			     "\xc8\xed\xed\x0f\x8d\x0b\xfa\x6e"
			     "\xbc\xe3\x88\x53\xca\x8f\xc8\xd9"
			     "\x41\x26\x7a\x3d\x21\xdb\x1a\x3c"
			     "\x01\x1d\xc9\xe9\xb7\x3a\x78\x67"
			     "\x57\x20\x94\xf1\x1e\xfd\xce\x39"
			     "\x99\x57\x69\x39\xa5\xd0\x8d\xd9"
			     "\x43\xfe\x1d\x66\x04\x3c\x27\x6a"
			     "\xe1\x0d\xe7\xc9\xfa\xc9\x07\x56"
			     "\xa5\xb3\xec\xd9\x1f\x42\x65\x66"
			     "\xaa\xbf\x87\x9b\xc5\x41\x9c\x27"
			     "\x3f\x2f\xa9\x55\x93\x01\x27\x33"
			     "\x43\x99\x4d\x81\x85\xae\x82\x00"
			     "\x6c\xd0\xd1\xa3\x57\x18\x06\xcc"
			     "\xec\x72\xf7\x8e\x87\x2d\x1f\x5e"
			     "\xd7\x5b\x1f\x36\x4c\xfa\xfd\x18"
			     "\x89\x76\xd3\x5e\xb5\x5a\xc0\x01"
			     "\xd2\xa1\x9a\x50\xe6\x08\xb4\x76"
			     "\x56\x4f\x0e\xbc\x54\xfc\x67\xe6"
			     "\xb9\xc0\x28\x4b\xb5\xc3\xff\x79"
			     "\x52\xea\xa1\x90\xc3\xaf\x08\x70"
			     "\x12\x02\x0c\xdb\x94\x00\x38\x95"
			     "\xed\xfd\x08\xf7\xe8\x04",
		.psize = 222,
		.digest = "\x41\xfc\xd4\x29\xfe\xe7\x85\x17",
	}
};

static const struct hash_testvec sha256_tv[] = {
	{
		.plaintext = "",
		.psize	= 0,
		.digest	= "\xe3\xb0\xc4\x42\x98\xfc\x1c\x14"
			  "\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24"
			  "\x27\xae\x41\xe4\x64\x9b\x93\x4c"
			  "\xa4\x95\x99\x1b\x78\x52\xb8\x55",
	}, {
		.plaintext = "abc",
		.psize	= 3,
		.digest	= "\xba\x78\x16\xbf\x8f\x01\xcf\xea"
			  "\x41\x41\x40\xde\x5d\xae\x22\x23"
			  "\xb0\x03\x61\xa3\x96\x17\x7a\x9c"
			  "\xb4\x10\xff\x61\xf2\x00\x15\xad",
	}, {
		.plaintext = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		.psize	= 56,
		.digest	= "\x24\x8d\x6a\x61\xd2\x06\x38\xb8"
			  "\xe5\xc0\x26\x93\x0c\x3e\x60\x39"
			  "\xa3\x3c\xe4\x59\x64\xff\x21\x67"
			  "\xf6\xec\xed\xd4\x19\xdb\x06\xc1",
	}, {
		.plaintext = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-",
		.psize	= 64,
		.digest = "\xb5\xfe\xad\x56\x7d\xff\xcb\xa4"
			  "\x2c\x32\x29\x32\x19\xbb\xfb\xfa"
			  "\xd6\xff\x94\xa3\x72\x91\x85\x66"
			  "\x3b\xa7\x87\x77\x58\xa3\x40\x3a",
	}, {
		.plaintext = "\x08\x9f\x13\xaa\x41\xd8\x4c\xe3"
			     "\x7a\x11\x85\x1c\xb3\x27\xbe\x55"
			     "\xec\x60\xf7\x8e\x02\x99\x30\xc7"
			     "\x3b\xd2\x69\x00\x74\x0b\xa2\x16"
			     "\xad\x44\xdb\x4f\xe6\x7d\x14\x88"
			     "\x1f\xb6\x2a\xc1\x58\xef\x63\xfa"
			     "\x91\x05\x9c\x33\xca\x3e\xd5\x6c"
			     "\x03\x77\x0e\xa5\x19\xb0\x47\xde"
			     "\x52\xe9\x80\x17\x8b\x22\xb9\x2d"
			     "\xc4\x5b\xf2\x66\xfd\x94\x08\x9f"
			     "\x36\xcd\x41\xd8\x6f\x06\x7a\x11"
			     "\xa8\x1c\xb3\x4a\xe1\x55\xec\x83"
			     "\x1a\x8e\x25\xbc\x30\xc7\x5e\xf5"
			     "\x69\x00\x97\x0b\xa2\x39\xd0\x44"
			     "\xdb\x72\x09\x7d\x14\xab\x1f\xb6"
			     "\x4d\xe4\x58\xef\x86\x1d\x91\x28"
			     "\xbf\x33\xca\x61\xf8\x6c\x03\x9a"
			     "\x0e\xa5\x3c\xd3\x47\xde\x75\x0c"
			     "\x80\x17\xae\x22\xb9\x50\xe7\x5b"
			     "\xf2\x89\x20\x94\x2b\xc2\x36\xcd"
			     "\x64\xfb\x6f\x06\x9d\x11\xa8\x3f"
			     "\xd6\x4a\xe1\x78\x0f\x83\x1a\xb1"
			     "\x25\xbc\x53\xea\x5e\xf5\x8c\x00"
			     "\x97\x2e\xc5\x39\xd0\x67\xfe\x72"
			     "\x09\xa0\x14\xab\x42\xd9\x4d\xe4"
			     "\x7b\x12\x86\x1d\xb4\x28\xbf\x56"
			     "\xed\x61\xf8\x8f\x03\x9a\x31\xc8"
			     "\x3c\xd3\x6a\x01\x75\x0c\xa3\x17"
			     "\xae\x45\xdc\x50\xe7\x7e\x15\x89"
			     "\x20\xb7\x2b\xc2\x59\xf0\x64\xfb"
			     "\x92\x06\x9d\x34\xcb\x3f\xd6\x6d"
			     "\x04\x78\x0f\xa6\x1a\xb1\x48\xdf"
			     "\x53\xea\x81\x18\x8c\x23\xba\x2e"
			     "\xc5\x5c\xf3\x67\xfe\x95\x09\xa0"
			     "\x37\xce\x42\xd9\x70\x07\x7b\x12"
			     "\xa9\x1d\xb4\x4b\xe2\x56\xed\x84"
			     "\x1b\x8f\x26\xbd\x31\xc8\x5f\xf6"
			     "\x6a\x01\x98\x0c\xa3\x3a\xd1\x45"
			     "\xdc\x73\x0a\x7e\x15\xac\x20\xb7"
			     "\x4e\xe5\x59\xf0\x87\x1e\x92\x29"
			     "\xc0\x34\xcb\x62\xf9\x6d\x04\x9b"
			     "\x0f\xa6\x3d\xd4\x48\xdf\x76\x0d"
			     "\x81\x18\xaf\x23\xba\x51\xe8\x5c"
			     "\xf3\x8a\x21\x95\x2c\xc3\x37\xce"
			     "\x65\xfc\x70\x07\x9e\x12\xa9\x40"
			     "\xd7\x4b\xe2\x79\x10\x84\x1b\xb2"
			     "\x26\xbd\x54\xeb\x5f\xf6\x8d\x01"
			     "\x98\x2f\xc6\x3a\xd1\x68\xff\x73"
			     "\x0a\xa1\x15\xac\x43\xda\x4e\xe5"
			     "\x7c\x13\x87\x1e\xb5\x29\xc0\x57"
			     "\xee\x62\xf9\x90\x04\x9b\x32\xc9"
			     "\x3d\xd4\x6b\x02\x76\x0d\xa4\x18"
			     "\xaf\x46\xdd\x51\xe8\x7f\x16\x8a"
			     "\x21\xb8\x2c\xc3\x5a\xf1\x65\xfc"
			     "\x93\x07\x9e\x35\xcc\x40\xd7\x6e"
			     "\x05\x79\x10\xa7\x1b\xb2\x49\xe0"
			     "\x54\xeb\x82\x19\x8d\x24\xbb\x2f"
			     "\xc6\x5d\xf4\x68\xff\x96\x0a\xa1"
			     "\x38\xcf\x43\xda\x71\x08\x7c\x13"
			     "\xaa\x1e\xb5\x4c\xe3\x57\xee\x85"
			     "\x1c\x90\x27\xbe\x32\xc9\x60\xf7"
			     "\x6b\x02\x99\x0d\xa4\x3b\xd2\x46"
			     "\xdd\x74\x0b\x7f\x16\xad\x21\xb8"
			     "\x4f\xe6\x5a\xf1\x88\x1f\x93\x2a"
			     "\xc1\x35\xcc\x63\xfa\x6e\x05\x9c"
			     "\x10\xa7\x3e\xd5\x49\xe0\x77\x0e"
			     "\x82\x19\xb0\x24\xbb\x52\xe9\x5d"
			     "\xf4\x8b\x22\x96\x2d\xc4\x38\xcf"
			     "\x66\xfd\x71\x08\x9f\x13\xaa\x41"
			     "\xd8\x4c\xe3\x7a\x11\x85\x1c\xb3"
			     "\x27\xbe\x55\xec\x60\xf7\x8e\x02"
			     "\x99\x30\xc7\x3b\xd2\x69\x00\x74"
			     "\x0b\xa2\x16\xad\x44\xdb\x4f\xe6"
			     "\x7d\x14\x88\x1f\xb6\x2a\xc1\x58"
			     "\xef\x63\xfa\x91\x05\x9c\x33\xca"
			     "\x3e\xd5\x6c\x03\x77\x0e\xa5\x19"
			     "\xb0\x47\xde\x52\xe9\x80\x17\x8b"
			     "\x22\xb9\x2d\xc4\x5b\xf2\x66\xfd"
			     "\x94\x08\x9f\x36\xcd\x41\xd8\x6f"
			     "\x06\x7a\x11\xa8\x1c\xb3\x4a\xe1"
			     "\x55\xec\x83\x1a\x8e\x25\xbc\x30"
			     "\xc7\x5e\xf5\x69\x00\x97\x0b\xa2"
			     "\x39\xd0\x44\xdb\x72\x09\x7d\x14"
			     "\xab\x1f\xb6\x4d\xe4\x58\xef\x86"
			     "\x1d\x91\x28\xbf\x33\xca\x61\xf8"
			     "\x6c\x03\x9a\x0e\xa5\x3c\xd3\x47"
			     "\xde\x75\x0c\x80\x17\xae\x22\xb9"
			     "\x50\xe7\x5b\xf2\x89\x20\x94\x2b"
			     "\xc2\x36\xcd\x64\xfb\x6f\x06\x9d"
			     "\x11\xa8\x3f\xd6\x4a\xe1\x78\x0f"
			     "\x83\x1a\xb1\x25\xbc\x53\xea\x5e"
			     "\xf5\x8c\x00\x97\x2e\xc5\x39\xd0"
			     "\x67\xfe\x72\x09\xa0\x14\xab\x42"
			     "\xd9\x4d\xe4\x7b\x12\x86\x1d\xb4"
			     "\x28\xbf\x56\xed\x61\xf8\x8f\x03"
			     "\x9a\x31\xc8\x3c\xd3\x6a\x01\x75"
			     "\x0c\xa3\x17\xae\x45\xdc\x50\xe7"
			     "\x7e\x15\x89\x20\xb7\x2b\xc2\x59"
			     "\xf0\x64\xfb\x92\x06\x9d\x34\xcb"
			     "\x3f\xd6\x6d\x04\x78\x0f\xa6\x1a"
			     "\xb1\x48\xdf\x53\xea\x81\x18\x8c"
			     "\x23\xba\x2e\xc5\x5c\xf3\x67\xfe"
			     "\x95\x09\xa0\x37\xce\x42\xd9\x70"
			     "\x07\x7b\x12\xa9\x1d\xb4\x4b\xe2"
			     "\x56\xed\x84\x1b\x8f\x26\xbd\x31"
			     "\xc8\x5f\xf6\x6a\x01\x98\x0c\xa3"
			     "\x3a\xd1\x45\xdc\x73\x0a\x7e\x15"
			     "\xac\x20\xb7\x4e\xe5\x59\xf0\x87"
			     "\x1e\x92\x29\xc0\x34\xcb\x62\xf9"
			     "\x6d\x04\x9b\x0f\xa6\x3d\xd4\x48"
			     "\xdf\x76\x0d\x81\x18\xaf\x23\xba"
			     "\x51\xe8\x5c\xf3\x8a\x21\x95\x2c"
			     "\xc3\x37\xce\x65\xfc\x70\x07\x9e"
			     "\x12\xa9\x40\xd7\x4b\xe2\x79\x10"
			     "\x84\x1b\xb2\x26\xbd\x54\xeb\x5f"
			     "\xf6\x8d\x01\x98\x2f\xc6\x3a\xd1"
			     "\x68\xff\x73\x0a\xa1\x15\xac\x43"
			     "\xda\x4e\xe5\x7c\x13\x87\x1e\xb5"
			     "\x29\xc0\x57\xee\x62\xf9\x90\x04"
			     "\x9b\x32\xc9\x3d\xd4\x6b\x02\x76"
			     "\x0d\xa4\x18\xaf\x46\xdd\x51\xe8"
			     "\x7f\x16\x8a\x21\xb8\x2c\xc3\x5a"
			     "\xf1\x65\xfc\x93\x07\x9e\x35\xcc"
			     "\x40\xd7\x6e\x05\x79\x10\xa7\x1b"
			     "\xb2\x49\xe0\x54\xeb\x82\x19\x8d"
			     "\x24\xbb\x2f\xc6\x5d\xf4\x68\xff"
			     "\x96\x0a\xa1\x38\xcf\x43\xda\x71"
			     "\x08\x7c\x13\xaa\x1e\xb5\x4c",
		.psize     = 1023,
		.digest    = "\xc5\xce\x0c\xca\x01\x4f\x53\x3a"
			     "\x32\x32\x17\xcc\xd4\x6a\x71\xa9"
			     "\xf3\xed\x50\x10\x64\x8e\x06\xbe"
			     "\x9b\x4a\xa6\xbb\x05\x89\x59\x51",
	}
};

static const struct hash_testvec blake2b_256_tv[] = {
	{
		.plaintext =
			"",
		.psize     = 0,
		.digest    =
			"\x0e\x57\x51\xc0\x26\xe5\x43\xb2"
			"\xe8\xab\x2e\xb0\x60\x99\xda\xa1"
			"\xd1\xe5\xdf\x47\x77\x8f\x77\x87"
			"\xfa\xab\x45\xcd\xf1\x2f\xe3\xa8",
	}, {
		.plaintext =
			"\x00\x01\x02\x03\x04\x05\x06\x07"
			"\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			"\x10\x11\x12\x13\x14\x15\x16\x17"
			"\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
			"\x20\x21\x22\x23\x24\x25\x26\x27"
			"\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
			"\x30\x31\x32\x33\x34\x35\x36\x37"
			"\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f",
		.psize     = 64,
		.digest    =
			"\x10\xd8\xe6\xd5\x34\xb0\x09\x39"
			"\x84\x3f\xe9\xdc\xc4\xda\xe4\x8c"
			"\xdf\x00\x8f\x6b\x8b\x2b\x82\xb1"
			"\x56\xf5\x40\x4d\x87\x48\x87\xf5",
	}, {
		.plaintext = "\x08\x9f\x13\xaa\x41\xd8\x4c\xe3"
			     "\x7a\x11\x85\x1c\xb3\x27\xbe\x55"
			     "\xec\x60\xf7\x8e\x02\x99\x30\xc7"
			     "\x3b\xd2\x69\x00\x74\x0b\xa2\x16"
			     "\xad\x44\xdb\x4f\xe6\x7d\x14\x88"
			     "\x1f\xb6\x2a\xc1\x58\xef\x63\xfa"
			     "\x91\x05\x9c\x33\xca\x3e\xd5\x6c"
			     "\x03\x77\x0e\xa5\x19\xb0\x47\xde"
			     "\x52\xe9\x80\x17\x8b\x22\xb9\x2d"
			     "\xc4\x5b\xf2\x66\xfd\x94\x08\x9f"
			     "\x36\xcd\x41\xd8\x6f\x06\x7a\x11"
			     "\xa8\x1c\xb3\x4a\xe1\x55\xec\x83"
			     "\x1a\x8e\x25\xbc\x30\xc7\x5e\xf5"
			     "\x69\x00\x97\x0b\xa2\x39\xd0\x44"
			     "\xdb\x72\x09\x7d\x14\xab\x1f\xb6"
			     "\x4d\xe4\x58\xef\x86\x1d\x91\x28"
			     "\xbf\x33\xca\x61\xf8\x6c\x03\x9a"
			     "\x0e\xa5\x3c\xd3\x47\xde\x75\x0c"
			     "\x80\x17\xae\x22\xb9\x50\xe7\x5b"
			     "\xf2\x89\x20\x94\x2b\xc2\x36\xcd"
			     "\x64\xfb\x6f\x06\x9d\x11\xa8\x3f"
			     "\xd6\x4a\xe1\x78\x0f\x83\x1a\xb1"
			     "\x25\xbc\x53\xea\x5e\xf5\x8c\x00"
			     "\x97\x2e\xc5\x39\xd0\x67\xfe\x72"
			     "\x09\xa0\x14\xab\x42\xd9\x4d\xe4"
			     "\x7b\x12\x86\x1d\xb4\x28\xbf\x56"
			     "\xed\x61\xf8\x8f\x03\x9a\x31\xc8"
			     "\x3c\xd3\x6a\x01\x75\x0c\xa3\x17"
			     "\xae\x45\xdc\x50\xe7\x7e\x15\x89"
			     "\x20\xb7\x2b\xc2\x59\xf0\x64\xfb"
			     "\x92\x06\x9d\x34\xcb\x3f\xd6\x6d"
			     "\x04\x78\x0f\xa6\x1a\xb1\x48\xdf"
			     "\x53\xea\x81\x18\x8c\x23\xba\x2e"
			     "\xc5\x5c\xf3\x67\xfe\x95\x09\xa0"
			     "\x37\xce\x42\xd9\x70\x07\x7b\x12"
			     "\xa9\x1d\xb4\x4b\xe2\x56\xed\x84"
			     "\x1b\x8f\x26\xbd\x31\xc8\x5f\xf6"
			     "\x6a\x01\x98\x0c\xa3\x3a\xd1\x45"
			     "\xdc\x73\x0a\x7e\x15\xac\x20\xb7"
			     "\x4e\xe5\x59\xf0\x87\x1e\x92\x29"
			     "\xc0\x34\xcb\x62\xf9\x6d\x04\x9b"
			     "\x0f\xa6\x3d\xd4\x48\xdf\x76\x0d"
			     "\x81\x18\xaf\x23\xba\x51\xe8\x5c"
			     "\xf3\x8a\x21\x95\x2c\xc3\x37\xce"
			     "\x65\xfc\x70\x07\x9e\x12\xa9\x40"
			     "\xd7\x4b\xe2\x79\x10\x84\x1b\xb2"
			     "\x26\xbd\x54\xeb\x5f\xf6\x8d\x01"
			     "\x98\x2f\xc6\x3a\xd1\x68\xff\x73"
			     "\x0a\xa1\x15\xac\x43\xda\x4e\xe5"
			     "\x7c\x13\x87\x1e\xb5\x29\xc0\x57"
			     "\xee\x62\xf9\x90\x04\x9b\x32\xc9"
			     "\x3d\xd4\x6b\x02\x76\x0d\xa4\x18"
			     "\xaf\x46\xdd\x51\xe8\x7f\x16\x8a"
			     "\x21\xb8\x2c\xc3\x5a\xf1\x65\xfc"
			     "\x93\x07\x9e\x35\xcc\x40\xd7\x6e"
			     "\x05\x79\x10\xa7\x1b\xb2\x49\xe0"
			     "\x54\xeb\x82\x19\x8d\x24\xbb\x2f"
			     "\xc6\x5d\xf4\x68\xff\x96\x0a\xa1"
			     "\x38\xcf\x43\xda\x71\x08\x7c\x13"
			     "\xaa\x1e\xb5\x4c\xe3\x57\xee\x85"
			     "\x1c\x90\x27\xbe\x32\xc9\x60\xf7"
			     "\x6b\x02\x99\x0d\xa4\x3b\xd2\x46"
			     "\xdd\x74\x0b\x7f\x16\xad\x21\xb8"
			     "\x4f\xe6\x5a\xf1\x88\x1f\x93\x2a"
			     "\xc1\x35\xcc\x63\xfa\x6e\x05\x9c"
			     "\x10\xa7\x3e\xd5\x49\xe0\x77\x0e"
			     "\x82\x19\xb0\x24\xbb\x52\xe9\x5d"
			     "\xf4\x8b\x22\x96\x2d\xc4\x38\xcf"
			     "\x66\xfd\x71\x08\x9f\x13\xaa\x41"
			     "\xd8\x4c\xe3\x7a\x11\x85\x1c\xb3"
			     "\x27\xbe\x55\xec\x60\xf7\x8e\x02"
			     "\x99\x30\xc7\x3b\xd2\x69\x00\x74"
			     "\x0b\xa2\x16\xad\x44\xdb\x4f\xe6"
			     "\x7d\x14\x88\x1f\xb6\x2a\xc1\x58"
			     "\xef\x63\xfa\x91\x05\x9c\x33\xca"
			     "\x3e\xd5\x6c\x03\x77\x0e\xa5\x19"
			     "\xb0\x47\xde\x52\xe9\x80\x17\x8b"
			     "\x22\xb9\x2d\xc4\x5b\xf2\x66\xfd"
			     "\x94\x08\x9f\x36\xcd\x41\xd8\x6f"
			     "\x06\x7a\x11\xa8\x1c\xb3\x4a\xe1"
			     "\x55\xec\x83\x1a\x8e\x25\xbc\x30"
			     "\xc7\x5e\xf5\x69\x00\x97\x0b\xa2"
			     "\x39\xd0\x44\xdb\x72\x09\x7d\x14"
			     "\xab\x1f\xb6\x4d\xe4\x58\xef\x86"
			     "\x1d\x91\x28\xbf\x33\xca\x61\xf8"
			     "\x6c\x03\x9a\x0e\xa5\x3c\xd3\x47"
			     "\xde\x75\x0c\x80\x17\xae\x22\xb9"
			     "\x50\xe7\x5b\xf2\x89\x20\x94\x2b"
			     "\xc2\x36\xcd\x64\xfb\x6f\x06\x9d"
			     "\x11\xa8\x3f\xd6\x4a\xe1\x78\x0f"
			     "\x83\x1a\xb1\x25\xbc\x53\xea\x5e"
			     "\xf5\x8c\x00\x97\x2e\xc5\x39\xd0"
			     "\x67\xfe\x72\x09\xa0\x14\xab\x42"
			     "\xd9\x4d\xe4\x7b\x12\x86\x1d\xb4"
			     "\x28\xbf\x56\xed\x61\xf8\x8f\x03"
			     "\x9a\x31\xc8\x3c\xd3\x6a\x01\x75"
			     "\x0c\xa3\x17\xae\x45\xdc\x50\xe7"
			     "\x7e\x15\x89\x20\xb7\x2b\xc2\x59"
			     "\xf0\x64\xfb\x92\x06\x9d\x34\xcb"
			     "\x3f\xd6\x6d\x04\x78\x0f\xa6\x1a"
			     "\xb1\x48\xdf\x53\xea\x81\x18\x8c"
			     "\x23\xba\x2e\xc5\x5c\xf3\x67\xfe"
			     "\x95\x09\xa0\x37\xce\x42\xd9\x70"
			     "\x07\x7b\x12\xa9\x1d\xb4\x4b\xe2"
			     "\x56\xed\x84\x1b\x8f\x26\xbd\x31"
			     "\xc8\x5f\xf6\x6a\x01\x98\x0c\xa3"
			     "\x3a\xd1\x45\xdc\x73\x0a\x7e\x15"
			     "\xac\x20\xb7\x4e\xe5\x59\xf0\x87"
			     "\x1e\x92\x29\xc0\x34\xcb\x62\xf9"
			     "\x6d\x04\x9b\x0f\xa6\x3d\xd4\x48"
			     "\xdf\x76\x0d\x81\x18\xaf\x23\xba"
			     "\x51\xe8\x5c\xf3\x8a\x21\x95\x2c"
			     "\xc3\x37\xce\x65\xfc\x70\x07\x9e"
			     "\x12\xa9\x40\xd7\x4b\xe2\x79\x10"
			     "\x84\x1b\xb2\x26\xbd\x54\xeb\x5f"
			     "\xf6\x8d\x01\x98\x2f\xc6\x3a\xd1"
			     "\x68\xff\x73\x0a\xa1\x15\xac\x43"
			     "\xda\x4e\xe5\x7c\x13\x87\x1e\xb5"
			     "\x29\xc0\x57\xee\x62\xf9\x90\x04"
			     "\x9b\x32\xc9\x3d\xd4\x6b\x02\x76"
			     "\x0d\xa4\x18\xaf\x46\xdd\x51\xe8"
			     "\x7f\x16\x8a\x21\xb8\x2c\xc3\x5a"
			     "\xf1\x65\xfc\x93\x07\x9e\x35\xcc"
			     "\x40\xd7\x6e\x05\x79\x10\xa7\x1b"
			     "\xb2\x49\xe0\x54\xeb\x82\x19\x8d"
			     "\x24\xbb\x2f\xc6\x5d\xf4\x68\xff"
			     "\x96\x0a\xa1\x38\xcf\x43\xda\x71"
			     "\x08\x7c\x13\xaa\x1e\xb5\x4c",
		.psize     = 1023,
		.digest    = "\xb4\x82\x0f\x69\x9c\xc9\x3e\x51"
			     "\xbf\xe6\x82\x94\x3f\x59\x91\xee"
			     "\x18\xa6\x27\xc3\x00\x12\xa5\xd0"
			     "\x92\x3a\xb1\xc6\xa3\x46\x5a\xea",
	}
};

static const struct hash_testspec test_spec[] = {
	{
		.name = "CRC32C-ref",
		.digest_size = 4,
		.testvec = crc32c_tv,
		.count = ARRAY_SIZE(crc32c_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_crc32c,
	}, {
		.name = "CRC32C-NI",
		.digest_size = 4,
		.testvec = crc32c_tv,
		.count = ARRAY_SIZE(crc32c_tv),
		.cpu_flag = CPU_FLAG_SSE42,
		.hash = hash_crc32c
	}, {
		.name = "XXHASH",
		.digest_size = 8,
		.testvec = xxhash64_tv,
		.count = ARRAY_SIZE(xxhash64_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_xxhash
	}, {
		.name = "SHA256-ref",
		.digest_size = 32,
		.testvec = sha256_tv,
		.count = ARRAY_SIZE(sha256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_sha256
	}, {
		.name = "SHA256-gcrypt",
		.digest_size = 32,
		.testvec = sha256_tv,
		.count = ARRAY_SIZE(sha256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_sha256,
		.backend = CRYPTOPROVIDER_LIBGCRYPT + 1
	}, {
		.name = "SHA256-sodium",
		.digest_size = 32,
		.testvec = sha256_tv,
		.count = ARRAY_SIZE(sha256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_sha256,
		.backend = CRYPTOPROVIDER_LIBSODIUM + 1
	}, {
		.name = "SHA256-kcapi",
		.digest_size = 32,
		.testvec = sha256_tv,
		.count = ARRAY_SIZE(sha256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_sha256,
		.backend = CRYPTOPROVIDER_LIBKCAPI + 1
	}, {
		.name = "SHA256-NI",
		.digest_size = 32,
		.testvec = sha256_tv,
		.count = ARRAY_SIZE(sha256_tv),
		.cpu_flag = CPU_FLAG_SHA,
		.hash = hash_sha256,
		.backend = CRYPTOPROVIDER_BUILTIN + 1
	}, {
		.name = "BLAKE2-ref",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_BUILTIN + 1
	}, {
		.name = "BLAKE2-gcrypt",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_LIBGCRYPT + 1
	}, {
		.name = "BLAKE2-sodium",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_LIBSODIUM + 1
	}, {
		.name = "BLAKE2-kcapi",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_NONE,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_LIBKCAPI + 1
	}, {
		.name = "BLAKE2-SSE2",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_SSE2,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_BUILTIN + 1
	}, {
		.name = "BLAKE2-SSE41",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_SSE41,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_BUILTIN + 1
	}, {
		.name = "BLAKE2-AVX2",
		.digest_size = 32,
		.testvec = blake2b_256_tv,
		.count = ARRAY_SIZE(blake2b_256_tv),
		.cpu_flag = CPU_FLAG_AVX2,
		.hash = hash_blake2b,
		.backend = CRYPTOPROVIDER_BUILTIN + 1
	}
};

static int test_hash(const struct hash_testspec *spec)
{
	int i;
	bool header = false;

	for (i = 0; i < spec->count; i++) {
		int ret;
		const struct hash_testvec *vec = &spec->testvec[i];
		u8 csum[CRYPTO_HASH_SIZE_MAX];

		if (spec->cpu_flag != 0 && !cpu_has_feature(spec->cpu_flag)) {
			printf("%s skipped, no CPU support\n", spec->name);
			continue;
		}
		/* Backend not compiled in */
		if (spec->backend == 1)
			continue;

		if (!header) {
			printf("TEST: name=%s vectors=%zd\n", spec->name, spec->count);
			header = true;
		}

		if (spec->cpu_flag) {
			cpu_set_level(spec->cpu_flag);
			hash_init_accel();
		}
		ret = spec->hash((const u8 *)vec->plaintext, vec->psize, csum);
		cpu_reset_level();
		if (ret < 0) {
			error("hash %s = %d", spec->name, ret);
			return 1;
		}
		if (memcmp(csum, vec->digest, spec->digest_size) == 0) {
			printf("%s vector %d: match\n", spec->name, i);
		} else {
			int j;

			printf("%s vector %d: MISMATCH\n", spec->name, i);
			printf("  want:");
			for (j = 0; j < spec->digest_size; j++)
				printf(" %02hhx", vec->digest[j]);
			putchar('\n');
			printf("  have:");
			for (j = 0; j < spec->digest_size; j++)
				printf(" %02hhx", csum[j]);
			putchar('\n');
		}
	}

	return 0;
}

int main(int argc, char **argv) {
	int i;

	cpu_detect_flags();
	cpu_print_flags();
	hash_init_accel();

	printf("Implementation: %s\n", CRYPTOPROVIDER);
	for (i = 0; i < ARRAY_SIZE(test_spec); i++)
		test_hash(&test_spec[i]);

	return 0;
}
