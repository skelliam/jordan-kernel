/*
 * linux/arch/sh/mm/init.c
 *
 *  Copyright (C) 1999  Niibe Yutaka
 *  Copyright (C) 2002 - 2007  Paul Mundt
 *
 *  Based on linux/arch/i386/mm/init.c:
 *   Copyright (C) 1995  Linus Torvalds
 */
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/proc_fs.h>
#include <linux/pagemap.h>
#include <linux/percpu.h>
#include <linux/io.h>
<<<<<<< HEAD
=======
#include <linux/memblock.h>
#include <linux/dma-mapping.h>
>>>>>>> 95f72d1... lmb: rename to memblock
#include <asm/mmu_context.h>
#include <asm/tlb.h>
#include <asm/cacheflush.h>
#include <asm/sections.h>
#include <asm/cache.h>

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);
pgd_t swapper_pg_dir[PTRS_PER_PGD];

<<<<<<< HEAD
#ifdef CONFIG_SUPERH32
/*
 * Handle trivial transitions between cached and uncached
 * segments, making use of the 1:1 mapping relationship in
 * 512MB lowmem.
 *
 * This is the offset of the uncached section from its cached alias.
 * Default value only valid in 29 bit mode, in 32bit mode will be
 * overridden in pmb_init.
 */
unsigned long cached_to_uncached = P2SEG - P1SEG;
#endif
=======
void __init generic_mem_init(void)
{
	memblock_add(__MEMORY_START, __MEMORY_SIZE);
}

void __init __weak plat_mem_setup(void)
{
	/* Nothing to see here, move along. */
}
>>>>>>> 95f72d1... lmb: rename to memblock

#ifdef CONFIG_MMU
static void set_pte_phys(unsigned long addr, unsigned long phys, pgprot_t prot)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = pgd_offset_k(addr);
	if (pgd_none(*pgd)) {
		pgd_ERROR(*pgd);
		return;
	}

	pud = pud_alloc(NULL, pgd, addr);
	if (unlikely(!pud)) {
		pud_ERROR(*pud);
		return;
	}

	pmd = pmd_alloc(NULL, pud, addr);
	if (unlikely(!pmd)) {
		pmd_ERROR(*pmd);
		return;
	}

	pte = pte_offset_kernel(pmd, addr);
	if (!pte_none(*pte)) {
		pte_ERROR(*pte);
		return;
	}

	set_pte(pte, pfn_pte(phys >> PAGE_SHIFT, prot));
	local_flush_tlb_one(get_asid(), addr);
}

/*
 * As a performance optimization, other platforms preserve the fixmap mapping
 * across a context switch, we don't presently do this, but this could be done
 * in a similar fashion as to the wired TLB interface that sh64 uses (by way
 * of the memory mapped UTLB configuration) -- this unfortunately forces us to
 * give up a TLB entry for each mapping we want to preserve. While this may be
 * viable for a small number of fixmaps, it's not particularly useful for
 * everything and needs to be carefully evaluated. (ie, we may want this for
 * the vsyscall page).
 *
 * XXX: Perhaps add a _PAGE_WIRED flag or something similar that we can pass
 * in at __set_fixmap() time to determine the appropriate behavior to follow.
 *
 *					 -- PFM.
 */
void __set_fixmap(enum fixed_addresses idx, unsigned long phys, pgprot_t prot)
{
	unsigned long address = __fix_to_virt(idx);

	if (idx >= __end_of_fixed_addresses) {
		BUG();
		return;
	}

	set_pte_phys(address, phys, prot);
}

