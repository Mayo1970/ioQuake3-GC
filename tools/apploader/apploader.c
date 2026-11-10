typedef unsigned char u8;
typedef unsigned int u32;

typedef void (*report_fn)(const char *fmt, ...);
typedef void (*init_fn)(report_fn report);
typedef int (*main_fn)(void **dst, u32 *size, u32 *offset);
typedef void *(*close_fn)(void);

extern u8 __bss_start[];
extern u8 __bss_end[];

static report_fn g_report;
static u32 g_state;
static u32 g_dol_offset;
static u32 g_section;

static u8 g_boot[32] __attribute__((aligned(32)));
static u8 g_header[0x100] __attribute__((aligned(32)));

static u32 read_be32(const u8 *p)
{
	return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}

/* Disc DMA bypasses the cache; invalidate before the CPU reads the buffer. */
static void dcache_invalidate(void *p, u32 len)
{
	u32 addr = (u32)p & ~31u;
	u32 end = ((u32)p + len + 31u) & ~31u;

	while (addr < end) {
		__asm__ volatile("dcbi 0, %0" : : "r"(addr) : "memory");
		addr += 32;
	}
}

static void dcache_flush(void *p, u32 len)
{
	u32 addr = (u32)p & ~31u;
	u32 end = ((u32)p + len + 31u) & ~31u;

	while (addr < end) {
		__asm__ volatile("dcbf 0, %0" : : "r"(addr) : "memory");
		addr += 32;
	}
}

static void apploader_init(report_fn report)
{
	g_report = report;
	g_report("ioQuake3-GC apploader\n");
}

static int apploader_main(void **dst, u32 *size, u32 *offset)
{
	if (g_state == 0) {
		*dst = g_boot;
		*size = 32;
		*offset = 0x420;
		g_state = 1;
		return 1;
	}

	if (g_state == 1) {
		dcache_invalidate(g_boot, sizeof(g_boot));
		g_dol_offset = read_be32(g_boot);
		*dst = g_header;
		*size = 0x100;
		*offset = g_dol_offset;
		g_state = 2;
		return 1;
	}

	if (g_state == 2) {
		dcache_invalidate(g_header, sizeof(g_header));
		g_state = 3;
		g_section = 0;
	}

	while (g_section < 18) {
		u32 index = g_section++;
		u32 sec_size;
		u32 sec_addr;
		u32 sec_off;

		if (index < 7) {
			sec_off = read_be32(g_header + 0x00 + index * 4);
			sec_addr = read_be32(g_header + 0x48 + index * 4);
			sec_size = read_be32(g_header + 0x90 + index * 4);
		} else {
			u32 i = index - 7;
			sec_off = read_be32(g_header + 0x1C + i * 4);
			sec_addr = read_be32(g_header + 0x64 + i * 4);
			sec_size = read_be32(g_header + 0xAC + i * 4);
		}

		if (sec_size == 0)
			continue;

		*dst = (void *)sec_addr;
		*size = (sec_size + 31u) & ~31u;
		*offset = g_dol_offset + sec_off;
		return 1;
	}

	return 0;
}

static void *apploader_close(void)
{
	u32 bss_addr = read_be32(g_header + 0xD8);
	u32 bss_size = read_be32(g_header + 0xDC);
	u32 entry = read_be32(g_header + 0xE0);
	u8 *p = (u8 *)bss_addr;
	u32 i;

	for (i = 0; i < bss_size; i++)
		p[i] = 0;
	dcache_flush((void *)bss_addr, bss_size);

	return (void *)entry;
}

void entry(init_fn *init, main_fn *main, close_fn *close) __attribute__((section(".text.entry")));

void entry(init_fn *init, main_fn *main, close_fn *close)
{
	u8 *p = __bss_start;

	/* The IPL does not zero the apploader BSS. */
	while (p < __bss_end)
		*p++ = 0;

	*init = apploader_init;
	*main = apploader_main;
	*close = apploader_close;
}
