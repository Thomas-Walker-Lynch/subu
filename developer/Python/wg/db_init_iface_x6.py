# db_init_iface_x6.py
from db_init_client_incommon import upsert_client

def init_iface_x6(conn):
  # iface x6 with dedicated table 'x6' and host /32
  return upsert_client(conn, iface="x6", addr_cidr="10.8.0.2/32", rt_table_name="x6")
