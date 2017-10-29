# -*- coding: utf-8 -*-
import sys
sys.path.append("module/ips_module")
sys.path.append("module/http_module")
from ips_60 import *
import datetime, time

def main():
  ips6 =  ips_60('10.0.8.49','443','1')
  ip = ips6.get_ipaddr()
  print ip
  port = ips6.get_port()
  print port
  sn_num = ips6.get_sn_number()
  print sn_num

  date = time.localtime(time.time())
  day=time.strftime("%Y%m%d%H%M",date)
  print "time "+day


  #시간 동기화
  ips6.refresh_time()
  ips6.collect_ips_information()

  return 0;


if __name__ == "__main__":
      main()
