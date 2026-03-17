// Copyright (C) Kumo inc. and its affiliates.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <tally/sigar/sigar.h>

typedef enum {
    SIGAR_FILETYPE_NOFILE = 0,     /**< no file type determined */
    SIGAR_FILETYPE_REG,            /**< a regular file */
    SIGAR_FILETYPE_DIR,            /**< a directory */
    SIGAR_FILETYPE_CHR,            /**< a character device */
    SIGAR_FILETYPE_BLK,            /**< a block device */
    SIGAR_FILETYPE_PIPE,           /**< a FIFO / pipe */
    SIGAR_FILETYPE_LNK,            /**< a symbolic link */
    SIGAR_FILETYPE_SOCK,           /**< a [unix domain] socket */
    SIGAR_FILETYPE_UNKFILE         /**< a file of some other unknown type */
} sigar_file_type_e; 

#define SIGAR_UREAD       0x0400 /**< Read by user */
#define SIGAR_UWRITE      0x0200 /**< Write by user */
#define SIGAR_UEXECUTE    0x0100 /**< Execute by user */

#define SIGAR_GREAD       0x0040 /**< Read by group */
#define SIGAR_GWRITE      0x0020 /**< Write by group */
#define SIGAR_GEXECUTE    0x0010 /**< Execute by group */

#define SIGAR_WREAD       0x0004 /**< Read by others */
#define SIGAR_WWRITE      0x0002 /**< Write by others */
#define SIGAR_WEXECUTE    0x0001 /**< Execute by others */

typedef struct {
    /** The access permissions of the file.  Mimics Unix access rights. */
    sigar_uint64_t permissions;
    sigar_file_type_e type;
    /** The user id that owns the file */
    sigar_uid_t uid;
    /** The group id that owns the file */
    sigar_gid_t gid;
    /** The inode of the file. */
    sigar_uint64_t inode;
    /** The id of the device the file is on. */
    sigar_uint64_t device;
    /** The number of hard links to the file. */
    sigar_uint64_t nlink;
    /** The size of the file */
    sigar_uint64_t size;
    /** The time the file was last accessed */
    sigar_uint64_t atime;
    /** The time the file was last modified */
    sigar_uint64_t mtime;
    /** The time the file was last changed */
    sigar_uint64_t ctime;
} sigar_file_attrs_t;

typedef struct {
    sigar_uint64_t total;
    sigar_uint64_t files;
    sigar_uint64_t subdirs;
    sigar_uint64_t symlinks;
    sigar_uint64_t chrdevs;
    sigar_uint64_t blkdevs;
    sigar_uint64_t sockets;
    sigar_uint64_t disk_usage;
} sigar_dir_stat_t;

typedef sigar_dir_stat_t sigar_dir_usage_t;

SIGAR_DECLARE(const char *)
sigar_file_attrs_type_string_get(sigar_file_type_e type);

SIGAR_DECLARE(int) sigar_file_attrs_get(sigar_t *sigar,
                                        const char *file,
                                        sigar_file_attrs_t *fileattrs);

SIGAR_DECLARE(int) sigar_link_attrs_get(sigar_t *sigar,
                                        const char *file,
                                        sigar_file_attrs_t *fileattrs);

SIGAR_DECLARE(int)sigar_file_attrs_mode_get(sigar_uint64_t permissions);

SIGAR_DECLARE(char *)
sigar_file_attrs_permissions_string_get(sigar_uint64_t permissions,
                                       char *str);

SIGAR_DECLARE(int) sigar_dir_stat_get(sigar_t *sigar,
                                      const char *dir,
                                      sigar_dir_stat_t *dirstats);

SIGAR_DECLARE(int) sigar_dir_usage_get(sigar_t *sigar,
                                       const char *dir,
                                       sigar_dir_usage_t *dirusage);
