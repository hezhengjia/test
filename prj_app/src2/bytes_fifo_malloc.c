#include "bytes_fifo_malloc.h"



/*

#define MAX_ELEMENTS  100
STU_DATAS stu_datas[MAX_ELEMENTS];
STU_QUEUE_MALLOC StuQueuemalloc;
queue_malloc_init(&StuQueuemalloc,stu_datas,MAX_ELEMENTS);

*/

void queue_malloc_init(STU_QUEUE_MALLOC *StuQueuemalloc,STU_DATAS *stu_datas,uint16_t max_elements)
{

    StuQueuemalloc->dt_in=0;      //uint16_t
    StuQueuemalloc->dt_out=0;     //uint16_t
    StuQueuemalloc->max_element=max_elements;
    StuQueuemalloc->stu_datas=stu_datas;


}

/*
return :
true: queue is empty
false: queue have data
*/

bool queue_malloc_is_empty(STU_QUEUE_MALLOC *StuQueuemalloc)
{
    if(StuQueuemalloc->dt_in==StuQueuemalloc->dt_out)
        return true;
    else
        return false;
}


/*
return :
true: queue have space
false: queue full

*/

bool queue_malloc_in(STU_QUEUE_MALLOC *StuQueuemalloc,uint8_t *buf,uint16_t len)
{

    STU_DATAS stu_datas;
    bool ret=true;
    if(((StuQueuemalloc->dt_in+1)%StuQueuemalloc->max_element)==StuQueuemalloc->dt_out)//queue is full
    {
        ret=false;
    }

    stu_datas.len=len;
    stu_datas.pdata=QUEUE_MALLOC(stu_datas.len);
    memcpy(stu_datas.pdata,buf,len);



    StuQueuemalloc->stu_datas[StuQueuemalloc->dt_in] = stu_datas;
    StuQueuemalloc->dt_in++;
    StuQueuemalloc->dt_in%=StuQueuemalloc->max_element;
    return ret;
}

/*

len不为0，返回数据的指针
否则返回0

*/

uint8_t* queue_malloc_out(STU_QUEUE_MALLOC *StuQueuemalloc,uint16_t* len)
{

    if(StuQueuemalloc->dt_in == StuQueuemalloc->dt_out)
    {
        *len=0;
        return 0;
    }
    else
    {
        *len=StuQueuemalloc->stu_datas[StuQueuemalloc->dt_out].len;
        return StuQueuemalloc->stu_datas[StuQueuemalloc->dt_out].pdata;
    }
}

/*
para:   cnt -->1 means deletes n frams

*/
void queue_malloc_delete(STU_QUEUE_MALLOC *StuQueuemalloc,uint16_t cnt)
{
    uint16_t i,length;

    if(cnt == 0) return;

    length=cnt;

    for(i=0; i<length; i++)
    {

        if(StuQueuemalloc->dt_in == StuQueuemalloc->dt_out)
        {
            return;
        }
        QUEUE_FREE(StuQueuemalloc->stu_datas[StuQueuemalloc->dt_out].pdata);
        StuQueuemalloc->dt_out++;
        StuQueuemalloc->dt_out%=StuQueuemalloc->max_element;

    }
}





