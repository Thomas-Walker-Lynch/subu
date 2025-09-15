sudo ./install_staged_tree.py
sudo systemctl daemon-reload
# enable nft snippet (once):
sudo sed -i '1{/table inet filter {/!{h;s/.*/include "\/etc\/nftables.d\/30-dnsredir.nft"/;H;x}}' /etc/nftables.conf || true
sudo nft -f /etc/nftables.conf

# bring up Unbound instances (they wait for wg links):
sudo systemctl enable --now unbound@US unbound@x6
