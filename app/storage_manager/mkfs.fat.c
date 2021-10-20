/* mkfs.fat.c - utility to create FAT/MS-DOS filesystems

   Copyright (C) 1991 Linus Torvalds <torvalds@klaava.helsinki.fi>
   Copyright (C) 1992-1993 Remy Card <card@masi.ibp.fr>
   Copyright (C) 1993-1994 David Hudson <dave@humbug.demon.co.uk>
   Copyright (C) 1998 H. Peter Anvin <hpa@zytor.com>
   Copyright (C) 1998-2005 Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
   Copyright (C) 2008-2014 Daniel Baumann <mail@daniel-baumann.ch>
   Copyright (C) 2015-2016 Andreas Bombe <aeb@debian.org>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.

   The complete text of the GNU General Public License
   can be found in /usr/share/common-licenses/GPL-3 file.
*/

/* Description: Utility to allow an MS-DOS filesystem to be created
   under Linux.  A lot of the basic structure of this program has been
   borrowed from Remy Card's "mke2fs" code.

   As far as possible the aim here is to make the "mkfs.fat" command
   look almost identical to the other Linux filesystem make utilties,
   eg bad blocks are still specified as blocks, not sectors, but when
   it comes down to it, DOS is tied to the idea of a sector (512 bytes
   as a rule), and not the block.  For example the boot block does not
   occupy a full cluster.

   Fixes/additions May 1998 by Roman Hodek
   <Roman.Hodek@informatik.uni-erlangen.de>:
   - Atari format support
   - New options -A, -S, -C
   - Support for filesystems > 2GB
   - FAT32 support */

/* Include the header files */

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <getopt.h>
#include <glib.h>

#include "msdos_fs.h"
#include "device_info.h"

#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mkfs.fat.c"

/* Constant definitions */

#define TEST_BUFFER_BLOCKS 16
#define BLOCK_SIZE         1024
#define HARD_SECTOR_SIZE   512
#define SECTORS_PER_BLOCK ( BLOCK_SIZE / HARD_SECTOR_SIZE )

#define NO_NAME "NO NAME    "

/* Macro definitions */

/* Report a failure message and return a failure error code */

#define die( str ) fatal_error( "%s: " str "\n" )

/* Compute ceil(a/b) */

static inline int cdiv(int a, int b)
{
    return (a + b - 1) / b;
}

/* FAT values */
#define FAT_EOF      (para->atari_format ? 0x0fffffff : 0x0ffffff8)
#define FAT_BAD      0x0ffffff7

#define MSDOS_EXT_SIGN 0x29 /* extended boot sector signature */
#define MSDOS_FAT12_SIGN "FAT12   " /* FAT12 filesystem signature */
#define MSDOS_FAT16_SIGN "FAT16   " /* FAT16 filesystem signature */
#define MSDOS_FAT32_SIGN "FAT32   " /* FAT32 filesystem signature */

#define BOOT_SIGN 0xAA55    /* Boot sector magic number */

#define MAX_CLUST_12    ((1 << 12) - 16)
#define MAX_CLUST_16    ((1 << 16) - 16)
#define MIN_CLUST_32    65529
/* M$ says the high 4 bits of a FAT32 FAT entry are reserved and don't belong
 * to the cluster number. So the max. cluster# is based on 2^28 */
#define MAX_CLUST_32    ((1 << 28) - 16)

#define FAT12_THRESHOLD 4085

#define OLDGEMDOS_MAX_SECTORS   32765
#define GEMDOS_MAX_SECTORS  65531
#define GEMDOS_MAX_SECTOR_SIZE  (16*1024)

#define BOOTCODE_SIZE       448
#define BOOTCODE_FAT32_SIZE 420

/* __attribute__ ((packed)) is used on all structures to make gcc ignore any
 * alignments */

struct msdos_volume_info {
    uint8_t drive_number;   /* BIOS drive number */
    uint8_t RESERVED;       /* Unused */
    uint8_t ext_boot_sign;  /* 0x29 if fields below exist (DOS 3.3+) */
    uint8_t volume_id[4];   /* Volume ID number */
    uint8_t volume_label[11];   /* Volume label */
    uint8_t fs_type[8];     /* Typically FAT12 or FAT16 */
} __attribute__ ((packed));

struct msdos_boot_sector {
    uint8_t boot_jump[3];   /* Boot strap short or near jump */
    uint8_t system_id[8];   /* Name - can be used to special case
                   partition manager volumes */
    uint8_t sector_size[2]; /* bytes per logical sector */
    uint8_t cluster_size;   /* sectors/cluster */
    uint16_t reserved;      /* reserved sectors */
    uint8_t fats;       /* number of FATs */
    uint8_t dir_entries[2]; /* root directory entries */
    uint8_t sectors[2];     /* number of sectors */
    uint8_t media;      /* media code (unused) */
    uint16_t fat_length;    /* sectors/FAT */
    uint16_t secs_track;    /* sectors per track */
    uint16_t heads;     /* number of heads */
    uint32_t hidden;        /* hidden sectors (unused) */
    uint32_t total_sect;    /* number of sectors (if sectors == 0) */
    union {
        struct {
            struct msdos_volume_info vi;
            uint8_t boot_code[BOOTCODE_SIZE];
        } __attribute__ ((packed)) _oldfat;
        struct {
            uint32_t fat32_length;  /* sectors/FAT */
            uint16_t flags;     /* bit 8: fat mirroring, low 4: active fat */
            uint8_t version[2];     /* major, minor filesystem version */
            uint32_t root_cluster;  /* first cluster in root directory */
            uint16_t info_sector;   /* filesystem info sector */
            uint16_t backup_boot;   /* backup boot sector */
            uint16_t reserved2[6];  /* Unused */
            struct msdos_volume_info vi;
            uint8_t boot_code[BOOTCODE_FAT32_SIZE];
        } __attribute__ ((packed)) _fat32;
    } __attribute__ ((packed)) fstype;
    uint16_t boot_sign;
} __attribute__ ((packed));
#define fat32   fstype._fat32
#define oldfat  fstype._oldfat

struct fat32_fsinfo {
    uint32_t reserved1;     /* Nothing as far as I can tell */
    uint32_t signature;     /* 0x61417272L */
    uint32_t free_clusters; /* Free cluster count.  -1 if unknown */
    uint32_t next_cluster;  /* Most recently allocated cluster.
                 * Unused under Linux. */
    uint32_t reserved2[4];
};

