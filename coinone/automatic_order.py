import urllib2
import json
import datetime

response={}

def read_url(idx) :
  if idx == 1:
    f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/ticker/?currency=xrp')
  elif idx == 2:
    f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/orderbook/?currency=xrp')

  js = json.loads(f.read())
#  print(js)
  return js

def Get_Current_Ticker() :
  global response
  response = read_url(1)
  timestamp = response['timestamp']
  st = datetime.datetime.fromtimestamp(
            int(timestamp)
        ).strftime('%Y-%m-%d %H:%M:%S')
  print "  1 .Current Time : " + st
  print "  2. Current Sell : " + response['last']

def Get_My_Order_Info() :
  global response
  response = read_url(2)
  for ( i = 0 ; i < 
  print "  1, Sell List : " response[
  return 0

def main() :
  Get_Current_Ticker()
  Get_My_Order_Info()

if __name__ == "__main__":
  main()

