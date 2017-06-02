import urllib2
import json

config={}

def read_url() :
  f = response_ticker = urllib2.urlopen('https://api.coinone.co.kr/ticker/?currency=xrp')
  js = json.loads(f.read())
  print(js)
  return js

def main() :
  global config
  config = read_url()
  timestamp = config['timestamp']
  print "repos value : " + timestamp

if __name__ == "__main__":
  main()