/* The "boot code" we put into the filesystem... it writes a message and
   tells the user to try again */

unsigned char dummy_boot_jump[3] = { 0xeb, 0x3c, 0x90 };

unsigned char dummy_boot_jump_m68k[2] = { 0x60, 0x1c };

#define MSG_OFFSET_OFFSET 3
char dummy_boot_code[BOOTCODE_SIZE] = "\x0e"    /* push cs */
                                      "\x1f"          /* pop ds */
                                      "\xbe\x5b\x7c"      /* mov si, offset message_txt */
                                      /* write_msg: */
                                      "\xac"          /* lodsb */
                                      "\x22\xc0"          /* and al, al */
                                      "\x74\x0b"          /* jz key_press */
                                      "\x56"          /* push si */
                                      "\xb4\x0e"          /* mov ah, 0eh */
                                      "\xbb\x07\x00"      /* mov bx, 0007h */
                                      "\xcd\x10"          /* int 10h */
                                      "\x5e"          /* pop si */
                                      "\xeb\xf0"          /* jmp write_msg */
                                      /* key_press: */
                                      "\x32\xe4"          /* xor ah, ah */
                                      "\xcd\x16"          /* int 16h */
                                      "\xcd\x19"          /* int 19h */
                                      "\xeb\xfe"          /* foo: jmp foo */
                                      /* message_txt: */
                                      "This is not a bootable disk.  Please insert a bootable floppy and\r\n"
                                      "press any key to try again ... \r\n";

#define MESSAGE_OFFSET 29   /* Offset of message in above code */

struct mkfs_para {
    char *device_name;    /* Name of the device on which to create the filesystem */
    int atari_format;    /* Use Atari variation of MS-DOS FS format */
    int verbose;     /* Default to verbose mode off */
    long volume_id;      /* Volume ID number */
    time_t create_time;  /* Creation time */
    char volume_name[16];    /* Volume name */
    uint64_t blocks; /* Number of blocks in filesystem */
    unsigned sector_size;  /* Size of a logical sector */
    int sector_size_set; /* User selected sector size */
    int backup_boot; /* Sector# of backup boot sector */
    int reserved_sectors;    /* Number of reserved sectors */
    int badblocks;   /* Number of bad blocks in the filesystem */
    int nr_fats;     /* Default number of FATs to produce */
    int size_fat;    /* Size in bits of FAT entries */
    int size_fat_by_user;    /* 1 if FAT size user selected */
    int dev;        /* FS block device file handle */
    int ignore_full_disk;    /* Ignore warning about 'full' disk devices */
    off_t currently_testing; /* Block currently being tested (if autodetect bad blocks) */
    struct msdos_boot_sector bs; /* Boot sector data */
    int start_data_sector;   /* Sector number for the start of the data area */
    int start_data_block;    /* Block number for the start of the data area */
    unsigned char *fat;  /* File allocation table */
    unsigned alloced_fat_length; /* # of FAT sectors we can keep in memory */
    unsigned fat_entries;        /* total entries in FAT table (including reserved) */
    unsigned char *info_sector;  /* FAT32 info sector */
    struct msdos_dir_entry *root_dir;    /* Root directory */
    int size_root_dir;   /* Size of the root directory in bytes */
    uint32_t num_sectors;        /* Total number of sectors in device */
    int sectors_per_cluster; /* Number of sectors per disk cluster */
    int root_dir_entries;    /* Number of root directory entries */
    char *blank_sector;  /* Blank sector - all zeros */
    int hidden_sectors;  /* Number of hidden sectors */
    int hidden_sectors_by_user;  /* -h option invoked */
    int drive_number_option; /* drive number */
    int drive_number_by_user;    /* drive number option invoked */
    int fat_media_byte;  /* media byte in header and starting FAT */
    int malloc_entire_fat;   /* Whether we should malloc() the entire FAT or not */
    int align_structures; /* Whether to enforce alignment */
    int orphaned_sectors;    /* Sectors that exist in the last block of filesystem */
    int invariant;              /* Whether to set normally randomized or
                       current time based values to
                       constants */
};

/* Global variables - the root of all evil :-) - see these and weep! */

static const char *program_name = "mkfs.fat";   /* Name of the program */

/* Function prototype definitions */

static void fatal_error(const char *fmt_string) __attribute__ ((noreturn));
static void mark_FAT_cluster(struct mkfs_para *para, int cluster, unsigned int value);
static void establish_params(struct mkfs_para *para, struct device_info *info);
static void setup_tables(struct mkfs_para *para);
static void write_tables(struct mkfs_para *para, int *prog);

/* The function implementations */

/* Handle the reporting of fatal errors.  Volatile to let gcc know that this doesn't return */

static void fatal_error(const char *fmt_string)
{
    fprintf(stderr, fmt_string, program_name, "device_name");
    LOG_ERROR("%s\n", fmt_string);
    exit(1);            /* The error exit code is 1! */
}

/* Mark the specified cluster as having a particular value */

static void mark_FAT_cluster(struct mkfs_para *para, int cluster, unsigned int value)
{

    if (cluster < 0 || cluster >= para->fat_entries)
        die("Internal error: out of range cluster number in mark_FAT_cluster");

    switch (para->size_fat) {
    case 12:
        value &= 0x0fff;
        if (((cluster * 3) & 0x1) == 0) {
            para->fat[3 * cluster / 2] = (unsigned char)(value & 0x00ff);
            para->fat[(3 * cluster / 2) + 1] =
                (unsigned char)((para->fat[(3 * cluster / 2) + 1] & 0x00f0)
                                | ((value & 0x0f00) >> 8));
        } else {
            para->fat[3 * cluster / 2] =
                (unsigned char)((para->fat[3 * cluster / 2] & 0x000f) |
                                ((value & 0x000f) << 4));
            para->fat[(3 * cluster / 2) + 1] = (unsigned char)((value & 0x0ff0) >> 4);
        }
        break;

    case 16:
        value &= 0xffff;
        para->fat[2 * cluster] = (unsigned char)(value & 0x00ff);
        para->fat[(2 * cluster) + 1] = (unsigned char)(value >> 8);
        break;

    case 32:
        value &= 0xfffffff;
        para->fat[4 * cluster] = (unsigned char)(value & 0x000000ff);
        para->fat[(4 * cluster) + 1] = (unsigned char)((value & 0x0000ff00) >> 8);
        para->fat[(4 * cluster) + 2] = (unsigned char)((value & 0x00ff0000) >> 16);
        para->fat[(4 * cluster) + 3] = (unsigned char)((value & 0xff000000) >> 24);
        break;

    default:
        die("Bad FAT size (not 12, 16, or 32)");
    }
}

