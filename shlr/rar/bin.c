#include <stdio.h>
#include <stdlib.h>
// crc32
#include "../../libr/hash/crc32.c"

#define eprintf(x,y...) fprintf(stderr,x,##y)

#pragma pack(1)
typedef struct rar_block_t {
	unsigned short crc;
	unsigned char type;
	unsigned short flags;
	unsigned short size;
	unsigned int add_size;
} __attribute((__packed__)) RarBlock;

typedef struct rar_block_archive_t {
	unsigned int pack_size;
	unsigned int unpack_size;
	unsigned char os; // [msdos,os2,w32,unix,osx,beos]
	unsigned int crc;
	unsigned int ftime; // datetime (dos format)
	unsigned char rarversion; // datetime
	unsigned char packmethod;
#if 0
	0×30 – storing
	0×31 – fastest compression
	0×32 – fast compression
	0×33 – normal compression
	0×34 – good compression
	0×35 – best compression
#endif
	unsigned short filenamesize;
	unsigned int file_attr;
	unsigned int high_pack_size;
	unsigned int high_unpack_size;
	char *filename;
	// unsigned long long salt;
	// ext_time: present if flags &0x1000) variable size
} __attribute((__packed__)) RarBlockArchive;
typedef struct {
	unsigned short    crc;
	unsigned char     type;
	unsigned short    flags;
	unsigned short    size;
} hdr_t;
struct filehdr {
	hdr_t       hdr;
	unsigned int    PackSize;
	unsigned int    UnpSize;
	unsigned char     HostOS;
	unsigned int    FileCRC;
	unsigned int    FileTime;
	unsigned char     UnpVer;
	unsigned char     Method;
	unsigned short    NameSize;
	union {
		unsigned int    FileAttr;
		unsigned int    SubFlags;
	};
	unsigned char     FileName[6];
};
#if 0
HIGH_PACK_SIZE  High 4 bytes of 64 bit value of compressed file size.
4 bytes         Optional value, presents only if bit 0×100 in HEAD_FLAGS
is set.

HIGH_UNP_SIZE   High 4 bytes of 64 bit value of uncompressed file size.
4 bytes         Optional value, presents only if bit 0×100 in HEAD_FLAGS
is set.

FILE_NAME       File name – string of NAME_SIZE bytes size

SALT            present if (HEAD_FLAGS & 0×400) != 0
8 bytes

EXT_TIME        present if (HEAD_FLAGS & 0×1000) != 0
variable size
#endif

static inline unsigned short shortswap(unsigned short s) {
	unsigned char c, *b = (unsigned char *)&s;
	c = b[1]; b[1] = b[0]; b[0] = c;
	return s;
}

static int israr(unsigned char *b, int sz) {
	return (sz>4 && b[0]==0x52 && b[1]==0x61);
}

static int parseBlock(unsigned char *b, int sz) {
#if 0
HEAD_CRC       2 bytes     CRC of total block or block part
HEAD_TYPE      1 byte      Block type
HEAD_FLAGS     2 bytes     Block flags
HEAD_SIZE      2 bytes     Block size
ADD_SIZE       4 bytes     Optional field – added block size
Field ADD_SIZE present only if (HEAD_FLAGS & 0×8000) != 0
#endif
struct filehdr *fhdr;
	RarBlockArchive *rba;
	RarBlock *rb = (RarBlock*) b;
	int i, n, blocksize; // = headsize
	// TODO: check size of rarblockstruct and sz
	//printf ("Flags : %x\n", rb->flags);
	printf ("Type: 0x%x\n", rb->type);
	//rb->flags = shortswap (rb->flags);
	printf ("   Flags: 0x%x\n", rb->flags);
	printf ("   Size: 0x%x\n", rb->size);
#if 0
	// flags
0x4000 : ignored block on old rars
		 0x8000 : add size
#endif
	 switch (rb->type) {
	 case 0x72:
		 //sequence: 0×52 0×61 0×72 0×21 0x1a 0×07 0×00
		 eprintf ("   + marker block\n");
		 break;
	 case 0x73:
		eprintf ("   + archive header (flags=%x)\n", rb->flags);
		rba = b+7;
		for (i=0;i<rb->size&&i<sz; i++) 
			printf ("%02x ", b[7+i]);
		printf("\n");
		fhdr = b;
		printf ("~~~~~(0x%x)~~~~~\n",
				 fhdr->FileAttr);
printf ("SZ %x %x %x\n", fhdr->PackSize, fhdr->UnpSize , fhdr->FileCRC);
		 if (fhdr->FileAttr & 0x20000000) {
			 printf ("IS CODE!\n");
		 }
		 //rba->file_attr);
		 printf ("FILENAME (");
		{
			int i;
			char mark = b[rb->size+31];
			for (i=0; b[rb->size+32+i] != mark; i++) {
				printf ("%c", b[rb->size+32+i]);
			}
		}
		printf (")\n");
//printf ("FILENAME (%s)\n", b+rb->size +32);//rba->filename);
		if (rb->flags & 0x400) {
			printf ("~~~~bonus 8\n");
		}
		if (rb->flags & 0x200) {
			printf ("utf8\n");
		}
		//sequence: 0×52 0×61 0×72 0×21 0x1a 0×07 0×00
#if 0
HEAD_FLAGS      Bit flags:
2 bytes
0×0001  – Volume attribute (archive volume)
0×0002  – Archive comment present
RAR 3.x uses the separate comment block
and does not set this flag.

0×0004  – Archive lock attribute
0×0008  – Solid attribute (solid archive)
0×0010  – New volume naming scheme (‘volname.partN.rar’)
0×0020  – Authenticity information present
RAR 3.x does not set this flag.

0×0040  – Recovery record present
0×0080  – Block headers are encrypted
0×0100  – First volume (set only by RAR 3.0 and later)
#endif
		break;
	case 0x74:
// File header full size including file name and comments
		eprintf ("   + file header\n");
for(i=0;i<32;i++) {
printf ("%02x ", b[i+38]);
}
printf ("\n");
return 32;
		break;
	case 0x7a:
		eprintf ("   + old block is old\n");
		break;
	default:
		eprintf ("   + Unknown block type 0x%x\n", rb->type);
		break;
	}
#if 0
HEAD_TYPE=0×72          marker block
HEAD_TYPE=0×73          archive header
HEAD_TYPE=0×74          file header
HEAD_TYPE=0×75          old style comment header
HEAD_TYPE=0×76          old style authenticity information
HEAD_TYPE=0×77          old style subblock
HEAD_TYPE=0×78          old style recovery record
HEAD_TYPE=0×79          old style authenticity information
HEAD_TYPE=0x7a          subblock
#endif
	if (rb->flags & 0x8000) {
		n = rb->size + rb->add_size;
	} else {
		n = rb->size;
	}
	printf ("   + SIZE: %d\n", n);
	return n;
}

static int parse_rar(unsigned char *b, int sz) {
	int idx = 0;
	if (!israr (b, sz)) {
		eprintf ("File is not rar\n");
		return 0;
	}
	while (idx<sz) {
		idx += parseBlock (b+idx, sz-idx);
	}
}

int main() {
	const char *rarfile = "helloworld.rar";
	FILE *fd = fopen (rarfile, "r");
	unsigned char buf[4096];
	if (fd) {
		int size = fread (buf, 1, sizeof (buf), fd);
		parse_rar (buf, size);
		fclose (fd);
	} else eprintf ("Cannot open rarfile\n");
	return 0;
}
