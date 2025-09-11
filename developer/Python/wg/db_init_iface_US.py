# db_init_iface_US.py
from db_init_client_incommon import upsert_client

def init_iface_US(conn):
  # iface US with dedicated table 'US' and a distinct host /32
  return upsert_client(conn, iface="US", addr_cidr="10.8.0.3/32", rt_table_name="US")