/* Establish the geometry and media parameters for the device */

static void establish_params(struct mkfs_para *para, struct device_info *info)
{
    unsigned int sec_per_track = 63;
    unsigned int heads = 255;
    unsigned int media = 0xf8;
    unsigned int cluster_size = 4;  /* starting point for FAT12 and FAT16 */
    int def_root_dir_entries = 512;

    if (info->size < 512 * 1024 * 1024) {
        /*
         * These values are more or less meaningless, but we can at least
         * use less extreme values for smaller filesystems where the large
         * dummy values signifying LBA only access are not needed.
         */
        sec_per_track = 32;
        heads = 64;
    }

    if (info->type != TYPE_FIXED) {
        /* enter default parameters for floppy disks if the size matches */
        switch (info->size / 1024) {
        case 360:
            sec_per_track = 9;
            heads = 2;
            media = 0xfd;
            cluster_size = 2;
            def_root_dir_entries = 112;
            break;

        case 720:
            sec_per_track = 9;
            heads = 2;
            media = 0xf9;
            cluster_size = 2;
            def_root_dir_entries = 112;
            break;

        case 1200:
            sec_per_track = 15;
            heads = 2;
            media = 0xf9;
            cluster_size = (para->atari_format ? 2 : 1);
            def_root_dir_entries = 224;
            break;

        case 1440:
            sec_per_track = 18;
            heads = 2;
            media = 0xf0;
            cluster_size = (para->atari_format ? 2 : 1);
            def_root_dir_entries = 224;
            break;

        case 2880:
            sec_per_track = 36;
            heads = 2;
            media = 0xf0;
            cluster_size = 2;
            def_root_dir_entries = 224;
            break;
        }
    }

    if (!para->size_fat && info->size >= 512 * 1024 * 1024) {
        if (para->verbose)
            LOG_INFO("Auto-selecting FAT32 for large filesystem\n");
        para->size_fat = 32;
    }
    if (para->size_fat == 32) {
        /*
         * For FAT32, try to do the same as M$'s format command
         * (see http://www.win.tue.nl/~aeb/linux/fs/fat/fatgen103.pdf p. 20):
         * fs size <= 260M: 0.5k clusters
         * fs size <=   8G:   4k clusters
         * fs size <=  16G:   8k clusters
         * fs size <=  32G:  16k clusters
         * fs size >   32G:  32k clusters
         *
         * This only works correctly for 512 byte sectors!
         */
        uint32_t sz_mb = info->size / (1024 * 1024);
        cluster_size =
            sz_mb > 32 * 1024 ? 64 : sz_mb > 16 * 1024 ? 32 : sz_mb >
            8 * 1024 ? 16 : sz_mb > 260 ? 8 : 1;
    }

    if (info->geom_heads > 0) {
        heads = info->geom_heads;
        sec_per_track = info->geom_sectors;
    }

    if (!para->hidden_sectors_by_user && info->geom_start >= 0)
        para->hidden_sectors = htole32(info->geom_start);

    if (!para->root_dir_entries)
        para->root_dir_entries = def_root_dir_entries;

    para->bs.secs_track = htole16(sec_per_track);
    para->bs.heads = htole16(heads);
    para->bs.media = media;
    para->bs.cluster_size = cluster_size;
}

/*
 * If alignment is enabled, round the first argument up to the second; the
 * latter must be a power of two.
 */
static unsigned int align_object(struct mkfs_para *para, unsigned int sectors, unsigned int clustsize)
{
    if (para->align_structures)
        return (sectors + clustsize - 1) & ~(clustsize - 1);
    else
        return sectors;
}

/* Create the filesystem data tables */

