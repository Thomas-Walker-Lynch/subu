// -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// eBPF: force sockets inside this cgroup to use a specific ifindex
// Hooks: cgroup/connect4 and cgroup/sendmsg4
// Logic: read ifindex from array map[0], then setsockopt(SO_BINDTOIFINDEX)

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __uint(max_entries, 1);
  __type(key, __u32);
  __type(value, __u32);   // ifindex
  __uint(pinning, LIBBPF_PIN_BY_NAME);
} force_ifindex_map SEC(".maps");

static __always_inline int force_bind(struct bpf_sock_addr *ctx)
{
  __u32 k = 0;
  __u32 *ifx = bpf_map_lookup_elem(&force_ifindex_map, &k);
  if (!ifx || !*ifx)
    return 1; // allow pass-through if not configured

  int val = (int)*ifx;
  // This sets sk->sk_bound_dev_if equivalently to userland SO_BINDTOIFINDEX.
  // Ignore return (verifier- & failure-friendly).
  (void)bpf_setsockopt(ctx, SOL_SOCKET, SO_BINDTOIFINDEX, &val, sizeof(val));
  return 1;
}

SEC("cgroup/connect4")
int force_dev_connect4(struct bpf_sock_addr *ctx)
{
  return force_bind(ctx);
}

SEC("cgroup/sendmsg4")
int force_dev_sendmsg4(struct bpf_sock_addr *ctx)
{
  return force_bind(ctx);
}

char _license[] SEC("license") = "GPL";
