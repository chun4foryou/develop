import smtplib
from email.MIMEText import MIMEText

def sendMail() :
  HOST = 'localhost'   # smtp
  me = 'chun4foryou@coinone.com' #
  you = 'chun4foryou@gmail.com' #
  contents = 'Found My Limit Mony'
  msg = MIMEText(contents, _charset='euc-kr')
  msg['Subject'] = 'Found My Limit Mony'
  msg['From'] = me
  msg['To'] = you
  s = smtplib.SMTP(HOST)
  s.sendmail(me, [you], msg.as_string())
  s.quit()

def main() :
  sendMail()

if __name__ == "__main__":
  main()