static void setup_tables(struct mkfs_para *para)
{
    unsigned cluster_count = 0, fat_length;
    struct tm *ctime;
    struct msdos_volume_info *vi =
        (para->size_fat == 32 ? &para->bs.fat32.vi : &para->bs.oldfat.vi);

    if (para->atari_format) {
        /* On Atari, the first few bytes of the boot sector are assigned
         * differently: The jump code is only 2 bytes (and m68k machine code
         * :-), then 6 bytes filler (ignored), then 3 byte serial number. */
        para->bs.boot_jump[2] = 'm';
        memcpy((char *)para->bs.system_id, "kdosf", strlen("kdosf"));
    } else
        memcpy((char *)para->bs.system_id, "mkfs.fat", strlen("mkfs.fat"));
    if (para->sectors_per_cluster)
        para->bs.cluster_size = (char)para->sectors_per_cluster;

    if (para->fat_media_byte)
        para->bs.media = (char) para->fat_media_byte;

    if (para->bs.media == 0xf8)
        vi->drive_number = 0x80;
    else
        vi->drive_number = 0x00;

    if (para->drive_number_by_user)
        vi->drive_number = (char) para->drive_number_option;

    if (para->size_fat == 32) {
        /* Under FAT32, the root dir is in a cluster chain, and this is
         * signalled by bs.dir_entries being 0. */
        para->root_dir_entries = 0;
    }

    if (para->atari_format) {
        para->bs.system_id[5] = (unsigned char)(para->volume_id & 0x000000ff);
        para->bs.system_id[6] = (unsigned char)((para->volume_id & 0x0000ff00) >> 8);
        para->bs.system_id[7] = (unsigned char)((para->volume_id & 0x00ff0000) >> 16);
    } else {
        vi->volume_id[0] = (unsigned char)(para->volume_id & 0x000000ff);
        vi->volume_id[1] = (unsigned char)((para->volume_id & 0x0000ff00) >> 8);
        vi->volume_id[2] = (unsigned char)((para->volume_id & 0x00ff0000) >> 16);
        vi->volume_id[3] = (unsigned char)(para->volume_id >> 24);
    }

    if (!para->atari_format) {
        memcpy(vi->volume_label, para->volume_name, 11);

        memcpy(para->bs.boot_jump, dummy_boot_jump, 3);
        /* Patch in the correct offset to the boot code */
        para->bs.boot_jump[1] = ((para->size_fat == 32 ?
                            (char *)&para->bs.fat32.boot_code :
                            (char *)&para->bs.oldfat.boot_code) - (char *)&para->bs) - 2;

        if (para->size_fat == 32) {
            int offset = (char *)&para->bs.fat32.boot_code -
                         (char *)&para->bs + MESSAGE_OFFSET + 0x7c00;
            if (dummy_boot_code[BOOTCODE_FAT32_SIZE - 1])
                LOG_INFO("Warning: message too long; truncated\n");
            dummy_boot_code[BOOTCODE_FAT32_SIZE - 1] = 0;
            memcpy(para->bs.fat32.boot_code, dummy_boot_code, BOOTCODE_FAT32_SIZE);
            para->bs.fat32.boot_code[MSG_OFFSET_OFFSET] = offset & 0xff;
            para->bs.fat32.boot_code[MSG_OFFSET_OFFSET + 1] = offset >> 8;
        } else {
            memcpy(para->bs.oldfat.boot_code, dummy_boot_code, BOOTCODE_SIZE);
        }
        para->bs.boot_sign = htole16(BOOT_SIGN);
    } else {
        memcpy(para->bs.boot_jump, dummy_boot_jump_m68k, 2);
    }
    if (para->verbose >= 2)
        LOG_INFO("Boot jump code is %02x %02x\n",
               para->bs.boot_jump[0], para->bs.boot_jump[1]);

    if (!para->reserved_sectors)
        para->reserved_sectors = (para->size_fat == 32) ? 32 : 1;
    else {
        if (para->size_fat == 32 && para->reserved_sectors < 2)
            die("On FAT32 at least 2 reserved sectors are needed.");
    }
    para->bs.reserved = htole16(para->reserved_sectors);
    if (para->verbose >= 2)
        LOG_INFO("Using %d reserved sectors\n", para->reserved_sectors);
    para->bs.fats = (char)para->nr_fats;
    if (!para->atari_format || para->size_fat == 32)
        para->bs.hidden = htole32(para->hidden_sectors);
    else {
        /* In Atari format, hidden is a 16 bit field */
        uint16_t hidden = htole16(para->hidden_sectors);
        if (para->hidden_sectors & ~0xffff)
            die("#hidden doesn't fit in 16bit field of Atari format\n");
        memcpy(&para->bs.hidden, &hidden, 2);
    }

    if ((long long)(para->blocks * BLOCK_SIZE / para->sector_size) + para->orphaned_sectors >
        UINT32_MAX) {
        LOG_INFO("Warning: target too large, space at end will be left unused\n");
        para->num_sectors = UINT32_MAX;
        para->blocks = (uint64_t)UINT32_MAX * para->sector_size / BLOCK_SIZE;
    } else {
        para->num_sectors =
            (long long)(para->blocks * BLOCK_SIZE / para->sector_size) + para->orphaned_sectors;
    }

    if (!para->atari_format) {
        unsigned fatdata1216;   /* Sectors for FATs + data area (FAT12/16) */
        unsigned fatdata32; /* Sectors for FATs + data area (FAT32) */
        unsigned fatlength12, fatlength16, fatlength32;
        unsigned maxclust12, maxclust16, maxclust32;
        unsigned clust12, clust16, clust32;
        int maxclustsize;
        unsigned root_dir_sectors = cdiv(para->root_dir_entries * 32, para->sector_size);

        /*
         * If the filesystem is 8192 sectors or less (4 MB with 512-byte
         * sectors, i.e. floppy size), don't align the data structures.
         */
        if (para->num_sectors <= 8192) {
            if (para->align_structures && para->verbose >= 2)
                LOG_INFO("Disabling alignment due to tiny filesystem\n");

            para->align_structures = FALSE;
        }

        if (para->sectors_per_cluster)
            para->bs.cluster_size = maxclustsize = para->sectors_per_cluster;
        else
            /* An initial guess for bs.cluster_size should already be set */
            maxclustsize = 128;

        do {
            fatdata32 = para->num_sectors
                        - align_object(para, para->reserved_sectors, para->bs.cluster_size);
            fatdata1216 = fatdata32
                          - align_object(para, root_dir_sectors, para->bs.cluster_size);

            if (para->verbose >= 2)
                LOG_INFO("Trying with %d sectors/cluster:\n", para->bs.cluster_size);

            /* The factor 2 below avoids cut-off errors for nr_fats == 1.
             * The "nr_fats*3" is for the reserved first two FAT entries */
            clust12 = 2 * ((long long)fatdata1216 * para->sector_size + para->nr_fats * 3) /
                      (2 * (int)para->bs.cluster_size * para->sector_size + para->nr_fats * 3);
            fatlength12 = cdiv(((clust12 + 2) * 3 + 1) >> 1, para->sector_size);
            fatlength12 = align_object(para, fatlength12, para->bs.cluster_size);
            /* Need to recalculate number of clusters, since the unused parts of the
             * FATS and data area together could make up space for an additional,
             * not really present cluster. */
            clust12 = (fatdata1216 - para->nr_fats * fatlength12) / para->bs.cluster_size;
            maxclust12 = (fatlength12 * 2 * para->sector_size) / 3;
            if (maxclust12 > MAX_CLUST_12)
                maxclust12 = MAX_CLUST_12;
            if (para->verbose >= 2)
                LOG_INFO("FAT12: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
                       clust12, fatlength12, maxclust12, MAX_CLUST_12);
            if (clust12 > maxclust12 - 2) {
                clust12 = 0;
                if (para->verbose >= 2)
                    LOG_INFO("FAT12: too much clusters\n");
            }

            clust16 = ((long long)fatdata1216 * para->sector_size + para->nr_fats * 4) /
                      ((int)para->bs.cluster_size * para->sector_size + para->nr_fats * 2);
            fatlength16 = cdiv((clust16 + 2) * 2, para->sector_size);
            fatlength16 = align_object(para, fatlength16, para->bs.cluster_size);
            /* Need to recalculate number of clusters, since the unused parts of the
             * FATS and data area together could make up space for an additional,
             * not really present cluster. */
            clust16 = (fatdata1216 - para->nr_fats * fatlength16) / para->bs.cluster_size;
            maxclust16 = (fatlength16 * para->sector_size) / 2;
            if (maxclust16 > MAX_CLUST_16)
                maxclust16 = MAX_CLUST_16;
            if (para->verbose >= 2)
                LOG_INFO("FAT16: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
                       clust16, fatlength16, maxclust16, MAX_CLUST_16);
            if (clust16 > maxclust16 - 2) {
                if (para->verbose >= 2)
                    LOG_INFO("FAT16: too much clusters\n");
                clust16 = 0;
            }
            /* The < 4078 avoids that the filesystem will be misdetected as having a
             * 12 bit FAT. */
            if (clust16 < FAT12_THRESHOLD
                && !(para->size_fat_by_user && para->size_fat == 16)) {
                if (para->verbose >= 2)
                    LOG_INFO("FAT16: would be misdetected as FAT12\n");
                clust16 = 0;
            }

            clust32 = ((long long)fatdata32 * para->sector_size + para->nr_fats * 8) /
                      ((int)para->bs.cluster_size * para->sector_size + para->nr_fats * 4);
            fatlength32 = cdiv((clust32 + 2) * 4, para->sector_size);
            fatlength32 = align_object(para, fatlength32, para->bs.cluster_size);
            /* Need to recalculate number of clusters, since the unused parts of the
             * FATS and data area together could make up space for an additional,
             * not really present cluster. */
            clust32 = (fatdata32 - para->nr_fats * fatlength32) / para->bs.cluster_size;
            maxclust32 = (fatlength32 * para->sector_size) / 4;
            if (maxclust32 > MAX_CLUST_32)
                maxclust32 = MAX_CLUST_32;
            if (clust32 && clust32 < MIN_CLUST_32
                && !(para->size_fat_by_user && para->size_fat == 32)) {
                clust32 = 0;
                if (para->verbose >= 2)
                    LOG_INFO("FAT32: not enough clusters (%d)\n", MIN_CLUST_32);
            }
            if (para->verbose >= 2)
                LOG_INFO("FAT32: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
                       clust32, fatlength32, maxclust32, MAX_CLUST_32);
            if (clust32 > maxclust32) {
                clust32 = 0;
                if (para->verbose >= 2)
                    LOG_INFO("FAT32: too much clusters\n");
            }

            if ((clust12 && (para->size_fat == 0 || para->size_fat == 12)) ||
                (clust16 && (para->size_fat == 0 || para->size_fat == 16)) ||
                (clust32 && para->size_fat == 32))
                break;

            para->bs.cluster_size <<= 1;
        } while (para->bs.cluster_size && para->bs.cluster_size <= maxclustsize);

        /* Use the optimal FAT size if not specified;
         * FAT32 is (not yet) choosen automatically */
        if (!para->size_fat) {
            para->size_fat = (clust16 > clust12) ? 16 : 12;
            if (para->verbose >= 2)
                LOG_INFO("Choosing %d bits for FAT\n", para->size_fat);
        }

        switch (para->size_fat) {
        case 12:
            cluster_count = clust12;
            fat_length = fatlength12;
            para->bs.fat_length = htole16(fatlength12);
            memcpy(vi->fs_type, MSDOS_FAT12_SIGN, 8);
            break;

        case 16:
            if (clust16 < FAT12_THRESHOLD) {
                if (para->size_fat_by_user) {
                    LOG_ERROR("WARNING: Not enough clusters for a "
                            "16 bit FAT! The filesystem will be\n"
                            "misinterpreted as having a 12 bit FAT without "
                            "mount option \"fat=16\".\n");
                } else {
                    LOG_ERROR("This filesystem has an unfortunate size. "
                            "A 12 bit FAT cannot provide\n"
                            "enough clusters, but a 16 bit FAT takes up a little "
                            "bit more space so that\n"
                            "the total number of clusters becomes less than the "
                            "threshold value for\n"
                            "distinction between 12 and 16 bit FATs.\n");
                    die("Make the filesystem a bit smaller manually.");
                }
            }
            cluster_count = clust16;
            fat_length = fatlength16;
            para->bs.fat_length = htole16(fatlength16);
            memcpy(vi->fs_type, MSDOS_FAT16_SIGN, 8);
            break;

        case 32:
            if (clust32 < MIN_CLUST_32)
                LOG_ERROR("WARNING: Not enough clusters for a 32 bit FAT!\n");
            cluster_count = clust32;
            fat_length = fatlength32;
            para->bs.fat_length = htole16(0);
            para->bs.fat32.fat32_length = htole32(fatlength32);
            memcpy(vi->fs_type, MSDOS_FAT32_SIGN, 8);
            para->root_dir_entries = 0;
            break;

        default:
            die("FAT not 12, 16 or 32 bits");
        }

        /* Adjust the reserved number of sectors for alignment */
        para->reserved_sectors = align_object(para, para->reserved_sectors, para->bs.cluster_size);
        para->bs.reserved = htole16(para->reserved_sectors);

        /* Adjust the number of root directory entries to help enforce alignment */
        if (para->align_structures) {
            para->root_dir_entries = align_object(para, root_dir_sectors, para->bs.cluster_size)
                               * (para->sector_size >> 5);
        }
    } else {
        unsigned clusters, maxclust, fatdata;

        /* GEMDOS always uses a 12 bit FAT on floppies, and always a 16 bit FAT on
         * hard disks. So use 12 bit if the size of the filesystem suggests that
         * this fs is for a floppy disk, if the user hasn't explicitly requested a
         * size.
         */
        if (!para->size_fat)
            para->size_fat = (para->num_sectors == 1440 || para->num_sectors == 2400 ||
                        para->num_sectors == 2880 || para->num_sectors == 5760) ? 12 : 16;
        if (para->verbose >= 2)
            LOG_INFO("Choosing %d bits for FAT\n", para->size_fat);

        /* Atari format: cluster size should be 2, except explicitly requested by
         * the user, since GEMDOS doesn't like other cluster sizes very much.
         * Instead, tune the sector size for the FS to fit.
         */
        para->bs.cluster_size = para->sectors_per_cluster ? para->sectors_per_cluster : 2;
        if (!para->sector_size_set) {
            while (para->num_sectors > GEMDOS_MAX_SECTORS) {
                para->num_sectors >>= 1;
                para->sector_size <<= 1;
            }
        }
        if (para->verbose >= 2)
            LOG_INFO("Sector size must be %d to have less than %d log. sectors\n",
                   para->sector_size, GEMDOS_MAX_SECTORS);

        /* Check if there are enough FAT indices for how much clusters we have */
        do {
            fatdata = para->num_sectors - cdiv(para->root_dir_entries * 32, para->sector_size) -
                      para->reserved_sectors;
            /* The factor 2 below avoids cut-off errors for nr_fats == 1 and
             * size_fat == 12
             * The "2*nr_fats*size_fat/8" is for the reserved first two FAT entries
             */
            clusters =
                (2 *
                 ((long long)fatdata * para->sector_size -
                  2 * para->nr_fats * para->size_fat / 8)) / (2 * ((int)para->bs.cluster_size *
                                                       para->sector_size +
                                                       para->nr_fats * para->size_fat / 8));
            fat_length = cdiv((clusters + 2) * para->size_fat / 8, para->sector_size);
            /* Need to recalculate number of clusters, since the unused parts of the
             * FATS and data area together could make up space for an additional,
             * not really present cluster. */
            clusters = (fatdata - para->nr_fats * fat_length) / para->bs.cluster_size;
            maxclust = (fat_length * para->sector_size * 8) / para->size_fat;
            if (para->verbose >= 2)
                LOG_INFO("ss=%d: #clu=%d, fat_len=%d, maxclu=%d\n",
                       para->sector_size, clusters, fat_length, maxclust);

            /* last 10 cluster numbers are special (except FAT32: 4 high bits rsvd);
             * first two numbers are reserved */
            if (maxclust <=
                (para->size_fat == 32 ? MAX_CLUST_32 : (1 << para->size_fat) - 0x10)
                && clusters <= maxclust - 2)
                break;
            if (para->verbose >= 2) {
                if (clusters > maxclust - 2)
                    LOG_INFO("Too many clusters\n");
                else
                    LOG_INFO("FAT too big\n");
            }

            /* need to increment sector_size once more to  */
            if (para->sector_size_set)
                die("With this sector size, the maximum number of FAT entries "
                    "would be exceeded.");
            para->num_sectors >>= 1;
            para->sector_size <<= 1;
        } while (para->sector_size <= GEMDOS_MAX_SECTOR_SIZE);

        if (para->sector_size > GEMDOS_MAX_SECTOR_SIZE)
            die("Would need a sector size > 16k, which GEMDOS can't work with");

        cluster_count = clusters;
        if (para->size_fat != 32)
            para->bs.fat_length = htole16(fat_length);
        else {
            para->bs.fat_length = 0;
            para->bs.fat32.fat32_length = htole32(fat_length);
        }
    }

    para->bs.sector_size[0] = (char)(para->sector_size & 0x00ff);
    para->bs.sector_size[1] = (char)((para->sector_size & 0xff00) >> 8);

    para->bs.dir_entries[0] = (char)(para->root_dir_entries & 0x00ff);
    para->bs.dir_entries[1] = (char)((para->root_dir_entries & 0xff00) >> 8);

    if (para->size_fat == 32) {
        /* set up additional FAT32 fields */
        para->bs.fat32.flags = htole16(0);
        para->bs.fat32.version[0] = 0;
        para->bs.fat32.version[1] = 0;
        para->bs.fat32.root_cluster = htole32(2);
        para->bs.fat32.info_sector = htole16(1);
        if (!para->backup_boot)
            para->backup_boot = (para->reserved_sectors >= 7) ? 6 :
                          (para->reserved_sectors >= 2) ? para->reserved_sectors - 1 : 0;
        else {
            if (para->backup_boot == 1)
                die("Backup boot sector must be after sector 1");
            else if (para->backup_boot >= para->reserved_sectors)
                die("Backup boot sector must be a reserved sector");
        }
        if (para->verbose >= 2)
            LOG_INFO("Using sector %d as backup boot sector (0 = none)\n",
                   para->backup_boot);
        para->bs.fat32.backup_boot = htole16(para->backup_boot);
        memset(&para->bs.fat32.reserved2, 0, sizeof(para->bs.fat32.reserved2));
    }

    if (para->atari_format) {
        /* Just some consistency checks */
        if (para->num_sectors >= GEMDOS_MAX_SECTORS)
            die("GEMDOS can't handle more than 65531 sectors");
        else if (para->num_sectors >= OLDGEMDOS_MAX_SECTORS)
            LOG_INFO("Warning: More than 32765 sector need TOS 1.04 "
                   "or higher.\n");
    }
    if (para->num_sectors >= 65536) {
        para->bs.sectors[0] = (char)0;
        para->bs.sectors[1] = (char)0;
        para->bs.total_sect = htole32(para->num_sectors);
    } else {
        para->bs.sectors[0] = (char)(para->num_sectors & 0x00ff);
        para->bs.sectors[1] = (char)((para->num_sectors & 0xff00) >> 8);
        if (!para->atari_format)
            para->bs.total_sect = htole32(0);
    }

    if (!para->atari_format)
        vi->ext_boot_sign = MSDOS_EXT_SIGN;

    if (!cluster_count) {
        if (para->sectors_per_cluster)    /* If yes, die if we'd spec'd sectors per cluster */
            die("Too many clusters for filesystem - try more sectors per cluster");
        else
            die("Attempting to create a too large filesystem");
    }
    para->fat_entries = cluster_count + 2;

    /* The two following vars are in hard sectors, i.e. 512 byte sectors! */
    para->start_data_sector = (para->reserved_sectors + para->nr_fats * fat_length +
                         cdiv(para->root_dir_entries * 32, para->sector_size)) *
                        (para->sector_size / HARD_SECTOR_SIZE);
    para->start_data_block = (para->start_data_sector + SECTORS_PER_BLOCK - 1) /
                       SECTORS_PER_BLOCK;

    if (para->blocks < para->start_data_block + 32) /* Arbitrary undersize filesystem! */
        die("Too few blocks for viable filesystem");

    if (para->verbose) {
        LOG_INFO("%s has %d head%s and %d sector%s per track,\n",
               para->device_name, le16toh(para->bs.heads),
               (le16toh(para->bs.heads) != 1) ? "s" : "", le16toh(para->bs.secs_track),
               (le16toh(para->bs.secs_track) != 1) ? "s" : "");
        LOG_INFO("hidden sectors 0x%04x;\n",  para->hidden_sectors);
        LOG_INFO("logical sector size is %d,\n", para->sector_size);
        LOG_INFO("using 0x%02x media descriptor, with %d sectors;\n",
               (int)(para->bs.media), para->num_sectors);
        LOG_INFO("drive number 0x%02x;\n", (int) (vi->drive_number));
        LOG_INFO("filesystem has %d %d-bit FAT%s and %d sector%s per cluster.\n",
               (int)(para->bs.fats), para->size_fat, (para->bs.fats != 1) ? "s" : "",
               (int)(para->bs.cluster_size), (para->bs.cluster_size != 1) ? "s" : "");
        LOG_INFO("FAT size is %d sector%s, and provides %d cluster%s.\n",
               fat_length, (fat_length != 1) ? "s" : "",
               cluster_count, (cluster_count != 1) ? "s" : "");
        LOG_INFO("There %s %u reserved sector%s.\n",
               (para->reserved_sectors != 1) ? "are" : "is",
               para->reserved_sectors, (para->reserved_sectors != 1) ? "s" : "");

        if (para->size_fat != 32) {
            unsigned root_dir_entries =
                para->bs.dir_entries[0] + ((para->bs.dir_entries[1]) * 256);
            unsigned root_dir_sectors =
                cdiv(root_dir_entries * 32, para->sector_size);
            LOG_INFO("Root directory contains %u slots and uses %u sectors.\n",
                   root_dir_entries, root_dir_sectors);
        }
        LOG_INFO("Volume ID is %08lx, ", para->volume_id &
               (para->atari_format ? 0x00ffffff : 0xffffffff));
        if (strcmp(para->volume_name, NO_NAME))
            LOG_INFO("volume label %s.\n", para->volume_name);
        else
            LOG_INFO("no volume label.\n");
    }

    /* Make the file allocation tables! */

    if (para->malloc_entire_fat)
        para->alloced_fat_length = fat_length;
    else
        para->alloced_fat_length = 1;

    if ((para->fat =
             (unsigned char *)malloc(para->alloced_fat_length * para->sector_size)) == NULL)
        die("unable to allocate space for FAT image in memory");

    memset(para->fat, 0, para->alloced_fat_length * para->sector_size);

    mark_FAT_cluster(para, 0, 0xffffffff);    /* Initial fat entries */
    mark_FAT_cluster(para, 1, 0xffffffff);
    para->fat[0] = (unsigned char)para->bs.media;   /* Put media type in first byte! */
    if (para->size_fat == 32) {
        /* Mark cluster 2 as EOF (used for root dir) */
        mark_FAT_cluster(para, 2, FAT_EOF);
    }

    /* Make the root directory entries */

    para->size_root_dir = (para->size_fat == 32) ?
                    para->bs.cluster_size * para->sector_size :
                    (((int)para->bs.dir_entries[1] * 256 + (int)para->bs.dir_entries[0]) *
                     sizeof(struct msdos_dir_entry));
    if ((para->root_dir = (struct msdos_dir_entry *)malloc(para->size_root_dir)) == NULL) {
        free(para->fat);      /* Tidy up before we die! */
        die("unable to allocate space for root directory in memory");
    }

    memset(para->root_dir, 0, para->size_root_dir);
    if (memcmp(para->volume_name, NO_NAME, MSDOS_NAME)) {
        struct msdos_dir_entry *de = &para->root_dir[0];
        memcpy(de->name, para->volume_name, MSDOS_NAME);
        de->attr = ATTR_VOLUME;
        if (!para->invariant)
            ctime = localtime(&para->create_time);
        else
            ctime = gmtime(&para->create_time);
        de->time = htole16((unsigned short)((ctime->tm_sec >> 1) +
                                            (ctime->tm_min << 5) +
                                            (ctime->tm_hour << 11)));
        de->date =
            htole16((unsigned short)(ctime->tm_mday +
                                     ((ctime->tm_mon + 1) << 5) +
                                     ((ctime->tm_year - 80) << 9)));
        de->ctime_cs = 0;
        de->ctime = de->time;
        de->cdate = de->date;
        de->adate = de->date;
        de->starthi = htole16(0);
        de->start = htole16(0);
        de->size = htole32(0);
    }

    if (para->size_fat == 32) {
        /* For FAT32, create an info sector */
        struct fat32_fsinfo *info;

        if (!(para->info_sector = malloc(para->sector_size)))
            die("Out of memory");
        memset(para->info_sector, 0, para->sector_size);
        /* fsinfo structure is at offset 0x1e0 in info sector by observation */
        info = (struct fat32_fsinfo *)(para->info_sector + 0x1e0);

        /* Info sector magic */
        para->info_sector[0] = 'R';
        para->info_sector[1] = 'R';
        para->info_sector[2] = 'a';
        para->info_sector[3] = 'A';

        /* Magic for fsinfo structure */
        info->signature = htole32(0x61417272);
        /* We've allocated cluster 2 for the root dir. */
        info->free_clusters = htole32(cluster_count - 1);
        info->next_cluster = htole32(2);

        /* Info sector also must have boot sign */
        *(uint16_t *) (para->info_sector + 0x1fe) = htole16(BOOT_SIGN);
    }

    if (!(para->blank_sector = malloc(para->sector_size)))
        die("Out of memory");
    memset(para->blank_sector, 0, para->sector_size);
}

