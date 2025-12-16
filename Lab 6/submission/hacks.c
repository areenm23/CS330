#include<hacks.h>
#include<sigexit.h>
#include<entry.h>
#include<lib.h>
#include<context.h>
#include<memory.h>

struct hack_config{
	             long cur_hack_config;
		     u64 usr_handler_addr;
};

//Used to store the current hack configuration
static struct hack_config hconfig = {-1, -1};


int valid_user_space_addr(struct exec_context *ctx, u64 ip) {
	
	struct mm_segment seg = ctx->mms[0];
	if(seg.start <= ip && seg.end > ip) {
		return 1;
	}
		
	return 0;

}



//system call handler to configure the hack sematics
//user space connection is already created, You need to
//store the hack semantics in the 'hconfig' structure (after validation)
//which will be used when division-by-zero occurs

long sys_config_hs(struct exec_context *ctx, long hack_mode, void *uhaddr)
{
		

	if(hack_mode !=  DIV_ZERO_OPER_CHANGE && hack_mode != DIV_ZERO_SKIP && hack_mode != DIV_ZERO_USH_EXIT && hack_mode != DIV_ZERO_SKIP_FUNC) {
		hconfig.cur_hack_config = 11;
		hconfig.usr_handler_addr = 0;
		return -EINVAL;
	}

	 hconfig.cur_hack_config = hack_mode;


	if(hack_mode == DIV_ZERO_USH_EXIT) {

		u64 ip = (u64) uhaddr;
		if(!valid_user_space_addr(ctx, ip)) {
			hconfig.usr_handler_addr = 0;
			return -EINVAL;
		} else {
			
			hconfig.usr_handler_addr = (u64) ip;	
			
		}
	}	



	return 0;
	
		
}


/*This is the handler for division by zero
 * 'regs' is a structure defined in include/context.h which
 * is already filled with the user execution state (by the asm handler)
 * and will be restored back when the function returns 
 *
 */
int do_div_by_zero(struct user_regs *regs) {	
	
	if(hconfig.cur_hack_config == 11) {		
		printk("Error...exiting\n");
		do_exit(0);
	}


	switch(hconfig.cur_hack_config) {
		case DIV_ZERO_OPER_CHANGE: {
			regs->rcx = 1;
		        regs->rax = 0;			
			return 0;				   
		}
		

		case DIV_ZERO_SKIP: {
			regs->entry_rip += (u64) 3;
			return 0;
		} 

		case DIV_ZERO_USH_EXIT: {

			if(!hconfig.usr_handler_addr) {
				printk("Error...exiting\n");
				do_exit(0);
			}

			regs->rdi = regs->entry_rip;
			regs->entry_rip = hconfig.usr_handler_addr;
			return 0;
		}
		
		case DIV_ZERO_SKIP_FUNC: {

			u64 saved_rbp = *((u64*)(regs->rbp + 0));
			u64 ret_add = *((u64*)(regs->rbp + 8));
			
			regs->rax = 1;
			regs->entry_rip = ret_add;
			regs->entry_rsp = regs->rbp + 16;	
			regs->rbp = saved_rbp;

			return 0;

		} 

		default: {			
			printk("Error...exiting\n");
                        do_exit(0);                       
		}
	}
	
    return 0;   	
}
