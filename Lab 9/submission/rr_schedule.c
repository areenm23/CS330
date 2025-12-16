#include<context.h>
#include<page.h>
#include<memory.h>
#include<lib.h>





//////////////////////  Q1: RR Scheduling   ///////////////////////////////////////
//args:
//      ctx: new exec_context to be added in the linked list
void rr_add_context(struct exec_context *ctx)
{
      /*TODO*/
	
//	printk("context added: %d\n", ctx->pid);
	if(rr_list_head == NULL) {
		rr_list_head = ctx;
		ctx->next = NULL;
		return;
	}

	struct exec_context* temp = rr_list_head;

	while(temp->next != NULL) temp = temp->next;

	temp->next = ctx;
	ctx->next = NULL;

      	 return;
}

//args:
//      ctx: exec_context to be removed from the linked list
void rr_remove_context(struct exec_context *ctx)
{
      /*TODO*/

//	printk("context removed: %d\n", ctx->pid);
	u32 curr_pid = ctx->pid;
	struct exec_context* temp = rr_list_head;
	struct exec_context* prev = NULL;
	while(temp->pid != curr_pid) {
		prev = temp;	
		temp = temp->next;
	}

	if(prev == NULL) rr_list_head = temp->next;
	prev->next = temp->next;
	
	

      return;
}

//args:
//      ctx: exec_context corresponding the currently running process
struct exec_context *rr_pick_next_context(struct exec_context *ctx)
{
    /*TODO*/

//	 printk("CPU has Process %d\n", ctx->pid);

	if(rr_list_head == NULL)
     			return get_ctx_by_pid(0);


	if(rr_list_head->next == NULL || ctx->pid == 0) return rr_list_head;

	struct exec_context* temp = (ctx->next) ? ctx->next : rr_list_head;

	while(temp->state == RUNNING) {
		if(temp->next == NULL) 
			temp = rr_list_head;

		else temp = temp->next;
	}

//	printk("Process %d is scheduled\n", temp->pid);

	return temp;

}

//////////////////////  Q2: Get the PAGE TABLE details for given address   ///////////////////////////////////////


//args:
//      ctx: exec_context corresponding the currently running process
//      addr: address for which the PAGE TABLE details are to be printed

int do_walk_pt(struct exec_context *ctx, unsigned long addr)
{
    u64 *vaddr_base = (u64 *)osmap(ctx->pgd);
    /*TODO*/
	

            u64 msk = 0xFFFFFFFF;
	
	    u64 pgd_offset = (addr & PGD_SHIFT) >> PGD_SHIFT;
	    u64 pud_offset = (addr & PUD_MASK) >> PUD_SHIFT;
 	    u64 pmd_offset = (addr & PMD_MASK) >> PMD_SHIFT;
 	    u64 pte_offset = (addr & PTE_MASK) >> PTE_SHIFT;
	    u64 pf_offset = (addr & 0xFFF);		    

		
	    u64 *vir_pud_base, *vir_pmd_base, *vir_pte_base;
	    int pud_valid, pmd_valid, pte_valid;
		
	// PGD
		

	u64 *pgd_addr = (vaddr_base + pgd_offset);		

       u64 pud_flags = (u64)(*pgd_addr) & 0xFFF;
        u64 pud_base = ((*pgd_addr >> 12) & msk) ;

	if(pud_flags % 2) {
		printk("L1-entry addr: %x, ", pgd_addr);
		printk("L1-entry contents: %x, ", *pgd_addr);
		printk("PFN: %x, ", pud_base);
		printk("Flags: %x\n", pud_flags);

	}


	 vir_pud_base = (u64 *)osmap(pud_base);

	


	// PUD
	

	u64* pud_addr = (vir_pud_base + pud_offset);
	
	u64 pmd_flags = (*pud_addr) & 0xFFF;
        u64 pmd_base = ((*pud_addr >> 12) & msk);

	if(pmd_flags % 2) {

        	printk("L2-entry addr: %x, ", pud_addr);
        	printk("L2-entry contents: %x, ", *pud_addr);
        	printk("PFN: %x, ", pmd_base);
        	printk("Flags: %x\n", pmd_flags);

	} else {

		printk("No L1 entry\n");

	}
	 vir_pmd_base = (u64 *)osmap(pmd_base);
	

	

	// PMD
	
	
	u64* pmd_addr = (vir_pmd_base + pmd_offset);

	 u64 pte_flags = (*pmd_addr) & 0xFFF;

        u64 pte_base = ((*pmd_addr >> 12) & msk) ;

	if(pte_flags % 2) {
        	printk("L3-entry addr: %x, ", pmd_addr);
        	printk("L3-entry contents: %x, ", *pmd_addr);
        	printk("PFN: %x, ", pte_base);
        	printk("Flags: %x\n", pte_flags);

	} else {

		printk("No L3 entry\n");

	}

         vir_pte_base  = (u64 *)osmap(pte_base);
	
	pte_valid = pte_flags % 2;

	
	// PTE 
	
	u64* pte_addr = (vir_pte_base + pte_offset);

	u64 pf_flags = (*pte_addr) & 0xFFF;

        u64 pf_base = ((*pte_addr >> 12) & msk) ;


	if(pf_flags % 2) {
       	 	printk("L4-entry addr: %x, ", pte_addr);
        	printk("L4-entry contents: %x, ", *pte_addr);
        	printk("PFN: %x, ", pf_base);
        	printk("Flags: %x\n", pf_flags);

	} else {

		printk("No L4 entry\n");

	}


    return 0;
}

