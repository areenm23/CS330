#include<tb.h>
#include<lib.h>
#include<file.h>
#include<context.h>
#include<memory.h>

/*
 * *  Trace buffer implementation.
 *
 */

#define TRACE_BUFFER_SIZE 4096

///////////////////////////////////////////////////////////////////////////
////           TODO:     Trace buffer functionality                   /////
///////////////////////////////////////////////////////////////////////////

int is_valid_mem_range(struct exec_context* ctx, unsigned long buff, u32 count, int acflags, char type) {

	int r = acflags % 2;
	r = (type == 'R') ? r : 1;

	for(int i=0; i<4; i++) {

		if(i == MM_SEG_CODE || i == MM_SEG_RODATA && r) {


			if(ctx->mms[i].start <= buff && buff+count < ctx->mms[i].next_free) return 1;

		}
		
		if(i == MM_SEG_DATA) {
			        if(ctx->mms[i].start <= buff && buff+count < ctx->mms[i].next_free) return 1;

		}

		if( i == MM_SEG_STACK) {

			if(ctx->mms[i].start <= buff && buff+count < ctx->mms[i].end) return 1;

		}

	}

	return 0;


}


int is_valid_vma_range(struct exec_context* ctx, unsigned long buff, u32 count, int acflags, char type) {

	
	int r = acflags % 2;
        acflags >>= 1;
        int w = acflags % 2;	
	
	struct vm_area* temp = ctx->vm_area;
	while(temp) {
		int access = temp->access_flags;
		int r_vm = access % 2;
		access >>= 1;
		int w_vm = access % 2;

		switch(type) {
		
			case 'R': {
				 if(temp->vm_start <= buff && buff+count < temp->vm_end &&  w_vm == w) return 1;

			}

			case 'W': {

			         if(temp->vm_start <= buff && buff+count < temp->vm_end && r_vm == r) return 1;

			}
		}

		temp = temp->vm_next;
	}

	return 0;

}

// Check whether passed buffer is valid memory location for read.
static int tb_validate(unsigned long buff, u32 count, int acflags) 
{

	char type = (acflags == 2) ? 'R' : 'W';
	struct exec_context* ctx = get_current_ctx();
	int v_mem = is_valid_mem_range(ctx, buff, count, acflags, type) ;
	int v_vma = is_valid_vma_range(ctx, buff, count, acflags, type);
// 	printk("mem: %d vam: %d\n", v_mem, v_vma);
	return v_vma | v_mem;
	
      
}

static long tb_close(struct file *filep)
{
	os_page_free(USER_REG, filep->tb->tbuff);
	os_page_free(USER_REG, filep->tb->clr);
	os_free(filep->tb, sizeof(struct tb_info));
	os_free(filep->fops, sizeof(struct fileops));
	os_free(filep, sizeof(struct file));
        if(filep) return -EINVAL;
	return 0;
}

static int tb_read(struct file *filep, char *buff, u32 count)
{

	if(filep == NULL || buff == NULL) {
		printk("Error\n");
		    return -1;
	}

	

	if(!tb_validate((unsigned long)buff, count, 2)) return -EBADMEM;

        int cnt = 0;
        struct tb_info* tb = filep->tb;
        int readidx = tb->R;
   

        while(cnt < count && tb->clr[readidx] == 'g') {
                buff[cnt++] = tb->tbuff[readidx];
		tb->clr[readidx] = 'w';
                readidx = (readidx + 1) % TRACE_BUFFER_SIZE;
        }
	
	filep->offp = readidx;
	filep->tb->R = readidx;

	return cnt;

}

static int tb_write(struct file *filep, char *buff, u32 count)
{
	
	if(filep == NULL || buff  == NULL) {
                    printk("Error\n");
                    return -1;
        }

	if(!tb_validate((unsigned long) buff, count, 1)) return -EBADMEM;

        int cnt = 0;
        struct tb_info* tb = filep->tb;
        int writeidx = tb->W;      

        while(cnt < count && tb->clr[writeidx] == 'w') {
                tb->tbuff[writeidx] = buff[cnt++];
		tb->clr[writeidx] = 'g';
                writeidx = (writeidx + 1) % TRACE_BUFFER_SIZE;
        }
        
        filep->offp = writeidx;
        filep->tb->W = writeidx;

        return cnt;

}

int sys_create_tb(struct exec_context *ctx, int mode)
{
   	 int ret_fd = -1;


	for(int i=0; i<MAX_OPEN_FILES; i++) {

		if(ctx->files[i] != NULL) continue;		
		
		struct file* obj = (struct file *)os_alloc(sizeof(struct file));
		
		if(obj == NULL) {
			    printk("Error\n");
			    return ret_fd;

		}

		obj->mode = mode;
		obj->offp = 0;
		obj->type = TRACE_BUFFER;
		obj->ref_count = 0;

		struct tb_info* trace_info = (struct tb_info*) os_alloc(sizeof(struct tb_info));
				
		trace_info->R = 0;
		trace_info->W = 0;
		trace_info->tbuff = (char *) os_page_alloc(USER_REG);
		trace_info->clr = (char *) os_page_alloc(USER_REG);  	
		
		for(int i=0; i<TRACE_BUFFER_SIZE; i++) trace_info->clr[i] = 'w';

		obj->tb = trace_info;
		
		struct fileops* ops = (struct fileops*) os_alloc(sizeof(struct fileops));
		
		ops->read = tb_read;
		ops->write = tb_write;
		ops->lseek = NULL;
		ops->close = tb_close;
		obj->fops = ops;

		ctx->files[i] = obj;
		
		return i;


	}



     	
    return ret_fd;
}