void __init page_table_range_init(unsigned long start, unsigned long end,
					 pgd_t *pgd_base)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	int i, j, k;
	unsigned long vaddr;

	vaddr = start;
	i = __pgd_offset(vaddr);
	j = __pud_offset(vaddr);
	k = __pmd_offset(vaddr);
	pgd = pgd_base + i;

	for ( ; (i < PTRS_PER_PGD) && (vaddr != end); pgd++, i++) {
		pud = (pud_t *)pgd;
		for ( ; (j < PTRS_PER_PUD) && (vaddr != end); pud++, j++) {
			pmd = (pmd_t *)pud;
			for (; (k < PTRS_PER_PMD) && (vaddr != end); pmd++, k++) {
				if (pmd_none(*pmd)) {
					pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
					pmd_populate_kernel(&init_mm, pmd, pte);
					BUG_ON(pte != pte_offset_kernel(pmd, 0));
				}
				vaddr += PMD_SIZE;
			}
			k = 0;
		}
		j = 0;
	}
}
#endif	/* CONFIG_MMU */

<<<<<<< HEAD
/*
 * paging_init() sets up the page tables
 */
=======
void __init allocate_pgdat(unsigned int nid)
{
	unsigned long start_pfn, end_pfn;
#ifdef CONFIG_NEED_MULTIPLE_NODES
	unsigned long phys;
#endif

	get_pfn_range_for_nid(nid, &start_pfn, &end_pfn);

#ifdef CONFIG_NEED_MULTIPLE_NODES
	phys = __memblock_alloc_base(sizeof(struct pglist_data),
				SMP_CACHE_BYTES, end_pfn << PAGE_SHIFT);
	/* Retry with all of system memory */
	if (!phys)
		phys = __memblock_alloc_base(sizeof(struct pglist_data),
					SMP_CACHE_BYTES, memblock_end_of_DRAM());
	if (!phys)
		panic("Can't allocate pgdat for node %d\n", nid);

	NODE_DATA(nid) = __va(phys);
	memset(NODE_DATA(nid), 0, sizeof(struct pglist_data));

	NODE_DATA(nid)->bdata = &bootmem_node_data[nid];
#endif

	NODE_DATA(nid)->node_start_pfn = start_pfn;
	NODE_DATA(nid)->node_spanned_pages = end_pfn - start_pfn;
}

static void __init bootmem_init_one_node(unsigned int nid)
{
	unsigned long total_pages, paddr;
	unsigned long end_pfn;
	struct pglist_data *p;
	int i;

	p = NODE_DATA(nid);

	/* Nothing to do.. */
	if (!p->node_spanned_pages)
		return;

	end_pfn = p->node_start_pfn + p->node_spanned_pages;

	total_pages = bootmem_bootmap_pages(p->node_spanned_pages);

	paddr = memblock_alloc(total_pages << PAGE_SHIFT, PAGE_SIZE);
	if (!paddr)
		panic("Can't allocate bootmap for nid[%d]\n", nid);

	init_bootmem_node(p, paddr >> PAGE_SHIFT, p->node_start_pfn, end_pfn);

	free_bootmem_with_active_regions(nid, end_pfn);

	/*
	 * XXX Handle initial reservations for the system memory node
	 * only for the moment, we'll refactor this later for handling
	 * reservations in other nodes.
	 */
	if (nid == 0) {
		/* Reserve the sections we're already using. */
		for (i = 0; i < memblock.reserved.cnt; i++)
			reserve_bootmem(memblock.reserved.region[i].base,
					memblock_size_bytes(&memblock.reserved, i),
					BOOTMEM_DEFAULT);
	}

	sparse_memory_present_with_active_regions(nid);
}

static void __init do_init_bootmem(void)
{
	int i;

	/* Add active regions with valid PFNs. */
	for (i = 0; i < memblock.memory.cnt; i++) {
		unsigned long start_pfn, end_pfn;
		start_pfn = memblock.memory.region[i].base >> PAGE_SHIFT;
		end_pfn = start_pfn + memblock_size_pages(&memblock.memory, i);
		__add_active_range(0, start_pfn, end_pfn);
	}

	/* All of system RAM sits in node 0 for the non-NUMA case */
	allocate_pgdat(0);
	node_set_online(0);

	plat_mem_setup();

	for_each_online_node(i)
		bootmem_init_one_node(i);

	sparse_init();
}

