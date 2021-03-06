#include <linux/bpf.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <stddef.h>
#include <stdbool.h>
#include "bpf_helpers.h"
#include "bpf_endian.h"

#define MAGIC 0x216C7174786A7A21ULL

struct packet_struct {
    __u64 magic;
    __u64 tag;
    __u64 data[16];//package data is double
};

static __always_inline __u32 double_to_u32(__u64 x){
    return ((x==(__u64)0)?(__u32)0:(__u32)(((x&(__u64)0x000FFFFFFFFFFFFF)|(__u64)0x0010000000000000)>>(((__u32)1065>(__u32)(x >> 52))?(__u32)1065-(__u32)(x >> 52):(__u32)(x >> 52)-(__u32)1065)));
}

SEC("xdp")
int process_packet(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
	void *data = (void *)(long)ctx->data;
    __u32 off = sizeof(struct ethhdr) + 28;
    struct packet_struct *raw;
    raw = data + off;
    if (raw + 1 > data_end) return XDP_PASS;
    if (raw->magic != MAGIC) {
        return XDP_PASS;
    } else {
        if (raw->tag != 0) return XDP_PASS;
        //for first 16 data (type __u32), calculate mean and var
        __u64 mean=(__u64)0;
        __u64 var=(__u64)0;
        __u32 dt;
        dt=double_to_u32(raw->data[0]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[1]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[2]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[3]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[4]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[5]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[6]);mean+=dt;var+=(__u64)dt*dt;
        dt=double_to_u32(raw->data[7]);mean+=dt;var+=(__u64)dt*dt;
        mean>>=3;var>>=3;var-=mean*mean;
        __u32 mean2=(__u32)mean;
        const __u32 e=1638;
        const __u64 e2=2684191;
        __u16 tot=(__u32)0;

        dt=double_to_u32(raw->data[8]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[9]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[10]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[11]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[12]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[13]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[14]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        dt=double_to_u32(raw->data[15]);tot+=( dt>=mean2 ? (dt-mean2>=e ? (__u16)1 : (__u16)0) : (mean2-dt>=e ? (__u16)1 : (__u16)0) );
        if (tot*e2 > (var<<3)){
            raw->tag = (__u64)1;// hua fen！
        }else{
            raw->tag = (__u64)2;// no hua fen
        }
    }
    return XDP_PASS;
}
    
