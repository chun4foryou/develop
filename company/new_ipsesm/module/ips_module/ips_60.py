# -*- coding: utf-8 -*-

import sys
sys.path.append("../http_module/")
from libhttp import *
from libio import *
from common_module import *
import urllib

# IPS 6.0 information class
class ips_60(common_query,https_info,file_io_controller):
  def __init__(self,ip_addr, port, sn_number):
    self.sn_number = sn_number
    self.ip_addr = ip_addr
    self.port = port

  def get_ipaddr(self):
    return self.ip_addr

  def get_port(self):
    return self.port

  def get_sn_number(self):
    return self.sn_number

  def refresh_time(self):
    date = time.localtime(time.time())
    self.cur_time=time.strftime("%Y%m%d%H%M",date)
    #1분후 시간
    date = time.localtime(time.time()+60)
    self.next_time=time.strftime("%Y%m%d%H%M",date)

  #Overrding 
#  def get_total_traffic(self):
#    print "ips 6.0"

  def collect_ips_information(self):
    self.get_protect_traffic()
    self.get_total_traffic()
    self.get_protocol_traffic()
    self.get_service_traffic()
    self.get_ip_top_n()
    self.get_sensor_time()

#    get_protocol_traffic()
#    get_topn_dest_tcp()
#    get_topn_dest_udp()
#    get_topn_srcip()
#    get_topn_dstip()
#    get_protect_dns_traffic()
#    get_aggprotectevent_redir()
#    get_acl_list()
#    get_ips_fod_mode()
#    get_ips_process_info()
#    get_ips_sms_info()
#    get_black_list()
#    get_acl_list_ipv6()
#    get_black_list_ipv6()
#    get_topn_url()