/* Write the new filesystem's data tables to wherever they're going to end up! */

#define error(str)              \
  do {                      \
    free (para->fat);                 \
    if (para->info_sector) free (para->info_sector);    \
    free (para->root_dir);                \
    die (str);                  \
  } while(0)

#define seekto(pos,errstr)                      \
  do {                                  \
    off_t __pos = (pos);                        \
    if (lseek (para->dev, __pos, SEEK_SET) != __pos)              \
    error ("seek to " errstr " failed whilst writing tables");  \
  } while(0)

#define writebuf(buf,size,errstr)           \
  do {                          \
    int __size = (size);                \
    if (write (para->dev, buf, __size) != __size)     \
    error ("failed whilst writing " errstr);    \
  } while(0)

static void write_tables(struct mkfs_para *para, int *prog)
{
    int x;
    int fat_length;

    fat_length = (para->size_fat == 32) ?
                 le32toh(para->bs.fat32.fat32_length) : le16toh(para->bs.fat_length);

    seekto(0, "start of device");
    /* clear all reserved sectors */
    for (x = 0; x < para->reserved_sectors; ++x)
        writebuf(para->blank_sector, para->sector_size, "reserved sector");
    /* seek back to sector 0 and write the boot sector */
    seekto(0, "boot sector");
    writebuf((char *)&para->bs, sizeof(struct msdos_boot_sector), "boot sector");
    /* on FAT32, write the info sector and backup boot sector */
    if (para->size_fat == 32) {
        seekto(le16toh(para->bs.fat32.info_sector) * para->sector_size, "info sector");
        writebuf(para->info_sector, 512, "info sector");
        if (para->backup_boot != 0) {
            seekto(para->backup_boot * para->sector_size, "backup boot sector");
            writebuf((char *)&para->bs, sizeof(struct msdos_boot_sector),
                     "backup boot sector");
        }
    }
    /* seek to start of FATS and write them all */
    seekto(para->reserved_sectors * para->sector_size, "first FAT");
    int total = para->nr_fats * (fat_length - para->alloced_fat_length);
    int cnt = 1;
    for (x = 1; x <= para->nr_fats; x++) {
        int y;
        int blank_fat_length = fat_length - para->alloced_fat_length;
        writebuf(para->fat, para->alloced_fat_length * para->sector_size, "FAT");
        for (y = 0; y < blank_fat_length; y++) {
            int tmp = 100 * cnt / total;
            if (tmp != *prog) {
                *prog = tmp;
                //LOG_INFO("%d\n", *prog);
            }
            cnt++;
            writebuf(para->blank_sector, para->sector_size, "FAT");
        }
    }

    /* Write the root directory directly after the last FAT. This is the root
     * dir area on FAT12/16, and the first cluster on FAT32. */
    writebuf((char *)para->root_dir, para->size_root_dir, "root directory");

    if (para->blank_sector)
        free(para->blank_sector);
    if (para->info_sector)
        free(para->info_sector);
    free(para->root_dir);     /* Free up the root directory space from setup_tables */
    free(para->fat);          /* Free up the fat table space reserved during setup_tables */
}

