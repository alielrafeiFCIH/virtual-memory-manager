#include <stdio.h>

typedef struct
{
    unsigned int Page_No;
    unsigned int Frame_No;
} TLB;

TLB lookup[16];
int Page_Table[256];
char buffer[256];
int Frame_N=0;
int memory[256][256];
int s_tlb=0;
float faultCount;
float hitCount;
int addressesCount;
int backlogFlag=0;
char flag_arg ;
int Search_TLB(int PN)
{

    for(int i=0; i<16; i++)
    {
        if(lookup[i].Page_No==PN)
        {
            hitCount++;
            return lookup[i].Frame_No;
        }
    }
    return -1;
}

int Search_Page_Table(int PN)
{

    if(Page_Table[PN]!=-1)
    {
        InsertIntoTLB(PN,Page_Table[PN]);
        return Page_Table[PN];
    }
    else
    {
        faultCount++;
        return -1;
    }
}


int Search_Back_log(int PN)
{

    FILE *binFile=fopen("BACKING_STORE.bin", "rb");
    fseek(binFile,PN*256,SEEK_SET);
    fread(buffer,1,256,binFile);


    Page_Table[PN]=Frame_N;
    InsertIntoTLB(PN,Frame_N);
    for(int i = 0; i < 256; i++)
    {
        memory[Frame_N][i]=buffer[i];
    }
    Frame_N++;
    return Frame_N-1;
}

void InsertIntoTLB(int pageno, int frameno)
{
    if(s_tlb!=15)
    {
        lookup[s_tlb].Page_No=  pageno;
        lookup[s_tlb].Frame_No= frameno;
        s_tlb++;
    }
    else
    {
       // if(flag_arg=='l'){
        //lifo(pageno,frameno);
        //}else {
        fifo(pageno,frameno);
        //}

    }

}

void fifo(int p, int f)
{

    for(int i=15; i>0; i--)
    {
        lookup[i].Page_No=  lookup[i-1].Page_No;
        lookup[i].Frame_No= lookup[i-1].Frame_No;
    }
    lookup[0].Page_No=  p;
    lookup[0].Frame_No=  f;
}

void lifo (int p,int f)
{
    for(int i=0; i<15; i++)
    {
        lookup[i].Page_No=  lookup[i+1].Page_No;
        lookup[i].Frame_No= lookup[i+1].Frame_No;
    }
    lookup[15].Page_No=  p;
    lookup[15].Frame_No=  f;


}

int main(void)
{
//    printf("%s\n",argv);
//    if(argv=='f'){
//    flag_arg = 'f';
//
//    }else if(argv=='l'){
//
//    flag_arg  = 'l';
//    printf("lifo working %s\n",flag_arg);
//    }else{
//    flag_arg = 'f';
//    }
    int i;
    for (i = 0 ; i<16; i++)
    {
        lookup[i].Frame_No=-1;
        lookup[i].Page_No=-1;
    }
    unsigned long int offsetMask =      255;     //00000000000000000000000011111111
    unsigned long int AddressMask =     65535;   //00000000000000001111111111111111
    for(int i=0; i<256; i++)
    {
        Page_Table[i]=-1;
    }
    char str[32];
    FILE *pFile=fopen("addresses.txt", "r");
    FILE *output = fopen("output_lifo.txt", "w");
    while (fgets(str, 10, pFile) != NULL)
    {
        addressesCount++;
        int address = atoi(str);
        address = address&AddressMask;
        unsigned int offset =    address&offsetMask;
        unsigned int pageno =    address>>8;
        unsigned int frameno;
        int tlb_flag =   Search_TLB(pageno);
        if(tlb_flag!=-1)
        {
            frameno =   tlb_flag;
        }
        else
        {
            int pt_flag = Search_Page_Table(pageno);

            if (pt_flag!=-1)
            {
                frameno =   pt_flag;
            }
            else
            {
                frameno= Search_Back_log(pageno);
            }
        }
        unsigned int phy =((256)*Page_Table[pageno])+offset;

        fprintf(output,"Logical Address: %d Physical Address: %d\n",address, phy);
    }
    fprintf(output,"Hits: %f\nHit Rate: %f\n",hitCount,(hitCount/addressesCount));
    fprintf(output,"Fault count: %f\nFault Rate: %f",faultCount,(faultCount/addressesCount));
    printf("Hits: %f\nHit Rate: %f\n",hitCount,(hitCount/addressesCount));
    printf("Fault count: %f\nFault Rate: %f\n",faultCount,(faultCount/addressesCount));


    return 0;
}
