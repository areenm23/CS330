#include<procinfo.h>
#include<lib.h>
#include<file.h>
#include<context.h>

static char* segment_names[MAX_MM_SEGS+1] = {"code", "rodata", "data", "stack", "invalid"}; 
static char* file_types[MAX_FILE_TYPE+1] = {"stdin", "stdout", "stderr", "reg", "pipe", "invalid"}; 

long get_process_info(struct exec_context *ctx, long cmd, char *ubuf, long len)
{
    long retval = -EINVAL;	
    /*
     * TODO your code goes in here
     * */


  switch(cmd) {

    case GET_PINFO_GEN: {
	
	if(len < sizeof(struct general_info)) {

//               printk( "return value: %d\n", retval);
                return retval;
        }

     	if(ubuf == NULL) {
//              printk( "return value: %d\n", retval);      
                return retval;
        }

        struct general_info info;
        info.pid = ctx->pid;
        info.ppid = ctx->ppid;
        strcpy(info.pname, ctx->name);
        info.pcb_addr = (unsigned long) ctx;

        memcpy(ubuf, &info, len);

//       printk( "return value: %d\n", 1);

         return (long) 1;



	}

	case GET_PINFO_FILE: {


		int total = 0;
		for(int i=0; i<MAX_OPEN_FILES; i++) {
			if(ctx->files[i] == NULL) continue;
			total++;
		}
	

		if(len < total*sizeof(struct file_info)) return retval;
	
		if(total == 0) return total;

		int offset = 0;

		for(int i=0; i<MAX_OPEN_FILES; i++) {

			struct file* fptr = ctx->files[i];
			if(!fptr) continue;

			struct file_info f;

			f.mode = fptr->mode;
			f.filepos = fptr->offp;
			f.ref_count = fptr->ref_count;
			strcpy(f.file_type, file_types[fptr->type]);
			memcpy(ubuf+offset, &f, sizeof(f));

			offset += (int) sizeof(f);

		}	

        	
		return total;

  	}



	case GET_PINFO_MSEG : {


    		int total = 0;
                for(int i=0; i<MAX_MM_SEGS; i++) {
             
                        total++;
                }


                if(len < total*sizeof(struct mem_segment_info)) return retval;

                if(total == 0) return total;

                int offset = 0;

                for(int i=0; i<MAX_MM_SEGS; i++) {

                        struct mm_segment memfptr = ctx->mms[i];
                       

                        struct mem_segment_info f;

                        f.start = memfptr.start;
                        f.end = memfptr.end;
                        f.next_free = memfptr.next_free;
                        
			int access = memfptr.access_flags;
			int idx = 0;
			while(idx < 3) {
				 int r = access%2;
				 if(r == 1) {

					if(idx == 0) f.perm[idx++] = 'R';
					else if(idx == 1) f.perm[idx++] = 'W';
					else if(idx == 2) f.perm[idx++] = 'X';

				 }
				 else f.perm[idx++] = '_';
				 access >>= 1;
			}

			f.perm[3] = '\0';
			
			strcpy(f.segname, segment_names[i]);

                        memcpy(ubuf+offset, &f, sizeof(f));

                        offset += (int) sizeof(f);

                }


                return total;

		


	}



	case GET_PINFO_VMA : {


		int total = 0;
                struct vm_area* temp = ctx->vm_area->vm_next;
		while(temp) {
			total++; temp = temp->vm_next;
		}


                if(len < total*sizeof(struct vm_area_info)) return retval;

                if(total == 0) return total;

                int offset = 0;
		
		temp = ctx->vm_area->vm_next;
                while(temp) {

                        
                        struct vm_area_info f;

                        f.start = temp->vm_start;
                        f.end = temp->vm_end;
                       

                        int access = temp->access_flags;
                        int idx = 0;
                        while(idx < 3) {
                                 int r = access%2;
                                 if(r == 1) {

                                        if(idx == 0) f.perm[idx++] = 'R';
                                        else if(idx == 1) f.perm[idx++] = 'W';
                                        else if(idx == 2) f.perm[idx++] = 'X';

                                 }
                                 else f.perm[idx++] = '_';
                                 access >>= 1;
                        }

                        f.perm[3] = '\0';

                        

                        memcpy(ubuf+offset, &f, sizeof(f));

                        offset += (int) sizeof(f);

			temp = temp->vm_next;

                }


                return total;	




	}
	 

  }


}