static void init(struct mkfs_para *para)
{
    memset(para, 0, sizeof(struct mkfs_para));
    para->device_name = NULL;
    para->atari_format = 0;
    para->verbose = 0;
    para->sector_size = 512;
    para->sector_size_set = 0;
    para->backup_boot = 0;
    para->reserved_sectors = 0;
    para->badblocks = 0;
    para->nr_fats = 2;
    para->size_fat = 0;
    para->size_fat_by_user = 0;
    para->dev = -1;
    para->ignore_full_disk = 0;
    para->currently_testing = 0;
    para->sectors_per_cluster = 0;
    para->root_dir_entries = 0;
    para->hidden_sectors = 0;
    para->hidden_sectors_by_user = 0;
    para->drive_number_option = 0;
    para->drive_number_by_user = 0;
    para->fat_media_byte = 0;
    para->malloc_entire_fat = FALSE;
    para->align_structures = TRUE;
    para->orphaned_sectors = 0;
    para->invariant = 0;
}

char *mkfs_vfat(char *device, int *prog)
{
    char *ret = NULL;
    int c;
    char *tmp;
    FILE *msgfile;
    struct device_info devinfo;
    int i = 0, pos, ch;
    int create = 0;
    uint64_t cblocks = 0;
    int blocks_specified = 0;
    struct timeval create_timeval;

    struct mkfs_para para;

    init(&para);

    para.device_name = device;

    gettimeofday(&create_timeval, NULL);
    para.create_time = create_timeval.tv_sec;
    para.volume_id = (uint32_t) ((create_timeval.tv_sec << 20) | create_timeval.tv_usec);    /* Default volume ID = creation time, fudged for more uniqueness */

    if (!create) {
        if (is_device_mounted(para.device_name)) {
            ret = g_strdup_printf("%s contains a mounted filesystem.", para.device_name);
            goto mkfs_vfat_err;
        }

        para.dev = open(para.device_name, O_EXCL | O_RDWR);   /* Is it a suitable device to build the FS on? */
        if (para.dev < 0) {
            ret = g_strdup_printf("unable to open %s: %s",
                    para.device_name, strerror(errno));
            goto mkfs_vfat_err;
        }
    } else {
        /* create the file */
        para.dev = open(para.device_name, O_EXCL | O_RDWR | O_CREAT, 0666);
        if (para.dev < 0) {
            ret = g_strdup_printf("unable to open %s: %s",
                    para.device_name, strerror(errno));
            goto mkfs_vfat_err;
        }
        /* expand to desired size */
        if (ftruncate(para.dev, para.blocks * BLOCK_SIZE)) {
            ret = g_strdup_printf("unable to resize");
            goto mkfs_vfat_err;
        }
    }

    if (get_device_info(para.dev, &devinfo) < 0) {
        ret = g_strdup_printf("error collecting information about");
        goto mkfs_vfat_err;
    }

    if (devinfo.size <= 0) {
        ret = g_strdup_printf("unable to discover size of");
        goto mkfs_vfat_err;
    }

    if (devinfo.sector_size > 0) {
        if (para.sector_size_set) {
            if (para.sector_size < devinfo.sector_size) {
                para.sector_size = devinfo.sector_size;
                LOG_ERROR("Warning: sector size was set to %d (minimal for this device)\n",
                        para.sector_size);
            }
        } else {
            para.sector_size = devinfo.sector_size;
            para.sector_size_set = 1;
        }
    }

    if (para.sector_size > 4096)
        LOG_ERROR("Warning: sector size %d > 4096 is non-standard, filesystem may not be usable\n",
                para.sector_size);

    cblocks = devinfo.size / BLOCK_SIZE;
    para.orphaned_sectors = (devinfo.size % BLOCK_SIZE) / para.sector_size;

    if (blocks_specified) {
        if (para.blocks != cblocks) {
            LOG_ERROR("Warning: block count mismatch: ");
            LOG_ERROR("found %llu but assuming %llu.\n",
                    (unsigned long long)cblocks, (unsigned long long)para.blocks);
        }
    } else {
        para.blocks = cblocks;
    }

    /*
     * Ignore any 'full' fixed disk devices, if -I is not given.
     */
    if (!para.ignore_full_disk && devinfo.type == TYPE_FIXED &&
        devinfo.partition == 0) {
        ret = g_strdup_printf("Device partition expected, not making filesystem on entire device '%s' (use -I to override)", para.device_name);
        goto mkfs_vfat_err;
    }

    if (!para.ignore_full_disk && devinfo.has_children > 0) {
        ret = g_strdup_printf("Partitions or virtual mappings on device '%s', not making filesystem (use -I to override)", para.device_name);
        goto mkfs_vfat_err;
    }

    establish_params(&para, &devinfo);
    /* Establish the media parameters */

    setup_tables(&para);     /* Establish the filesystem tables */

    write_tables(&para, prog);     /* Write the filesystem tables away! */

    close(para.dev);

    return 0;

mkfs_vfat_err:
    if (para.dev >= 0)
        close(para.dev);
    *prog = -1;

    return ret;
}
