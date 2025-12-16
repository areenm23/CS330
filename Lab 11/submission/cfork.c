#include <fork.h>
#include <page.h>
#include <mmap.h>
#include <apic.h>

/* #################################################*/

static inline void invlpg(unsigned long addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}
/**
 * cfork system call implemenations
 */

long do_cfork(){
    u32 pid;
    struct exec_context *new_ctx = get_new_ctx();
    struct exec_context *ctx = get_current_ctx();
     /* Do not modify above lines
     * 
     * */   
     /*--------------------- Your code [start]---------------*/
     	 pid = new_ctx->pid;
         *new_ctx = *ctx;
		


		






     /*--------------------- Your code [end] ----------------*/
    
     /*
     * The remaining part must not be changed
     */
    copy_os_pts(ctx->pgd, new_ctx->pgd);
    do_file_fork(new_ctx);
    setup_child_context(new_ctx);
    reset_timer();

    return pid;
}


/* Cow fault handling, for the entire user address space
 * For address belonging to memory segments (i.e., stack, data) 
 * it is called when there is a CoW violation in these areas. 
 */

long handle_cow_fault(struct exec_context *current, u64 vaddr, int access_flags)
{

	

	


  long retval = -1;

  return retval;
}
