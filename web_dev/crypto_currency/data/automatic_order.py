import urllib2
import json
import datetime
import os
# Import smtplib for the actual sending function
import smtplib
# Import the email modules we'll need
from email.mime.text import MIMEText


MONEY_TYPE=''
sleep_time=4
want_ticker=[361]
want_tacker=[320]

response={}
current_list=[]

ACCESS_TOKEN = '0fd27ac4-a404-4bb1-b052-f05d6b533944'
SECRET_KEY = 'ac1873c7-fc35-4274-b94a-771e7a17505a'

def read_url(idx) :
  if idx == 1:
    f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/ticker/?currency='+MONEY_TYPE)
  elif idx == 2:
    f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/orderbook/?currency='+MONEY_TYPE)
  elif idx == 3:
    f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/orderbook/?currency='+MONEY_TYPE)
  elif idx == 4:
    f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/trades/?currency='+MONEY_TYPE)

  js = json.loads(f.read())
#  print(js)
  return js

def Get_Current_Ticker() :
  global response

  response = read_url(1)
#  print response
  timestamp = response['timestamp']
  st = datetime.datetime.fromtimestamp(
            int(timestamp)
        ).strftime('%Y-%m-%d %H:%M:%S')
  current_list.append("   1 .Current Time   : " + st)
  current_list.append("   2. Current volume : " + response['volume'])
  current_list.append("   3. Current Sell   : " + response['last'])




def Get_My_Order_Info(want_ticker, want_tacker) :
  global response
  response = read_url(2)
  current_list.append("======ticker list=======")
  Check_My_Order(0, want_ticker,response)
  Check_My_Order(1, want_tacker,response)

#  for i in [20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0] :
  for i in [5,4,3,2,1,0] :

    current_list.append("   4. ticker : " + response['ask'][i]['price'] + "--" + response['ask'][i]['qty'].rjust(12," "))
   
  current_list.append("======tacker list=======")

  for i in range(0,5) :
    current_list.append("   5. tacker : " + response['bid'][i]['price'] + "--" + response['bid'][i]['qty'].rjust(12," "))

  return 0

def Check_My_Order(cmd, my_order_list,response) :
  # sell price
  if int(cmd) == 0 :
    for want_value in my_order_list :
      sell_price = response['ask'][0]['price']
      current_list.append("want value : "+str(want_value))
      current_list.append("buy : "+str(sell_price))
      if int(want_value) == int(sell_price) :
        print("sendMail")
        Send_Mail(cmd,want_value)
        my_order_list[0] = 0
        exit(1)
  # buy price 
  if int(cmd) == 1 :
    for want_value in my_order_list :
      buy_price = response['bid'][0]['price']
      current_list.append("sell"+str(buy_price))
      if int(want_value) == int(buy_price) :
        Send_Mail(cmd,want_value)
        my_order_list[0] = 0




def Send_Mail(cmd,value) :
  HOST = 'localhost'   # smtp
  me = 'chun4foryou@coinone.com' #
  you = 'chun4foryou@gmail.com' #
  contents = 'Found My Limit Mony' + str(value)
  msg = MIMEText(contents, _charset='euc-kr')
  if int(cmd) == 0 :
    msg['Subject'] = 'Found Sell price'
  else :
    msg['Subject'] = 'Found Buy Price'
  msg['From'] = me
  msg['To'] = you
  s = smtplib.SMTP(HOST)
  s.sendmail(me, [you], msg.as_string())
  s.quit()

#Currency Complete Order
def comple_order():
  global response
  response = read_url(4)
  max_range= len(response['completeOrders'])
  current_list.append("\n\n Complete Corder List")
  for idx in reversed(range(max_range-5,max_range)):
    timestamp  = response['completeOrders'][idx]['timestamp']
    st = datetime.datetime.fromtimestamp(
      int(timestamp)
    ).strftime('%Y-%m-%d %H:%M:%S')
    price = response['completeOrders'][idx]['price']
    qty = response['completeOrders'][idx]['qty']
    current_list.append(str(st) + " " 
                        + str(price.rjust(5," ")) + "  " 
                        + str(qty.rjust(12," ")))

def main() :
  global MONEY_TYPE
  global sleep_time
  while 1 :
    os.system('sleep '+ str(sleep_time)) 
    del  current_list[:] 
    current_list.append('######### BTC  ########')
    MONEY_TYPE='qtum'
    Get_Current_Ticker()
    Get_My_Order_Info(want_ticker, want_tacker)
    comple_order()
    current_list.append( '\n\n')

    current_list.append( '######## XRP ########')
    MONEY_TYPE='xrp'
    Get_Current_Ticker()
    Get_My_Order_Info(want_ticker, want_tacker)
    comple_order()
    os.system('clear') 
    for value in current_list :
      print value
#    return 0

if __name__ == "__main__":
  main()

