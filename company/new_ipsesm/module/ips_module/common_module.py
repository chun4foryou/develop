# -*- coding: utf-8 -*-
import sys
sys.path.append("../http_module/")
from libhttp import *
from libio import *
import datetime, time

class common_query(https_info,file_io_controller):
  # 실시간 트래픽
  def get_protect_traffic(self):
    response = self.get_url(self.ip_addr,self.port,"/sniper.atx?ocx=trf?mode=10000")
    if response == False:
      self.save_file("./protect_traffic.txt","Error")
    else:
      self.save_file("./protect_traffic.txt",response)

  # 분단위 전체 트래픽
  def get_total_traffic(self):
    response = self.get_url(self.ip_addr,self.port,"/Sniper.atx?ocx=report?mode=c30000?param=sdate="+ self.cur_time +"&edate="+ self.next_time + "&bview=0&backup=0")
    if response == False:
      self.save_file("./total_traffic.txt","Error")
    else:
      self.save_file("./total_traffic.txt",response)

  # 프로토콜 별 분단위 트래픽 총량
  def get_protocol_traffic(self):
    response = self.get_url(self.ip_addr,self.port,"/Sniper.atx?ocx=report?mode=c31000?param=sdate="+ self.cur_time +"&edate="+ self.next_time + "&bview=0&backup=0")
    if response == False:
      self.save_file("./protocol_traffic.txt","Error")
    else:
      self.save_file("./protocol_traffic.txt",response)

  # 서비스 별 분단위 트래픽 총량 
  def get_service_traffic(self):
    response = self.get_url(self.ip_addr,self.port,"/Sniper.atx?ocx=report?mode=c32000?param=sdate="+ self.cur_time +"&edate="+ self.next_time +"&protocol=0&bview=0&backup=0&")
    if response == False:
      self.save_file("./service_traffic.txt","Error")
    else:
      self.save_file("./service_traffic.txt",response)

  #IP TopN
  def get_ip_top_n(self):
    cur_time = time.localtime(time.time())
    day=time.strftime("%Y%m%d",cur_time)
    day_time=time.strftime("%Y%m%d%H%M",cur_time)
    url= "/dbms/mpls/" + day + "/" + day_time + "_tot_ip_topn.html"
    response = self.get_url(self.ip_addr,self.port,url)
    if response == False:
      self.save_file("./ip_top_n","Error")
    else:
      self.save_file("./ip_top_n",response)

  # sensor 시간
  def get_sensor_time(self):
    response = self.get_url(self.ip_addr,self.port,"/sniper.atx?ocx=audit?mode=7000")
    if response == False:
      self.save_file("./sensor_time","Error")
    else:
      self.save_file("./sensor_time",response)






