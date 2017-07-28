import httplib

# SSL information class 
class https_info:
  def get_url(self,ip,port,url):
    self.conn = httplib.HTTPSConnection(ip,port)
    self.conn.request("GET", url)
    data = self.conn.getresponse()
    status = data.status

    if status == 200:
      payload = data.read()
      self.conn.close()
      return payload
    else:
      return False