static void __init early_reserve_mem(void)
{
	unsigned long start_pfn;

	/*
	 * Partially used pages are not usable - thus
	 * we are rounding upwards:
	 */
	start_pfn = PFN_UP(__pa(_end));

	/*
	 * Reserve the kernel text and Reserve the bootmem bitmap. We do
	 * this in two steps (first step was init_bootmem()), because
	 * this catches the (definitely buggy) case of us accidentally
	 * initializing the bootmem allocator with an invalid RAM area.
	 */
	memblock_reserve(__MEMORY_START + CONFIG_ZERO_PAGE_OFFSET,
		    (PFN_PHYS(start_pfn) + PAGE_SIZE - 1) -
		    (__MEMORY_START + CONFIG_ZERO_PAGE_OFFSET));

	/*
	 * Reserve physical pages below CONFIG_ZERO_PAGE_OFFSET.
	 */
	if (CONFIG_ZERO_PAGE_OFFSET != 0)
		memblock_reserve(__MEMORY_START, CONFIG_ZERO_PAGE_OFFSET);

	/*
	 * Handle additional early reservations
	 */
	check_for_initrd();
	reserve_crashkernel();
}

>>>>>>> 95f72d1... lmb: rename to memblock
void __init paging_init(void)
{
	unsigned long max_zone_pfns[MAX_NR_ZONES];
	unsigned long vaddr, end;
	int nid;

<<<<<<< HEAD
=======
	memblock_init();

	sh_mv.mv_mem_init();

	early_reserve_mem();

	memblock_enforce_memory_limit(memory_limit);
	memblock_analyze();

	memblock_dump_all();

	/*
	 * Determine low and high memory ranges:
	 */
	max_low_pfn = max_pfn = memblock_end_of_DRAM() >> PAGE_SHIFT;
	min_low_pfn = __MEMORY_START >> PAGE_SHIFT;

	nodes_clear(node_online_map);

	memory_start = (unsigned long)__va(__MEMORY_START);
	memory_end = memory_start + (memory_limit ?: memblock_phys_mem_size());

	uncached_init();
	pmb_init();
	do_init_bootmem();
	ioremap_fixed_init();

>>>>>>> 95f72d1... lmb: rename to memblock
	/* We don't need to map the kernel through the TLB, as
	 * it is permanatly mapped using P1. So clear the
	 * entire pgd. */
	memset(swapper_pg_dir, 0, sizeof(swapper_pg_dir));

	/* Set an initial value for the MMU.TTB so we don't have to
	 * check for a null value. */
	set_TTB(swapper_pg_dir);

	/*
	 * Populate the relevant portions of swapper_pg_dir so that
	 * we can use the fixmap entries without calling kmalloc.
	 * pte's will be filled in by __set_fixmap().
	 */
	vaddr = __fix_to_virt(__end_of_fixed_addresses - 1) & PMD_MASK;
	end = (FIXADDR_TOP + PMD_SIZE - 1) & PMD_MASK;
	page_table_range_init(vaddr, end, swapper_pg_dir);

	kmap_coherent_init();

	memset(max_zone_pfns, 0, sizeof(max_zone_pfns));

	for_each_online_node(nid) {
		pg_data_t *pgdat = NODE_DATA(nid);
		unsigned long low, start_pfn;

		start_pfn = pgdat->bdata->node_min_pfn;
		low = pgdat->bdata->node_low_pfn;

		if (max_zone_pfns[ZONE_NORMAL] < low)
			max_zone_pfns[ZONE_NORMAL] = low;

		printk("Node %u: start_pfn = 0x%lx, low = 0x%lx\n",
		       nid, start_pfn, low);
	}

	free_area_init_nodes(max_zone_pfns);

	/* Set up the uncached fixmap */
	set_fixmap_nocache(FIX_UNCACHED, __pa(&__uncached_start));
}

