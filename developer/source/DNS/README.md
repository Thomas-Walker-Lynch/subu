# Unbound per-tunnel setup (US + x6)

This bundle provides two Unbound instances that each egress via a specific
WireGuard tunnel, plus an nftables rule to steer DNS from specific UIDs to
the corresponding local stub resolver port.

## Topology
- US instance
  - listens: 127.0.0.1:5301
  - egress:  10.0.0.1 (WG US local address)
  - intended UID: 2017 (e.g., user `Thomas-US`)
- x6 instance
  - listens: 127.0.0.1:5302
  - egress:  10.8.0.2 (WG x6 local address)
  - intended UID: 2018 (e.g., user `Thomas-x6`)

Both instances bind ONLY on loopback (so they survive tunnel flaps) and set
`outgoing-interface` to the WG /32 address so queries exit via the tunnel.
IPv6 is disabled (consistent with your environment).

## Install
Copy files:

    sudo cp stage/etc/unbound/unbound-US.conf /etc/unbound/
    sudo cp stage/etc/unbound/unbound-x6.conf /etc/unbound/
    sudo cp stage/etc/systemd/system/unbound@.service /etc/systemd/system/
    sudo mkdir -p /etc/nftables.d
    sudo cp stage/etc/nftables.d/30-dnsredir.nft /etc/nftables.d/

Include the nft snippet and reload nftables:

    # add to /etc/nftables.conf (near end):
    #   include "/etc/nftables.d/30-dnsredir.nft"
    sudo nft -f /etc/nftables.conf

Systemd:

    sudo systemctl daemon-reload
    sudo systemctl enable --now unbound@US unbound@x6

> The unit naturally waits for the matching tunnel: `After=wg-quick@%i.service`.

## Optional (root hints + DNSSEC trust anchor)
Recommended once (before or after starting):

    sudo install -d -m 0755 /var/lib/unbound
    sudo wget -O /var/lib/unbound/root.hints https://www.internic.net/domain/named.root
    sudo unbound-anchor -a /var/lib/unbound/root.key

The configs enable DNSSEC via `auto-trust-anchor-file`.

## Test

    # US path
    sudo -u Thomas-US dig @127.0.0.1 -p 5301 example.com +short
    sudo -u Thomas-US curl -s ifconfig.co/country

    # x6 path
    sudo -u Thomas-x6 dig @127.0.0.1 -p 5302 example.com +short
    sudo -u Thomas-x6 curl -s ifconfig.co/country

    # Fail-closed example: stop a tunnel; queries from its UID should fail
    sudo systemctl stop wg-quick@US
    sudo -u Thomas-US dig example.com +short

## Notes
- If resolv.conf is changed by NetworkManager or others, nftables redirection
  still forces DNS to the right local stub (ports 5301/5302) per-UID.
- You can switch to forwarding instead of recursion by uncommenting the
  `forward-zone` block in each config.
