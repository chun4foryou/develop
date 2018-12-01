import smtplib
#from email.message import EmailMessage

# 1. 접속
# smtp = smtplib.SMTP('smtp.gmail.com', 587)ㄴ
smtp = smtplib.SMTP_SSL('smtp.gmail.com', 465)

# 2. SMTP에게 "hello" 메시지 보내기
smtp.ehlo()

# TLS 암호화 시작 <SMTP_SSL 이면 색략>
# smtp.starttls()

# 3. SMTP 서버에 로그인
smtp.login('chun4foryou@gmail.com', 'giddms42!@#')

# 4. 이메일 보내기
# EmailMessage 객체의 msg 를 사용
msg = EmailMessage()
msg['Subject'] = "안녕 python"
msg['From'] = 'jk722@wins21.co.kr'
msg['To'] = 'chun4foryou@gmail.com'
msg.set_content('하 모르겠다 어떤게 정답인지')
smtp.send_message(msg)

# 5. SMTP 접속 끊키
smtp.quit()