void __init mem_init(void)
{
	int codesize, datasize, initsize;
	int nid;

	num_physpages = 0;
	high_memory = NULL;

	for_each_online_node(nid) {
		pg_data_t *pgdat = NODE_DATA(nid);
		unsigned long node_pages = 0;
		void *node_high_memory;

		num_physpages += pgdat->node_present_pages;

		if (pgdat->node_spanned_pages)
			node_pages = free_all_bootmem_node(pgdat);

		totalram_pages += node_pages;

		node_high_memory = (void *)__va((pgdat->node_start_pfn +
						 pgdat->node_spanned_pages) <<
						 PAGE_SHIFT);
		if (node_high_memory > high_memory)
			high_memory = node_high_memory;
	}

	/* Set this up early, so we can take care of the zero page */
	cpu_cache_init();

	/* clear the zero-page */
	memset(empty_zero_page, 0, PAGE_SIZE);
	__flush_wback_region(empty_zero_page, PAGE_SIZE);

	codesize =  (unsigned long) &_etext - (unsigned long) &_text;
	datasize =  (unsigned long) &_edata - (unsigned long) &_etext;
	initsize =  (unsigned long) &__init_end - (unsigned long) &__init_begin;

	printk(KERN_INFO "Memory: %luk/%luk available (%dk kernel code, "
	       "%dk data, %dk init)\n",
		nr_free_pages() << (PAGE_SHIFT-10),
		num_physpages << (PAGE_SHIFT-10),
		codesize >> 10,
		datasize >> 10,
		initsize >> 10);

	/* Initialize the vDSO */
	vsyscall_init();
}

void free_initmem(void)
{
	unsigned long addr;

	addr = (unsigned long)(&__init_begin);
	for (; addr < (unsigned long)(&__init_end); addr += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(addr));
		init_page_count(virt_to_page(addr));
		free_page(addr);
		totalram_pages++;
	}
	printk("Freeing unused kernel memory: %ldk freed\n",
	       ((unsigned long)&__init_end -
	        (unsigned long)&__init_begin) >> 10);
}

#ifdef CONFIG_BLK_DEV_INITRD
void free_initrd_mem(unsigned long start, unsigned long end)
{
	unsigned long p;
	for (p = start; p < end; p += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(p));
		init_page_count(virt_to_page(p));
		free_page(p);
		totalram_pages++;
	}
	printk("Freeing initrd memory: %ldk freed\n", (end - start) >> 10);
}
#endif

#if THREAD_SHIFT < PAGE_SHIFT
static struct kmem_cache *thread_info_cache;

struct thread_info *alloc_thread_info(struct task_struct *tsk)
{
	struct thread_info *ti;

	ti = kmem_cache_alloc(thread_info_cache, GFP_KERNEL);
	if (unlikely(ti == NULL))
		return NULL;
#ifdef CONFIG_DEBUG_STACK_USAGE
	memset(ti, 0, THREAD_SIZE);
#endif
	return ti;
}

void free_thread_info(struct thread_info *ti)
{
	kmem_cache_free(thread_info_cache, ti);
}

void thread_info_cache_init(void)
{
	thread_info_cache = kmem_cache_create("thread_info", THREAD_SIZE,
					      THREAD_SIZE, 0, NULL);
	BUG_ON(thread_info_cache == NULL);
}
#endif /* THREAD_SHIFT < PAGE_SHIFT */

#ifdef CONFIG_MEMORY_HOTPLUG
int arch_add_memory(int nid, u64 start, u64 size)
{
	pg_data_t *pgdat;
	unsigned long start_pfn = start >> PAGE_SHIFT;
	unsigned long nr_pages = size >> PAGE_SHIFT;
	int ret;

	pgdat = NODE_DATA(nid);

	/* We only have ZONE_NORMAL, so this is easy.. */
	ret = __add_pages(nid, pgdat->node_zones + ZONE_NORMAL,
				start_pfn, nr_pages);
	if (unlikely(ret))
		printk("%s: Failed, __add_pages() == %d\n", __func__, ret);

	return ret;
}
EXPORT_SYMBOL_GPL(arch_add_memory);

#ifdef CONFIG_NUMA
int memory_add_physaddr_to_nid(u64 addr)
{
	/* Node 0 for now.. */
	return 0;
}
EXPORT_SYMBOL_GPL(memory_add_physaddr_to_nid);
#endif
#endif /* CONFIG_MEMORY_HOTPLUG */
